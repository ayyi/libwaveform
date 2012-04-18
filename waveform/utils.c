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

  ---------------------------------------------------------------

  WaveformView is a Gtk widget based on GtkDrawingArea.
  It displays an audio waveform represented by a Waveform object.

*/
#define __utils_c__
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <sys/ioctl.h>
#include <glib.h>
#include "waveform/peak.h"
#include "waveform/utils.h"

int wf_debug = 0;


void
wf_debug_printf(const char* func, int level, const char* format, ...)
{
    va_list args;

    va_start(args, format);
    if (level <= wf_debug) {
        fprintf(stderr, "%s(): ", func);
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
    }
    va_end(args);
}


void
deinterleave(float* src, float** dest, uint64_t n_frames)
{
	int f; for(f=0;f<n_frames;f++){
		int c; for(c=0;c<WF_STEREO;c++){
			dest[c][f] = src[f * WF_STEREO + c];
		}
	}
}


void
deinterleave16(short* src, short** dest, uint64_t n_frames)
{
	int f; for(f=0;f<n_frames;f++){
		int c; for(c=0;c<WF_STEREO;c++){
			dest[c][f] = src[f * WF_STEREO + c];
		}
	}
}


int
wf_power_of_two(int a)
{
	// return the next power of two up from the given value.

	int i = 0;
	int orig = a;
	while(a){
		a = a >> 1;
		i++;
	}
	dbg (2, "%i -> %i", orig, 1 << i);
	return 1 << i;
}


float
wf_int2db(short x)
{
	//converts a signed 16bit int to a dB value.

	float y;

	if(x != 0){
		y = -20.0 * log10(32768.0/ABS(x));
		//printf("int2db: %f\n", 32768.0/abs(x));
	} else {
		y = -100.0;
	}

	return y;
}


void
wf_colour_rgba_to_float(WfColourFloat* colour, uint32_t rgba)
{
	//returned values are in the range 0.0 to 1.0;

	g_return_if_fail(colour);

	colour->r = (float)((rgba & 0xff000000) >> 24) / 0xff;
	colour->g = (float)((rgba & 0x00ff0000) >> 16) / 0xff;
	colour->b = (float)((rgba & 0x0000ff00) >>  8) / 0xff;
}


void
wf_rgba_to_float(uint32_t rgba, float* r, float* g, float* b)
{
	double _r = (rgba & 0xff000000) >> 24;
	double _g = (rgba & 0x00ff0000) >> 16;
	double _b = (rgba & 0x0000ff00) >>  8;

	*r = _r / 0xff;
	*g = _g / 0xff;
	*b = _b / 0xff;
	dbg (3, "%08x --> %.2f %.2f %.2f", rgba, *r, *g, *b);
}


