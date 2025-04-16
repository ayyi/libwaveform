/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | Test waveform rendering in V_HI mode when the actor is very          |
 | large and uses scrolling to control the visible part.                |
 |                                                                      |
 | `WaveformActor` does not support scrolling in itself, instead        |
 | the test uses the scrolling of the parent actor.                     |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <getopt.h>
#include <gdk/gdkkeysyms.h>
#include "agl/gtk-area.h"
#include "agl/behaviours/split.h"
#include "agl/behaviours/scrollable_h.h"
#include "waveform/actor.h"
#include "waveform/debug_helper.h"
#include "test/common.h"

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
	{}
};

static const char* const short_options = "n";

#define WAV "7_blocks.wav"

AGlScene*        scene    = NULL;
WaveformContext* wfc      = NULL;
Waveform*        waveform = NULL;
WaveformActor*   a        = NULL;
WaveformActor*   a2       = NULL;
float            vzoom    = 1.0;

KeyHandler
	zoom_in,
	zoom_out,
	vzoom_up,
	vzoom_down,
	toggle_animate,
	delete,
	quit;

Key keys[] = {
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

static void on_realise        (GtkWidget*, gpointer);
static void on_allocate       (GtkWidget*, GtkAllocation*, gpointer);
static void set_len_px        (int);
static bool test_delete       ();

#define SCROLL_WIDTH 50000 // V_HI ~10px/sample


static void
window_content (GtkWindow* window, GdkGLConfig* glconfig)
{
	GlArea* area = gl_area_new();
	GtkWidget* canvas = (GtkWidget*)area;

	scene = area->scene;
	scene->selected = (AGlActor*)scene;
	((AGlActor*)scene)->scrollable = (AGliRegion){ .x2 = SCROLL_WIDTH };
	agl_actor__add_behaviour((AGlActor*)scene, agl_split());

	gtk_widget_add_events (canvas, (gint) (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_SCROLL_MASK));

	gtk_widget_set_size_request(canvas, 1024, 256);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);

	wfc = wf_context_new((AGlActor*)area->scene);

	g_autofree char* filename = find_wav(WAV);
	waveform = waveform_load_new(filename);

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);

	gboolean key_press (GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		return agl_actor__on_event (scene, (GdkEvent*)event);
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), NULL);
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
on_realise (GtkWidget* canvas, gpointer user_data)
{
	if (a) return;

	GlArea* area = (GlArea*)canvas;

	int n_frames = waveform_get_n_frames(waveform);

	agl_actor__add_child((AGlActor*)area->scene, ({
		a = wf_context_add_new_actor(wfc, waveform);
#ifdef DEBUG
		agl_actor__add_behaviour((AGlActor*)a, debug_helper());
#endif
		(AGlActor*)a;
	}));
	wf_actor_set_region(a, &(WfSampleRegion){ .len = n_frames });

	a2 = wf_context_add_new_actor(wfc, waveform);
	agl_actor__add_child((AGlActor*)area->scene, (AGlActor*)a2);
	wf_actor_set_region(a2, &(WfSampleRegion){ .len = n_frames });

	void on_scroll (AGlObservable* observable, AGlVal value, gpointer scrollbar)
	{
		AGlActor* root = (AGlActor*)scene;

		int width = agl_actor__scrollable_width(root);

		root->scrollable.x1 = -value.i;
		root->scrollable.x2 = -value.i + width;

		agl_actor__invalidate(root);
		agl_actor__set_size(root);
		wf_actor_scroll_to (a, 0);
	}
	AGlBehaviour* scrollable = agl_actor__add_behaviour((AGlActor*)scene, scrollable_h());
	agl_observable_subscribe((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll, on_scroll, NULL);

	g_object_unref(waveform); // transfer ownership of the waveform to the Scene

	agl_actor__set_size((AGlActor*)scene);

	// scroll to end of 1st block
	agl_observable_set_int((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll, 6200);
}


static void
on_allocate (GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	uint64_t default_len = waveform_get_n_frames(waveform);
	wfc->samples_per_pixel = default_len / allocation->width;
}


static void
set_len_px (int len)
{
	AGlActor* root = (AGlActor*)scene;

	root->scrollable.x2 = len;
	wf_actor_set_rect(a, &(WfRectangle){ .left = 0, .len = len });
	wf_actor_set_rect(a2, &(WfRectangle){ .left = 0, .len = len });
}


void
zoom_in (gpointer _)
{
	AGlActor* root = (AGlActor*)scene;

	int len = MIN(
		(root->scrollable.x2 * 4) / 3,
		waveform->n_frames
	);
	set_len_px(len);
}


void
zoom_out (gpointer _)
{
	AGlActor* root = (AGlActor*)scene;

	set_len_px(MAX(
		(root->scrollable.x2 * 3) / 4,
		agl_actor__width(root)
	));
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
		perr("waveform was not free'd");
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
