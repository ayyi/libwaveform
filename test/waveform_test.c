/*

  Low level tests for libwaveform

  --------------------------------------------------------------

  Copyright (C) 2012-2015 Tim Orford <tim@orford.org>

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
#include "waveform/alphabuf.h"
#include "waveform/worker.h"
#include "test/ayyi_utils.h"
#include "test/common.h"

TestFn test_peakgen, test_audiodata, test_audio_cache, test_alphabuf, test_worker;

gpointer tests[] = {
	test_peakgen,
	test_audiodata,
	test_audio_cache,
	test_alphabuf,
	test_worker,
};

#define WAV "test/data/mono_0:10.wav"


int
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%i\n", sizeof(off_t)); exit(1); }

	test_init(tests, G_N_ELEMENTS(tests));

	g_main_loop_run (g_main_loop_new (NULL, 0));

	exit(1);
}


void
test_peakgen()
{
	START_TEST;

	if(!wf_peakgen__sync(WAV, WAV ".peak")){
		FAIL_TEST("local peakgen failed");
	}

	//create peakfile in the cache directory
	Waveform* w = waveform_new(WAV);
	char* p = waveform_ensure_peakfile__sync(w);
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
	ready_handler = g_signal_connect (w, "hires-ready", (GCallback)_on_peakdata_ready, NULL);

	int b; for(b=0;b<tot_blocks;b++){
		waveform_load_audio(w, b, n_tiers_needed, NULL, NULL);
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
			waveform_load_audio(waveform, n, n_tiers_needed, NULL, NULL);
		}else{
			g_signal_handler_disconnect((gpointer)waveform, ready_handler);
			g_object_unref(waveform);
			FINISH_TEST;
		}
	}
	ready_handler = g_signal_connect (w, "hires-ready", (GCallback)_on_peakdata_ready, NULL);

	waveform_load_audio(w, 0, n_tiers_needed, NULL, NULL);
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
	ready_handler = g_signal_connect (w, "hires-ready", (GCallback)_on_peakdata_ready, NULL);

	int b; for(b=0;b<tot_blocks;b++){
		waveform_load_audio(w, b, n_tiers_needed, NULL, NULL);
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
			AlphaBuf* alphabuf = wf_alphabuf_new(w, b, scale[s], false, 0);
			assert(alphabuf, "alphabuf create failed");
			assert(alphabuf->buf, "alphabuf has no buf");
			assert(alphabuf->buf_size, "alphabuf size not set");
			wf_alphabuf_free(alphabuf);
		}
	}

	g_object_unref(w);
	FINISH_TEST;
}


void
test_worker()
{
	// run jobs in two worker threads with activity in main thread.

	// TODO test with multiple waveforms

	START_TEST;

	static Waveform* w; w = waveform_new(WAV);

	static int n_workers = 2;
	static int n_workers_done = 0;
	static int W = -1;

	static int i_max = 20;

	typedef struct {
		WfWorker worker;
		int      i_done;
	} WW;
	static WW ww[2] = {{{0,},0,}, {{0,},0,}};

	void _test_worker()
	{
		WW* _ww = &ww[++W];
		wf_worker_init(&_ww->worker);

		typedef struct {
			gpointer buffer;
			WW*      ww;
		} C;

		void work(Waveform* w, gpointer _c)
		{
			C* c = _c;

			fflush(stdout); printf("starting job %i of %i...\n", c->ww->i_done + 1, i_max);

			int j = 0;
			int i; for(i=0;i<500000;i++){
				GFile* file = g_file_new_for_path(WAV);
				j++;
				g_file_hash(file);
				g_object_unref(file);
			}
		}

		void work_done(Waveform* waveform, gpointer _c)
		{
			C* c = _c;

			dbg(0, "%i", c->ww->i_done + 1);
			if(++c->ww->i_done == i_max || !w){
				bool __finish(gpointer _)
				{
					dbg(0, "all jobs done");
					if(++n_workers_done == n_workers){
						FINISH_TEST_TIMER_STOP;
					}
					return G_SOURCE_REMOVE;
				}
				g_idle_add(__finish, NULL);
			}

			reset_timeout(10000);
		}

		void c_free(gpointer _c)
		{
			C* c = _c;
			if(c){
				g_free(c->buffer);
				g_free(c);
			}
		}

		bool add_job(gpointer _ww)
		{
			WW* ww = _ww;

			if(w){
				C* c = g_new0(C, 1);
				c->buffer = g_malloc0(1000);
				c->ww = ww;
				wf_worker_push_job(&ww->worker, w, work, work_done, c_free, c);
			}
			return G_SOURCE_REMOVE;
		}

		int i; for(i=0;i<i_max;i++){
			g_timeout_add(i * get_random_int(2000), add_job, _ww);
		}

		static gpointer mem = NULL;
		bool main_thread()
		{
			if(__test_idx != current_test) return G_SOURCE_REMOVE;

			if(mem) g_free(mem);
			mem = g_malloc(get_random_int(1024 * 1024));
			return TIMER_CONTINUE;
		}
		g_timeout_add(250, main_thread, NULL);
	}

	bool add_worker(gpointer _)
	{
		_test_worker();
		return G_SOURCE_REMOVE;
	}
	int i; for(i=0;i<n_workers;i++) g_timeout_add(i * 5000, add_worker, NULL);

	// eventually, unref the waveform, which will cause the jobs to be cancelled and the test to end

	bool unref(gpointer _w)
	{
		bool stop(gpointer _)
		{
			// in case the test has not already stopped.
			FINISH_TEST_TIMER_STOP;
		}

		dbg(0, "");
		waveform_unref0(w);
		g_timeout_add(15000, stop, NULL);
		return G_SOURCE_REMOVE;
	}

	g_timeout_add(30000, unref, w);
}


