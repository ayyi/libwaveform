/*

  Low level tests for libwaveform

  --------------------------------------------------------------

  Copyright (C) 2012 Tim Orford <tim@orford.org>

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
#define __ayyi_private__
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
#include <signal.h>
#include <glib.h>
#include "waveform/waveform.h"
#include "waveform/peakgen.h"
#include "test/ayyi_utils.h"
#include "test/common.h"

typedef void (TestFn)();
TestFn test_peakgen, test_audiodata, test_audio_cache, test_alphabuf;

#define STOP false;
gboolean   abort_on_fail  = true;
gboolean   passed         = false;
int        test_finished  = false;  //current test has finished. Go onto the next test.
char       current_test_name[64];

gpointer   tests[] = {
	test_peakgen,
	test_audiodata,
	test_audio_cache,
	test_alphabuf,
};


#define WAV "test/data/mono_1.wav"

int
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%i\n", sizeof(off_t)); exit(1); }

	test_init(tests, G_N_ELEMENTS(tests));

	gboolean fn(gpointer user_data)
	{
		next_test();

		return IDLE_STOP;
	}
	g_idle_add(fn, NULL);
	g_main_loop_run (g_main_loop_new (NULL, 0));

	exit(1);
}


static void reset_timeout(int ms)
{
	if(app.timeout) g_source_remove (app.timeout);

	bool on_test_timeout(gpointer _user_data)
	{
		FAIL_TEST_TIMER("TEST TIMEOUT\n");
		return TIMER_STOP;
	}
	app.timeout = g_timeout_add(ms, on_test_timeout, NULL);
}


void
test_peakgen()
{
	START_TEST;

	if(!wf_peakgen(WAV, WAV ".peak")){
		FAIL_TEST("local peakgen failed");
	}

	//create peakfile in the cache directory
	Waveform* w = waveform_new(WAV);
	char* p = waveform_ensure_peakfile(w);
	assert(p, "cache dir peakgen failed");

	FINISH_TEST;
}


void
test_audiodata()
{
	START_TEST;

	Waveform* w = waveform_new(WAV);

	static int tot_blocks; tot_blocks = waveform_get_n_audio_blocks(w);
																			tot_blocks = 3;
	static int n = 0;
	static int n_tiers_needed = 3;//4;
	static guint ready_handler = 0;

	void finalize_notify(gpointer data, GObject* was)
	{
		dbg(0, "!");
	}
	g_object_weak_ref((GObject*)w, finalize_notify, NULL);

	void _on_peakdata_ready(Waveform* waveform, int block, gpointer data)
	{
		printf("\n");
		dbg(0, "block=%i", block);
		reset_timeout(5000);

		n++;
		if(n >= tot_blocks){
			g_signal_handler_disconnect((gpointer)waveform, ready_handler);
			ready_handler = 0;
			g_object_unref(waveform);
			FINISH_TEST;
		}
	}
	ready_handler = g_signal_connect (w, "peakdata-ready", (GCallback)_on_peakdata_ready, NULL);

	int b; for(b=0;b<tot_blocks;b++){
		waveform_load_audio_async(w, b, n_tiers_needed);
	}
}


void
test_audiodata_slow()
{
	//queues the requests separately.

	START_TEST;

	Waveform* w = waveform_new(WAV);

	static int tot_blocks; tot_blocks = waveform_get_n_audio_blocks(w);
	static int n = 0;
	static int n_tiers_needed = 4;
	static guint ready_handler = 0;

	void _on_peakdata_ready(Waveform* waveform, int block, gpointer data)
	{
		printf("\n");
		dbg(0, "block=%i", block);
		reset_timeout(5000);

		n++;
		if(n < tot_blocks){
			waveform_load_audio_async(waveform, n, n_tiers_needed);
		}else{
			g_signal_handler_disconnect((gpointer)waveform, ready_handler);
			g_object_unref(waveform);
			FINISH_TEST;
		}
	}
	ready_handler = g_signal_connect (w, "peakdata-ready", (GCallback)_on_peakdata_ready, NULL);

	waveform_load_audio_async(w, 0, n_tiers_needed);
}


void
test_audio_cache()
{
	//test that the cache is empty once Waveform's have been destroyed.

	START_TEST;

	Waveform* w = waveform_new(WAV);

	static int tot_blocks; tot_blocks = MIN(20, waveform_get_n_audio_blocks(w)); //cannot do too many parallel requests as the cache will fill.
	static int n = 0;
	static int n_tiers_needed = 3;//4;
	static guint ready_handler = 0;

	void finalize_notify(gpointer data, GObject* was)
	{
		dbg(0, "...");
	}
	g_object_weak_ref((GObject*)w, finalize_notify, NULL);

	void _on_peakdata_ready(Waveform* waveform, int block, gpointer data)
	{
		dbg(1, "block=%i", block);
		reset_timeout(5000);

		n++;
		if(n >= tot_blocks){
			g_signal_handler_disconnect((gpointer)waveform, ready_handler);
			ready_handler = 0;
			g_object_unref(waveform);

			gboolean after_unref(gpointer data)
			{
				WF* wf = wf_get_instance();

				assert_and_stop(!wf->audio.mem_size, "cache memory not zero");

				FINISH_TEST_TIMER_STOP;
			}
			// dont know why, but the idle fn runs too early.
			//g_idle_add_full(G_PRIORITY_LOW, after_unref, NULL, NULL);
			g_timeout_add(400, after_unref, NULL);
		}
	}
	ready_handler = g_signal_connect (w, "peakdata-ready", (GCallback)_on_peakdata_ready, NULL);

	int b; for(b=0;b<tot_blocks;b++){
		waveform_load_audio_async(w, b, n_tiers_needed);
	}
}


void
test_alphabuf()
{
	START_TEST;

	Waveform* w = waveform_load_new(WAV);

	int scale[] = {1, WF_PEAK_STD_TO_LO};
	int b; for(b=0;b<2;b++){
		int s; for(s=0;s<G_N_ELEMENTS(scale);s++){
			AlphaBuf* alphabuf = wf_alphabuf_new(w, b, scale[s], false);
			assert(alphabuf, "alphabuf create failed");
			assert(alphabuf->buf, "alphabuf has no buf");
			assert(alphabuf->buf_size, "alphabuf size not set");
			wf_alphabuf_free(alphabuf);
		}
	}

	g_object_unref(w);
	FINISH_TEST;
}


