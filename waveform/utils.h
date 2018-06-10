/*
  Copyright (C) 2012-2018 Tim Orford <tim@orford.org>

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
#ifndef __waveform_utils_h__
#define __waveform_utils_h__
#include "stdint.h"
#ifndef __wf_utils_c__
extern int wf_debug;
#endif

#ifndef true
#define true TRUE
#define false FALSE
#endif
#ifndef g_list_free0
#define g_list_free0(var) ((var == NULL) ? NULL : (var = (g_list_free (var), NULL)))
#endif

#ifdef __wf_private__
#ifndef __ayyi_debug_h__
#define dbg(A, B, ...) wf_debug_printf(__func__, A, B, ##__VA_ARGS__)
#define perr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)
#endif
#endif

#ifndef __ayyi_utils_h__
#ifdef __wf_private__
#ifndef bool
#define bool gboolean
#endif
#endif
#ifndef PF
#define PF {if(wf_debug) printf("%s()...\n", __func__);}
#define PF0 {printf("%s()...\n", __func__);}
#define PF2 {if(wf_debug > 1) printf("%s()...\n", __func__);}
#endif
#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define gerr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)
#define TIMER_STOP FALSE
#define TIMER_CONTINUE TRUE
#ifndef g_free0
#define g_free0(var) ((var == NULL) ? NULL : (var = (g_free(var), NULL)))
#endif
#define call(FN, A, ...) if(FN) (FN)(A, ##__VA_ARGS__)
#define WF_NEW(T, ...) ({T* obj = g_new0(T, 1); *obj = (T){__VA_ARGS__}; obj;})
#include "waveform/typedefs.h"

void       wf_debug_printf            (const char* func, int level, const char* format, ...);
void       wf_deinterleave            (float* src, float** dest, uint64_t n_frames);
void       wf_deinterleave16          (short* src, short** dest, uint64_t n_frames);
#endif //__ayyi_utils_h__
float      wf_int2db                  (short);

#ifndef __ayyi_utils_h__
#ifdef __wf_private__
#ifdef __GTK_H__
uint32_t   wf_get_gtk_fg_color        (GtkWidget*, GtkStateType);
uint32_t   wf_get_gtk_text_color      (GtkWidget*, GtkStateType);
uint32_t   wf_get_gtk_base_color      (GtkWidget*, GtkStateType, char alpha);
uint32_t   wf_color_gdk_to_rgba       (GdkColor*);
#endif
void       wf_colour_rgba_to_float    (AGlColourFloat*, uint32_t rgba);
bool       wf_colour_is_dark_rgba     (uint32_t);

bool       wf_get_filename_for_other_channel(const char* filename, char* other, int n_chars);
guint64    wf_get_time                ();
#endif

#endif //__ayyi_utils_h__

#endif //__waveform_utils_h__
