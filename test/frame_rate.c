/*

  Test program to measure framerates using the libwaveform WaveformView
  widget.

  Redraws are requested at 100Hz and the resultant number of draws
  is counted.

  --------------------------------------------------------------

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
#include "agl/event.h"
#include "agl/behaviours/fullsize.h"
#include "waveform/actor.h"
#include "test/common2.h"

//#define WAV "mono_0:10.wav"
#define WAV "stereo_0:10.wav"

static uint64_t get_time  ();
static void     set_start (WaveformActor*, int64_t);

static const struct option long_options[] = {
	{ "autoquit",  0, NULL, 'q' },
};

static const char* const short_options = "q";


int
run (int argc, char* argv[])
{
	AGlWindow* window = agl_window ("Framerate", -1, -1, 320, 160, 0);
	AGlScene* scene = window->scene;
	AGlActor* root = (AGlActor*)scene;

	WaveformContext* wfc = wf_context_new(root);
	char* filename = find_wav(WAV);
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
/*
				case GDK_KEY_2:
					if(!waveform[1]){
						gtk_box_pack_start((GtkBox*)box, (GtkWidget*)(waveform[1] = waveform_view_plus_new(NULL)), TRUE, TRUE, 0);
						char* filename = g_build_filename(g_get_current_dir(), WAV, NULL);
						waveform_view_plus_load_file(waveform[1], filename, NULL, NULL);
						g_free(filename);
						gtk_widget_show((GtkWidget*)waveform[1]);
					}
					dbg(0, "2");
					break;
*/
				case 113:
					exit(EXIT_SUCCESS);
					break;
				default:
					dbg(0, "%i", key->keyval);
					break;
			}
			return AGL_HANDLED;
		}
		return AGL_NOT_HANDLED;
	}

	((AGlActor*)wa)->on_event = event;

	gboolean on_timeout (gpointer _wa)
	{
		WaveformActor* wa = _wa;

		static uint64_t frame = 0;
		static uint64_t t0    = 0;

		if (!frame)
			t0 = get_time();
		else {
			uint64_t time = get_time();
			if (!(frame % 200))
				printf("rate=%.1f fps\n", ((float)frame / ((float)(time - t0))) * 1000.0);

			if (!(frame % 8)) {
				float v = (frame % 16) ? 1.5 : 2.0/3.0;
				if (v > 16.0) v = 1.0;
				wf_context_set_zoom(wa->context, wa->context->zoom->value.f * v);
			}

			agl_scene_queue_draw (((AGlActor*)wa)->root);
		}
		frame++;
		return G_SOURCE_CONTINUE;
	}
	g_timeout_add(15, on_timeout, wa);

	g_main_loop_run (agl_main_loop_new());

	agl_window_destroy (&window);
	XCloseDisplay (dpy);

	return EXIT_SUCCESS;
}


uint64_t
get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
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


#include "test/_x11.c"
