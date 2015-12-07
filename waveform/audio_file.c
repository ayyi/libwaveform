
/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>
  copyright (C) 2011 Robin Gareus <robin@gareus.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include "waveform/waveform.h"
#include "waveform/utils.h"
#include "waveform/audio_file.h"

static bool    wf_ff_info                   (FF*);
static ssize_t wf_ff_read_float_p           (FF*, WfBuf16*, size_t len);
static ssize_t wf_ff_read_short_interleaved (FF*, WfBuf16*, size_t len);


static void
wf_ff_init()
{
	static bool ffinit = 0;
	if(!ffinit++){
		av_register_all();
		avcodec_register_all();
		av_log_set_level(wf_debug > 1 ? AV_LOG_VERBOSE : AV_LOG_QUIET);
	}
}


bool
wf_ff_open(FF* f, const char* filename)
{
	wf_ff_init();

	if(avformat_open_input(&f->format_context, filename, NULL, NULL)){
		dbg(1, "ff open failed: %s", filename);
		return false;
	}

	if (avformat_find_stream_info(f->format_context, NULL) < 0){
		dbg(1, "av_find_stream_info failed");
		goto out;
	}

	f->audio_stream = -1;
	int i; for (i=0; i<f->format_context->nb_streams; i++) {
		if (f->format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			f->audio_stream = i;
			break;
		}
	}
	if (f->audio_stream == -1) {
		dbg(1, "No Audio Stream found in file");
		goto out;
	}

	f->codec_context = f->format_context->streams[f->audio_stream]->codec;

	if (!(f->codec = avcodec_find_decoder(f->codec_context->codec_id))) {
		dbg(1, "Codec not supported by ffmpeg");
		goto out;
	}

	if (avcodec_open2(f->codec_context, f->codec, NULL) < 0) {
		dbg(1, "avcodec_open failed" );
		goto out;
	}

	dbg(1, "ffmpeg - audio tics: %i/%i [sec]", f->format_context->streams[f->audio_stream]->time_base.num, f->format_context->streams[f->audio_stream]->time_base.den);

	f->format_context->flags |= AVFMT_FLAG_GENPTS;
	f->format_context->flags |= AVFMT_FLAG_IGNIDX;

	if (!wf_ff_info(f)) {
		dbg(1, "invalid file info");
		goto out;
	}

	switch(f->codec_context->sample_fmt){
		case AV_SAMPLE_FMT_S16:
			dbg(1, "S16");
			f->read = wf_ff_read_short_interleaved;
			break;
		case AV_SAMPLE_FMT_S16P:
			dbg(1, "S16P");
			f->read = wf_ff_read_short;
			break;
		case AV_SAMPLE_FMT_FLTP:
			dbg(1, "FLTP");
			f->read = wf_ff_read_float_p;
			break;
		default:
			gwarn("format may not be supported: sample_fmt=%i %s", f->codec_context->sample_fmt, av_get_sample_fmt_name(f->codec_context->sample_fmt));
			f->read = wf_ff_read_short;
			break;
	}

	dbg(1, "ffmpeg - sr:%i ch:%i dur:%"PRIi64" fr:%"PRIi64, f->info.sample_rate, f->info.channels, f->info.length, f->info.frames);

	av_init_packet(&f->packet);

	return true;

  out:
	avformat_close_input(&f->format_context);
	return false;
}


void
wf_ff_close(FF* f)
{
	avformat_close_input(&f->format_context);
}


static bool
wf_ff_info(FF* f)
{
	if (!f) return false;

	int64_t len = f->format_context->duration - f->format_context->start_time;
	unsigned int sample_rate = f->codec_context->sample_rate;
	if (!sample_rate) return false;
	int64_t frames = (int64_t)(len * sample_rate / AV_TIME_BASE);

	f->info = (AudioInfo){
		.sample_rate = f->codec_context->sample_rate,
		.channels    = f->codec_context->channels,
		.frames      = frames,
		.length      = (frames * 1000) / sample_rate,
		.bit_rate    = f->format_context->bit_rate,
		.bit_depth   = 0
	};
	return true;
}


static void
int16_to_float(int16_t* in, float* out, int num_channels, int num_samples, int out_offset)
{
	int i, ii;
	for (i=0;i<num_samples;i++) {
		for (ii=0;ii<num_channels;ii++) {
			out[(i + out_offset) * num_channels + ii] = (float) in[i * num_channels + ii] / 32768.0;
		}
	}
}


int64_t
wf_ff_seek(FF* f, int64_t pos)
{
	if(!f) return -1;
	if(pos == f->output_clock) return pos;

	// flush internal buffer
	f->m_tmpBufferLen = 0;
	f->seek_frame = pos;
	f->output_clock = pos;
	f->pkt_len = 0;
	f->pkt_ptr = NULL;
	f->decoder_clock = 0;

	// Seek at least 1 packet before target in case the seek position is in the middle of a frame.
	pos = MAX(0, pos - 8192);

	const int64_t timestamp = pos / av_q2d(f->format_context->streams[f->audio_stream]->time_base) / f->info.sample_rate;
	dbg(2, "seek frame:%"PRIi64" - idx:%"PRIi64, pos, timestamp);

	if(av_seek_frame(f->format_context, f->audio_stream, timestamp, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD) < 0) return -1;
	avcodec_flush_buffers(f->codec_context);

	return pos;
}


static ssize_t
wf_ff_read_short_interleaved(FF* f, WfBuf16* buf, size_t len)
{
	AVFrame frame;
	int64_t n_samples = 0;

	int data_size = av_get_bytes_per_sample(f->codec_context->sample_fmt);
	g_return_if_fail(data_size == 2);

	while(!av_read_frame(f->format_context, &f->packet)){
		if(f->packet.stream_index == f->audio_stream){
			memset(&frame, 0, sizeof(AVFrame));
			av_frame_unref(&frame);

			int got_frame = 0;
			if(avcodec_decode_audio4(f->codec_context, &frame, &got_frame, &f->packet) < 0){
				dbg(0, "Error decoding audio");
			}
			if(got_frame){
				int size = av_samples_get_buffer_size (NULL, f->codec_context->channels, frame.nb_samples, f->codec_context->sample_fmt, 1);
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = frame.best_effort_timestamp * f->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				int i; for(i=0; i<frame.nb_samples; i++){
					if(n_samples >= len) break;
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							memcpy(buf->buf[ch] + n_samples, frame.data[0] + f->codec_context->channels * data_size * i + data_size * ch, data_size);
						}
						n_samples++;
					}
				}
				f->output_clock = fr + n_samples;

				if(n_samples >= len) goto stop;
			}
		}
		av_free_packet(&f->packet);
		continue;

		stop:
			av_free_packet(&f->packet);
			break;
	}

	return n_samples;
}


static ssize_t
wf_ff_read_float_p(FF* f, WfBuf16* buf, size_t len)
{
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_context->sample_fmt);
	g_return_if_fail(data_size == 4);

	bool have_frame = false;
	if(f->frame.nb_samples && f->frame_iter < f->frame.nb_samples){
		have_frame = true;
	}
	while(have_frame || !av_read_frame(f->format_context, &f->packet)){
		have_frame = false;
		if(f->packet.stream_index == f->audio_stream){
			int got_frame = false;
			if(f->frame_iter && f->frame_iter < f->frame.nb_samples){
				got_frame = true;
			}else{
				memset(&f->frame, 0, sizeof(AVFrame));
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				if(avcodec_decode_audio4(f->codec_context, &f->frame, &got_frame, &f->packet) < 0){
					gwarn("error decoding audio");
				}
			}
			if(got_frame){
				int size = av_samples_get_buffer_size (NULL, f->codec_context->channels, f->frame.nb_samples, f->codec_context->sample_fmt, 1);
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * f->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
#ifdef DEBUG
				for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
					g_return_if_fail(f->frame.data[ch]);
				}
#endif
				int i; for(i=f->frame_iter; i<f->frame.nb_samples; i++){
					if(n_fr_done >= len) break;
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							float* src = (float*)(f->frame.data[ch] + data_size * i);
							buf->buf[ch][n_fr_done] = (*src) * (1 << 15);
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= len) goto stop;
			}
		}
		av_free_packet(&f->packet);
		continue;

		stop:
			av_free_packet(&f->packet);
			break;
	}

	return n_fr_done;
}


ssize_t
wf_ff_read_short(FF* f, WfBuf16* buf, size_t len)
{
	AVFrame frame;
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_context->sample_fmt);
	g_return_if_fail(data_size == 2);

	while(!av_read_frame(f->format_context, &f->packet)){
		if(f->packet.stream_index == f->audio_stream){
			memset(&frame, 0, sizeof(AVFrame));
			av_frame_unref(&frame);

			int got_frame = 0;
			if(avcodec_decode_audio4(f->codec_context, &frame, &got_frame, &f->packet) < 0){
				dbg(0, "Error decoding audio");
			}
			if(got_frame){
				int size = av_samples_get_buffer_size (NULL, f->codec_context->channels, frame.nb_samples, f->codec_context->sample_fmt, 1);
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = frame.best_effort_timestamp * f->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				int i; for(i=0; i<frame.nb_samples; i++){
					if(n_fr_done + i >= len) break;
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							memcpy(buf->buf[ch] + n_fr_done, frame.data[ch] + data_size * i, data_size);
						}
						n_fr_done++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= len) goto stop;
			}
		}
		av_free_packet(&f->packet);
		continue;

		stop:
			av_free_packet(&f->packet);
			break;
	}

	return n_fr_done;
}


ssize_t
wf_ff_read(FF* f, float* d, size_t len)
{
	if (!f) return -1;
	size_t frames = len / f->info.channels;

	int written = 0;
	int ret = 0;
	while (ret >= 0 && written < frames) {
//		dbg(3, "loop: %i/%i (bl:%lu)", written, frames, f->m_tmpBufferLen );
		if (!f->seek_frame && f->m_tmpBufferLen > 0) {
			int s = MIN(f->m_tmpBufferLen / f->info.channels, frames - written);
			int16_to_float(f->m_tmpBufferStart, d, f->info.channels, s, written);
			written += s;
			f->output_clock += s;
			s = s * f->info.channels;
			f->m_tmpBufferStart += s;
			f->m_tmpBufferLen -= s;
			ret = 0;
		} else {
			f->m_tmpBufferStart = f->m_tmpBuffer;
			f->m_tmpBufferLen = 0;

			if (!f->pkt_ptr || f->pkt_len < 1) {
				if (f->packet.data) av_free_packet(&f->packet);
				ret = av_read_frame(f->format_context, &f->packet);
				if (ret < 0) { dbg(2, "reached end of file."); break; }
				f->pkt_len = f->packet.size;
				f->pkt_ptr = f->packet.data;
			}

			if (f->packet.stream_index != f->audio_stream) {
				f->pkt_ptr = NULL;
				continue;
			}

			/* decode all chunks in packet */
			int data_size = WF_MAX_AUDIO_FRAME_SIZE;
#if 0 // TODO  ffcompat.h -- this works but is not optimal (channels may not be planar/interleaved)
			AVFrame avf; // TODO statically allocate
			memset(&avf, 0, sizeof(AVFrame)); // not sure if that is needed
			int got_frame = 0;
			ret = avcodec_decode_audio4(priv->codecContext, &avf, &got_frame, &priv->packet);
			data_size = avf.linesize[0];
			memcpy(priv->m_tmpBuffer, avf.data[0], avf.linesize[0] * sizeof(uint8_t));
#else // this was deprecated in LIBAVCODEC_VERSION_MAJOR 53
			ret = avcodec_decode_audio3(f->codec_context, f->m_tmpBuffer, &data_size, &f->packet);
#endif

			if (ret < 0 || ret > f->pkt_len) {
#if 0
				dbg(0, "audio decode error");
				return -1;
#endif
				f->pkt_len = 0;
				ret = 0;
				continue;
			}

			f->pkt_len -= ret;
			f->pkt_ptr += ret;

			/* sample exact alignment  */
			if (f->packet.pts != AV_NOPTS_VALUE) {
				f->decoder_clock = f->info.sample_rate * av_q2d(f->format_context->streams[f->audio_stream]->time_base) * f->packet.pts;
			} else {
				dbg(0, "!!! NO PTS timestamp in file");
				f->decoder_clock += (data_size >> 1) / f->info.channels;
			}

			if (data_size > 0) {
				f->m_tmpBufferLen += (data_size >> 1); // 2 bytes per sample
			}

			/* align buffer after seek. */
			if (f->seek_frame > 0) { 
				const int diff = f->output_clock - f->decoder_clock;
				if (diff < 0) {
					/* seek ended up past the wanted sample */
					dbg(0, " !!! Audio seek failed.");
					return -1;
				} else if (f->m_tmpBufferLen < (diff * f->info.channels)) {
					/* wanted sample not in current buffer - keep going */
					dbg(2, " !!! seeked sample was not in decoded buffer. frames-to-go: %li", diff);
					f->m_tmpBufferLen = 0;
				} else if (diff != 0 && data_size > 0) {
					/* wanted sample is in current buffer but not at the beginnning */
					dbg(2, " !!! sync buffer to seek. (diff:%i)", diff);
					f->m_tmpBufferStart+= diff * f->codec_context->channels;
					f->m_tmpBufferLen  -= diff * f->codec_context->channels;
#if 1
					memmove(f->m_tmpBuffer, f->m_tmpBufferStart, f->m_tmpBufferLen);
					f->m_tmpBufferStart = f->m_tmpBuffer;
#endif
					f->seek_frame = 0;
					f->decoder_clock += diff;
				} else if (data_size > 0) {
					dbg(2, "Audio exact sync-seek (%"PRIi64" == %"PRIi64")", f->decoder_clock, f->seek_frame);
					f->seek_frame = 0;
				} else {
					dbg(0, "Error: no audio data in packet");
				}
			}
			//dbg(0, "PTS: decoder:%"PRIi64". - want: %"PRIi64, priv->decoder_clock, f->output_clock);
			//dbg(0, "CLK: frame:  %"PRIi64"  T:%.3fs", priv->decoder_clock, (float)f->decoder_clock / f->samplerate);
		}
	}
	if (written != frames) {
		dbg(2, "short-read");
	}
	return written * f->info.channels;
}


