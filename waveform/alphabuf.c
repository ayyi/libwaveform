/*
  copyright (C) 2012 Tim Orford <tim@orford.org>

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
#define __waveform_peak_c__
#define __wf_private__
#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <sndfile.h>
#include <gtk/gtk.h>
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/alphabuf.h"

//#define WF_TEXTURE_HEIGHT 128 //1024
#define WF_TEXTURE_HEIGHT 256 //intel 945 seems to work better with square textures
#define BITS_PER_PIXEL 8

static int   waveform_get_n_textures    (Waveform*);


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
wf_alphabuf_new(Waveform* waveform, int blocknum, int scale, gboolean is_rms, int border)
{
	//copy part of the audio peakfile to an Alphabuf suitable for use as a GL texture.
	// @param blocknum - if -1, use the whole peakfile.
	// @param scale    - normally 1. If eg 16, then the alphabuf will have one line per 16 of the input.

	PF2;
	g_return_val_if_fail(waveform->num_peaks, NULL);
	GdkColor fg_colour = {0, 0xffff, 0xffff, 0xffff};

	int px_start;
	int px_stop;
	int width;
	if(blocknum == -1){
		px_start = 0;
		px_stop  = width = waveform->num_peaks;
	}else{
		int n_blocks = waveform_get_n_textures(waveform);
		dbg(2, "block %i/%i", blocknum, n_blocks);
		gboolean is_last = (blocknum == n_blocks - 1);

		px_start =  blocknum      * WF_PEAK_TEXTURE_SIZE;
		px_stop  = (blocknum + 1) * WF_PEAK_TEXTURE_SIZE;

		width = WF_PEAK_TEXTURE_SIZE;
		if(is_last){
			int width_ = waveform->num_peaks % WF_PEAK_TEXTURE_SIZE;
			dbg(1, "is_last width_=%i", width_);
			width      = wf_power_of_two(width_);
			//px_stop  = px_start + WF_PEAK_TEXTURE_SIZE * blocks->last_fraction;
			px_stop  = px_start + width_;
		}
		dbg (2, "block_num=%i width=%i px_stop=%i", blocknum, width, px_stop, is_last);
	}
	AlphaBuf* buf = _alphabuf_new(width, is_rms ? WF_TEXTURE_HEIGHT / 2: WF_TEXTURE_HEIGHT);

	if(is_rms){
		#define SCALE_BODGE 2;
		double samples_per_px = WF_PEAK_TEXTURE_SIZE * SCALE_BODGE;
		uint32_t bg_colour = 0x00000000;
		waveform_rms_to_alphabuf(waveform, buf, &px_start, &px_stop, samples_per_px, &fg_colour, bg_colour);
	}else{

		uint32_t bg_colour = 0x00000000;
		waveform_peak_to_alphabuf(waveform, buf, scale, &px_start, &px_stop, &fg_colour, bg_colour, border);
	}

#if 0
	//put dots in corner for debugging:
	{
		buf->buf [0]                              = 0xff;
		buf->buf [buf->width   - 1]               = 0xff;
		buf->buf [buf->width * (buf->height - 1)] = 0xff;
		buf->buf [buf->buf_size -1]               = 0xff;
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


static int
waveform_get_n_textures(Waveform* waveform)
{
	WfGlBlock* blocks = waveform->textures;
	if(blocks) return blocks->size;
	gerr("!! no glblocks\n");
	return -1;
}


