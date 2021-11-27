/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. http://ayyi.org               |
 | copyright (C) 2013-2021 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <stdbool.h>
#include <math.h>
#include <glib.h>
#include <sndfile.h>
#include "debug/debug.h"
#include "test/runner.h"


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
create_large_file (char* filename)
{
	printf("  %s\n", filename);

	int n_channels = 2;
	long n_frames = 2048;
	double* buffer = (double*) g_malloc0(n_frames * sizeof(double) * n_channels);

	for (int i=0;i<n_frames;i++) {
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
	if (!sndfile) {
		fprintf(stderr, "Sndfile open failed: %s\n", sf_strerror(sndfile));
		FAIL_TEST("%s", sf_strerror(sndfile));
	}

	for (int i=0;i<1<<16;i++) {
		if (sf_writef_double(sndfile, buffer, n_frames) != n_frames) {
			fprintf(stderr, "Write failed\n");
			sf_close(sndfile);
			FAIL_TEST("write failed");
		}
	}

	sf_write_sync(sndfile);
	sf_close(sndfile);
	g_free(buffer);
}
