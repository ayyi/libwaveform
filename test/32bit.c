/*

  libwaveform test of 32bit float wav files.

  Currently the test is not very good at detecting errors

  --------------------------------------------------------------

  Copyright (C) 2013-2020 Tim Orford <tim@orford.org>

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
#include <glib.h>
#include <glib/gstdio.h>
#include <sndfile.h>
#include <agl/utils.h>
#include "waveform/debug.h"
#include "waveform/audio.h"
#include "waveform/peakgen.h"
#include "test/common.h"

TestFn create_files, test_audiodata, test_load, delete_files;

gpointer tests[] = {
	create_files,
	test_load,
	test_audiodata,
	delete_files,
};

#define WAV1 "32bit.wav"
static char* wavs[] = {WAV1};

float first_peak[WF_PEAK_VALUES_PER_SAMPLE] = {0,};


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

		for(i=0;i<WF_PEAK_RATIO;i++){
			first_peak[0] = MAX(first_peak[0], buffer[i * n_channels]);
			first_peak[1] = MIN(first_peak[1], buffer[i * n_channels]);
		}

		SF_INFO info = {
			0,
			44100,
			n_channels,
			SF_FORMAT_WAV | SF_FORMAT_FLOAT
		};

		SNDFILE* sndfile = sf_open(filename, SFM_WRITE, &info);
		if(!sndfile) {
			fprintf(stderr, "Sndfile open failed: %s %s\n", sf_strerror(sndfile), filename);
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

	const char* dir = find_data_dir();
	char* filename = g_build_filename(dir, WAV1, NULL);
	create_file(WAV1);
	g_free(filename);

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
		dbg(1, "...");
	}

	bool check_pixbuf(GdkPixbuf* pixbuf)
	{
		guchar* pixels = gdk_pixbuf_get_pixels (pixbuf);
		int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
		int n_channels = gdk_pixbuf_get_n_channels (pixbuf);
		int value = 0;
		int y = 0; for(y=0;y<gdk_pixbuf_get_height(pixbuf);y++){
			int x; for(x=0;x<gdk_pixbuf_get_width(pixbuf);x++){
				guchar* p = pixels + y * rowstride + x * n_channels;
				value += p[0];
			}
		}
		return (bool)value;
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

		char* filename = find_wav(wavs[c->wi]);
		Waveform* w = waveform_new(filename);
		g_free(filename);
		WaveformPrivate* _w = w->priv;
		g_object_weak_ref((GObject*)w, finalize_notify, NULL);
		assert(waveform_load_sync(w), "failed to load wav");

		assert(&_w->peak, "peak not loaded");
		assert(_w->peak.size, "peak size not set");
		assert(_w->peak.buf[WF_LEFT], "peak not loaded");
		assert(_w->peak.buf[WF_RIGHT], "peak not loaded");

		int expected[] = {
			first_peak[0] * (1 << 15),
			first_peak[1] * (1 << 15)
		};
		assert(w->priv->peak.buf[WF_LEFT][0] == expected[0] && w->priv->peak.buf[WF_LEFT][1] == expected[1], "%s: peak file contains wrong values: %i, %i", wavs[c->wi], w->priv->peak.buf[WF_LEFT][0], w->priv->peak.buf[WF_LEFT][1]);

		c->wi++;

		// check MED res
		// the pixbuf generation does not report errors so is difficult to test
		{
			GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 20, 32);
			guchar* pixels = gdk_pixbuf_get_pixels (pixbuf);
			memset(pixels, 0, gdk_pixbuf_get_rowstride(pixbuf) * gdk_pixbuf_get_height(pixbuf));
			waveform_peak_to_pixbuf(w, pixbuf, NULL, 0xffffffff, 0x000000ff, false);
			assert(check_pixbuf(pixbuf), "MED waveform is blank");
#if 0
			gdk_pixbuf_save(pixbuf, "tmp1medres.png", "png", NULL, NULL);
#endif
			g_object_unref(pixbuf);

		}

		// check HI res
		{
			GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 40, 64);
			guchar* pixels = gdk_pixbuf_get_pixels (pixbuf);
			memset(pixels, 0, gdk_pixbuf_get_rowstride(pixbuf) * gdk_pixbuf_get_height(pixbuf));
			waveform_peak_to_pixbuf(w, pixbuf, NULL, 0xffffffff, 0x000000ff, false);
			assert(check_pixbuf(pixbuf), "HI waveform is blank");
#if 0
			gdk_pixbuf_save(pixbuf, "tmp1hires.png", "png", NULL, NULL);
#endif
			g_object_unref(pixbuf);
		}

		/* render_data is not set until drawn onscreen
		WaveformModeRender* r = _w->render_data[MODE_MED];
		assert(r, "texture container not allocated");
		assert(r->n_blocks, "no blocks in renderer");
		dbg(0, "n_blocks=%i", r->n_blocks);
		*/

		g_object_unref(w);
		c->next(c);
	}
	c->next = next_wav;
	next_wav(c);
}


void
test_audiodata()
{
	//instantiate a Waveform and check that all the hires-ready signals are emitted.
	//(copied from another test. not strictly needed here)

	// TODO check the actual data

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

	void next_wav(C* c)
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
	}
	c->next = next_wav;
	next_wav(c);
}


