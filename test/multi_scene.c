/*
  Demonstration of the libwaveform WaveformActor interface

  Several waveforms are drawn onto a single canvas with
  different colours and zoom levels. The canvas can be zoomed
  and panned.

  This example demonstrates the use of two separate agl scenes
  rendered together on the same display.

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
#define USE_SHADERS true

#define __wf_private__
#include "config.h"
#include <sys/time.h>
#include <gtk/gtk.h>
#include "gdk/x11/gdkx.h"
#include "waveform/actor.h"
#include "test/common.h"

#define WIDTH 256.
#define HEIGHT 256.
#define VBORDER 8

GtkWidget*       canvas    = NULL;
AGlScene*        scene1    = NULL;
AGlRootActor*    scene2    = NULL;
WaveformContext* wfc       = NULL;
Waveform*        w2        = NULL;
WaveformActor*   a[4]      = {NULL,};
float            zoom      = 1.0;
float            vzoom     = 1.0;
gpointer         tests[]   = {};

static void start_zoom        (float);
uint64_t    get_time          ();

KeyHandler
	zoom_in,
	zoom_out,
	vzoom_up,
	vzoom_down,
	scroll_left,
	scroll_right,
	toggle_animate;

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
	{XK_Delete,     NULL},
	{0},
};


static GdkGLContext* on_create_context (GtkGLArea*, GdkGLContext*);

struct _GdkX11GLContextGLX
{
  GObject parent_instance;
  cairo_region_t *old_updated_area[2];
  GLXContext glx_context;
};


static GdkGLContext*
on_create_context (GtkGLArea* gl_area, GdkGLContext* context)
{
	if ((scene1->gl.gdk.context = agl_get_gl_context ())) {
		g_object_ref (scene1->gl.gdk.context);
		g_object_ref (scene1->gl.gdk.context);
	} else {
		GdkGLContext* context = gdk_surface_create_gl_context (gtk_native_get_surface (gtk_widget_get_native ((GtkWidget*)gl_area)), NULL);
		gdk_gl_context_realize (context, NULL);
		scene1->gl.gdk.context = context;
		scene2->gl.gdk.context = context;
		agl_set_gl_context (scene1->gl.gdk.context);
	}
	gdk_gl_context_make_current (scene1->gl.gdk.context);
	scene1->drawable = scene2->drawable = glXGetCurrentDrawable();
	agl_gl_init();
	GdkDisplay* display = gdk_gl_context_get_display (scene1->gl.gdk.context);
  	agl_get_instance()->xdisplay = gdk_x11_display_get_xdisplay (display);
	scene1->glxcontext = scene2->glxcontext = ((struct _GdkX11GLContextGLX*)scene1->gl.gdk.context)->glx_context;

	agl_actor__init((AGlActor*)scene1);
	agl_actor__init((AGlActor*)scene2);

	return scene1->gl.gdk.context;
}


static gboolean
render (GtkGLArea* area, GdkGLContext* context)
{
	glClearColor (0, 0, 0, 1);
	glClear (GL_COLOR_BUFFER_BIT);
 
	agl_actor__paint((AGlActor*)scene1);
	agl_actor__paint((AGlActor*)scene2);

	return TRUE;
}


static void
allocate (GtkWidget* widget, int width, int height, gpointer data)
{
#if 0
	// optimise drawing by telling the canvas which area is visible
	wf_context_set_viewport(wfc, &(WfViewPort){0, 0, width, height});
#endif

	((AGlActor*)scene1)->region = (AGlfRegion){0., 0., width, height};
	((AGlActor*)scene2)->region = (AGlfRegion){0., 0., width, height};

	start_zoom(zoom);
}


static void
activate (GtkApplication* app, gpointer user_data)
{
	set_log_handlers();

	wf_debug = 0;

	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), 320, 128);

	GtkWidget* box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_window_set_child (GTK_WINDOW (window), box);

	GtkWidget* widget = gtk_gl_area_new();
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_widget_set_vexpand (widget, TRUE);
	gtk_box_append (GTK_BOX(box), widget);

	g_signal_connect (widget, "resize", G_CALLBACK (allocate), NULL);
	g_signal_connect (widget, "create-context", G_CALLBACK (on_create_context), NULL);
	g_signal_connect ((GtkGLArea*)widget, "render", G_CALLBACK (render), NULL);

	scene1 = ({
		AGlScene* scene = (AGlScene*)agl_actor__new_root (CONTEXT_TYPE_GTK);
		scene->gl.gdk.widget = widget;
		scene->user_data = widget;
		scene->selected = (AGlActor*)scene1;
		scene;
	});

	scene2 = ({
		AGlScene* scene = (AGlScene*)agl_actor__new_root (CONTEXT_TYPE_GTK);
		scene->gl.gdk.widget = widget;
		scene->user_data = widget;
		scene->selected = (AGlActor*)scene2;
		scene;
	});

	void _agl_gtk_area_queue_render (AGlScene* scene)
	{
		gtk_widget_queue_draw (scene->gl.gdk.widget);
	}
	scene1->queue_draw = _agl_gtk_area_queue_render;

	char* filename = find_wav("mono_0:10.wav");
	Waveform* wav = waveform_load_new(filename);
	g_free(filename);

	agl_get_instance()->pref_use_shaders = USE_SHADERS;

	wfc = wf_context_new((AGlActor*)scene1);

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
		agl_actor__add_child((AGlActor*)(i < 2 ? scene1 : scene2), (AGlActor*)a[i]);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	add_key_handlers_gtk ((GtkWindow*)window, NULL, (AGlKey*)&keys);

	gtk_widget_show (window);
}


static void
start_zoom (float target_zoom)
{
	// when zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	PF0;
	zoom = MAX(0.1, target_zoom);

	AGlActor* parent = (AGlActor*)scene1;

	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if (a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			0.,
			i * parent->region.y2 / 4.,
			WIDTH * target_zoom,
			parent->region.y2 / 4. * 0.95
		});
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
	zoom = MIN(vzoom, 100.0);
	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if (a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
vzoom_down (gpointer _)
{
	vzoom /= 1.1;
	zoom = MAX(vzoom, 1.0);
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
	uint64_t get_time ()
	{
		struct timeval start;
		gettimeofday(&start, NULL);
		return start.tv_sec * 1000 + start.tv_usec / 1000;
	}

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
			if (v > 16.0) v = 1.0;
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

#include "test/_gtk.c"
