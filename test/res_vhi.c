/*
 +---------------------------------------------------------------------
 | This file is part of the Ayyi project. https://www.ayyi.org
 | copyright (C) 2012-2022 Tim Orford <tim@orford.org>
 +---------------------------------------------------------------------
 | This program is free software; you can redistribute it and/or modify
 | it under the terms of the GNU General Public License version 3
 | as published by the Free Software Foundation.
 +----------------------------------------------
 |
 | Test waveform rendering in V_HI mode
 |
 */

#include "config.h"
#include <getopt.h>
#include "agl/gtk-area.h"
#include "waveform/actor.h"
#include "test/common.h"

#define WAV "piano.wav"

#define VBORDER 8

AGlScene*        scene    = NULL;
WaveformContext* wfc      = NULL;
Waveform*        w1       = NULL;
WaveformActor*   a[1]     = {NULL,};
WaveformActor*   split[2] = {NULL,};
float            vzoom    = 1.0;
gpointer         tests[]  = {};

KeyHandler
	zoom_in,
	zoom_out,
	vzoom_up,
	vzoom_down,
	scroll_left,
	scroll_right,
	toggle_animate,
	delete;

AGlKey keys[] = {
	{KEY_Left,      scroll_left},
	{KEY_KP_Left,   scroll_left},
	{KEY_Right,     scroll_right},
	{KEY_KP_Right,  scroll_right},
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

static bool test_delete       ();


static void
activate (GtkApplication* app, gpointer user_data)
{
	set_log_handlers();

	wf_debug = 0;

	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), 320, 256);

	GtkWidget* box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_window_set_child (GTK_WINDOW (window), box);

	GtkWidget* widget = agl_gtk_area_new();
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_widget_set_vexpand (widget, TRUE);
	gtk_box_append (GTK_BOX(box), widget);
	AGlGtkArea* area = (AGlGtkArea*)widget;

	scene = area->scene;
	wfc = wf_context_new((AGlActor*)scene);

	char* filename = find_wav(WAV);
	w1 = waveform_load_new(filename);
	g_free(filename);

	int n_frames = waveform_get_n_frames(w1) / 128;
	int start = 6 * n_frames;
	int len = 7 * n_frames;

	WfSampleRegion region[] = {
		{start, len},
	};

	uint32_t colours[][2] = {
		{0xffffff77, 0x0000ffff},
		{0x66eeffff, 0x0000ffff},
		{0xffdd66ff, 0x0000ffff},
	};

	for (int i=0;i<G_N_ELEMENTS(a);i++) {

		a[i] = wf_context_add_new_actor(wfc, w1);
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)a[i]);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	for (int i=0;i<G_N_ELEMENTS(split);i++) {
		split[i] = wf_context_add_new_actor(wfc, w1);
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)split[i]);
		wf_actor_set_colour(split[i], colours[1 + i][0]);
	}

	void on_zoom (AGlObservable* o, AGlVal zoom, gpointer _)
	{
		int n_frames = waveform_get_n_frames(w1) / 128;
		int start = 6 * n_frames;
		int len = 7 * n_frames;

		wf_actor_set_region(split[0], &(WfSampleRegion){start, len / 2});

		WfSampleRegion region = {start + ((float)(len / 2)) / zoom.f, len / 2};
		wf_actor_set_region(split[1], &region);
	}
	agl_observable_subscribe_with_state(wfc->zoom, on_zoom, NULL);

	void set_size (AGlActor* scene)
	{
		AGlfPt size = {agl_actor__width(scene), agl_actor__height(scene)};

		for (int i=0;i<G_N_ELEMENTS(a);i++) {
			wfc->samples_per_pixel = a[i]->region.len / size.x;

			if (a[i]) wf_actor_set_rect(a[i], &(WfRectangle) {
				0.0,
				i * size.y / 2,
				size.x * wfc->zoom->value.f,
				size.y / 2 * 0.95
			});
		}

		int r = 1;
		WfRectangle rect = {
			.left = 0.,
			.top = r * size.y / 2,
			.len = size.x * wfc->zoom->value.f / 2.,
			.height = size.y / 2 * 0.95
		};

		wf_actor_set_rect(split[0], &rect);

		rect.left = size.x / 2;
		wf_actor_set_rect(split[1], &rect);
	}
	((AGlActor*)scene)->set_size = set_size;

	g_object_unref(w1); // this effectively transfers ownership of the waveform to the Scene

	gtk_widget_show (window);
}


static gboolean
automated (gpointer app)
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

	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
vzoom_down (gpointer _)
{
	vzoom = MAX(vzoom / 1.2, 1.0);

	for(int i=0;i<G_N_ELEMENTS(a);i++)
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

	w1 = NULL;
	finalize_done = true;
}


static bool
test_delete ()
{
	if (!a[0]) return false;

	g_object_weak_ref((GObject*)w1, finalize_notify, NULL);

	if (finalize_done) {
		pwarn("waveform should not be free'd");
		return false;
	}

	for (int i=0;i<G_N_ELEMENTS(a);i++)
		a[i] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[i]), NULL);
	for (int i=0;i<G_N_ELEMENTS(split);i++)
		split[i] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)split[i]), NULL);

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
