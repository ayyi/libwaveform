/*
  This file is part of the Ayyi Project. http://ayyi.org
  copyright (C) 2004-2014 Tim Orford <tim@orford.org>

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
#define __common_c__
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include <sndfile.h>
#include "waveform/utils.h"
#include "waveform/wf_private.h"
#include "test/ayyi_utils.h"
#include "test/common.h"

int      n_failed = 0;
int      n_passed = 0;
gboolean abort_on_fail  = true;
gboolean passed         = false;
int      test_finished  = false;  //current test has finished. Go onto the next test.
int      current_test = -1;

extern char     current_test_name[];
extern gpointer tests[];

static int __n_tests = 0;

//TODO is dupe
#define FAIL_TEST_TIMER(msg) \
	{test_finished = true; \
	passed = false; \
	printf("%s%s%s\n", red, msg, wf_white); \
	test_finished_(); \
	return TIMER_STOP;}
//------------------


void
test_init(gpointer tests[], int n_tests)
{
	__n_tests = n_tests;
	dbg(2, "n_tests=%i", __n_tests);

	memset(&app, 0, sizeof(struct _app));

	set_log_handlers();

	g_type_init();

	gboolean fn(gpointer user_data) { next_test(); return IDLE_STOP; }
	g_idle_add(fn, NULL);
}


void
next_test()
{
	bool
	run_test(gpointer test)
	{
		((Test)test)();
		return TIMER_STOP;
	}

	bool _exit()
	{
		exit(EXIT_SUCCESS);
		return TIMER_STOP;
	}

	printf("\n");
	current_test++;
	if(current_test < __n_tests){
		if(app.timeout) g_source_remove (app.timeout);
		test_finished = false;
		gboolean (*test)() = tests[current_test];
		dbg(2, "test %i of %i.", current_test + 1, __n_tests);
		g_timeout_add(300, run_test, test);

		bool on_test_timeout(gpointer _user_data)
		{
			FAIL_TEST_TIMER("TEST TIMEOUT\n");
			return TIMER_STOP;
		}
		app.timeout = g_timeout_add(20000, on_test_timeout, NULL);
	}
	else{ printf("finished all. passed=%s%i%s failed=%s%i%s\n", green, app.n_passed, wf_white, (n_failed ? red : wf_white), n_failed, wf_white); g_timeout_add(1000, _exit, NULL); }
}


void
test_finished_()
{
	dbg(2, "... passed=%i", passed);
	if(passed) app.n_passed++; else n_failed++;
	//log_print(passed ? LOG_OK : LOG_FAIL, "%s", current_test_name);
	if(!passed && abort_on_fail) current_test = 1000;
	next_test();
}


void
add_key_handlers(GtkWindow* window, WaveformView* waveform, Key keys[])
{
	//list of keys must be terminated with a key of value zero.

	static KeyHold key_hold = {0, NULL};
	static bool key_down = false;
	static GHashTable* key_handlers = NULL;

	key_handlers = g_hash_table_new(g_int_hash, g_int_equal);
	int i = 0; while(true){
		Key* key = &keys[i];
		if(i > 100 || !key->key) break;
		g_hash_table_insert(key_handlers, &key->key, key->handler);
		i++;
	}

	gboolean key_hold_on_timeout(gpointer user_data)
	{
		WaveformView* waveform = user_data;
		if(key_hold.handler) key_hold.handler(waveform);
		return TIMER_CONTINUE;
	}

	gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		if(key_down){
			// key repeat
			return true;
		}
		key_down = true;

		KeyHandler* handler = g_hash_table_lookup(key_handlers, &event->keyval);
		if(handler){
			if(key_hold.timer) gwarn("timer already started");
			key_hold.timer = g_timeout_add(100, key_hold_on_timeout, user_data);
			key_hold.handler = handler;
	
			handler(user_data);
		}
		else dbg(1, "%i", event->keyval);

		return true;
	}

	gboolean key_release(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		PF0;
		if(!key_down){ /* gwarn("key_down not set"); */ return true; } //sometimes happens at startup

		key_down = false;
		if(key_hold.timer) g_source_remove(key_hold.timer);
		key_hold.timer = 0;

		return true;
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), waveform);
	g_signal_connect(window, "key-release-event", G_CALLBACK(key_release), waveform);
}


void
reset_timeout(int ms)
{
	if(app.timeout) g_source_remove (app.timeout);

	bool on_test_timeout(gpointer _user_data)
	{
		FAIL_TEST_TIMER("TEST TIMEOUT\n");
		return TIMER_STOP;
	}
	app.timeout = g_timeout_add(ms, on_test_timeout, NULL);
}


gboolean
get_random_boolean()
{
	int r = rand();
	int s = RAND_MAX / 2;
	int t = r / s;
	return t;
}


int
get_random_int(int max)
{
	//printf("R=%i %i\n", RAND_MAX, 1 << sizeof(int));
	//int R = RAND_MAX / 2 + 100000;
	int t;
	//int i; for(i=0;i<10;i++){
	int r = rand();
		int s = RAND_MAX / max;
		t = r / s;
	//	printf("r=%i s=%i t=%i\n", r, s, t);
	//}
   return t;
}


void
errprintf4(char* format, ...)
{
	char str[256];

	va_list argp;           //points to each unnamed arg in turn
	va_start(argp, format); //make ap (arg pointer) point to 1st unnamed arg
	vsprintf(str, format, argp);
	va_end(argp);           //clean up

	printf("%s%s%s\n", red, str, wf_white);
}


void
create_large_file(char* filename)
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


char*
find_wav(const char* wav)
{
	char* filename = g_build_filename(g_get_current_dir(), wav, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	g_free(filename);
	filename = g_build_filename(g_get_current_dir(), "../", wav, NULL);
	return filename;
}


