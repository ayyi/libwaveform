
/*
  copyright (C) 2012-2016 Tim Orford <tim@orford.org>
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
static ssize_t wf_ff_read_float_interleaved (FF*, WfBuf16*, size_t len);
static ssize_t wf_ff_read_u8_interleaved    (FF*, WfBuf16*, size_t len);

#define U8_TO_SHORT(V) ((V - 128) * 128)


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
			f->read = wf_ff_read_short_p;
			break;
		case AV_SAMPLE_FMT_FLT:
			dbg(1, "FLT");
			f->read = wf_ff_read_float_interleaved;
			break;
		case AV_SAMPLE_FMT_FLTP:
			dbg(1, "FLTP");
			f->read = wf_ff_read_float_p;
			break;
		case AV_SAMPLE_FMT_U8:
			dbg(1, "U8");
			f->read = wf_ff_read_u8_interleaved;
			break;
		default:
			gwarn("format may not be supported: %i %s", f->codec_context->sample_fmt, av_get_sample_fmt_name(f->codec_context->sample_fmt));
			f->read = wf_ff_read_short_p;
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


#if 0
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
#endif


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
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_context->sample_fmt);
	g_return_val_if_fail(data_size == 2, 0);

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
					if(n_fr_done >= len) break;
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							memcpy(buf->buf[ch] + n_fr_done, frame.data[0] + f->codec_context->channels * data_size * i + data_size * ch, data_size);
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


static ssize_t
wf_ff_read_u8_interleaved(FF* f, WfBuf16* buf, size_t len)
{
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_context->sample_fmt);
	g_return_val_if_fail(data_size == 1, 0);

	bool have_frame = false;
	if(f->frame.nb_samples && f->frame_iter < f->frame.nb_samples){
		have_frame = true;
	}

	while(have_frame || !av_read_frame(f->format_context, &f->packet)){
		have_frame = false;
		if(f->packet.stream_index == f->audio_stream){
			int got_frame = 0;
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

				int64_t fr = f->frame.best_effort_timestamp * f->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				int i; for(i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < len); i++){
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							buf->buf[ch][n_fr_done] = U8_TO_SHORT(*(f->frame.data[0] + f->codec_context->channels * i + ch));
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


static ssize_t
wf_ff_read_float_interleaved(FF* f, WfBuf16* buf, size_t len)
{
	AVFrame frame;
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_context->sample_fmt);
	g_return_val_if_fail(data_size == 4, 0);

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
					if(n_fr_done >= len) break;
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							float* src = (float*)(frame.data[0] + f->codec_context->channels * data_size * i + data_size * ch);
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


static ssize_t
wf_ff_read_float_p(FF* f, WfBuf16* buf, size_t len)
{
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_context->sample_fmt);
	g_return_val_if_fail(data_size == 4, 0);

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
					g_return_val_if_fail(f->frame.data[ch], 0);
				}
#endif
				int i; for(i=f->frame_iter; i<f->frame.nb_samples; i++){
					if(n_fr_done >= len) break;
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							float* src = (float*)(f->frame.data[ch] + data_size * i);
							// aac values can exceed 1.0 so clamping is needed
							buf->buf[ch][n_fr_done] = CLAMP((*src), -1.0f, 1.0f) * 32767.0f;
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
wf_ff_read_short_p(FF* f, WfBuf16* buf, size_t len)
{
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_context->sample_fmt);
	g_return_val_if_fail(data_size == 2, 0);

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
					dbg(0, "Error decoding audio");
				}
			}
			if(got_frame){
				int size = av_samples_get_buffer_size (NULL, f->codec_context->channels, f->frame.nb_samples, f->codec_context->sample_fmt, 1);
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * f->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
				int i; for(i=f->frame_iter; (i<f->frame.nb_samples) && (n_fr_done < len); i++){
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							buf->buf[ch][n_fr_done] = *(((int16_t*)f->frame.data[ch]) + i);
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


