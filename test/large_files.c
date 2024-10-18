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
 | libwaveform large files test
 |
 | This is a non-interactive test.
 |
 */

#define __wf_private__

#include "config.h"
#include <getopt.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <sndfile.h>
#include <agl/utils.h>
#include "wf/audio.h"
#include "wf/peakgen.h"
#include "test/utils.h"
#include "test/wf_runner.h"
#include "test/large_files.h"

SetupFn create_large_files;
TeardownFn delete_large_files;

#define WAV1 "test/data/large1.wav"
#define WAV2 "test/data/large2.wav"


int
create_large_files ()
{
	// test_reset_timeout(60000);

	create_large_file(WAV1);
	create_large_file(WAV2);

	return 0;
}


int
setup ()
{
	return create_large_files();
}


void
delete_large_files ()
{
	START_TEST;

	if (g_unlink(WAV1)) {
		FAIL_TEST("delete failed");
	}
	assert(!g_unlink(WAV2), "delete failed");

	FINISH_TEST;
}


void
teardown()
{
	delete_large_files();
}


void
test_load ()
{
	// Test that the large files are loaded and unloaded properly.

	START_TEST;

	static char* wavs[] = {WAV1, WAV2};
	static int wi; wi = 0;
	static int iter; iter = 0;

	typedef struct C {
		void (*next)(struct C*);
	} C;

	void finalize_notify (gpointer data, GObject* was)
	{
		dbg(0, "...");
	}

	void next_wav (C* c)
	{
		if (wi >= G_N_ELEMENTS(wavs)) {
			if (iter++ < 2) {
				wi = 0;
			} else {
				g_free(c);
				FINISH_TEST;
			}
		}

		dbg(0, "==========================================================");
		test_reset_timeout(40000);

		Waveform* w = waveform_new(wavs[wi++]);
		g_object_weak_ref((GObject*)w, finalize_notify, NULL);
		waveform_load_sync(w);

#if 0 // due to internal changes, these tests are no longer valid
		WfGlBlock* blocks = (WfGlBlock*)w->priv->render_data[MODE_MED];
		assert(blocks, "texture container not allocated");
		assert(!blocks->peak_texture[WF_LEFT].main[0], "textures allocated"); // no textures are expected to be allocated.
		assert(!blocks->peak_texture[WF_RIGHT].main[0], "textures allocated");
#endif
		assert(&w->priv->peak, "peak not loaded");
		assert(w->priv->peak.size, "peak size not set");
		assert(w->priv->peak.buf[WF_LEFT], "peak not loaded");
		assert(w->priv->peak.buf[WF_RIGHT], "peak not loaded");

		g_object_unref(w);
		c->next(c);
	}

	next_wav(WF_NEW(C,
		.next = next_wav,
	));
}


void
test_audiodata ()
{
	// Instantiate a Waveform and check that all the hires-ready signals are emitted.

	START_TEST;

	static int wi; wi = 0;
	static char* wavs[] = {WAV1, WAV2};

	static int n = 0;
	static int n_tiers_needed = 3;//4;
	static guint ready_handler = 0;
	static int tot_blocks = 0;

	typedef struct C {
		void (*next)(struct C*);
	} C;

	void finalize_notify (gpointer data, GObject* was)
	{
		dbg(0, "!");
	}

	void test_on_peakdata_ready (Waveform* waveform, int block, gpointer data)
	{
		C* c = data;

		dbg(1, ">> block=%i", block);
		test_reset_timeout(5000);

		WfAudioData* audio = &waveform->priv->audio;
		if (audio->buf16) {
			WfBuf16* buf = audio->buf16[block];
			assert(buf, "no data in buffer! %i", block);
			assert(buf->buf[WF_LEFT], "no data in buffer (L)! %i", block);
			assert(buf->buf[WF_RIGHT], "no data in buffer (R)! %i", block);
		} else pwarn("no data!");

		if(_debug_) printf("\n");
		n++;
		if (n >= tot_blocks) {
			g_signal_handler_disconnect((gpointer)waveform, ready_handler);
			ready_handler = 0;
			g_object_unref(waveform);
			c->next(c);
		}
	}

	void next_wav (C* c)
	{
		if(wi >= G_N_ELEMENTS(wavs)){
			g_free(c);
			FINISH_TEST;
		}

		Waveform* w = waveform_new(wavs[wi++]);
		g_object_weak_ref((GObject*)w, finalize_notify, NULL);
		ready_handler = g_signal_connect (w, "hires-ready", (GCallback)test_on_peakdata_ready, c);
		n = 0;

		tot_blocks = waveform_get_n_audio_blocks(w);

		// trying to load the whole file at once is slightly dangerous but seems to work ok.
		// the callback is called before the cache is cleared for the block.
		for (int b=0;b<tot_blocks;b++) {
			waveform_load_audio(w, b, n_tiers_needed, NULL, NULL);
		}

		//wi++;
	}

	next_wav(WF_NEW(C,
		.next = next_wav,
	));
}
