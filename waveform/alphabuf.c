/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __waveform_peak_c__
#define __wf_private__
#include "config.h"
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <sndfile.h>
#include "agl/debug.h"
#include "agl/utils.h"
#include "waveform/waveform.h"
#include "waveform/alphabuf.h"

#define WF_TEXTURE_HEIGHT 256 //intel 945 seems to work better with square textures
#define BITS_PER_PIXEL 8


static AlphaBuf*
_alphabuf_new(int width, int height)
{
	AlphaBuf* a = g_new0(AlphaBuf, 1);
	a->width = width;
	a->height = height;
	a->buf_size = width * height;
	a->buf = g_malloc(a->buf_size);
	return a;
}


AlphaBuf*
wf_alphabuf_new(Waveform* waveform, int blocknum, int scale, gboolean is_rms, int overlap)
{
	//copy part of the audio peakfile to an Alphabuf suitable for use as a GL texture.
	// @param blocknum - if -1, use the whole peakfile.
	// @param scale    - normally 1. If eg 16, then the alphabuf will have one line per 16 of the input.
	// @param overlap  - the ends of the alphabuf will contain this much data from the adjacent blocks.
	//                   TODO ensure that the data to fill the RHS border is loaded (only applies to hi-res mode)

	/*
	block overlapping ('/' denotes non-visible border area)

	----------
	      |//|      block N
	----------
	   ----------
	   |//|         block N+1 - note that the start of this block is offset by 2 * BORDER from the end of the previous block
	   ----------
	*/
	PF2;
	WaveformPrivate* _w = waveform->priv;
	g_return_val_if_fail(_w->num_peaks, NULL);
	GdkColor fg_colour = {0, 0xffff, 0xffff, 0xffff};

	int x_start;
	int x_stop;
	int width;
	if(blocknum == -1){
		x_start = 0;
		x_stop  = width = _w->num_peaks;
	}else{
		int n_blocks = waveform->priv->n_blocks;
		dbg(2, "block %i/%i", blocknum, n_blocks);
		gboolean is_last = (blocknum == n_blocks - 1);

		x_start = blocknum * (WF_PEAK_TEXTURE_SIZE - 2 * overlap) - overlap;
		x_stop  = x_start + WF_PEAK_TEXTURE_SIZE; // irrespective of overlap, the whole texture is used.

		width = WF_PEAK_TEXTURE_SIZE;
		if(is_last){
			int width_ = _w->num_peaks % (WF_PEAK_TEXTURE_SIZE - 2 * overlap);
			if(!width_) width_ = width;
			dbg(2, "is_last: width_=%i", width_);
			width = agl_power_of_two(width_ -1);
			x_stop  = MIN(_w->num_peaks, x_start + width_);
		}
		dbg (1, "block_num=%i width=%i px_start=%i px_stop=%i (%i)", blocknum, width, x_start, x_stop, x_stop - x_start);
	}
	AlphaBuf* buf = _alphabuf_new(width, is_rms ? WF_TEXTURE_HEIGHT / 2: WF_TEXTURE_HEIGHT);

	if(is_rms){
		#define SCALE_BODGE 2;
		double samples_per_px = WF_PEAK_TEXTURE_SIZE * SCALE_BODGE;
		uint32_t bg_colour = 0x00000000;
		waveform_rms_to_alphabuf(waveform, buf, &x_start, &x_stop, samples_per_px, &fg_colour, bg_colour);
	}else{

		x_start += 1; //TODO waveform_peak_to_alphabuf has a 1px offset.

		memset(buf->buf, 0, buf->buf_size); //TODO only clear start of first block and end of last block
		waveform_peak_to_alphabuf(waveform, buf, scale, &x_start, &x_stop, &fg_colour);
	}

#if 0
	//put dots in corner for debugging:
	{
		buf->buf [TEX_BORDER]                                  = 0xff;
		buf->buf [buf->width - TEX_BORDER  - 1]                = 0xff;
		buf->buf [buf->width * (buf->height - 1) + TEX_BORDER] = 0xff;
		buf->buf [buf->buf_size - TEX_BORDER -1]               = 0xff;
	}
#endif

	return buf;
}


AlphaBuf*
wf_alphabuf_new_hi(Waveform* waveform, int blocknum, int Xscale, gboolean is_rms, int overlap)
{
	//TODO merge back with wf_alphabuf_new()
	// -width is different
	// -calls waveform_peak_to_alphabuf_hi instead of waveform_peak_to_alphabuf

	//copy part of the audio peakfile to an Alphabuf suitable for use as a GL texture.
	// @param blocknum - if -1, use the whole peakfile.
	// @param scale    - normally 1. If eg 16, then the alphabuf will have one line per 16 of the input.
	// @param overlap  - the ends of the alphabuf will contain this much data from the adjacent blocks.
	//                   TODO ensure that the data to fill the RHS border is loaded (only applies to hi-res mode)

	/*
	block overlapping ('/' denotes non-visible border area)

	----------
	      |//|      block N
	----------
	   ----------
	   |//|         block N+1 - note that the start of this block is offset by 2 * BORDER from the end of the previous block
	   ----------
	*/
	PF2;
	WaveformPrivate* _w = waveform->priv;
	g_return_val_if_fail(_w->num_peaks, NULL);
	GdkColor fg_colour = {0, 0xffff, 0xffff, 0xffff};

	int x_start;
	int x_stop;
	int width;
	if(blocknum == -1){
		x_start = 0;
		x_stop  = width = _w->num_peaks;
	}else{
		int n_blocks = waveform->priv->n_blocks;
		dbg(2, "block %i/%i", blocknum, n_blocks);
		gboolean is_last = (blocknum == n_blocks - 1);

		width = WF_PEAK_TEXTURE_SIZE * 16;
		//TODO intel 945 has max texture size of 2048
		width = WF_PEAK_TEXTURE_SIZE * 8;

		x_start = - overlap;
		x_stop  = x_start + width; // irrespective of overlap, the whole texture is used.

		if(is_last){
			int width_ = _w->num_peaks % (WF_PEAK_TEXTURE_SIZE - 2 * overlap);
			if(!width_) width_ = width;
			dbg(2, "is_last: width_=%i", width_);
			width = agl_power_of_two(width_ -1);
			x_stop  = MIN(_w->num_peaks, x_start + width_);
		}
		dbg (0, "block_num=%i width=%i px_start=%i px_stop=%i (%i)", blocknum, width, x_start, x_stop, x_stop - x_start);
	}
	AlphaBuf* buf = _alphabuf_new(width, is_rms ? WF_TEXTURE_HEIGHT / 2: WF_TEXTURE_HEIGHT);

	if(is_rms){
		#define SCALE_BODGE 2;
		double samples_per_px = WF_PEAK_TEXTURE_SIZE * SCALE_BODGE;
		uint32_t bg_colour = 0x00000000;
		waveform_rms_to_alphabuf(waveform, buf, &x_start, &x_stop, samples_per_px, &fg_colour, bg_colour);
	}else{

		//x_start += 1; //TODO waveform_peak_to_alphabuf has a 1px offset.

		memset(buf->buf, 0, buf->buf_size); //TODO only clear start of first block and end of last block
		WfSampleRegion region = {x_start, x_stop - x_start};
		waveform_peak_to_alphabuf_hi(waveform, buf, blocknum, region, &fg_colour);
	}

#if 0
	//put dots in corner for debugging:
	{
		buf->buf [TEX_BORDER]                                  = 0xff;
		buf->buf [buf->width - TEX_BORDER  - 1]                = 0xff;
		buf->buf [buf->width * (buf->height - 1) + TEX_BORDER] = 0xff;
		buf->buf [buf->buf_size - TEX_BORDER -1]               = 0xff;
	}
#endif

	return buf;
}


void
wf_alphabuf_free(AlphaBuf* a)
{
	if(a){
		g_free(a->buf);
		g_free(a);
	}
}


GdkPixbuf*
wf_alphabuf_to_pixbuf(AlphaBuf* a)
{
	GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, BITS_PER_PIXEL, a->width, a->height);
	guchar* buf = gdk_pixbuf_get_pixels(pixbuf);
	int n = gdk_pixbuf_get_n_channels(pixbuf);

	int y; for(y=0;y<a->height;y++){
		int py = y * gdk_pixbuf_get_rowstride(pixbuf);
		//g_return_val_if_fail(py + (a->width -1) * n + 2 < (gdk_pixbuf_get_rowstride(pixbuf) * a->height), pixbuf);
		int x; for(x=0;x<a->width;x++){
			buf[py + x * n    ] = a->buf[y * a->width + x];
			buf[py + x * n + 1] = a->buf[y * a->width + x];
			buf[py + x * n + 2] = a->buf[y * a->width + x];
		}
	}
	return pixbuf;
}


