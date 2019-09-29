/*

  Low level tests for libwaveform

  --------------------------------------------------------------

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
#define __wf_private__
#include "config.h"
#include <getopt.h>
#include <time.h>
#include <inttypes.h>
#include <sys/time.h>
#include <glib.h>
#include "decoder/ad.h"
#include "waveform/waveform.h"
#include "waveform/peakgen.h"
#include "waveform/alphabuf.h"
#include "waveform/worker.h"
#include "test/common.h"

TestFn test_peakgen, test_bad_wav, test_audio_file, test_audiodata, test_audio_cache, test_alphabuf, test_worker, test_thumbnail;

static void finalize_notify(gpointer, GObject*);

gpointer tests[] = {
	test_peakgen,
	test_bad_wav,
	test_audio_file,
	test_audiodata,
	test_audio_cache,
	test_alphabuf,
	test_worker,
	test_thumbnail,
};

#define WAV "mono_0:10.wav"


int
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%zu\n", sizeof(off_t)); exit(1); }

	test_init(tests, G_N_ELEMENTS(tests));

	g_main_loop_run (g_main_loop_new (NULL, 0));

	exit(1);
}


void
test_peakgen ()
{
	START_TEST;

	char* filename = find_wav(WAV);
	assert(filename, "cannot find file %s", WAV);

	if(!wf_peakgen__sync(filename, WAV ".peak", NULL)){
		FAIL_TEST("local peakgen failed");
	}

	// create peakfile in the cache directory
	Waveform* w = waveform_new(filename);
	g_free(filename);
	char* p = waveform_ensure_peakfile__sync(w);
	assert(p, "cache dir peakgen failed");

	FINISH_TEST;
}


typedef struct {
	WfTest test;
	int    n;
	int    tot_blocks;
	guint  ready_handler;
} C1;


/*
 *  Check the load callback gets called if loading fails
 */
void
test_bad_wav ()
{
	START_TEST;
	if(__test_idx == -1) printf("\n"); // stop compiler warning

	bool a = wf_peakgen__sync("bad.wav", "bad.peak", NULL);
	assert(!a, "peakgen was expected to fail");

	GError* error = NULL;
	a = wf_peakgen__sync("bad.wav", "bad.peak", &error);
	assert(error, "expected error");
	g_error_free(error);

	void callback(Waveform* w, GError* error, gpointer _c)
	{
		PF;
		WfTest* c = _c;

		assert(error, "GError not set")

		WF_TEST_FINISH;
	}

	Waveform* w = waveform_new(NULL);
	waveform_set_file(w, "bad.wav");
	waveform_load(w, callback,
		WF_NEW(C1,
			.test = {
				.test_idx = current_test,
			}
		)
	);
}


/*
 *  Test reading of audio files.
 */
void
test_audio_file ()
{
	START_TEST;

	char* filenames[] = {"mono_0:10.wav", "stereo_0:10.wav", "mono_0:10.mp3", "stereo_0:10.mp3", "mono_0:10.m4a", "stereo_0:10.m4a", "mono_0:10.opus", "stereo_0:10.opus", "mono_24b_0:10.wav", "stereo_24b_0:10.wav"};

	int i; for(i=0;i<G_N_ELEMENTS(filenames);i++){
		WfDecoder f = {{0,}};
		char* filename = find_wav(filenames[i]);
		if(!ad_open(&f, filename)) FAIL_TEST("file open: %s", filenames[i]);

		if(!g_str_has_suffix(filenames[i], ".opus")){
			assert(f.info.sample_rate == 44100, "samplerate: %i (expected 44100)", f.info.sample_rate);
		}else{
			assert(f.info.sample_rate == 48000, "samplerate: %i (expected 48000)", f.info.sample_rate);
		}

		int n = 8;
		int read_len = WF_PEAK_RATIO * n;

		int16_t data[f.info.channels][read_len];
		WfBuf16 buf = {
			.buf = {data[0], data[1]},
			.size = n * WF_PEAK_RATIO
		};

		size_t readcount = 0;
		size_t total = 0;
		do {
			readcount = ad_read_short(&f, &buf);
			total += readcount;
		} while (readcount > 0);
		dbg(1, "diff=%zu", abs((int)total - (int)f.info.frames));
		if(g_str_has_suffix(filenames[i], ".wav") || g_str_has_suffix(filenames[i], ".flac")){
			assert(total == f.info.frames, "%s: incorrect number of frames read: %"PRIi64" (expected %"PRIi64")", filenames[i], total, f.info.frames);
			assert(!(total % 512) || !(total % 100), "%s: bad framecount: %zu", filenames[i], total); // test file sizes are always a round number
		}else{
			// for some file types, f.info.frames is only an estimate
			assert(abs((int)total - (int)f.info.frames) < 2048, "%s: incorrect number of frames read: %"PRIi64" (expected %"PRIi64")", filenames[i], total, f.info.frames);
		}

		ad_close(&f);
		g_free(filename);
	}

	FINISH_TEST;
}


	static void _on_peakdata_ready(Waveform* waveform, int block, gpointer _c)
	{
		WfTest* c = _c;
		C1* c1 = _c;

		if(wf_debug) printf("\n");
		dbg(1, "block=%i", block);
		reset_timeout(5000);

		if(++c1->n >= c1->tot_blocks){
			g_signal_handler_disconnect((gpointer)waveform, c1->ready_handler);
			c1->ready_handler = 0;
			g_object_unref(waveform);
			WF_TEST_FINISH;
		}
	}

void
test_audiodata ()
{
	START_TEST;
	if(__test_idx);

	C1* c = WF_NEW(C1,
		.test = {
			.test_idx = current_test,
		}
	);

	char* filename = find_wav(WAV);
	Waveform* w = waveform_new(filename);
	g_free(filename);

	c->tot_blocks = waveform_get_n_audio_blocks(w);

	static int n_tiers_needed = 3;//4;

	g_object_weak_ref((GObject*)w, finalize_notify, NULL);

	c->ready_handler = g_signal_connect (w, "hires-ready", (GCallback)_on_peakdata_ready, c);

	int b; for(b=0;b<c->tot_blocks;b++){
		waveform_load_audio(w, b, n_tiers_needed, NULL, NULL);
	}
}


	static int tot_blocks;
	static int n_tiers_needed = 4;
	static guint ready_handler = 0;

	void _slow_on_peakdata_ready(Waveform* waveform, int block, gpointer _c)
	{
		WfTest* c = _c;
		C1* c1 = _c;

		if(wf_debug) printf("\n");
		dbg(1, "block=%i", block);
		reset_timeout(5000);

		c1->n++;
		if(c1->n < tot_blocks){
			waveform_load_audio(waveform, c1->n, n_tiers_needed, NULL, NULL);
		}else{
			g_signal_handler_disconnect((gpointer)waveform, ready_handler);
			g_object_unref(waveform);
			WF_TEST_FINISH;
		}
	}

void
test_audiodata_slow ()
{
	// queues the requests separately.

	START_TEST;
	if(__test_idx);

#if 0
	C1* c = WF_NEW(C1,
		.test = {
			.test_idx = current_test,
		}
	);
#endif

	Waveform* w = waveform_new(WAV);

	tot_blocks = waveform_get_n_audio_blocks(w);

	ready_handler = g_signal_connect (w, "hires-ready", (GCallback)_slow_on_peakdata_ready, NULL);

	waveform_load_audio(w, 0, n_tiers_needed, NULL, NULL);
}


static bool
test_audio_cache__after_unref (gpointer _c)
{
	WfTest* c = _c;
	WF* wf = wf_get_instance();

	assert_and_stop(!wf->audio.mem_size, "cache memory not zero");

	WF_TEST_FINISH_TIMER_STOP;
}

	static void _on_peakdata_ready_3(Waveform* waveform, int block, gpointer _c)
	{
		C1* c = _c;

		dbg(1, "block=%i", block);
		reset_timeout(5000);

		c->n++;
		if(c->n >= tot_blocks){
			g_signal_handler_disconnect((gpointer)waveform, ready_handler);
			ready_handler = 0;
			g_object_unref(waveform);

			// dont know why, but the idle fn runs too early.
			//g_idle_add_full(G_PRIORITY_LOW, test_audio_cache__after_unref, NULL, NULL);
			g_timeout_add(400, test_audio_cache__after_unref, c);
		}
	}

void
test_audio_cache ()
{
	// test that the cache is empty once Waveform's have been destroyed.

	START_TEST;
	if(__test_idx);

	C1* c = WF_NEW(C1,
		.test = {
			.test_idx = current_test,
		}
	);

	WF* wf = wf_get_instance();
	assert(!wf->audio.mem_size, "cache memory not zero");

	char* filename = find_wav(WAV);
	Waveform* w = waveform_new(filename);
	g_free(filename);

	static int tot_blocks; tot_blocks = MIN(20, waveform_get_n_audio_blocks(w)); //cannot do too many parallel requests as the cache will fill.
	static int n_tiers_needed = 3;//4;

	g_object_weak_ref((GObject*)w, finalize_notify, NULL);

	ready_handler = g_signal_connect (w, "hires-ready", (GCallback)_on_peakdata_ready_3, c);

	int b; for(b=0;b<tot_blocks;b++){
		waveform_load_audio(w, b, n_tiers_needed, NULL, NULL);
	}
}


void
test_alphabuf ()
{
	START_TEST;

	char* filename = find_wav(WAV);
	Waveform* w = waveform_load_new(filename);
	g_free(filename);

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
test_worker ()
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

		void work_done(Waveform* waveform, GError* error, gpointer _c)
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

		waveform_unref0(w);
		g_timeout_add(15000, stop, NULL);
		return G_SOURCE_REMOVE;
	}

	g_timeout_add(30000, unref, w);
}


void
test_thumbnail ()
{
	START_TEST;

	char* filename = find_wav("thumbnail.mp3");

	WfDecoder dec = {0,};
	if(!ad_open(&dec, filename)){
		FAIL_TEST("failed to open file");
	}

	AdPicture picture;
	ad_thumbnail(&dec, &picture);

	assert(picture.width, "width");
	assert(picture.height, "height");
	assert(picture.row_stride, "rowstride");
	assert(picture.data, "data");

	ad_close(&dec);
	ad_thumbnail_free(NULL, &picture);

	g_free(filename);
	FINISH_TEST;
}


static void
finalize_notify (gpointer data, GObject* was)
{
	dbg(1, "...");
}

