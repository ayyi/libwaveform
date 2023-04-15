/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2024 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 | libwaveform unit tests
 |
 */

#define __wf_private__
#define __no_setup__

#include "config.h"
#include <glib.h>
#include "decoder/ad.h"
#include "transition/transition.h"
#include "wf/waveform.h"
#include "wf/peakgen.h"
#include "wf/worker.h"
#include "waveform/pixbuf.h"
#include "test/utils.h"
#include "test/runner.h"

TestFn test_decoder, test_peakgen, test_m4a, test_bad_wav, test_empty_wav, test_audio_file, test_audiodata, test_audio_cache, test_alphabuf, test_transition, test_worker, test_thumbnail;

static void finalize_notify (gpointer, GObject*);

gpointer tests[] = {
	test_decoder,
	test_peakgen,
	test_m4a,
	test_bad_wav,
	test_empty_wav,
	test_audio_file,
	test_audiodata,
	test_audio_cache,
	test_alphabuf,
	test_transition,
	test_worker,
#ifdef USE_FFMPEG
	test_thumbnail,
#endif
};

#include "test/common.c"

#define WAV "mono_0:10.wav"
#define WAV2 "stereo_0:10.wav"


void
test_decoder ()
{
	START_TEST;

	{
		WfDecoder f = {{0,}};

		g_autofree char* filename = find_wav(WAV2);
		assert(ad_open(&f, filename), "failed to open");

		assert(f.info.channels = 2, "channels");
	}

	{
		WfDecoder f = {{0,}};

		g_autofree char* filename = find_wav("stereo_0:10.m4a");
		assert(ad_open(&f, filename), "failed to open");

		assert(f.info.channels = 2, "channels");
	}

	FINISH_TEST;
}


void
test_peakgen ()
{
	START_TEST;

	char* filename = find_wav(WAV);
	assert(filename, "cannot find file %s", WAV);

	// create local peakfile
	{
		if (!wf_peakgen__sync(filename, WAV ".peak", NULL)) {
			FAIL_TEST("local peakgen failed");
		}
		gsize length;
		g_autofree gchar* contents;
		g_file_get_contents (WAV ".peak", &contents, &length, NULL);
		assert(length == 6970, "peakfile size %i", (int)length);
	}

	// create peakfile in the cache directory
	{
		Waveform* w = waveform_new(filename);

		char* p = waveform_ensure_peakfile__sync(w);
		assert(p, "cache dir peakgen failed");
		g_object_unref(w);
		g_free(p);
	}
	g_free(filename);

	FINISH_TEST;
}


void
test_m4a ()
{
	START_TEST;

	#define M4A "mono_0:10.m4a"

	char* filename = find_wav(M4A);
	assert(filename, "cannot find file %s", M4A);

	assert(wf_peakgen__sync(filename, M4A ".peak", NULL), "peakgen failed");

	Waveform* w = waveform_new(filename);
	g_free(filename);

	char* p = waveform_ensure_peakfile__sync(w);
	assert(p, "cache dir peakgen failed");

	assert(waveform_load_sync(w), "failed to load");
	assert(w->n_frames == 441000 || w->n_frames == 442058, "wrong frame count %"PRIi64, w->n_frames);
	assert(w->priv->peak.size >= 3440 && w->priv->peak.size <= 3446, "wrong peak count %i", w->priv->peak.size);

	g_object_unref(w);
	g_free(p);

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
	if (__test_idx == -1) printf("\n"); // stop compiler warning
	static Waveform* w;

	bool a = wf_peakgen__sync("bad.wav", "bad.peak", NULL);
	assert(!a, "peakgen was expected to fail");

	GError* error = NULL;
	a = wf_peakgen__sync("bad.wav", "bad.peak", &error);
	assert(error, "expected error");
	g_error_free(error);

	void callback (Waveform* w, GError* error, gpointer _c)
	{
		PF;
		WfTest* c = _c;

		assert(error, "GError not set")
		g_object_unref(w);

		WF_TEST_FINISH;
	}

	w = waveform_new(NULL);
	waveform_set_file(w, "bad.wav");
	waveform_load(w, callback,
		WF_NEW(C1,
			.test = {
				.test_idx = TEST.current.test,
			}
		)
	);
}


/*
 *  Test valid wav file with length 0
 */
void
test_empty_wav ()
{
	START_TEST;
	if (__test_idx);

	char* filename = find_wav ("mono_0:00.wav");
	static Waveform* w; w = waveform_new (filename);
	g_free (filename);

	void callback (Waveform* w, GError* error, gpointer _c)
	{
		WfTest* c = _c;

		assert(error, "expected GError")
		g_object_unref(w);

		WF_TEST_FINISH;
	}

	waveform_load (w, callback,
		WF_NEW(C1,
			.test = {
				.test_idx = TEST.current.test,
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

	char* filenames[] = {
		"mono_0:10.wav", "stereo_0:10.wav",
#ifdef USE_FFMPEG
		"mono_0:10.mp3", "stereo_0:10.mp3",
		"mono_0:10.m4a", "stereo_0:10.m4a",
		"mono_0:10.opus", "stereo_0:10.opus",
#endif
		"mono_24b_0:10.wav", "stereo_24b_0:10.wav"
	};

	for (int i=0;i<G_N_ELEMENTS(filenames);i++) {
		WfDecoder f = {{0,}};
		char* filename = find_wav(filenames[i]);
		if (!ad_open(&f, filename)) FAIL_TEST("file open: %s", filenames[i]);

		if (!g_str_has_suffix(filenames[i], ".opus")) {
			assert(f.info.sample_rate == 44100, "samplerate: %i (expected 44100)", f.info.sample_rate);
		} else {
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
		dbg(1, "diff=%i", abs((int)total - (int)f.info.frames));

		if (g_str_has_suffix(filenames[i], ".wav") || g_str_has_suffix(filenames[i], ".flac")) {
			assert(total == f.info.frames, "%s: incorrect number of frames read: %"PRIi64" (expected %"PRIi64")", filenames[i], total, f.info.frames);
			assert(!(total % 512) || !(total % 100), "%s: bad framecount: %zu", filenames[i], total); // test file sizes are always a round number
		} else {
			// for some file types, f.info.frames is only an estimate
			assert(abs((int)total - (int)f.info.frames) < 2048, "%s: incorrect number of frames read: %"PRIi64" (expected %"PRIi64")", filenames[i], total, f.info.frames);
		}

		ad_free_nfo(&f.info);
		ad_close(&f);
		g_free(filename);
	}

	FINISH_TEST;
}


void
test_audiodata ()
{
	START_TEST;
	if (__test_idx);

	C1* c = WF_NEW(C1,
		.test = {
			.test_idx = TEST.current.test,
		}
	);

	void _on_peakdata_ready (Waveform* waveform, int block, gpointer _c)
	{
		WfTest* c = _c;
		C1* c1 = _c;

		if (wf_debug) printf("\n");
		dbg(1, "block=%i", block);
		test_reset_timeout(5000);

		if (++c1->n >= c1->tot_blocks) {
			g_signal_handler_disconnect((gpointer)waveform, c1->ready_handler);
			c1->ready_handler = 0;
			g_object_unref(waveform);
			WF_TEST_FINISH;
		}
	}

	char* filename = find_wav(WAV);
	Waveform* w = waveform_new(filename);
	g_free(filename);

	c->tot_blocks = waveform_get_n_audio_blocks(w);

	static int n_tiers_needed = 3;//4;

	g_object_weak_ref((GObject*)w, finalize_notify, NULL);

	c->ready_handler = g_signal_connect (w, "hires-ready", (GCallback)_on_peakdata_ready, c);

	for (int b=0;b<c->tot_blocks;b++) {
		waveform_load_audio(w, b, n_tiers_needed, NULL, NULL);
	}
}


	static int tot_blocks;
	static int n_tiers_needed = 4;
	static guint ready_handler = 0;

	void _slow_on_peakdata_ready (Waveform* waveform, int block, gpointer _c)
	{
		WfTest* c = _c;
		C1* c1 = _c;

		if(wf_debug) printf("\n");
		dbg(1, "block=%i", block);
		test_reset_timeout(5000);

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
	if (__test_idx);

#if 0
	C1* c = WF_NEW(C1,
		.test = {
			.test_idx = TEST.current.test,
		}
	);
#endif

	Waveform* w = waveform_new(WAV);

	tot_blocks = waveform_get_n_audio_blocks(w);

	ready_handler = g_signal_connect (w, "hires-ready", (GCallback)_slow_on_peakdata_ready, NULL);

	waveform_load_audio(w, 0, n_tiers_needed, NULL, NULL);
}


static gboolean
test_audio_cache__after_unref (gpointer _c)
{
	WfTest* c = _c;
	WF* wf = wf_get_instance();

	assert_and_stop(!wf->audio.mem_size, "cache memory not zero");

	WF_TEST_FINISH_TIMER_STOP;
}

	static void _on_peakdata_ready_3 (Waveform* waveform, int block, gpointer _c)
	{
		C1* c = _c;

		dbg(1, "block=%i", block);
		test_reset_timeout(5000);

		c->n++;
		if (c->n >= c->tot_blocks) {
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
	if (__test_idx);

	WF* wf = wf_get_instance();
	assert(!wf->audio.mem_size, "cache memory not zero");

	char* filename = find_wav(WAV);
	Waveform* w = waveform_new(filename);
	g_free(filename);

	int tot_blocks = MIN(20, waveform_get_n_audio_blocks(w)); // cannot do too many parallel requests as the cache will fill.
	int n_tiers_needed = 3;//4;

	g_object_weak_ref((GObject*)w, finalize_notify, NULL);

	ready_handler = g_signal_connect (w, "hires-ready", (GCallback)_on_peakdata_ready_3, WF_NEW(C1,
		.tot_blocks = tot_blocks,
		.test = {
			.test_idx = TEST.current.test,
		}
	));

	for (int b=0;b<tot_blocks;b++) {
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
	for (int b=0;b<2;b++) {
		for(int s=0;s<G_N_ELEMENTS(scale);s++){
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
test_transition ()
{
	START_TEST;

	static int64_t value = 0;
	static int64_t iter_val = -1;

	WfAnimatable* animatable = WF_NEW(WfAnimatable,
		.target_val.b = 1000,
		.start_val.b  = 0,
		.val.b        = &value,
		.type         = WF_INT64
	);

	void done (WfAnimation* animation, gpointer _)
	{
		GList* members = animation->members;
		WfAnimActor* actor = members->data;
		GList* transitions = actor->transitions;
		WfAnimatable* animatable = transitions->data;

		assert(value == animatable->target_val.b, "final value");
		g_free(animatable);
		FINISH_TEST;
	}

	void on_frame (WfAnimation* animation, int time)
	{
		assert(value > iter_val, "not incremented");
		iter_val = value;
	}

	GList* animatables = g_list_prepend(NULL, animatable);

	WfAnimation* animation = wf_animation_new(done, NULL);
	animation->on_frame = on_frame;
	wf_transition_add_member(animation, animatables);

	wf_animation_start(animation);
}


void
test_worker ()
{
	// run jobs in two worker threads with activity in main thread.

	START_TEST;

	static Waveform* w[2];
	w[0] = waveform_new (WAV);
	w[1] = waveform_new (WAV2);

	#define n_hashes 250000
	#define n_workers 2

	static int n_workers_done = 0;
	static int W = -1;

	static int i_max = 15;

	typedef struct {
		WfWorker worker;
		int      i_started;
		int      i_done;
	} WW;
	static WW ww[2] = {{{0,}},};

	void _test_worker ()
	{
		WW* _ww = &ww[++W];
		wf_worker_init(&_ww->worker);

		typedef struct {
			gpointer buffer;
			WW*      ww;
		} C;

		void work (Waveform* w, gpointer _c)
		{
			C* c = _c;

			fflush(stdout); printf("starting job %i of %i...\n", ++c->ww->i_started, i_max);

			uint64_t t0 = g_get_monotonic_time();

			int j = 0;
			for (int i=0;i<n_hashes;i++) {
				GFile* file = g_file_new_for_path(WAV);
				j++;
				g_file_hash(file);
				g_object_unref(file);

				if (g_get_monotonic_time() - t0 > 800000) break;
			}
		}

		void work_done (Waveform* waveform, GError* error, gpointer _c)
		{
			C* c = _c;

			dbg(0, "%i", c->ww->i_done + 1);
			if (++c->ww->i_done == i_max || !w[0]) {
				gboolean __finish (gpointer _)
				{
					dbg(0, "all jobs done");
					if (++n_workers_done == n_workers) {
						FINISH_TEST_TIMER_STOP;
					}
					return G_SOURCE_REMOVE;
				}
				g_idle_add(__finish, NULL);
			}

			test_reset_timeout(10000);
		}

		void c_free (gpointer _c)
		{
			C* c = _c;
			if (c) {
				g_free(c->buffer);
				g_free(c);
			}
		}

		gboolean add_job (gpointer _ww)
		{
			WW* ww = _ww;

			if (w[0]) {
				wf_worker_push_job(&ww->worker, w[ww->i_started % 2], work, work_done, c_free, WF_NEW(C,
					.buffer = g_malloc0(1000),
					.ww = ww
				));
			}
			return G_SOURCE_REMOVE;
		}

		for (int i=0;i<i_max;i++) {
			g_timeout_add(i * get_random_int(200), add_job, _ww);
		}

		static gpointer mem = NULL;

		gboolean main_thread ()
		{
			if (__test_idx != TEST.current.test) return G_SOURCE_REMOVE;

			if (mem) g_free(mem);
			mem = g_malloc(get_random_int(1024 * 1024));

			return G_SOURCE_CONTINUE;
		}
		g_timeout_add(250, main_thread, NULL);
	}

	gboolean add_worker (gpointer _)
	{
		_test_worker();
		return G_SOURCE_REMOVE;
	}
	for (int i=0;i<n_workers;i++) g_timeout_add(i * 5000, add_worker, NULL);

	// eventually, unref the waveform, which will cause the jobs to be cancelled and the test to end

	gboolean unref (gpointer _w)
	{
		gboolean stop (gpointer _)
		{
			// in case the test has not already stopped.
			FINISH_TEST_TIMER_STOP;
		}

		waveform_unref0(w[0]);
		waveform_unref0(w[1]);
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
	ad_free_nfo(&dec.info);

	g_free(filename);
	FINISH_TEST;
}


static void
finalize_notify (gpointer data, GObject* was)
{
	dbg(1, "...");
}

