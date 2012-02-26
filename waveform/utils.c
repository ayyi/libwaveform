
#define __utils_c__
#define __wf_private__
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


