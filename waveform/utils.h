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
#ifndef __waveform_utils_h__
#define __waveform_utils_h__
#ifndef __utils_c__
extern int wf_debug;
#endif

#ifndef true
#define true TRUE
#define false FALSE
#endif
#ifndef __ayyi_utils_h__
#ifdef __wf_private__
#ifndef bool
#define bool gboolean
#endif
#define dbg(A, B, ...) wf_debug_printf(__func__, A, B, ##__VA_ARGS__)
#endif
#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define gerr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)
#define perr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)
#define PF {if(wf_debug) printf("%s()...\n", __func__);}
#define PF0 {printf("%s()...\n", __func__);}
#define PF2 {if(wf_debug > 1) printf("%s()...\n", __func__);}
#define IDLE_STOP FALSE
#define IDLE_CONTINUE TRUE
#define TIMER_STOP FALSE
#define TIMER_CONTINUE TRUE
#ifndef g_free0
#define g_free0(var) ((var == NULL) ? NULL : (var = (g_free(var), NULL)))
#endif
#ifndef g_list_free0
#define g_list_free0(var) ((var == NULL) ? NULL : (var = (g_list_free (var), NULL)))
#define call(FN, A, ...) if(FN) (FN)(A, ##__VA_ARGS__)
#endif
#include "waveform/typedefs.h"

void       wf_debug_printf         (const char* func, int level, const char* format, ...);
void       deinterleave            (float* src, float** dest, uint64_t n_frames);
void       deinterleave16          (short* src, short** dest, uint64_t n_frames);
int        wf_power_of_two         (int);
float      wf_int2db               (short);
#ifdef __wf_private__
void       wf_colour_rgba_to_float (WfColourFloat*, uint32_t rgba);
void       wf_rgba_to_float        (uint32_t rgba, float* r, float* g, float* b);
bool       wf_get_filename_for_other_channel(const char* filename, char* other, int n_chars);
#endif

#endif

#endif //__waveform_utils_h__
