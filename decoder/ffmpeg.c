/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2011-2025 Tim Orford <tim@orford.org>                  |
 | copyright (C) 2011 Robin Gareus <robin@gareus.org>                   |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <glib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "decoder/debug.h"
#include "decoder/ad.h"

// The thumbnail extraction using ffmpeg filters was replaced
// due to hard to resolve memory leaks
#undef USE_FFMPEG_FILTERS

#ifdef USE_FFMPEG_FILTERS
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#endif

#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
#endif

#ifndef false
#define false FALSE
#define true TRUE
#endif

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(60, 0, 0)
#define N_CHANNELS(F) (f->codec_context->ch_layout.nb_channels)
#else
#define N_CHANNELS(F) (f->codec_context->channels)
#endif

#define FLOAT_TO_S32(A) (A * 2147483647.) // INT_MAX

extern void int16_to_float (float* out, int16_t* in, int n_channels, int n_frames, int out_offset);

struct _WfBuf16 // also defined in waveform.h
{
    short*     buf[WF_STEREO];
    guint      size;
    uint32_t   stamp;
#ifdef DEBUG
    uint64_t   start_frame;
#endif
};

typedef struct _FFmpegAudioDecoder FFmpegAudioDecoder;

struct _FFmpegAudioDecoder
{
    AVFormatContext*   format_context;
    AVCodecContext*    codec_context;
    AVCodecParameters* codec_parameters;
    const AVCodec*     codec;
    AVPacket           packet;
    int                audio_stream;
    int                pkt_len;
    uint8_t*           pkt_ptr;

    AVFrame            frame;
    int                frame_iter;

    struct {
      int16_t          buf[AVCODEC_MAX_AUDIO_FRAME_SIZE];
      int16_t*         start;
      unsigned long    len;
    }                  tmp_buf;

    int64_t            decoder_clock;
    int64_t            output_clock;
    int64_t            seek_frame;

    struct {
      int              stream;
      enum AVCodecID   codec_id;
      AVCodecContext*  codec_context;
#ifdef USE_FFMPEG_FILTERS
      AVFilterGraph*   graph;
      AVFilterContext* filter_source;
      AVFilterContext* filter_sink;
      AVFrame*         frame;
      AVPacket*        packet;
#endif
    }                  thumbnail;

    ssize_t (*read)        (WfDecoder*, float*, size_t len);
    ssize_t (*read_planar) (WfDecoder*, WfBuf16*);
    ssize_t (*read_s32)    (WfDecoder*, int32_t*, size_t len);
};


static ssize_t ff_read_short_interleaved_to_planar          (WfDecoder*, WfBuf16*);
static ssize_t ff_read_float_interleaved_to_planar          (WfDecoder*, WfBuf16*);
static ssize_t ff_read_short_planar_to_planar               (WfDecoder*, WfBuf16*);
static ssize_t ff_read_float_planar_to_planar               (WfDecoder*, WfBuf16*);
static ssize_t ff_read_u8_interleaved_to_planar             (WfDecoder*, WfBuf16*);
static ssize_t ff_read_int32_interleaved_to_planar          (WfDecoder*, WfBuf16*);

static ssize_t ff_read_short_planar_to_interleaved          (WfDecoder*, float*, size_t);
static ssize_t ff_read_float_interleaved_to_interleaved     (WfDecoder*, float*, size_t);
static ssize_t ff_read_short_interleaved_to_interleaved     (WfDecoder*, float*, size_t);
static ssize_t ff_read_float_planar_to_interleaved          (WfDecoder*, float*, size_t);

#if 0
static ssize_t ff_read_default_interleaved                  (WfDecoder*, float*, size_t);
#endif

static ssize_t ff_read_float_planar_to_s32                  (WfDecoder*, int32_t*, size_t);

static bool    ad_metadata_array_set_tag_postion            (GPtrArray* tags, const char* tag_name, int pos);

#ifdef USE_FFMPEG_FILTERS
static bool    ff_decode_video_frame                        (FFmpegAudioDecoder*, AVFrame*);
static void    ff_filters_init                              (FFmpegAudioDecoder*, int size);
#endif

#define U8_TO_SHORT(V) ((V - 128) * 128)
#define INT32_TO_SHORT(V) (V >> 16)
#define SHORT_TO_FLOAT(A) (((float)A) / 32768.0)


/*
 *  Metadata must be freed with ad_free_nfo
 */
int
ad_info_ffmpeg (WfDecoder* d)
{
	FFmpegAudioDecoder* f = (FFmpegAudioDecoder*)d->d;
	g_return_val_if_fail(f, -1);

	if (!f->codec_parameters->sample_rate) return -1;

	int64_t len = f->format_context->duration - f->format_context->start_time;
	int64_t n_frames  = (int64_t)(len * f->codec_parameters->sample_rate / AV_TIME_BASE);

	if(d->info.meta_data){
		g_clear_pointer(&d->info.meta_data, g_ptr_array_unref);
	}

	d->info = (WfAudioInfo){
		.sample_rate = f->codec_parameters->sample_rate,
		.channels    = N_CHANNELS(f),
		.frames      = n_frames,
		.length      = (n_frames * 1000) / f->codec_parameters->sample_rate,
		.bit_rate    = f->format_context->bit_rate,
		.bit_depth   = av_get_bytes_per_sample(f->codec_parameters->format) * 8,
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

	if (tags->len) {
		// sort tags
		char* order[] = {"artist", "title", "album", "track", "date"};
		int p = 0;
		for (int i=0;i<G_N_ELEMENTS(order);i++)
			if (ad_metadata_array_set_tag_postion(tags, order[i], p))
				p++;

		d->info.meta_data = tags;
	}else
		g_ptr_array_free(tags, true);

	return 0;
}


void
get_scaled_thumbnail (WfDecoder* d, int size, AdPicture* picture)
{
	FFmpegAudioDecoder* f = (FFmpegAudioDecoder*)d->d;
	if (!f->thumbnail.codec_id)
		return;
 
	const AVCodec* codec = avcodec_find_decoder(f->thumbnail.codec_id);
	f->thumbnail.codec_context = avcodec_alloc_context3(codec);
	if (!f->thumbnail.codec_context)
		return;

    f->thumbnail.codec_context->workaround_bugs = 1;
	avcodec_parameters_to_context(f->thumbnail.codec_context, f->format_context->streams[f->thumbnail.stream]->codecpar);

	int ret;
	if ((ret = avcodec_open2(f->thumbnail.codec_context, codec, NULL)) < 0) {
		pwarn("error opening thumbnail codec");
	}
#ifdef USE_FFMPEG_FILTERS

	ff_filters_init(f, size);
	if (!f->thumbnail.graph)
		return;

	AVFrame* frame = f->thumbnail.frame = av_frame_alloc();

	if (!av_buffersrc_write_frame(f->thumbnail.filter_source, frame)) {
		pwarn("error writing frame");
		return;
	}

	int attempts = 0;
	int rc = av_buffersink_get_frame(f->thumbnail.filter_sink, frame);
	while (rc == AVERROR(EAGAIN) && attempts++ < 10) {
		if (!ff_decode_video_frame(f, frame)) {
			break;
		}

		if(av_buffersrc_write_frame(f->thumbnail.filter_source, frame)){
			pwarn("Failed to write frame to filter graph");
		}
		rc = av_buffersink_get_frame(f->thumbnail.filter_sink, frame);
	}

	if(rc < 0){
		switch(rc){
			case AVERROR(EAGAIN):
				dbg(0, "error decoding frame: EAGAIN");
				break;
			default:
				dbg(0, "error decoding frame: %i", rc);
				break;
		}
	}

	int width = frame->width;
	int height = frame->height;
	int line_size = frame->linesize[0];

	uint8_t* frame_data = g_malloc0(line_size * height);
	memcpy(frame_data, frame->data[0], line_size * height);
	dbg(1, "result: %ix%i row_stride=%i", width, height, line_size);

	if (f->thumbnail.graph) {
		// Free the graph and destroy its links
		avfilter_graph_free(&f->thumbnail.graph);
	}

	av_frame_unref(f->thumbnail.frame);

	av_packet_unref(f->thumbnail.packet);
	g_clear_pointer(&f->thumbnail.packet, g_free);

	*picture = (AdPicture){
		.data = frame_data,
		.width = width,
		.height = height,
		.row_stride = line_size
	};
#else
	AVFrame* frame = av_frame_alloc();

	AVCodecContext* codec_ctx = ({
		AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);

		AVCodecParameters par = {0,};
		if (avcodec_parameters_from_context(&par, f->thumbnail.codec_context) < 0) goto out;
		if (avcodec_parameters_to_context(codec_ctx, &par) < 0) goto out;

		if (avcodec_open2(codec_ctx, codec, NULL) < 0) goto out;

		codec_ctx;
	});

	AVPacket packet;
	while (av_read_frame(f->format_context, &packet) >= 0) {
		if (packet.stream_index == f->thumbnail.stream) {
			int ret = avcodec_send_packet(codec_ctx, &packet);
			if (ret == AVERROR(EAGAIN)) {
				ret = 0;
			}

			if (ret == AVERROR_EOF) {
				return;
			}

			if (ret) {
				perr("avcodec_send_packet failed to decode video frame");
				return;
			}

			/*
			 *  If successful, avcodec_receive_frame() will return
			 *  an AVFrame containing uncompressed video data.
			 */
			ret = avcodec_receive_frame(codec_ctx, frame);
			switch (ret) {
				case 0:
					break;
				case AVERROR(EAGAIN):
					goto out;

				default:
					perr("Failed to decode video frame: avcodec_receive_frame() < 0");
					goto out;
			}

			int width_dst = (codec_ctx->width * size) / f->thumbnail.codec_context->height;
			AVFrame* frameRGB = av_frame_alloc();
			int line_size = width_dst * 3;
			line_size = (line_size / 16 + (line_size % 16 > 0 ? 1 : 0)) * 16;

			#define ALIGN 1
			uint8_t* buffer = g_malloc(size * line_size);
			av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_RGB24, width_dst, size, ALIGN);
			frameRGB->linesize[0] = line_size;

			struct SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, width_dst, size, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

			// Convert the image from its native format to RGB
			sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, codec_ctx->height, frameRGB->data, frameRGB->linesize);

			*picture = (AdPicture){
				.data = buffer,
				.width = width_dst,
				.height = size,
				.row_stride = frameRGB->linesize[0]
			};

			av_packet_unref(&packet);
			av_frame_unref(frameRGB);
			av_free(frameRGB);
			sws_freeContext(sws_ctx);
			break;
		}

		av_packet_unref(&packet);
	}

  out:
	av_frame_unref(frame);
	av_free(frame);

	avcodec_free_context(&codec_ctx);
#endif

	avcodec_free_context(&f->thumbnail.codec_context);
}


bool
ad_open_ffmpeg (WfDecoder* decoder, const char* filename)
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
	for (int i=0; i<f->format_context->nb_streams; i++) {
		AVStream* stream = f->format_context->streams[i];
		switch (stream->codecpar->codec_type) {
			case AVMEDIA_TYPE_AUDIO:
				if (f->audio_stream == -1) {
					f->audio_stream = i;
				}
				break;
			case AVMEDIA_TYPE_VIDEO:
				if (stream->codecpar->codec_id == AV_CODEC_ID_MJPEG || stream->codecpar->codec_id == AV_CODEC_ID_PNG) {
					if (stream->metadata) {
						dbg(1, "found video stream with metadata");
						f->thumbnail.stream = i;
						f->thumbnail.codec_id = stream->codecpar->codec_id;
					}
				}
				break;
			default:
				break;
		}
	}

	if (f->audio_stream == -1) {
		dbg(1, "No Audio Stream found in file");
		avformat_close_input(&f->format_context);
		goto f;
	}

	if (!f->thumbnail.codec_context) {
		dbg(2, "no thumbnail found");
	}

	f->codec_parameters = f->format_context->streams[f->audio_stream]->codecpar;

	if (!(f->codec = avcodec_find_decoder(f->codec_parameters->codec_id))) {
		avformat_close_input(&f->format_context);
		dbg(1, "Codec not supported by ffmpeg");
		goto f;
	}

	f->codec_context = avcodec_alloc_context3(f->codec);
	avcodec_parameters_to_context(f->codec_context, f->codec_parameters);

	if (avcodec_open2(f->codec_context, f->codec, NULL) < 0) {
		dbg(1, "avcodec_open failed");
		goto f;
	}

	dbg(2, "ffmpeg - audio tics: %i/%i [sec]", f->format_context->streams[f->audio_stream]->time_base.num, f->format_context->streams[f->audio_stream]->time_base.den);

	f->format_context->flags |= AVFMT_FLAG_GENPTS;
	f->format_context->flags |= AVFMT_FLAG_IGNIDX;

	if (ad_info_ffmpeg(decoder)) {
		dbg(1, "invalid file info");
		goto f;
	}

#ifdef DEBUG
	WfAudioInfo* nfo = &decoder->info;
	dbg(2, "%s", filename);
	dbg(2, "sr:%i c:%i d:%"PRIi64" f:%"PRIi64" %s", nfo->sample_rate, nfo->channels, nfo->length, nfo->frames, av_get_sample_fmt_name(f->codec_parameters->format));
#endif

#if 0 // TODO why prints nothing?
	av_dump_format(f->format_context, f->audio_stream, filename, 0);
#endif

	switch (f->codec_parameters->format) {
		case AV_SAMPLE_FMT_S16:
			dbg(1, "S16 (TODO check float out)");

			// there are 2 implementations of this - see which one is more performant
			//f->read = ff_read_default_interleaved;
			f->read = ff_read_short_interleaved_to_interleaved;

			f->read_planar = ff_read_short_interleaved_to_planar;
			break;
		case AV_SAMPLE_FMT_S16P: // e.g. ape
			f->read = ff_read_short_planar_to_interleaved;
			f->read_planar = ff_read_short_planar_to_planar;
			break;
		case AV_SAMPLE_FMT_FLT:
			f->read = ff_read_float_interleaved_to_interleaved;
			f->read_planar = ff_read_float_interleaved_to_planar;
			break;
		case AV_SAMPLE_FMT_FLTP: // e.g. mp3, m4a, dsf, opus
			f->read = ff_read_float_planar_to_interleaved;
			f->read_planar = ff_read_float_planar_to_planar;
			f->read_s32 = ff_read_float_planar_to_s32;
			break;
		case AV_SAMPLE_FMT_S32:
			pwarn("no implementation of S32 to FLT");
			f->read_planar = ff_read_int32_interleaved_to_planar;
			break;
		case AV_SAMPLE_FMT_S32P:
			pwarn("planar (non-interleaved) unhandled!");
			f->read = NULL;
			break;
		case AV_SAMPLE_FMT_U8:
			f->read_planar = ff_read_u8_interleaved_to_planar;
			break;
		default:
			pwarn("sample format not handled: %i", f->codec_parameters->format);
			break;
	}

	return TRUE;

  f:
	g_clear_pointer(&decoder->d, g_free);

	return FALSE;
}


int
ad_close_ffmpeg (WfDecoder* d)
{
	FFmpegAudioDecoder* f = (FFmpegAudioDecoder*)d->d;
	if (!f) return -1;

#ifdef USE_FFMPEG_FILTERS
	g_clear_pointer(&f->thumbnail.packet, av_packet_unref);
#endif

	av_frame_unref(&f->frame);
	avcodec_free_context(&f->codec_context);
	avformat_close_input(&f->format_context);
	g_clear_pointer(&f, g_free);

	return 0;
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
ad_read_ffmpeg (WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = (FFmpegAudioDecoder*)d->d;
	if (!f) return -1;

	g_return_val_if_fail(f->read, -1);
	return f->read(d, out, len);
}


static ssize_t
ff_read_short (WfDecoder* d, WfBuf16* out)
{
	FFmpegAudioDecoder* ff = (FFmpegAudioDecoder*)d->d;
	return ff->read_planar(d, out);
}


static ssize_t
ff_read_s32 (WfDecoder* d, int32_t* out, size_t len)
{
	FFmpegAudioDecoder* ff = (FFmpegAudioDecoder*)d->d;
	return ff->read_s32(d, out, len);
}


/*
 *  S16 (interleaved) to float (interleaved)
 */
#if 0
ssize_t
ff_read_default_interleaved (WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = d->d;
	if (!f) return -1;

	size_t frames = len / d->info.channels;

	int written = 0;
	int ret = 0;
	while (ret >= 0 && written < frames) {
		dbg(3, "loop: %i/%zu (bl:%lu)", written, frames, f->tmp_buf.len);
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
				if (f->packet.data) av_packet_unref(&f->packet);
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
			AVFrame avf;
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
					pwarn("audio seek failed.");
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
#endif


static ssize_t
ff_read_short_interleaved_to_planar (WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;

	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
	g_return_val_if_fail(data_size == 2, 0);

	bool have_frame = false;
	if (f->frame.nb_samples && f->frame_iter < f->frame.nb_samples) {
		have_frame = true;
	}

	while (have_frame || !av_read_frame(f->format_context, &f->packet)) {
		have_frame = false;
		if (f->packet.stream_index == f->audio_stream) {
			int got_frame = 0;
			if (f->frame_iter && f->frame_iter < f->frame.nb_samples) {
				got_frame = true;
			} else {
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				int ret = avcodec_receive_frame(f->codec_context, &f->frame);
				if (ret == 0) got_frame = true;
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret < 0) pwarn("error decoding audio");
			}

			if (got_frame) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(60, 0, 0)
				int size = av_samples_get_buffer_size (NULL, f->codec_context->ch_layout.nb_channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#else
				int size = av_samples_get_buffer_size (NULL, f->codec_context->channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#endif
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				for (int i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < buf->size); i++) {
					if (fr >= f->seek_frame) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(60, 0, 0)
						for (ch=0; ch<MIN(2, f->codec_context->ch_layout.nb_channels); ch++) {
							memcpy(buf->buf[ch] + n_fr_done, f->frame.data[0] + data_size * (f->codec_context->ch_layout.nb_channels * i + ch), data_size);
#else
						for (ch=0; ch<MIN(2, f->codec_context->channels); ch++) {
							memcpy(buf->buf[ch] + n_fr_done, f->frame.data[0] + data_size * (f->codec_context->channels * i + ch), data_size);
#endif
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= buf->size) goto stop;
			}
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


static ssize_t
ff_read_short_planar_to_planar (WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
	g_return_val_if_fail(data_size == 2, 0);

	bool have_frame = false;
	if (f->frame.nb_samples && f->frame_iter < f->frame.nb_samples) {
		have_frame = true;
	}

	while (have_frame || !av_read_frame(f->format_context, &f->packet)) {
		have_frame = false;
		if (f->packet.stream_index == f->audio_stream) {

			int got_frame = false;
			if (f->frame_iter && f->frame_iter < f->frame.nb_samples) {
				got_frame = true;
			} else {
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				int ret = avcodec_receive_frame(f->codec_context, &f->frame);
				if (ret == 0) got_frame = true;
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret < 0) pwarn("error decoding audio");
			}
			if (got_frame) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(60, 0, 0)
				int size = av_samples_get_buffer_size (NULL, f->codec_context->ch_layout.nb_channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#else
				int size = av_samples_get_buffer_size (NULL, f->codec_context->channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#endif
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * f->codec_context->sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
				for (int i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < buf->size); i++) {
					if (fr >= f->seek_frame) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(60, 0, 0)
						for (ch=0; ch<MIN(2, f->codec_context->ch_layout.nb_channels); ch++) {
#else
						for (ch=0; ch<MIN(2, f->codec_context->channels); ch++) {
#endif
							buf->buf[ch][n_fr_done] = *(((int16_t*)f->frame.data[ch]) + i);
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if (n_fr_done >= buf->size) goto stop;
			}
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


static ssize_t
ff_read_short_planar_to_interleaved (WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = d->d;
	int n_frames = len / d->info.channels;
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
	g_return_val_if_fail(data_size == 2, 0);

	bool have_frame = false;
	if (f->frame.nb_samples && f->frame_iter < f->frame.nb_samples) {
		have_frame = true;
	}

	while (have_frame || !av_read_frame(f->format_context, &f->packet)) {
		have_frame = false;
		if (f->packet.stream_index == f->audio_stream) {

			int got_frame = false;
			if (f->frame_iter && f->frame_iter < f->frame.nb_samples) {
				got_frame = true;
			} else {
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				int ret = avcodec_receive_frame(f->codec_context, &f->frame);
				if (ret == 0) got_frame = true;
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret < 0) pwarn("error decoding audio");
			}
			if (got_frame) {
#ifdef HAVE_FFMPEG_60
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->ch_layout.nb_channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#else
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#endif
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * f->codec_context->sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
				for (int i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < n_frames); i++) {
					if (fr >= f->seek_frame) {
#ifdef HAVE_FFMPEG_60
						for (ch=0; ch<MIN(2, f->codec_parameters->ch_layout.nb_channels); ch++) {
							out[f->codec_parameters->ch_layout.nb_channels * n_fr_done + ch] = SHORT_TO_FLOAT(*(((int16_t*)f->frame.data[ch]) + i));
#else
						for (ch=0; ch<MIN(2, f->codec_parameters->channels); ch++) {
							out[f->codec_parameters->channels * n_fr_done + ch] = SHORT_TO_FLOAT(*(((int16_t*)f->frame.data[ch]) + i));
#endif
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= n_frames) goto stop;
			}
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done * d->info.channels;
}


static ssize_t
ff_read_float_interleaved_to_interleaved (WfDecoder* d, float* out, size_t len)
{
	pwarn("no implementation of FLT to FLT");
	return -1;
}


static ssize_t
ff_read_float_interleaved_to_planar (WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;
	AVFrame frame = {0,};
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
	g_return_val_if_fail(data_size == 4, 0);

	while (!av_read_frame(f->format_context, &f->packet)) {

		if (f->packet.stream_index == f->audio_stream) {

			av_frame_unref(&frame);

			int got_frame = 0;
			int ret = avcodec_receive_frame(f->codec_context, &f->frame);
			if (ret == 0) got_frame = true;
			if (ret == AVERROR(EAGAIN)) ret = 0;
			if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
			if (ret == AVERROR(EAGAIN)) ret = 0;
			if (ret < 0) pwarn("error decoding audio");

			if (got_frame) {
#ifdef HAVE_FFMPEG_60
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->ch_layout.nb_channels, frame.nb_samples, f->codec_parameters->format, 1);
#else
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->channels, frame.nb_samples, f->codec_parameters->format, 1);
#endif
				if (size < 0)  {
					dbg(1, "av_samples_get_buffer_size invalid value");
					goto stop;
				}

				int64_t fr = frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				for (int i=0; i<frame.nb_samples && (n_fr_done < buf->size); i++) {
					if (fr >= f->seek_frame) {
#ifdef HAVE_FFMPEG_60
						for (ch=0; ch<MIN(2, f->codec_context->ch_layout.nb_channels); ch++) {
							float* src = (float*)(frame.data[0] + f->codec_context->ch_layout.nb_channels * data_size * i + data_size * ch);
#else
						for (ch=0; ch<MIN(2, f->codec_context->channels); ch++) {
							float* src = (float*)(frame.data[0] + f->codec_context->channels * data_size * i + data_size * ch);
#endif
							buf->buf[ch][n_fr_done] = (*src) * (1 << 15);
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if (n_fr_done >= buf->size) goto stop;
			}
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


/*
 * TODO there are 2 implementations of S16 to FLT - see which one is more performant
 */
static ssize_t
ff_read_short_interleaved_to_interleaved (WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = d->d;
	int n_frames = len / d->info.channels;
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
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
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				int ret = avcodec_receive_frame(f->codec_context, &f->frame);
				if (ret == 0) got_frame = true;
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret < 0) pwarn("error decoding audio");
			}
			if (got_frame) {
#ifdef HAVE_FFMPEG_60
				int size = av_samples_get_buffer_size (NULL, f->codec_context->ch_layout.nb_channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#else
				int size = av_samples_get_buffer_size (NULL, f->codec_context->channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#endif
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * f->codec_context->sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
				for (int i=f->frame_iter; i<f->frame.nb_samples; i++) {
					if (n_fr_done >= n_frames) break;
					if (fr >= f->seek_frame) {
#ifdef HAVE_FFMPEG_60
						for (ch=0; ch<MIN(2, f->codec_context->ch_layout.nb_channels); ch++) {
							int16_t* src = (int16_t*)(f->frame.data[0] + f->codec_context->ch_layout.nb_channels * data_size * i + data_size * ch);
#else
						for (ch=0; ch<MIN(2, f->codec_context->channels); ch++) {
							int16_t* src = (int16_t*)(f->frame.data[0] + f->codec_context->channels * data_size * i + data_size * ch);
#endif
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
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


static ssize_t
ff_read_float_planar_to_planar (WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
	g_return_val_if_fail(data_size == 4, 0);

	bool have_prev_frame = (f->frame.nb_samples && f->frame_iter < f->frame.nb_samples);
	while (have_prev_frame || !av_read_frame(f->format_context, &f->packet)) {
		have_prev_frame = false;
		if (f->packet.stream_index == f->audio_stream) {
			int got_frame = false;
			if (f->frame_iter && f->frame_iter < f->frame.nb_samples) {
				got_frame = true;
			} else {
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				int ret = avcodec_receive_frame(f->codec_context, &f->frame);
				if (ret == 0) got_frame = true;
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret < 0){
					char errbuff[64] = {0,};
					pwarn("error decoding audio: %s", av_make_error_string(errbuff, 64, ret));
					goto stop;
				}
			}
			if (got_frame) {
#ifdef HAVE_FFMPEG_60
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->ch_layout.nb_channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#else
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#endif
				if (size < 0)  {
					pwarn("av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
#ifdef DEBUG
#ifdef HAVE_FFMPEG_60
				for (ch=0; ch<MIN(2, f->codec_parameters->ch_layout.nb_channels); ch++) {
#else
				for (ch=0; ch<MIN(2, f->codec_parameters->channels); ch++) {
#endif
					g_return_val_if_fail(f->frame.data[ch], 0);
				}
#endif
				for (int i=f->frame_iter; i<f->frame.nb_samples; i++) {
					if (n_fr_done >= buf->size) break;
					if (fr >= f->seek_frame) {
#ifdef HAVE_FFMPEG_60
						for (ch=0; ch<MIN(2, f->codec_parameters->ch_layout.nb_channels); ch++) {
#else
						for (ch=0; ch<MIN(2, f->codec_parameters->channels); ch++) {
#endif
							float* src = (float*)(f->frame.data[ch] + data_size * i);
							// aac values can exceed 1.0 so clamping is needed
							buf->buf[ch][n_fr_done] = CLAMP((*src), -1.0f, 1.0f) * 32767.0f;
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if (n_fr_done >= buf->size) goto stop;
			}
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


static ssize_t
ff_read_float_planar_to_interleaved (WfDecoder* d, float* out, size_t len)
{
	FFmpegAudioDecoder* f = d->d;

	int n_frames = len / d->info.channels;
	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
	g_return_val_if_fail(data_size == 4, 0);

	bool have_frame = false;
	if (f->frame.nb_samples && f->frame_iter < f->frame.nb_samples) {
		have_frame = true;
	}

	while (have_frame || !av_read_frame(f->format_context, &f->packet)) {
		have_frame = false;
		if (f->packet.stream_index == f->audio_stream) {

			int got_frame = false;
			if (f->frame_iter && f->frame_iter < f->frame.nb_samples) {
				got_frame = true;
			} else {
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				int ret = avcodec_receive_frame(f->codec_context, &f->frame);
				if (ret == 0) got_frame = true;
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret < 0) pwarn("error decoding audio");
			}
			if (got_frame) {
#ifdef HAVE_FFMPEG_60
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->ch_layout.nb_channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#else
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#endif
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * f->codec_context->sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				int ch;
				for(int i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < n_frames); i++) {
					if (fr >= f->seek_frame) {
#ifdef HAVE_FFMPEG_60
						for (ch=0; ch<MIN(2, f->codec_context->ch_layout.nb_channels); ch++) {
							out[f->codec_parameters->ch_layout.nb_channels * n_fr_done + ch] = *(((float*)f->frame.data[ch]) + i);
#else
						for (ch=0; ch<MIN(2, f->codec_context->channels); ch++) {
							out[f->codec_parameters->channels * n_fr_done + ch] = *(((float*)f->frame.data[ch]) + i);
#endif
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= n_frames) goto stop;
			}
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


static ssize_t
ff_read_int32_interleaved_to_planar (WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;

	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
	g_return_val_if_fail(data_size == 4, 0);

	bool have_frame = false;
	if (f->frame.nb_samples && f->frame_iter < f->frame.nb_samples) {
		have_frame = true;
	}

	while (have_frame || !av_read_frame(f->format_context, &f->packet)) {
		have_frame = false;
		if (f->packet.stream_index == f->audio_stream) {
			int got_frame = 0;
			if (f->frame_iter && f->frame_iter < f->frame.nb_samples) {
				got_frame = true;
			} else {
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				int ret = avcodec_receive_frame(f->codec_context, &f->frame);
				if (ret == 0) got_frame = true;
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret < 0) pwarn("error decoding audio");
			}

			if (got_frame) {
#ifdef HAVE_FFMPEG_60
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->ch_layout.nb_channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#else
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#endif
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				for (int i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < buf->size); i++) {
					if (fr >= f->seek_frame) {
#ifdef HAVE_FFMPEG_60
						for (ch=0; ch<MIN(2, f->codec_context->ch_layout.nb_channels); ch++) {
							buf->buf[ch][n_fr_done] = INT32_TO_SHORT(*((int32_t*)(f->frame.data[0] + (f->codec_context->ch_layout.nb_channels * i + ch) * data_size)));
#else
						for (ch=0; ch<MIN(2, f->codec_context->channels); ch++) {
							buf->buf[ch][n_fr_done] = INT32_TO_SHORT(*((int32_t*)(f->frame.data[0] + (f->codec_context->channels * i + ch) * data_size)));
#endif
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if (n_fr_done >= buf->size) goto stop;
			}
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


static ssize_t
ff_read_u8_interleaved_to_planar (WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;

	int64_t n_fr_done = 0;

#ifdef DEBUG
	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
	g_return_val_if_fail(data_size == 1, 0);
#endif

	bool have_frame = false;
	if (f->frame.nb_samples && f->frame_iter < f->frame.nb_samples) {
		have_frame = true;
	}

	while (have_frame || !av_read_frame(f->format_context, &f->packet)) {
		have_frame = false;
		if (f->packet.stream_index == f->audio_stream) {
			int got_frame = 0;
			if (f->frame_iter && f->frame_iter < f->frame.nb_samples) {
				got_frame = true;
			} else {
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				int ret = avcodec_receive_frame(f->codec_context, &f->frame);
				if (ret == 0) got_frame = true;
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret < 0) pwarn("error decoding audio");
			}

			if (got_frame) {
#ifdef HAVE_FFMPEG_60
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->ch_layout.nb_channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#else
				int size = av_samples_get_buffer_size (NULL, f->codec_parameters->channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#endif
				if (size < 0)  {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
				int ch;
				for (int i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < buf->size); i++) {
					if (fr >= f->seek_frame) {
#ifdef HAVE_FFMPEG_60
						for (ch=0; ch<MIN(2, f->codec_parameters->ch_layout.nb_channels); ch++) {
							buf->buf[ch][n_fr_done] = U8_TO_SHORT(*(f->frame.data[0] + f->codec_parameters->ch_layout.nb_channels * i + ch));
#else
						for (ch=0; ch<MIN(2, f->codec_parameters->channels); ch++) {
							buf->buf[ch][n_fr_done] = U8_TO_SHORT(*(f->frame.data[0] + f->codec_parameters->channels * i + ch));
#endif
						}
						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if(n_fr_done >= buf->size) goto stop;
			}
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


static inline ssize_t
ff_read_to_s32 (WfDecoder* d, int32_t* out, size_t len, void (*deliver)(WfDecoder* d, int32_t*, int i, int n_fr_done))
{
	FFmpegAudioDecoder* f = d->d;

	int n_frames = len / d->info.channels;
	int64_t n_fr_done = 0;

	bool have_frame = false;
	if (f->frame.nb_samples && f->frame_iter < f->frame.nb_samples) {
		have_frame = true;
	}

	while (have_frame || !av_read_frame(f->format_context, &f->packet)) {
		have_frame = false;
		if (f->packet.stream_index == f->audio_stream) {

			int got_frame = false;
			if (f->frame_iter && f->frame_iter < f->frame.nb_samples) {
				got_frame = true;
			} else {
				av_frame_unref(&f->frame);
				f->frame_iter = 0;

				int ret = avcodec_receive_frame(f->codec_context, &f->frame);
				if (ret == 0) got_frame = true;
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
				if (ret == AVERROR(EAGAIN)) ret = 0;
				if (ret < 0) pwarn("error decoding audio");
			}
			if (got_frame) {
				int size = av_samples_get_buffer_size (NULL, N_CHANNELS(f), f->frame.nb_samples, f->codec_parameters->format, 1);
				if (size < 0) {
					dbg(0, "av_samples_get_buffer_size invalid value");
				}

				int64_t fr = f->frame.best_effort_timestamp * f->codec_context->sample_rate / f->format_context->streams[f->audio_stream]->time_base.den + f->frame_iter;
				for (int i=f->frame_iter; i<f->frame.nb_samples && (n_fr_done < n_frames); i++) {
					if (fr >= f->seek_frame) {
						deliver(d, out, i, n_fr_done);

						n_fr_done++;
						f->frame_iter++;
					}
				}
				f->output_clock = fr + n_fr_done;

				if (n_fr_done >= n_frames) goto stop;
			}
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


static ssize_t
ff_read_float_planar_to_s32 (WfDecoder* d, int32_t* out, size_t len)
{
	inline void deliver (WfDecoder* d, int32_t* out, int i, int n_fr_done)
	{
		FFmpegAudioDecoder* f = d->d;

		for (int ch=0; ch<MIN(2, N_CHANNELS(f)); ch++) {
			out[N_CHANNELS(f) * n_fr_done + ch] = FLOAT_TO_S32(*(((float*)f->frame.data[ch]) + i));
		}
	}

	return ff_read_to_s32(d, out, len, deliver);
}


/*
 *  Modified version of the decoder that does correct de-interleaving
 *  for the peakfile.
 *  Peakfile format is L+, L-, R+, R-
 *  (If format was L+, R+, L-, R-, the default decode could be used)
 */
ssize_t
ff_read_peak (WfDecoder* d, WfBuf16* buf)
{
	FFmpegAudioDecoder* f = d->d;

	int64_t n_fr_done = 0;

	int data_size = av_get_bytes_per_sample(f->codec_parameters->format);
	g_return_val_if_fail(data_size == 2, 0);

	// File is read in 1024 frame chunks
	while (!av_read_frame(f->format_context, &f->packet)) {
		g_return_val_if_fail(f->packet.stream_index == f->audio_stream, 0);
		int got_frame = false;
		if (f->frame_iter && f->frame_iter < f->frame.nb_samples) {
			got_frame = true;
		} else {
			av_frame_unref(&f->frame);
			f->frame_iter = 0;

			int ret = avcodec_receive_frame(f->codec_context, &f->frame);
			if (ret == 0) got_frame = true;
			if (ret == AVERROR(EAGAIN)) ret = 0;
			if (ret == 0) ret = avcodec_send_packet(f->codec_context, &f->packet);
			if (ret == AVERROR(EAGAIN)) ret = 0;
			if (ret < 0) {
				pwarn("error decoding audio");
				goto stop;
			}
		}

		if (got_frame) {
#ifdef HAVE_FFMPEG_60
			int size = av_samples_get_buffer_size (NULL, f->codec_parameters->ch_layout.nb_channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#else
			int size = av_samples_get_buffer_size (NULL, f->codec_parameters->channels, f->frame.nb_samples, f->codec_parameters->format, 1);
#endif
			if (size < 0)  {
				dbg(0, "av_samples_get_buffer_size invalid value");
			}

#if 0
			int64_t fr = f->frame.best_effort_timestamp * d->info.sample_rate / f->format_context->streams[f->audio_stream]->time_base.den;
			g_assert(fr == f->output_clock);
#endif
			int ch;
#ifdef HAVE_FFMPEG_60
			int n_ch = MIN(2, f->codec_context->ch_layout.nb_channels);
#else
			int n_ch = MIN(2, f->codec_context->channels);
#endif
// TODO why does ffmpeg give us more frames than the buffer can hold?
			int remaining_in_buffer = buf->size - n_fr_done;
			int iter_max = MIN(remaining_in_buffer - 2, f->frame.nb_samples);
			//for(int i=f->frame_iter; (f->frame_iter < f->frame.nb_samples) && (n_fr_done + 2 < buf->size); i+=2){
			for (int i=f->frame_iter; f->frame_iter < iter_max; i+=2) {
				for (ch=0; ch<n_ch; ch++) {
#ifdef HAVE_FFMPEG_60
					memcpy(buf->buf[ch] + n_fr_done, f->frame.data[0] + data_size * (f->codec_context->ch_layout.nb_channels * i + 2 * ch), data_size * 2);
#else
					memcpy(buf->buf[ch] + n_fr_done, f->frame.data[0] + data_size * (f->codec_context->channels * i + 2 * ch), data_size * 2);
#endif
				}
				n_fr_done += 2;
				f->frame_iter += 2;
			}
			f->output_clock = n_fr_done;

			if (n_fr_done >= buf->size) goto stop;
		}
		av_packet_unref(&f->packet);
		continue;

		stop:
			av_packet_unref(&f->packet);
			break;
	}

	return n_fr_done;
}


static int64_t
ad_seek_ffmpeg (WfDecoder* d, int64_t pos)
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

	// TODO this looks like a memory leak
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
ad_eval_ffmpeg (const char* f)
{
	char* ext = strrchr(f, '.');
	if (!ext) return 10;

	return 40;
}


/*
 *  Move a metadata item up (can only be up) to the specified position.
 *
 *  returns true if the tag was found.
 */
static bool
ad_metadata_array_set_tag_postion (GPtrArray* tags, const char* tag_name, int pos)
{
	if((tags->len < 4) || (pos >= tags->len - 2)) return false;
	pos *= 2;

	char** data = (char**)tags->pdata;
	for (int i=pos;i<tags->len;i+=2) {
		if (!strcmp(data[i], tag_name)) {
			const char* item[] = {data[i], (const char*)data[i+1]};

			for (int j=i;j>pos;j-=2) {
				data[j    ] = data[j - 2    ];
				data[j + 1] = data[j - 2 + 1];
			}
			data[pos    ] = (char*)item[0];
			data[pos + 1] = (char*)item[1];

			return true;
		}
	}
	return false;
}


#ifdef USE_FFMPEG_FILTERS
static void
ff_filters_init (FFmpegAudioDecoder* f, int size)
{
	if(!f->thumbnail.codec_context)
		return;

#if HAVE_FFMPEG_4
	// not needed since ffmpeg-4.0
	avfilter_register_all();
#endif

	AVFilterGraph* graph = f->thumbnail.graph = avfilter_graph_alloc();
	g_assert(graph);

	AVRational time_base = f->format_context->streams[f->thumbnail.stream]->time_base;

	char* str = g_strdup_printf("video_size=%ix%i:pix_fmt=%i:time_base=%d/%d:pixel_aspect=%d/%d",
		f->thumbnail.codec_context->width, f->thumbnail.codec_context->height,
		f->thumbnail.codec_context->pix_fmt,
		time_base.num, time_base.den,
		f->thumbnail.codec_context->sample_aspect_ratio.num, f->thumbnail.codec_context->sample_aspect_ratio.den
	);

	if (avfilter_graph_create_filter(&f->thumbnail.filter_source, avfilter_get_by_name("buffer"), "thumb_buffer", str, NULL, graph) < 0) {
		pwarn("Failed to create filter source: %s", str);
		avfilter_graph_free(&f->thumbnail.graph);
		goto out;
	}
	if (avfilter_graph_create_filter(&f->thumbnail.filter_sink, avfilter_get_by_name("buffersink"), "thumb_buffersink", NULL, NULL, graph)) {
		pwarn("Failed to create filter sink");
	}
	AVFilterContext* scale_filter = NULL;
	float scale_ratio = (float)size / (float)f->thumbnail.codec_context->height;
	char scale[64] = {0,};
	sprintf(scale, "w=%i:h=%i", (int)((float)f->thumbnail.codec_context->width * scale_ratio), size);
	if(avfilter_graph_create_filter(&scale_filter, avfilter_get_by_name("scale"), "thumb_scale", scale, NULL, graph)){
		pwarn("Failed to create scale filter");
	}

	AVFilterContext* format_filter = NULL;
	if(avfilter_graph_create_filter(&format_filter, avfilter_get_by_name("format"), "thumb_format", "pix_fmts=rgb24", NULL, graph)){
		pwarn("Failed to create format filter");
	}
	//av_opt_set_int_list(f->thumbnail.filter_sink, "pix_fmts", pixel_formats, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
	//buffersinkParams.release();

	if(avfilter_link(format_filter, 0, f->thumbnail.filter_sink, 0)){
		pwarn("Failed to link final filter");
	}

	if(avfilter_link(scale_filter, 0, format_filter, 0)){
		pwarn("Failed to link scale filter");
	}

	if(avfilter_link(f->thumbnail.filter_source, 0, scale_filter, 0)){
		pwarn("Failed to link source filter");
	}

	if(avfilter_graph_config(graph, NULL)){
		pwarn("Failed to configure filter graph");
	}

  out:
	g_free(str);
}
#endif


#ifdef USE_FFMPEG_FILTERS
static bool
decode_video_packet (FFmpegAudioDecoder* f)
{
	if (f->thumbnail.packet->stream_index != f->thumbnail.stream) {
		return false;
	}

	av_frame_unref(f->thumbnail.frame);

	int frame_finished = false;

	/*
	 *  Send the packet to the decoder. The packet is not modified.
	 */
	int ret = avcodec_send_packet(f->thumbnail.codec_context, f->thumbnail.packet);
	if (ret == AVERROR(EAGAIN)) {
		ret = 0;
	}

	if (ret == AVERROR_EOF) {
		return false;
	}

	if (ret) {
		perr("avcodec_send_packet failed to decode video frame");
		return false;
	}

	/*
	 *  If successful, avcodec_receive_frame() will return
	 *  an AVFrame containing uncompressed video data.
	 */
	ret = avcodec_receive_frame(f->thumbnail.codec_context, f->thumbnail.frame);
	switch (ret) {
		case 0:
			return true;

		case AVERROR(EAGAIN):
			return false;

		default:
			perr("Failed to decode video frame: avcodec_receive_frame() < 0");
	}

	return frame_finished > 0;
}
#endif


#ifdef USE_FFMPEG_FILTERS
static bool
get_video_packet (FFmpegAudioDecoder* f)
{
	bool framesAvailable = true;
	bool frameDecoded = false;

	if (f->thumbnail.packet) {
		av_packet_unref(f->thumbnail.packet);
		f->thumbnail.packet = NULL;
	}

	f->thumbnail.packet = av_packet_alloc();

	while (framesAvailable && !frameDecoded) {
		/*
		 *  Fill the packet. For video, the packet contains exactly one frame.
		 */
		framesAvailable = av_read_frame(f->format_context, f->thumbnail.packet) >= 0;
		if (framesAvailable) {
			frameDecoded = f->thumbnail.packet->stream_index == f->thumbnail.stream;
			if (!frameDecoded) {
				av_packet_unref(f->thumbnail.packet);
			}
		}
	}

	return frameDecoded;
}
#endif


#ifdef USE_FFMPEG_FILTERS
static bool
ff_decode_video_frame (FFmpegAudioDecoder* f, AVFrame* frame)
{
	bool frame_finished = false;

	while (!frame_finished && get_video_packet(f)) {
		frame_finished = decode_video_packet(f);
	}

	if (!frame_finished) {
		perr("decode_video_frame failed: frame not finished");
	}

	return frame_finished;
}
#endif


const static AdPlugin ad_ffmpeg = {
	&ad_eval_ffmpeg,
	&ad_open_ffmpeg,
	&ad_close_ffmpeg,
	&ad_info_ffmpeg,
	&ad_seek_ffmpeg,
	&ad_read_ffmpeg,
	&ff_read_short,
	&ff_read_s32
};


/* dlopen handler */
const AdPlugin*
get_ffmpeg ()
{
	static int ffinit = 0;
	if (!ffinit) {
		ffinit = 1;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 5, 0)
		avcodec_init();
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
		av_register_all();
		avcodec_register_all();
#endif
		av_log_set_level(wf_debug > 1 ? AV_LOG_VERBOSE : AV_LOG_QUIET);
	}
	return &ad_ffmpeg;
}

