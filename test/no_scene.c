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
 | Demonstration of the libwaveform WaveformActor interface             |
 |                                                                      |
 | Several waveforms are drawn onto a single canvas with                |
 | different colours and zoom levels. The canvas can be zoomed          |
 | and panned.                                                          |
 |                                                                      |
 | This test does NOT use the AGl scene graph, though it is not         |
 | clear why one would not want to use the scene graph.                 |
 | It demonstates use of the WaveformActor as part of a larger scene    |
 | under manual control by the application.                             |
 |                                                                      |
 | For a simpler example, see test/actor.c                              |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <gtk/gtk.h>
#include "agl/text/renderer.h"
#include "agl/fbo.h"
#include "waveform/actor.h"
#include "test/common2.h"

GtkWidget*       area          = NULL;
WaveformContext* wfc           = NULL;
WaveformActor*   a[]           = {NULL, NULL, NULL, NULL};
float            zoom          = 1.0;
float            vzoom         = 1.0;

static gboolean render         (GtkGLArea*, GdkGLContext*);
static void     allocate       (GtkWidget*, int, int, gpointer);
static void     start_zoom     (float target_zoom);

AGlKeyHandler
	zoom_in,
	zoom_out,
	vzoom_up,
	vzoom_down,
	scroll_left,
	scroll_right,
	toggle_animate;

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
	{XK_Delete,     NULL},
	{0},
};


 
static void
activate (GtkApplication* app, gpointer user_data)
{
	set_log_handlers();

	wf_debug = 0;

	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "No scene");
	gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

	GtkWidget* box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_window_set_child (GTK_WINDOW (window), box);

	GtkWidget* widget = area = gtk_gl_area_new();
	gtk_widget_set_hexpand (area, TRUE);
	gtk_widget_set_vexpand ((GtkWidget*)widget, TRUE);
	gtk_box_append (GTK_BOX(box), GTK_WIDGET(widget));

	g_signal_connect_after (widget, "resize", G_CALLBACK (allocate), NULL);
	g_signal_connect ((GtkGLArea*)widget, "render", G_CALLBACK (render), NULL);

/*
	void queue_draw (AGlScene* scene)
	{
		PF0;
	}
*/

	wfc = wf_context_new(&(AGlActor){
		.root = &(AGlRootActor){
//			.queue_draw = queue_draw,
		}
	});

	char* filename = find_wav("mono_0:10.wav");
	Waveform* wav = waveform_load_new(filename);
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
		a[i] = wf_context_add_new_actor(wfc, wav);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	add_key_handlers_gtk ((GtkWindow*)window, NULL, (AGlKey*)&keys);
}


gboolean
_zoom (gpointer _)
{
	start_zoom(zoom);

	return G_SOURCE_REMOVE;
}


static gboolean
render (GtkGLArea* area, GdkGLContext* context)
{
	glClearColor (0, 0, 0, 1);
	glClear (GL_COLOR_BUFFER_BIT);
 
	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if (a[i]) {
			AGlActor* actor = (AGlActor*)a[i];
			if (actor->program) {
				agl_use_program(actor->program);
				builder()->offset.y = actor->region.y1;
				actor->paint(actor);
			}
		}

	return TRUE;
}


static void
allocate (GtkWidget* widget, int width, int height, gpointer data)
{
	// optimise drawing by telling the canvas which area is visible
	wf_context_set_viewport(wfc, &(WfViewPort){0, 0, width, height});

 	gtk_gl_area_make_current ((GtkGLArea*)widget);
	agl_gl_init();

	builder()->target->width = width;
	builder()->target->height = height;

	start_zoom(zoom);
}


static void
start_zoom (float target_zoom)
{
	// when zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	zoom = MAX(0.1, target_zoom);

	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if (a[i]) {
			wf_actor_set_rect(a[i], &(WfRectangle){
				0.0,
				i * builder()->target->height / 4,
				builder()->target->width * target_zoom,
				builder()->target->height / 4 * 0.95
			});
			((AGlActor*)a[i])->set_size((AGlActor*)a[i]);
		}

	gtk_gl_area_queue_render ((GtkGLArea*)area);
}


void
zoom_in (gpointer _)
{
	start_zoom(zoom * 1.5);
}


void
zoom_out (gpointer _)
{
	start_zoom(zoom / 1.5);
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


void
toggle_animate (gpointer _)
{
	gboolean on_idle (gpointer _)
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
				start_zoom(v);
			}
		}
		frame++;
		return G_SOURCE_CONTINUE;
	}
	g_timeout_add(50, on_idle, NULL);
}


#include "test/_gtk.c"
