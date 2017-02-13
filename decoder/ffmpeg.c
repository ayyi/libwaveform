/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2016 Tim Orford <tim@orford.org>                       |
* | copyright (C) 2011 Robin Gareus <robin@gareus.org>                   |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <gtk/gtk.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "decoder/debug.h"
#include "decoder/ad.h"

#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
#endif

#ifndef false
#define false FALSE
#define true TRUE
#endif

#ifndef bool
#define bool gboolean
#endif

struct _WfBuf16 // also defined in waveform.h
{
    short*     buf[WF_STEREO];
    guint      size;
    uint32_t   stamp;
#ifdef WF_DEBUG
    uint64_t   start_frame;
#endif
};

typedef struct _FFmpegAudioDecoder FFmpegAudioDecoder;

struct _FFmpegAudioDecoder
{
    AVFormatContext* format_context;
    AVCodecContext*  codec_context;
    AVCodec*         codec;
    AVPacket         packet;
    int              audio_stream;
    int              pkt_len;
    uint8_t*         pkt_ptr;

    AVFrame          frame;
    int              frame_iter;

    struct {
      int16_t        buf[AVCODEC_MAX_AUDIO_FRAME_SIZE];
      int16_t*       start;
      unsigned long  len;
    }                tmp_buf;

    int64_t          decoder_clock;
    int64_t          output_clock;
    int64_t          seek_frame;

    ssize_t (*read)  (WfDecoder*, float*, size_t len);
    ssize_t (*read_planar) (WfDecoder*, WfBuf16*);
};


static ssize_t ff_read_short_interleaved_to_planar          (WfDecoder*, WfBuf16*);
static ssize_t ff_read_float_interleaved_to_planar          (WfDecoder*, WfBuf16*);
static ssize_t ff_read_short_planar_to_planar               (WfDecoder*, WfBuf16*);
static ssize_t ff_read_float_planar_to_planar               (WfDecoder*, WfBuf16*);
static ssize_t ff_read_u8_interleaved_to_planar             (WfDecoder*, WfBuf16*);
static ssize_t ff_read_int32_interleaved_to_planar          (WfDecoder*, WfBuf16*);

static ssize_t ff_read_short_planar_to_interleaved          (WfDecoder*, float*, size_t);
static ssize_t ff_read_short_interleaved_to_interleaved     (WfDecoder*, float*, size_t);
static ssize_t ff_read_float_planar_to_interleaved          (WfDecoder*, float*, size_t);

static ssize_t ff_read_default_interleaved                  (WfDecoder*, float*, size_t);

static gboolean ad_metadata_array_set_tag_postion           (GPtrArray* tags, const char* tag_name, int pos);

#define U8_TO_SHORT(V) ((V - 128) * 128)
#define INT32_TO_SHORT(V) (V >> 16)
#define SHORT_TO_FLOAT(A) (((float)A) / 32768.0)


int
ad_info_ffmpeg(WfDecoder* d)
{
	FFmpegAudioDecoder* f = (FFmpegAudioDecoder*)d->d;
	g_return_val_if_fail(f, -1);

	if (!f->codec_context->sample_rate) return -1;

	int64_t len = f->format_context->duration - f->format_context->start_time;
	int64_t n_frames  = (int64_t)(len * f->codec_context->sample_rate / AV_TIME_BASE);

	d->info = (WfAudioInfo){
		.sample_rate = f->codec_context->sample_rate,
		.channels    = f->codec_context->channels,
		.frames      = n_frames,
		.length      = (n_frames * 1000) / f->codec_context->sample_rate,
		.bit_rate    = f->format_context->bit_rate,
		.bit_depth   = 0,
		.meta_data   = NULL
	};

	GPtrArray* tags = g_ptr_array_new_full(32, g_free);

	AVDictionaryEntry* tag = NULL;
	// Tags in container
	while ((tag = av_dict_get(f->format_context->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
		dbg(2, "FTAG: %s=%s", tag->key, tag->value);
		g_ptr_array_add(tags, g_utf8_strdown(tag->key, -1));
		g_ptr_array_add(tags, g_strdup(tag->value));
	}
	// Tags in stream
	tag = NULL;
	AVStream* stream = f->format_context->streams[f->audio_stream];
	while ((tag = av_dict_get(stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
		dbg(2, "STAG: %s=%s", tag->key, tag->value);
		g_ptr_array_add(tags, g_utf8_strdown(tag->key, -1));
		g_ptr_array_add(tags, g_strdup(tag->value));
	}

	while ((tag = av_dict_get(stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
		g_ptr_array_add(tags, g_utf8_strdown(tag->key, -1));
		g_ptr_array_add(tags, g_strdup(tag->value));
	}

	if(tags->len){
		// sort tags
		char* order[] = {"artist", "title", "album", "track", "date"};
		int p = 0;
		int i; for(i=0;i<G_N_ELEMENTS(order);i++) if(ad_metadata_array_set_tag_postion(tags, order[i], p)) p++;

		d->info.meta_data = tags;
	}else
		g_ptr_array_free(tags, true);

	return 0;
}


gboolean
ad_open_ffmpeg(WfDecoder* decoder, const char* filename)
{
	FFmpegAudioDecoder* f = decoder->d = g_new0(FFmpegAudioDecoder, 1);

	if (avformat_open_input(&f->format_context, filename, NULL, NULL) < 0) {
		dbg(1, "ffmpeg is unable to open file '%s'.", filename);
		goto f;
	}

	if (avformat_find_stream_info(f->format_context, NULL) < 0) {
		avformat_close_input(&f->format_context);
		dbg(1, "av_find_stream_info failed");
		goto f;
	}

	f->audio_stream = -1;
	int i;
	for (i=0; i<f->format_context->nb_streams; i++) {
		if (f->format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			f->audio_stream = i;
			break;
		}
	}
	if (f->audio_stream == -1) {
		dbg(1, "No Audio Stream found in file");
		avformat_close_input(&f->format_context);
		goto f;
	}

	f->codec_context = f->format_context->streams[f->audio_stream]->codec;
	f->codec         = avcodec_find_decoder(f->codec_context->codec_id);

	if (f->codec == NULL) {
		avformat_close_input(&f->format_context);
		dbg(1, "Codec not supported by ffmpeg");
		goto f;
	}
	if (avcodec_open2(f->codec_context, f->codec, NULL) < 0) {
		dbg(1, "avcodec_open failed" );
		goto f;
	}

	dbg(2, "ffmpeg - audio tics: %i/%i [sec]", f->format_context->streams[f->audio_stream]->time_base.num, f->format_context->streams[f->audio_stream]->time_base.den);

	f->format_context->flags |= AVFMT_FLAG_GENPTS;
	f->format_context->flags |= AVFMT_FLAG_IGNIDX;

#if 0
	f->samplerate = f->codec_context->sample_rate;
#endif
	//d->info.sample_rate = f->codec_context->sample_rate;
	//f->channels   = f->codec_context->channels;

	if (ad_info_ffmpeg(decoder)) {
		dbg(1, "invalid file info (samplerate = 0)");
		goto f;
	}

	WfAudioInfo* nfo = &decoder->info;
	dbg(2, "ffmpeg - %s", filename);
	dbg(2, "ffmpeg - sr:%i c:%i d:%"PRIi64" f:%"PRIi64" %s", nfo->sample_rate, nfo->channels, nfo->length, nfo->frames, av_get_sample_fmt_name(f->codec_context->sample_fmt));

#if 0 // TODO why prints nothing?
	av_dump_format(f->format_context, f->audio_stream, filename, 0);
#endif

	switch(f->codec_context->sample_fmt){
		case AV_SAMPLE_FMT_S16:
			dbg(1, "S16 (TODO check float out)");

			// there are 2 implementations of this - see which one is more performant
			//f->read = ff_read_default_interleaved;
			f->read = ff_read_short_interleaved_to_interleaved;

			f->read_planar = ff_read_short_interleaved_to_planar;
			break;
		case AV_SAMPLE_FMT_S16P:
			f->read = ff_read_short_planar_to_interleaved;
			f->read_planar = ff_read_short_planar_to_planar;
			break;
		case AV_SAMPLE_FMT_FLT:
			gwarn("no implementation of FLT to FLT");
			f->read_planar = ff_read_float_interleaved_to_planar;
			break;
		case AV_SAMPLE_FMT_FLTP:
			f->read = ff_read_float_planar_to_interleaved;
			f->read_planar = ff_read_float_planar_to_planar;
			break;
		case AV_SAMPLE_FMT_S32:
			gwarn("no implementation of S32 to FLT");
			f->read_planar = ff_read_int32_interleaved_to_planar;
			break;
		case AV_SAMPLE_FMT_S32P:
			gwarn("planar (non-interleaved) unhandled!");
			f->read = NULL;
			break;
		case AV_SAMPLE_FMT_U8:
			f->read_planar = ff_read_u8_interleaved_to_planar;
			break;
		default:
			gwarn("sample format not handled: %i", f->codec_context->sample_fmt);
			break;
	}

	return TRUE;

f:
	g_free(f);
	return FALSE;
}


int
ad_close_ffmpeg(WfDecoder* d)
{
	FFmpegAudioDecoder* priv = (FFmpegAudioDecoder*)d->d;
	if (!priv) return -1;

	avcodec_close(priv->codec_context);
	avformat_close_input(&priv->format_context);
	g_free(priv);

	return 0;
}


/*
 *  Input and output is both interleaved
 */
void
int16_to_float(float* out, int16_t* in, int n_channels, int n_frames, int out_offset)
{
	int f, c;
	for (f=0;f<n_frames;f++) {
		for (c=0;c<n_channels;c++) {
			out[(f+out_offset)*n_channels+c] = (float) in[f*n_channels+c] / 32768.0;
		}
	}
}

#if 0
static void
interleave(float* out, size_t len, Buf16* buf, int n_channels)
{
	int i,c; for(i=0; i<len/n_channels;i++){
		for(c=0;c<n_channels;c++){
			out[i * n_channels + c] = SHORT_TO_FLOAT(buf->buf[c][i]);
		}
	}
}
#endif


/*
 *  Implements ad_plugin.read
 *  Output is interleaved
 *  len is the size of the out array (n_frames * n_channels).
 */
ssize_t
ad_read_ffmpeg(WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = (FFmpegAudioDecoder*)d->d;
	if (!f) return -1;

	g_return_val_if_fail(f->read, -1);
	return f->read(d, out, len);
}


static ssize_t
ff_read_short(WfDecoder* d, WfBuf16* out)
{
	FFmpegAudioDecoder* ff = (FFmpegAudioDecoder*)d->d;
	return ff->read_planar(d, out);
}


/*
 *  S16 (interleaved) to float (interleaved)
 */
ssize_t
ff_read_default_interleaved(WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = d->d;
	if (!f) return -1;

	size_t frames = len / d->info.channels;

	int written = 0;
	int ret = 0;
	while (ret >= 0 && written < frames) {
		dbg(3, "loop: %i/%i (bl:%lu)", written, frames, f->tmp_buf.len);
		if (f->seek_frame == 0 && f->tmp_buf.len > 0) {
			int s = MIN(f->tmp_buf.len / d->info.channels, frames - written);
			int16_to_float(out, f->tmp_buf.start, d->info.channels, s, written);
			written += s;
			f->output_clock += s;
			s = s * d->info.channels;
			f->tmp_buf.start += s;
			f->tmp_buf.len -= s;
			ret = 0;
		} else {
			f->tmp_buf.start = f->tmp_buf.buf;
			f->tmp_buf.len = 0;

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

			int data_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
			AVFrame avf; // TODO statically allocate
			memset(&avf, 0, sizeof(AVFrame)); // not sure if that is needed
			int got_frame = 0;
			ret = avcodec_decode_audio4(f->codec_context, &avf, &got_frame, &f->packet);
			data_size = avf.linesize[0];
			memcpy(f->tmp_buf.buf, avf.data[0], avf.linesize[0] * sizeof(uint8_t));

			if (ret < 0 || ret > f->pkt_len) {
//#if 0
				dbg(0, "audio decode error");
				return -1;
//#endif
		        f->pkt_len = 0;
				ret = 0;
				continue;
			}

				f->pkt_len -= ret; f->pkt_ptr += ret;

				/* sample exact alignment  */
				if (f->packet.pts != AV_NOPTS_VALUE) {
					f->decoder_clock = f->codec_context->sample_rate * av_q2d(f->format_context->streams[f->audio_stream]->time_base) * f->packet.pts;
				} else {
					dbg(0, "!!! NO PTS timestamp in file");
					f->decoder_clock += (data_size >> 1) / d->info.channels;
				}

				if (data_size > 0) {
					f->tmp_buf.len += (data_size >> 1); // 2 bytes per sample
				}

				/* align buffer after seek. */
				if (f->seek_frame > 0) {
					const int diff = f->output_clock - f->decoder_clock;
				if (diff < 0) {
					/* seek ended up past the wanted sample */
					gwarn("audio seek failed.");
					return -1;
				} else if (f->tmp_buf.len < (diff * d->info.channels)) {
					/* wanted sample not in current buffer - keep going */
					dbg(2, " !!! seeked sample was not in decoded buffer. frames-to-go: %i", diff);
					f->tmp_buf.len = 0;
				} else if (diff!=0 && data_size > 0) {
					/* wanted sample is in current buffer but not at the beginnning */
					dbg(2, " !!! sync buffer to seek. (diff:%i)", diff);
					f->tmp_buf.start += diff * f->codec_context->channels;
					f->tmp_buf.len   -= diff * f->codec_context->channels;
#if 1
					memmove(f->tmp_buf.buf, f->tmp_buf.start, f->tmp_buf.len);
					f->tmp_buf.start = f->tmp_buf.buf;
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
			//dbg(0, "PTS: decoder:%"PRIi64". - want: %"PRIi64, f->decoder_clock, f->output_clock);
			//dbg(0, "CLK: frame:  %"PRIi64"  T:%.3fs",f->decoder_clock, (float) f->decoder_clock/f->samplerate);
		}
	}

	if (written != frames) dbg(2, "short-read");

	return written * d->info.channels;
}


static ssize_t
ff_read_short_interleaved_to_planar(WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;

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

				int64_t fr = f->frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				int i; for(i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < buf->size); i++){
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							memcpy(buf->buf[ch] + n_fr_done, f->frame.data[0] + data_size * (f->codec_context->channels * i + ch), data_size);
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= buf->size) goto stop;
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
ff_read_short_planar_to_planar(WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;
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

				int64_t fr = f->frame.best_effort_timestamp * f->codec_context->sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
				int i; for(i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < buf->size); i++){
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							buf->buf[ch][n_fr_done] = *(((int16_t*)f->frame.data[ch]) + i);
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= buf->size) goto stop;
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
ff_read_short_planar_to_interleaved(WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = d->d;
	int n_frames = len / d->info.channels;
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

				int64_t fr = f->frame.best_effort_timestamp * f->codec_context->sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
				int i; for(i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < n_frames); i++){
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							out[f->codec_context->channels * n_fr_done + ch] = SHORT_TO_FLOAT(*(((int16_t*)f->frame.data[ch]) + i));
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= n_frames) goto stop;
			}
		}
		av_free_packet(&f->packet);
		continue;

		stop:
			av_free_packet(&f->packet);
			break;
	}

	return n_fr_done * d->info.channels;
}


static ssize_t
ff_read_float_interleaved_to_planar(WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;
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

				int64_t fr = frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				int i; for(i=0; i<frame.nb_samples && (n_fr_done < buf->size); i++){
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

				if(n_fr_done >= buf->size) goto stop;
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


/*
 * TODO there are 2 implementations of S16 to FLT - see which one is more performant
 */
static ssize_t
ff_read_short_interleaved_to_interleaved(WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = d->d;
	int n_frames = len / d->info.channels;
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

				int64_t fr = f->frame.best_effort_timestamp * f->codec_context->sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
				int i; for(i=f->frame_iter; i<f->frame.nb_samples; i++){
					if(n_fr_done >= n_frames) break;
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							int16_t* src = (int16_t*)(f->frame.data[0] + f->codec_context->channels * data_size * i + data_size * ch);
							out[n_fr_done * d->info.channels + ch] = SHORT_TO_FLOAT(*src);
							//out[f->codec_context->channels * n_fr_done + ch] = SHORT_TO_FLOAT(*(((int16_t*)f->frame.data[ch]) + i));
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= n_frames) goto stop;
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
ff_read_float_planar_to_planar(WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;
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

				int64_t fr = f->frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
#ifdef DEBUG
				for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
					g_return_val_if_fail(f->frame.data[ch], 0);
				}
#endif
				int i; for(i=f->frame_iter; i<f->frame.nb_samples; i++){
					if(n_fr_done >= buf->size) break;
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

				if(n_fr_done >= buf->size) goto stop;
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
ff_read_float_planar_to_interleaved(WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = d->d;

	int n_frames = len / d->info.channels;
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
					dbg(0, "Error decoding audio");
				}
			}
			if(got_frame){
				int size = av_samples_get_buffer_size (NULL, f->codec_context->channels, f->frame.nb_samples, f->codec_context->sample_fmt, 1);
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * f->codec_context->sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
				int i; for(i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < n_frames); i++){
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							out[f->codec_context->channels * n_fr_done + ch] = *(((float*)f->frame.data[ch]) + i);
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= n_frames) goto stop;
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
ff_read_int32_interleaved_to_planar(WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;

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

				int64_t fr = f->frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				int i; for(i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < buf->size); i++){
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							buf->buf[ch][n_fr_done] = INT32_TO_SHORT(*((int32_t*)(f->frame.data[0] + (f->codec_context->channels * i + ch) * data_size)));
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= buf->size) goto stop;
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
ff_read_u8_interleaved_to_planar(WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;

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

				int64_t fr = f->frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				int i; for(i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < buf->size); i++){
					if(fr >= f->seek_frame){
						for(ch=0; ch<MIN(2, f->codec_context->channels); ch++){
							buf->buf[ch][n_fr_done] = U8_TO_SHORT(*(f->frame.data[0] + f->codec_context->channels * i + ch));
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= buf->size) goto stop;
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


static int64_t
ad_seek_ffmpeg(WfDecoder* d, int64_t pos)
{
	g_return_val_if_fail(d, -1);
	FFmpegAudioDecoder* f = (FFmpegAudioDecoder*)d->d;
	g_return_val_if_fail(f, -1);

	if (pos == f->output_clock) return pos;

	/* flush internal buffer */
	f->tmp_buf.len = 0;
	f->seek_frame = pos;
	f->output_clock = pos;
	f->pkt_len = 0;
	f->pkt_ptr = NULL;
	f->decoder_clock = 0;

	memset(&f->frame, 0, sizeof(AVFrame));
	av_frame_unref(&f->frame);
	f->frame_iter = 0;

	// Seek at least 1 packet before target in case the seek position is in the middle of a frame.
	pos = MAX(0, pos - 8192);

	const int64_t timestamp = pos / av_q2d(f->format_context->streams[f->audio_stream]->time_base) / f->codec_context->sample_rate;
	dbg(2, "seek frame:%"PRIi64" - idx:%"PRIi64, pos, timestamp);

	av_seek_frame(f->format_context, f->audio_stream, timestamp, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(f->codec_context);

	return pos;
}


static int
ad_eval_ffmpeg(const char* f)
{
	char* ext = strrchr(f, '.');
	if (!ext) return 10;
	// libavformat.. guess_format.. 
	return 40;
}


static gboolean
ad_metadata_array_set_tag_postion(GPtrArray* tags, const char* tag_name, int pos)
{
	// move a metadata item up (can only be up) to the specified position.
	// returns true if the tag was found.

	if((tags->len < 4) || (pos >= tags->len - 2)) return false;
	pos *= 2;

	char** data = (char**)tags->pdata;
	int i; for(i=pos;i<tags->len;i+=2){
		if(!strcmp(data[i], tag_name)){
			const char* artist[] = {data[i], (const char*)data[i+1]};

			int j; for(j=i;j>=pos;j-=2){
				data[j    ] = data[j - 2    ];
				data[j + 1] = data[j - 2 + 1];
			}
			data[pos    ] = (char*)artist[0];
			data[pos + 1] = (char*)artist[1];

			return true;
		}
	}
	return false;
}


const static AdPlugin ad_ffmpeg = {
	&ad_eval_ffmpeg,
	&ad_open_ffmpeg,
	&ad_close_ffmpeg,
	&ad_info_ffmpeg,
	&ad_seek_ffmpeg,
	&ad_read_ffmpeg,
	&ff_read_short
};


extern int wf_debug; // TODO

/* dlopen handler */
const AdPlugin*
get_ffmpeg()
{
	static int ffinit = 0;
	if (!ffinit) {
		ffinit = 1;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 5, 0)
		avcodec_init();
#endif
		av_register_all();
		avcodec_register_all();
		av_log_set_level(wf_debug > 1 ? AV_LOG_VERBOSE : AV_LOG_QUIET);
	}
	return &ad_ffmpeg;
}

