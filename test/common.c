/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform https://github.com/ayyi/libwaveform |
 | copyright (C) 2012-2021 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#define __common_c__
#define __wf_private__
#include "config.h"
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#if defined(USE_GTK) || defined(__GTK_H__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif
#include <glib-object.h>
#include <sndfile.h>
#include "agl/actor.h"
#include "wf/private.h"
#include "waveform/utils.h"
#include "test/runner.h"
#include "test/common.h"

extern char     current_test_name[];
extern gpointer tests[];


WfTest*
wf_test_new ()
{
	static WfTest* t = NULL;
	//if(t) g_free0(t); // valgrind says 'invalid free'

	return t = WF_NEW(WfTest, .test_idx = TEST.current.test);
}


bool
get_random_boolean ()
{
	int r = rand();
	int s = RAND_MAX / 2;
	int t = r / s;
	return t;
}


int
get_random_int (int max)
{
	if(max > RAND_MAX) pwarn("too high");
	int r = rand();
	int s = RAND_MAX / max;
	int t = r / s;
	return t;
}


void
errprintf4 (char* format, ...)
{
	char str[256];

	va_list argp;           //points to each unnamed arg in turn
	va_start(argp, format); //make ap (arg pointer) point to 1st unnamed arg
	vsprintf(str, format, argp);
	va_end(argp);           //clean up

	printf("%s%s%s\n", red, str, ayyi_white);
}


void
create_large_file (char* filename)
{
	printf("  %s\n", filename);

	int n_channels = 2;
	long n_frames = 2048;
	double* buffer = (double*) g_malloc0(n_frames * sizeof(double) * n_channels);

	int i; for(i=0;i<n_frames;i++){
		float i_f = (float)i;
		float freq = i_f * (1.0 - i_f / (n_frames * 2.0)) / 5.0; // reducing
		buffer[2 * i] = buffer[2 * i + 1] = sin(freq) * (n_frames - i) / n_frames;
	}

	SF_INFO info = {
		0,
		44100,
		n_channels,
		SF_FORMAT_WAV | SF_FORMAT_PCM_16
	};

	SNDFILE* sndfile = sf_open(filename, SFM_WRITE, &info);
	if(!sndfile) {
		fprintf(stderr, "Sndfile open failed: %s\n", sf_strerror(sndfile));
		FAIL_TEST("%s", sf_strerror(sndfile));
	}

	for(i=0;i<1<<16;i++){
		if(sf_writef_double(sndfile, buffer, n_frames) != n_frames){
			fprintf(stderr, "Write failed\n");
			sf_close(sndfile);
			FAIL_TEST("write failed");
		}
	}

	sf_write_sync(sndfile);
	sf_close(sndfile);
	g_free(buffer);
}

