/**
 +---------------------------------------------------------------------
 | This file is part of the Ayyi project. https://www.ayyi.org
 | copyright (C) 2012-2021 Tim Orford <tim@orford.org>
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
#include "agl/gtk.h"
#include "waveform/actor.h"
#include "test/common.h"

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
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
gpointer         tests[]  = {};

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
	GtkWidget* canvas = gtk_drawing_area_new();

#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus     (canvas, true);
#endif
	gtk_widget_set_size_request  (canvas, 320, 256);
	gtk_widget_set_gl_capability (canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events        (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);

	scene = (AGlRootActor*)agl_actor__new_root(canvas);
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

	for(int i=0;i<G_N_ELEMENTS(a);i++){

		a[i] = wf_context_add_new_actor(wfc, w1);
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)a[i]);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	for(int i=0;i<G_N_ELEMENTS(split);i++){
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


	g_object_unref(w1); // this effectively transfers ownership of the waveform to the Scene

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);
}


static gboolean
automated ()
{
	static bool done = false;
	if(!done){
		done = true;

		if(!test_delete())
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
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(3000, automated, NULL);
				break;
		}
	}

	if(g_getenv("NON_INTERACTIVE")){
		g_timeout_add(3000, automated, NULL);
	}

	return gtk_window((Key*)&keys, window_content);
}


static void
on_canvas_realise (GtkWidget* canvas, gpointer user_data)
{
	if(!gtk_widget_get_realized(canvas)) return;

	on_allocate(canvas, &canvas->allocation, user_data);
}


static void
on_allocate (GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	((AGlActor*)scene)->region.x2 = allocation->width;
	((AGlActor*)scene)->region.y2 = allocation->height;

	for(int i=0;i<G_N_ELEMENTS(a);i++){
		wfc->samples_per_pixel = a[i]->region.len / allocation->width;

		if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
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

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
vzoom_down (gpointer _)
{
	vzoom = MAX(vzoom / 1.2, 1.0);

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
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
	if(!a[0]) return false;

	g_object_weak_ref((GObject*)w1, finalize_notify, NULL);

	if(finalize_done){
		pwarn("waveform should not be free'd");
		return false;
	}

	a[0] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[0]), NULL);

	if(!finalize_done){
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
	exit(EXIT_SUCCESS);
}

