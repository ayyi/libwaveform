/*
 +---------------------------------------------------------------------
 | This file is part of the Ayyi project. https://www.ayyi.org
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>
 +---------------------------------------------------------------------
 | This program is free software; you can redistribute it and/or modify
 | it under the terms of the GNU General Public License version 3
 | as published by the Free Software Foundation.
 +----------------------------------------------
 |
 | Test waveform rendering in V_HI mode at block edge, showing
 | the change from the first to second blocks.
 |
 */

#include "config.h"
#include <getopt.h>
#include <gdk/gdkkeysyms.h>
#include "agl/gtk-area.h"
#include "agl/behaviours/split.h"
#include "waveform/actor.h"
#include "test/common2.h"

#define WAV "7_blocks.wav"

AGlScene*        scene    = NULL;
WaveformContext* wfc      = NULL;
Waveform*        waveform = NULL;
WaveformActor*   a        = NULL;
WaveformActor*   a2       = NULL;
float            vzoom    = 1.0;

AGlKeyHandler
	zoom_in,
	zoom_out,
	vzoom_up,
	vzoom_down,
	scroll_left,
	scroll_right,
	toggle_animate,
	delete;

AGlKey keys[] = {
	{XK_Left,      scroll_left},
	{XK_KP_Left,   scroll_left},
	{XK_Right,     scroll_right},
	{XK_KP_Right,  scroll_right},
	{61,           zoom_in},
	{45,           zoom_out},
	{(char)'w',    vzoom_up},
	{(char)'s',    vzoom_down},
	{XK_KP_Enter,  NULL},
	{(char)'<',    NULL},
	{(char)'>',    NULL},
	{(char)'a',    toggle_animate},
	{XK_Delete,    delete},
	{0},
};

static bool test_delete       ();


static void
activate (GtkApplication* app, gpointer user_data)
{
	set_log_handlers();

	wf_debug = 0;

	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW(window), "V Hi Res test");
	gtk_window_set_default_size (GTK_WINDOW(window), 1024, 256);

	GtkWidget* canvas = agl_gtk_area_new();
	AGlGtkArea* area = (AGlGtkArea*)canvas;
	scene = area->scene;
	agl_actor__add_behaviour((AGlActor*)scene, agl_split());

	gtk_widget_set_size_request(canvas, 1024, 256);
	gtk_window_set_child (GTK_WINDOW(window), canvas);

	wfc = wf_context_new((AGlActor*)area->scene);

	g_autofree char* filename = find_wav(WAV);
	waveform = waveform_load_new(filename);

	a = wf_context_add_new_actor(wfc, waveform);
	agl_actor__add_child((AGlActor*)area->scene, (AGlActor*)a);

	a2 = wf_context_add_new_actor(wfc, waveform);
	agl_actor__add_child((AGlActor*)area->scene, (AGlActor*)a2);

	void layout (AGlActor* actor)
	{
		actor->region.x1 = 100.;
		actor->region.x2 = actor->parent->region.x2 + 100.;
	}
	((AGlActor*)a2)->set_size = layout;

	void on_zoom (AGlObservable* o, AGlVal zoom, gpointer _)
	{
		int n_frames = waveform_get_n_frames(waveform) / 512;

		wf_actor_set_region(a, &(WfSampleRegion){
			.start = 71 * n_frames,
			.len = 7. * (float)n_frames / zoom.f,
		});
		wf_actor_set_region(a2, &(WfSampleRegion){
			.start = 71 * n_frames,
			.len = 7. * (float)n_frames / zoom.f,
		});
	}
	agl_observable_subscribe_with_state(wfc->zoom, on_zoom, NULL);

	g_object_unref(waveform); // transfer ownership of the waveform to the Scene

	void set_size (AGlActor* scene)
	{
		AGlfPt size = {agl_actor__width(scene), agl_actor__height(scene)};

		uint64_t default_len = 7. * (float)waveform_get_n_frames(waveform) / 512;
		wfc->samples_per_pixel = default_len / size.x;
	}
	((AGlActor*)scene)->set_size = set_size;

	gtk_widget_set_visible(window, true);
}


static gboolean
automated (void* app)
{
	static bool done = false;
	if (!done) {
		done = true;

		if (!test_delete())
			exit(EXIT_FAILURE);

		g_application_quit((GApplication*)app);
	}
	return G_SOURCE_REMOVE;
}


void
zoom_in (gpointer _)
{
	wf_context_set_zoom(wfc, wfc->zoom->value.f * 1.5);
}


void
zoom_out (gpointer _)
{
	wf_context_set_zoom(wfc, wfc->zoom->value.f / 1.5);
}


void
vzoom_up (gpointer _)
{
	vzoom = MIN(vzoom * 1.2, 100.0);

	wf_actor_set_vzoom(a, vzoom);
}


void
vzoom_down (gpointer _)
{
	vzoom = MAX(vzoom / 1.2, 1.0);

	wf_actor_set_vzoom(a, vzoom);
}


void
scroll_left (gpointer _)
{
	wf_actor_set_region(a, &(WfSampleRegion){
		.start = MAX(0, a->region.start - 100),
		.len = a->region.len,
	});
}


void
scroll_right (gpointer _)
{
	wf_actor_set_region(a, &(WfSampleRegion){
		.start = a->region.start + 100,
		.len = a->region.len,
	});
}


static gboolean
on_idle (gpointer _)
{
	static uint64_t frame = 0;
#ifdef DEBUG
	static uint64_t t0    = 0;
#endif

	if (!frame) {
#ifdef DEBUG
		t0 = g_get_monotonic_time();
#endif
	} else {
#ifdef DEBUG
		uint64_t time = g_get_monotonic_time();
		if (!(frame % 1000))
			dbg(0, "rate=%.2f fps", ((float)frame) / ((float)(time - t0)) / 1000.0);
#endif

		if (!(frame % 8)) {
			float v = (frame % 16) ? 2.0 : 1.0/2.0;
			if (v > 16.0) v = 1.0;
			wf_context_set_zoom(wfc, v);
		}
	}
	frame++;

	return G_SOURCE_CONTINUE;
}


void
toggle_animate (gpointer _)
{
	PF0;
	g_timeout_add(50, on_idle, NULL);
}


static int finalize_done = false;

static void
finalize_notify (gpointer data, GObject* was)
{
	PF;

	waveform = NULL;
	finalize_done = true;
}


static bool
test_delete ()
{
	if (!a) return false;

	g_object_weak_ref((GObject*)waveform, finalize_notify, NULL);

	if (finalize_done) {
		pwarn("waveform should not be free'd");
		return false;
	}

	a = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a), NULL);
	a2 = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a2), NULL);

	if (!finalize_done) {
		pwarn("waveform was not free'd");
		return false;
	}

	return true;
}


void
delete (gpointer _)
{
	test_delete();
}


#define HAVE_AUTOMATION
#include "test/_gtk.c"
