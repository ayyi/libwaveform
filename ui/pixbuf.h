/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform https://github.com/ayyi/libwaveform |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include "waveform/ui-typedefs.h"
#ifdef USE_GDK_PIXBUF
#include <gdk-pixbuf/gdk-pixdata.h>
#include "waveform/waveform.h"
#include "waveform/gdktexture.h"
#endif

typedef struct {
	int        width;
	int        height;
	guchar*    data;
	int        rowstride;
	bool       has_alpha;
} AGlPixbuf;

#ifdef USE_GDK_PIXBUF
typedef void (WfPixbufCallback) (Waveform*, GdkPixbuf*, gpointer);

void       waveform_peak_to_gdkpixbuf    (Waveform*, GdkPixbuf*, WfSampleRegion*, uint32_t colour, uint32_t bg_colour, bool single);
#endif
void       waveform_peak_to_pixbuf       (Waveform*, AGlPixbuf*, WfPeakParams*);
#ifdef USE_GDK_PIXBUF
void       waveform_peak_to_pixbuf_async (Waveform*, GdkPixbuf*, WfSampleRegion*, uint32_t colour, uint32_t bg_colour, WfPixbufCallback, gpointer);
#endif
void       waveform_peak_to_pixbuf_full  (Waveform*, AGlPixbuf*, uint32_t src_inset, int* start, int* end, double samples_per_px, uint32_t colour, uint32_t bg_colour, float gain, bool single_channel);
#ifdef USE_GDK_PIXBUF
void       waveform_rms_to_pixbuf        (Waveform*, GdkPixbuf*, uint32_t src_inset, int* start, int* end, double samples_per_px, uint32_t colour, uint32_t bg_colour, float gain);

AGlPixbuf* wf_gdkpixbuf_to_pixbuf        (AGlPixbuf*, GdkPixbuf*);
#endif

struct _alpha_buf {
	int        width;
	int        height;
	guchar*    buf;
	int        buf_size;
};

AlphaBuf*  wf_alphabuf_new              (Waveform*, int blocknum, int scale, bool is_rms, int border);
AlphaBuf*  wf_alphabuf_new_hi           (Waveform*, int blocknum, int scale, bool is_rms, int border);
void       wf_alphabuf_free             (AlphaBuf*);
#ifdef USE_GDK_PIXBUF
GdkPixbuf* wf_alphabuf_to_pixbuf        (AlphaBuf*);
#endif

void       waveform_peak_to_alphabuf    (Waveform*, AlphaBuf*, int scale, int* start, int* end, uint32_t colour);
void       waveform_peak_to_alphabuf_hi (Waveform*, AlphaBuf*, int block, WfSampleRegion, uint32_t colour);
void       waveform_rms_to_alphabuf     (Waveform*, AlphaBuf*, int* start, int* end, double samples_per_px, uint32_t colour_fg, uint32_t colour_bg);
