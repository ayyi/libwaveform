/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform https://github.com/ayyi/libwaveform |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __common_c__
#define __wf_private__
#include "config.h"
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#include <glib-object.h>
#include <sndfile.h>
#include "agl/actor.h"
#include "wf/private.h"
#include "waveform/utils.h"
#include "test/common.h"

int  n_failed      = 0;
int  n_passed      = 0;
bool abort_on_fail = true;
bool passed        = false;
int  test_finished = false;  // current test has finished. Go onto the next test.
int  current_test  = -1;

extern char     current_test_name[];
extern gpointer tests[];

static int __n_tests = 0;


static gboolean fn(gpointer user_data) { next_test(); return G_SOURCE_REMOVE; }


void
test_init (gpointer tests[], int n_tests)
{
	__n_tests = n_tests;
	dbg(2, "n_tests=%i", __n_tests);

	set_log_handlers();

	g_idle_add(fn, NULL);
}


static gboolean
run_test (gpointer test)
{
	((Test)test)();
	return G_SOURCE_REMOVE;
}


static gboolean
__exit ()
{
	exit(n_failed ? EXIT_FAILURE : EXIT_SUCCESS);
	return G_SOURCE_REMOVE;
}


static gboolean
on_test_timeout (gpointer _user_data)
{
	FAIL_TEST_TIMER("TEST TIMEOUT\n");
	return G_SOURCE_REMOVE;
}


void
next_test ()
{
	printf("\n");
	current_test++;
	if(app.timeout) g_source_remove (app.timeout);
	if(current_test < __n_tests){
		test_finished = false;
		gboolean (*test)() = tests[current_test];
		dbg(2, "test %i of %i.", current_test + 1, __n_tests);
		g_timeout_add(300, run_test, test);

		app.timeout = g_timeout_add(20000, on_test_timeout, NULL);
	}
	else{ printf("finished all. passed=%s %i %s failed=%s %i %s\n", green, app.n_passed, ayyi_white, (n_failed ? red : ayyi_white), n_failed, ayyi_white); g_timeout_add(1000, __exit, NULL); }
}


void
test_finished_ ()
{
	dbg(2, "... passed=%i", passed);
	if(passed) app.n_passed++; else n_failed++;
	//log_print(passed ? LOG_OK : LOG_FAIL, "%s", current_test_name);
	if(!passed && abort_on_fail) current_test = 1000;
	next_test();
}


WfTest*
wf_test_new ()
{
	static WfTest* t = NULL;
	//if(t) g_free0(t); // valgrind says 'invalid free'

	return t = WF_NEW(WfTest, .test_idx = current_test);
}


static gboolean
on_test_timeout_ (gpointer _user_data)
{
	FAIL_TEST_TIMER("TEST TIMEOUT\n");
	return G_SOURCE_REMOVE;
}


void
reset_timeout (int ms)
{
	if(app.timeout) g_source_remove (app.timeout);

	app.timeout = g_timeout_add(ms, on_test_timeout_, NULL);
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

