/*
  Demonstration of the WaveformActor object

  A single waveform is cut into several regions that should be
  seamlessly displayed to look like a single region.

  ---------------------------------------------------------------

  Copyright (C) 2012-2022 Tim Orford <tim@orford.org>

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

#include "config.h"
#include <getopt.h>
#include <sys/time.h>
#include "agl/x11.h"
#include "waveform/actor.h"
#include "agl/behaviours/key.h"
#include "test/common2.h"

#define WAV "mono_0:10.wav"

#define WIDTH 480.
#define HEIGHT 160.

WaveformActor* a[4]  = {NULL,};
float          zoom  = 1.0;

static void start_zoom         (float target_zoom);

ActorKeyHandler
	zoom_in,
	zoom_out,
	toggle_animate;

ActorKey keys[] = {
	{61,   zoom_in},
	{45,   zoom_out},
	{'a',  toggle_animate},
};

static const struct option long_options[] = {
	{ "autoquit",  0, NULL, 'q' },
};

static const char* const short_options = "q";


int
run (int argc, char* argv[])
{
	wf_debug = 0;

	AGlWindow* window = agl_window ("List", -1, -1, WIDTH, HEIGHT, 0);
	AGlActor* root = (AGlActor*)window->scene;
	window->scene->selected = root;

	WaveformContext* wfc = wf_context_new((AGlActor*)window->scene);

	char* filename = find_wav(WAV);
	Waveform* wav = waveform_load_new(filename);
	g_free(filename);

	int n_frames = waveform_get_n_frames(wav);

	WfSampleRegion region[] = {
		{0,                  n_frames / 4},
		{(1 * n_frames) / 4, n_frames / 4},
		{(2 * n_frames) / 4, n_frames / 4},
		{(3 * n_frames) / 4, n_frames / 4},
	};

	uint32_t colours[4][2] = {
		{0xffffff77, 0x0000ffff},
		{0x66eeffff, 0x0000ffff},
		{0xffdd66ff, 0x0000ffff},
		{0x66ff66ff, 0x0000ffff},
	};

	for (int i=0;i<G_N_ELEMENTS(a);i++) {
		a[i] = (WaveformActor*)agl_actor__add_child(root, (AGlActor*)wf_context_add_new_actor(wfc, wav));

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	void set_size (AGlActor* scene)
	{
		start_zoom(zoom);
	}
	root->set_size = set_size;

	#define KEYS(A) ((KeyBehaviour*)((AGlActor*)A)->behaviours[0])
	root->behaviours[0] = key_behaviour();
	KEYS(root)->keys = &keys;
	key_behaviour_init(root->behaviours[0], root);

	g_main_loop_run (agl_main_loop_new());

	agl_window_destroy (&window);
	XCloseDisplay (dpy);

	return EXIT_SUCCESS;
}


bool
zoom_in (AGlActor* _, AGlModifierType state)
{
	start_zoom(zoom * 1.5);

	return AGL_HANDLED;
}


bool
zoom_out (AGlActor* _, AGlModifierType state)
{
	start_zoom(zoom / 1.5);

	return AGL_HANDLED;
}


static void
start_zoom (float target_zoom)
{
	// When zooming in, the Region is preserved so the box gets bigger.

	// This example illustrates zooming by setting the object sizes directly.
	// Normally you would use wf_context_set_zoom() instead

	PF;
	zoom = MAX(0.1, target_zoom);

	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if (a[i]) wf_actor_set_rect(a[i], &(WfRectangle) {
			WIDTH * target_zoom * i / 4,
			0.,
			WIDTH * target_zoom / 4,
			HEIGHT / 4
		});
}


bool
toggle_animate (AGlActor* _, AGlModifierType state)
{
#ifdef DEBUG
	uint64_t
	get_time ()
	{
		struct timeval start;
		gettimeofday(&start, NULL);
		return start.tv_sec * 1000 + start.tv_usec / 1000;
	}
#endif

	PF0;
	gboolean on_idle (gpointer _)
	{
		static uint64_t frame = 0;
#ifdef DEBUG
		static uint64_t t0    = 0;
#endif
		if (!frame) {
#ifdef DEBUG
			t0 = get_time();
#endif
		} else {
#ifdef DEBUG
			uint64_t time = get_time();
			if (!(frame % 1000))
				dbg(0, "rate=%.2f fps", ((float)frame) / ((float)(time - t0)) / 1000.0);
#endif

			if (!(frame % 8)) {
				float v = (frame % 16) ? 2.0 : 1.0/2.0;
				if(v > 16.0) v = 1.0;
				start_zoom(v);
			}
		}
		frame++;
		return G_SOURCE_CONTINUE;
	}
	g_timeout_add(50, on_idle, NULL);

	return AGL_HANDLED;
}


#include "test/_x11.c"
