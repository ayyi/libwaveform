/*

  libwaveform large files test
  ----------------------------

  This is a non-interactive test.

  --------------------------------------------------------------

  Copyright (C) 2012-2020 Tim Orford <tim@orford.org>

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
#include <glib.h>
#include <glib/gstdio.h>
#include <sndfile.h>
#include <agl/utils.h>
#include "wf/audio.h"
#include "wf/peakgen.h"
#include "test/common.h"

TestFn create_large_files, test_audiodata, test_load, delete_large_files;

gpointer tests[] = {
	create_large_files,
	test_load,
	test_audiodata,
	delete_large_files,
};

#define WAV1 "test/data/large1.wav"
#define WAV2 "test/data/large2.wav"

int
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ perr("sizeof(off_t)=%zu\n", sizeof(off_t)); exit(1); }

	wf_debug = 1;

	test_init(tests, G_N_ELEMENTS(tests));

	g_main_loop_run (g_main_loop_new (NULL, 0));

	exit(1);
}


void
create_large_files ()
{
	START_TEST;
	reset_timeout(60000);

	create_large_file(WAV1);
	create_large_file(WAV2);

	FINISH_TEST;
}


void
delete_large_files ()
{
	START_TEST;

	if(g_unlink(WAV1)){
		FAIL_TEST("delete failed");
	}
	assert(!g_unlink(WAV2), "delete failed");

	FINISH_TEST;
}


void
test_load ()
{
	//test that the large files are loaded and unloaded properly.

	START_TEST;

	static char* wavs[] = {WAV1, WAV2};
	static int wi; wi = 0;
	static int iter; iter = 0;

	typedef struct _c C;
	struct _c {
		void (*next)(C*);
	};
	C* c = g_new0(C, 1);

	void finalize_notify(gpointer data, GObject* was)
	{
		dbg(0, "...");
	}

	void next_wav(C* c)
	{
		if(wi >= G_N_ELEMENTS(wavs)){
			if(iter++ < 2){
				wi = 0;
			}else{
				g_free(c);
				FINISH_TEST;
			}
		}

		dbg(0, "==========================================================");
		reset_timeout(40000);

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
	c->next = next_wav;
	next_wav(c);
}


void
test_audiodata ()
{
	//instantiate a Waveform and check that all the hires-ready signals are emitted.

	START_TEST;

	static int wi; wi = 0;
	static char* wavs[] = {WAV1, WAV2};

	static int n = 0;
	static int n_tiers_needed = 3;//4;
	static guint ready_handler = 0;
	static int tot_blocks = 0;

	typedef struct _c C;
	struct _c {
		void (*next)(C*);
	};
	C* c = g_new0(C, 1);

	void finalize_notify(gpointer data, GObject* was)
	{
		dbg(0, "!");
	}

	void test_on_peakdata_ready(Waveform* waveform, int block, gpointer data)
	{
		C* c = data;

		dbg(1, ">> block=%i", block);
		reset_timeout(5000);

		WfAudioData* audio = &waveform->priv->audio;
		if(audio->buf16){
			WfBuf16* buf = audio->buf16[block];
			assert(buf, "no data in buffer! %i", block);
			assert(buf->buf[WF_LEFT], "no data in buffer (L)! %i", block);
			assert(buf->buf[WF_RIGHT], "no data in buffer (R)! %i", block);
		} else pwarn("no data!");

		printf("\n");
		n++;
		if(n >= tot_blocks){
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

		dbg(0, "==========================================================");

		Waveform* w = waveform_new(wavs[wi++]);
		g_object_weak_ref((GObject*)w, finalize_notify, NULL);
		ready_handler = g_signal_connect (w, "hires-ready", (GCallback)test_on_peakdata_ready, c);
		n = 0;

		tot_blocks = waveform_get_n_audio_blocks(w);

		// trying to load the whole file at once is slightly dangerous but seems to work ok.
		// the callback is called before the cache is cleared for the block.
		int b; for(b=0;b<tot_blocks;b++){
			waveform_load_audio(w, b, n_tiers_needed, NULL, NULL);
		}

		//wi++;
	}
	c->next = next_wav;
	next_wav(c);
}
