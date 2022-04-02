/*

  For testing for memory leaks and texture usage.

  After a short delay, the 1st wav is unloaded.

  --------------------------------------------------------------

  Copyright (C) 2012-2021 Tim Orford <tim@orford.org>

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
#include "agl/x11.h"
#include "agl/event.h"
#include "agl/behaviours/fullsize.h"
#include "waveform/actor.h"
#include "test/common2.h"

#define WAV1 "mono_0:10.wav"
#define WAV2 "stereo_0:10.wav"

static void set_start (WaveformActor*, int64_t);

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";


int
main (int argc, char* argv[])
{
	set_log_handlers();

	wf_debug = 0;

	int opt;
	while ((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch (opt) {
			case 'n':
				g_timeout_add(4000, (gpointer)exit, NULL);
				break;
		}
	}

	AGlWindow* window = agl_window ("Resources", -1, -1, 320, 160, 0);
	AGlScene* scene = window->scene;
	AGlActor* root = (AGlActor*)scene;

	WaveformContext* wfc = wf_context_new(root);
	char* filename = find_wav(WAV1);
	Waveform* wav = waveform_new(filename);
	g_free(filename);

	WaveformActor* wa = wf_context_add_new_actor(wfc, wav);
	agl_actor__add_behaviour((AGlActor*)wa, fullsize());
	agl_actor__add_child(root, (AGlActor*)wa);
	wf_actor_set_region(wa, &(WfSampleRegion){.len = waveform_get_n_frames(wav)});
	scene->selected = (AGlActor*)wa;

	bool event (AGlActor* actor, AGlEvent* event, AGliPt xy)
	{
		WaveformActor* wa = (WaveformActor*)actor;

		if (event->type == AGL_KEY_PRESS) {
			AGlEventKey* key = (AGlEventKey*)event;

			switch (key->keyval) {
				case 61:
					wf_context_set_zoom(wa->context, wa->context->zoom->value.f * 1.5);
					break;
				case 45:
					wf_context_set_zoom(wa->context, wa->context->zoom->value.f / 1.5);
					break;
				case KEY_Left:
				case KEY_KP_Left:
					set_start(wa, wa->region.start - 8192 / wa->context->zoom->value.f);
					break;
				case KEY_Right:
				case KEY_KP_Right:
					set_start(wa, wa->region.start + 8192 / wa->context->zoom->value.f);
					break;
				case 113:
					exit(EXIT_SUCCESS);
					break;
				default:
					dbg(1, "%i", key->keyval);
					break;
			}
			return AGL_HANDLED;
		}
		return AGL_NOT_HANDLED;
	}

	((AGlActor*)wa)->on_event = event;

	gboolean swap_wav (gpointer data)
	{
		WaveformActor* wa = data;

		char* filename = find_wav(WAV2);
		wf_actor_set_waveform (wa, waveform_new(filename), NULL, NULL);
		g_free(filename);

		return G_SOURCE_REMOVE;
	}
	g_timeout_add(3000, swap_wav, wa);

	g_main_loop_run (agl_main_loop_new());

	agl_window_destroy (&window);
	XCloseDisplay (dpy);

	return EXIT_SUCCESS;
}


static void
set_start (WaveformActor* wa, int64_t start_frame)
{
	// the number of visible frames reduces as the zoom increases.
	int64_t n_frames_visible = agl_actor__width(((AGlActor*)wa)) * wa->context->samples_per_pixel / wa->context->zoom->value.f;

	start_frame = CLAMP(
		start_frame,
		0,
		(int64_t)(waveform_get_n_frames(wa->waveform) - MAX(10, n_frames_visible))
	);

	wf_context_set_start(wa->context, start_frame);

	wf_actor_set_region(wa, &(WfSampleRegion){
		start_frame,
		n_frames_visible
	});
}
