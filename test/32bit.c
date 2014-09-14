/*

  libwaveform test of 32bit float wav files.

  **** This is a work in progress. Currently it generates a peak file to be checked manually.

  --------------------------------------------------------------

  Copyright (C) 2013 Tim Orford <tim@orford.org>

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
#include <math.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <sndfile.h>
#include <agl/utils.h>
#include "waveform/audio.h"
#include "waveform/peakgen.h"
#include "test/ayyi_utils.h"
#include "test/common.h"

TestFn create_files, test_audiodata, test_load, delete_files;

gpointer tests[] = {
	create_files,
	test_load,
	test_audiodata,
	delete_files,
};

#define WAV1 "test/data/32bit.wav"
static char* wavs[] = {WAV1};


int
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%i\n", sizeof(off_t)); exit(1); }

	wf_debug = 1;

	test_init(tests, G_N_ELEMENTS(tests));

	g_main_loop_run (g_main_loop_new (NULL, 0));

	exit(1);
}


void
create_files()
{
	START_TEST;
	reset_timeout(60000);

	void create_file(char* filename){
		printf("  %s\n", filename);

		int n_channels = 2;
		long n_frames = 2048;
		float* buffer = (float*) g_malloc0(n_frames * sizeof(float) * n_channels);

		// generate a percussive waveform
		int i; for(i=0;i<n_frames;i++){
			float envelope = (n_frames - i) / (float)n_frames;

			float h1 = sin(    i / 10.0);
			float h2 = sin(3 * i / 10.0) * envelope;            // 3rd harmonic. quicker decay.
			float h3 = sin(5 * i / 10.0) * envelope * envelope; // 5th harmonic.

			buffer[i * n_channels    ] = (h1 + 0.5 * h2 + 0.5 * h3) * envelope / 1.5;
			buffer[i * n_channels + 1] = (h1 + 0.5 * h2 + 0.5 * h3) * envelope / 1.5;
		}

		SF_INFO info = {
			0,
			44100,
			n_channels,
			SF_FORMAT_WAV | SF_FORMAT_FLOAT
		};

		SNDFILE* sndfile = sf_open(filename, SFM_WRITE, &info);
		if(!sndfile) {
			fprintf(stderr, "Sndfile open failed: %s\n", sf_strerror(sndfile));
			FAIL_TEST("%s", sf_strerror(sndfile));
		}

		for(i=0;i<4;i++){
			if(sf_writef_float(sndfile, buffer, n_frames) != n_frames){
				fprintf(stderr, "Write failed\n");
				sf_close(sndfile);
				FAIL_TEST("write failed");
			}
		}

		sf_write_sync(sndfile);
		sf_close(sndfile);
		g_free(buffer);
	}
	create_file(WAV1);

	FINISH_TEST;
}


void
delete_files()
{
	START_TEST;

	assert(!g_unlink(wavs[0]), "delete failed");

	FINISH_TEST;
}


void
test_load()
{
	// -test that the wav files are loaded and unloaded properly.
	// -test that the values in the peak file are as expected.

	START_TEST;

	static int iter; iter = 0;

	typedef struct _c C;
	struct _c {
		void (*next)(C*);
		int  wi;
	};
	C* c = g_new0(C, 1);

	void finalize_notify(gpointer data, GObject* was)
	{
		dbg(0, "...");
	}

	void next_wav(C* c)
	{
		if(c->wi >= G_N_ELEMENTS(wavs)){
			if(iter++ < 2){
				c->wi = 0;
			}else{
				g_free(c);
				FINISH_TEST;
			}
		}

		dbg(0, "==========================================================");
		reset_timeout(40000);

		Waveform* w = waveform_new(wavs[c->wi++]);
		g_object_weak_ref((GObject*)w, finalize_notify, NULL);
		waveform_load(w);

		#warning invalid access to render_data
		if(agl_get_instance()->use_shaders) gerr("FIXME invalid access to render_data for current rendering method");

		WaveformPriv* _w = w->priv;
		WfGlBlock* blocks = (WfGlBlock*)_w->render_data[MODE_MED];
		assert(blocks, "texture container not allocated");
		assert(!blocks->peak_texture[WF_LEFT].main[0], "textures allocated"); // no textures are expected to be allocated.
		assert(!blocks->peak_texture[WF_RIGHT].main[0], "textures allocated");
		assert(&_w->peak, "peak not loaded");
		assert(_w->peak.size, "peak size not set");
		assert(_w->peak.buf[WF_LEFT], "peak not loaded");
		assert(_w->peak.buf[WF_RIGHT], "peak not loaded");

		// TODO calculate these values when the waveform is created.
		assert(w->priv->peak.buf[WF_LEFT][0] == 14252 && w->priv->peak.buf[WF_LEFT][1] == -13807, "peak file contains wrong values");

		g_object_unref(w);
		c->next(c);
	}
	c->next = next_wav;
	next_wav(c);
}


void
test_audiodata()
{
	//instantiate a Waveform and check that all the peakdata-ready signals are emitted.
	//(copied from another test. not strictly needed here)

	START_TEST;

	static int wi; wi = 0;

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

		WfAudioData* audio = waveform->priv->audio_data;
		if(audio->buf16){
			WfBuf16* buf = audio->buf16[block];
			assert(buf, "no data in buffer! %i", block);
			assert(buf->buf[WF_LEFT], "no data in buffer (L)! %i", block);
			assert(buf->buf[WF_RIGHT], "no data in buffer (R)! %i", block);
		} else gwarn("no data!");

		printf("\n");
		n++;
		if(n >= tot_blocks){
			g_signal_handler_disconnect((gpointer)waveform, ready_handler);
			ready_handler = 0;
			g_object_unref(waveform);
			c->next(c);
		}
	}

	void next_wav(C* c)
	{
		if(wi >= G_N_ELEMENTS(wavs)){
			g_free(c);
			FINISH_TEST;
		}

		dbg(0, "==========================================================");

		Waveform* w = waveform_new(wavs[wi++]);
		g_object_weak_ref((GObject*)w, finalize_notify, NULL);
		ready_handler = g_signal_connect (w, "peakdata-ready", (GCallback)test_on_peakdata_ready, c);
		n = 0;

		tot_blocks = waveform_get_n_audio_blocks(w);

		// trying to load the whole file at once is slightly dangerous but seems to work ok.
		// the callback is called before the cache is cleared for the block.
		int b; for(b=0;b<tot_blocks;b++){
			waveform_load_audio_async(w, b, n_tiers_needed);
		}
	}
	c->next = next_wav;
	next_wav(c);
}


