/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2022 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | test/actor.c                                                         |
 |                                                                      |
 | Demonstration of the libwaveform WaveformActor interface             |
 |                                                                      |
 | Several waveforms are drawn onto a single canvas with                |
 | different colours and zoom levels. The canvas can be zoomed          |
 | and panned.                                                          |
 |                                                                      |
 | In this example, drawing is managed by the AGl scene graph.          |
 | See actor_no_scene.c for a version that doesnt use the scene graph.  |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <getopt.h>
#include <gdk/gdkkeysyms.h>
#include "agl/actor.h"
#include "agl/x11.h"
#include "waveform/actor.h"
#include "test/common.h"
#include "agl/behaviours/simple_key.h"

static const struct option long_options[] = {
	{ "autoquit",  0, NULL, 'q' },
};

static const char* const short_options = "q";

#define WAV "mono_0:10.wav"

#define GL_WIDTH 256.0

AGlScene*        scene     = NULL;
Waveform*        wav       = NULL;
WaveformContext* wfc[4]    = {NULL,}; // This test has 4 separate contexts. Normally you would use a single context.
WaveformActor*   a[4]      = {NULL,};
float            vzoom     = 1.0;

KeyHandler
	zoom_in,
	zoom_out,
	vzoom_up,
	vzoom_down,
	scroll_left,
	scroll_right,
	toggle_animate,
	delete,
	quit;

AGlKey keys[] = {
	{XK_Left,       scroll_left},
	{XK_KP_Left,    scroll_left},
	{XK_Right,      scroll_right},
	{XK_KP_Right,   scroll_right},
	{61,            zoom_in},
	{45,            zoom_out},
	{(char)'w',     vzoom_up},
	{(char)'s',     vzoom_down},
	{XK_KP_Enter,   NULL},
	{(char)'<',     NULL},
	{(char)'>',     NULL},
	{(char)'a',     toggle_animate},
	{XK_Delete,     delete},
	{0},
};

static void start_zoom     (float target_zoom);
static bool test_delete    ();


#define HAVE_AUTOMATION

static gboolean
automated ()
{
	static bool done = false;
	if (!done) {
		done = true;

		if (!test_delete())
			exit(EXIT_FAILURE);

		exit(EXIT_SUCCESS);
	}
	return G_SOURCE_REMOVE;
}


int
run (int argc, char* argv[])
{
	wf_debug = 0;

	AGlWindow* window = agl_window ("Actor", -1, -1, 320, 160, 0);
	scene = window->scene;
	AGlActor* root = (AGlActor*)scene;
	scene->selected = root;

	char* filename = find_wav(WAV);
	wav = waveform_load_new(filename);
	g_free(filename);

	int n_frames = waveform_get_n_frames(wav);

	WfSampleRegion region[] = {
		{0,            n_frames     - 1},
		{0,            n_frames / 2 - 1},
		{n_frames / 4, n_frames / 4 - 1},
		{n_frames / 2, n_frames / 2 - 1},
	};

	uint32_t colours[4][2] = {
		{0xffffff77, 0x0000ffff},
		{0x66eeffff, 0x0000ffff},
		{0xffdd66ff, 0x0000ffff},
		{0x66ff66ff, 0x0000ffff},
	};

	for (int i=0;i<G_N_ELEMENTS(a);i++) {
		wfc[i] = wf_context_new((AGlActor*)scene); // each waveform has its own context so as to have a different zoom

		a[i] = wf_context_add_new_actor(wfc[i], wav);
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)a[i]);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	g_object_unref(wav); // transfer ownership of the waveform to the Scene

	void set_size (AGlActor* scene)
	{
		float height = agl_actor__height(scene);

		for (int i=0;i<G_N_ELEMENTS(a);i++) {
			wfc[i]->samples_per_pixel = a[i]->region.len / agl_actor__width(scene);
			if (a[i])
				wf_actor_set_rect(a[i], &(WfRectangle){
					0.,
					i * agl_actor__height(scene) / 4.,
					GL_WIDTH * wfc[0]->zoom->value.f,
					height / 4. * 0.95
				});
		}

		start_zoom(wfc[0]->zoom->value.f);
	}
	((AGlActor*)window->scene)->set_size = set_size;

	#define KEYS(A) ((SimpleKeyBehaviour*)((AGlActor*)A)->behaviours[0])
	root->behaviours[0] = simple_key_behaviour();
	KEYS(root)->keys = &keys;
	simple_key_behaviour_init(root->behaviours[0], root);

	g_main_loop_run (agl_main_loop_new());

	agl_window_destroy (&window);
	XCloseDisplay (dpy);

	return 0;
}


static void
start_zoom (float target_zoom)
{
	// When zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	wf_context_set_zoom(wfc[0], target_zoom);
}


void
zoom_in (gpointer _)
{
	start_zoom(wfc[0]->zoom->value.f * 1.5);
}


void
zoom_out (gpointer _)
{
	start_zoom(wfc[0]->zoom->value.f / 1.5);
}


void
vzoom_up (gpointer _)
{
	vzoom *= 1.1;
	vzoom = MIN(vzoom, 100.0);
	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if (a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
vzoom_down (gpointer _)
{
	vzoom /= 1.1;
	vzoom = MAX(vzoom, 1.0);
	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if (a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
scroll_left (gpointer _)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame - n_visible_frames / 10);
}


void
scroll_right (gpointer _)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame + n_visible_frames / 10);
}


static gboolean
on_idle (gpointer _)
{
	static uint64_t frame = 0;
#ifdef DEBUG
	static uint64_t t0    = 0;
#endif

	if (!frame)
#ifdef DEBUG
		t0 = g_get_monotonic_time();
#else
		;
#endif
	else {
#ifdef DEBUG
		uint64_t time = g_get_monotonic_time();
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

	wav = NULL;
	finalize_done = true;
}


static bool
test_delete ()
{
	if (!a[0]) return false;

	g_object_weak_ref((GObject*)wav, finalize_notify, NULL);

	a[0] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[0]), NULL);

	if (finalize_done) {
		pwarn("waveform should not be free'd");
		return false;
	}

	a[1] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[1]), NULL);

	a[2] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[2]), NULL);

	a[3] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[3]), NULL);

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


#include "test/_x11.c"
