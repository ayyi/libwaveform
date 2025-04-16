/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include "waveform/utils.h"

#ifndef true
#define true TRUE
#define false FALSE
#endif

float      wf_int2db                  (short);

#ifdef __wf_private__

#define TIMER_STOP FALSE
#define TIMER_CONTINUE TRUE

#include "waveform/ui-typedefs.h"

void       wf_deinterleave            (float* src, float** dest, uint64_t n_frames);
void       wf_deinterleave16          (short* src, short** dest, uint64_t n_frames);
#endif

#ifndef __ayyi_utils_h__
#ifdef __GTK_H__
uint32_t   wf_get_gtk_fg_color        (GtkWidget*, GtkStateType);
uint32_t   wf_get_gtk_text_color      (GtkWidget*, GtkStateType);
uint32_t   wf_get_gtk_base_color      (GtkWidget*, GtkStateType, char alpha);
uint32_t   wf_color_gdk_to_rgba       (GdkColor*);
#endif
void       wf_colour_rgba_to_float    (AGlColourFloat*, uint32_t rgba);
bool       wf_colour_is_dark_rgba     (uint32_t);

void       wf_load_texture_from_alphabuf
                                      (WaveformContext*, int texture_id, AlphaBuf*);

guint64    wf_get_time                ();

#endif //__ayyi_utils_h__
