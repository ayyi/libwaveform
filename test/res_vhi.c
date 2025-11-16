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
 | Test waveform rendering in V_HI mode
 |
 */

#include "config.h"
#include <getopt.h>
#include <gdk/gdkkeysyms.h>
#include "agl/gtk-area.h"
#include "waveform/actor.h"
#include "test/common.h"

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
	{}
};

static const char* const short_options = "n";

#define WAV "piano.wav"

#define VBORDER 8

AGlScene*        scene    = NULL;
WaveformContext* wfc      = NULL;
Waveform*        w1       = NULL;
WaveformActor*   a[1]     = {NULL,};
WaveformActor*   split[2] = {NULL,};
float            vzoom    = 1.0;

KeyHandler
	zoom_in,
	zoom_out,
	vzoom_up,
	vzoom_down,
	scroll_left,
	scroll_right,
	toggle_animate,
	debug_info,
	delete,
	quit;

Key keys[] = {
	{KEY_Left,      scroll_left},
	{KEY_KP_Left,   scroll_left},
	{KEY_Right,     scroll_right},
	{KEY_KP_Right,  scroll_right},
	{61,            zoom_in},
	{45,            zoom_out},
	{(char)'w',     vzoom_up},
	{(char)'s',     vzoom_down},
	{GDK_KP_Enter,  NULL},
	{(char)'<',     NULL},
	{(char)'>',     NULL},
	{(char)'a',     toggle_animate},
	{(char)'d',     debug_info},
	{GDK_Delete,    delete},
	{113,           quit},
	{0},
};

static void on_canvas_realise (GtkWidget*, gpointer);
static void on_allocate       (GtkWidget*, GtkAllocation*, gpointer);
static bool test_delete       ();


static void
window_content (GtkWindow* window, GdkGLConfig* glconfig)
{
	GlArea* area = gl_area_new();
	GtkWidget* canvas = (GtkWidget*)area;
	scene = area->scene;

	gtk_widget_set_size_request(canvas, 320, 256);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);

	wfc = wf_context_new((AGlActor*)area->scene);

	g_autofree char* filename = find_wav(WAV);
	w1 = waveform_load_new_sync(filename);

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
		agl_actor__add_child((AGlActor*)area->scene, (AGlActor*)a[i]);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	for (int i=0;i<G_N_ELEMENTS(split);i++) {
		split[i] = wf_context_add_new_actor(wfc, w1);
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)split[i]);
		wf_actor_set_colour(split[i], colours[1 + i][0]);
	}
	g_free(((AGlActor*)split[0])->name);
	((AGlActor*)split[0])->name = g_strdup("Split-lhs");
	g_free(((AGlActor*)split[1])->name);
	((AGlActor*)split[1])->name = g_strdup("Split-rhs");

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

	g_object_unref(w1); // this effectively transfers ownership of the waveform to the Scene
	g_object_unref(wfc); // context is freed when all actors are removed

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
}


static gboolean
automated (void* _)
{
	static bool done = false;
	if (!done) {
		done = true;

		if (!test_delete())
			exit(EXIT_FAILURE);

		gtk_main_quit();
	}
	return G_SOURCE_REMOVE;
}


int
main (int argc, char* argv[])
{
	set_log_handlers();

	wf_debug = 0;

	gtk_init(&argc, &argv);

	int opt;
	while ((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch (opt) {
			case 'n':
				g_timeout_add(3000, automated, NULL);
				break;
		}
	}

	if (g_getenv("NON_INTERACTIVE")) {
		g_timeout_add(3000, automated, NULL);
	}

	return gtk_window((Key*)&keys, window_content);
}


static void
on_canvas_realise (GtkWidget* canvas, gpointer user_data)
{
	if (!gtk_widget_get_realized(canvas)) return;

	on_allocate(canvas, &canvas->allocation, user_data);
}


static void
on_allocate (GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	wfc->samples_per_pixel = a[0]->region.len / allocation->width;

	for (int i=0;i<G_N_ELEMENTS(a);i++) {
		if (a[i]) wf_actor_set_rect(a[i], &(WfRectangle) {
			0.0,
			i * allocation->height / 2,
			allocation->width * wfc->zoom->value.f,
			allocation->height / 2 * 0.95
		});
	}

	int r = 1;
	WfRectangle rect = {
		.left = 0.,
		.top = r * allocation->height / 2,
		.len = allocation->width * wfc->zoom->value.f / 2.,
		.height = allocation->height / 2 * 0.95
	};

	wf_actor_set_rect(split[0], &rect);

	rect.left = allocation->width / 2;
	wf_actor_set_rect(split[1], &rect);
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
	void set_region (WaveformActor* actor)
	{
		WfSampleRegion region = actor->region;
		region.start = MAX(0, region.start - 1024);
		wf_actor_set_region(actor, &region);
	}

	set_region(a[0]);
	set_region(split[0]);

	wf_actor_set_region(split[1], &(WfSampleRegion){
		split[0]->region.start + 384 * wfc->samples_per_pixel / wfc->zoom->value.f,
		split[1]->region.len,
	});
}


void
scroll_right (gpointer _)
{
	void set_region (WaveformActor* actor)
	{
		wf_actor_set_region(actor, &(WfSampleRegion){
			.start = MIN(a[0]->waveform->n_frames - actor->region.len, actor->region.start + 1024),
			.len = actor->region.len
		});
	}

	set_region(a[0]);
	set_region(split[0]);
	set_region(split[1]);
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


void
debug_info (gpointer view)
{
#ifdef DEBUG
    extern void agl_actor__print_tree (AGlActor*);
    agl_actor__print_tree((AGlActor*)scene);
#endif
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


void
quit (gpointer _)
{
	agl_actor__free((AGlActor*)scene);
	exit(EXIT_SUCCESS);
}

