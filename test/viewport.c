/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2018 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
* | test/viewport.c                                                      |
* | Demonstrate different use cases for AGlActor 'scrollable' property   |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <GL/gl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "agl/actor.h"
#include "agl/shader.h"
#include "waveform/utils.h"
#include "test/common2.h"

struct
{
	int timeout;
} app;

#define GL_WIDTH 256.0
#define GL_HEIGHT 256.0
#define VBORDER 8

GdkGLConfig* glconfig       = NULL;
static bool  gl_initialised = false;
GtkWidget*   canvas         = NULL;
AGlScene*    scene          = NULL;
gpointer     tests[]        = {};
static AGl* agl = NULL;

static AGlActor* container          (WaveformActor*);
static AGlActor* actor_with_viewport(WaveformActor*);
static void      on_canvas_realise  (GtkWidget*, gpointer);
static void      on_allocate        (GtkWidget*, GtkAllocation*, gpointer);

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";


int
main (int argc, char *argv[])
{
	agl = agl_get_instance();
	set_log_handlers();

	wf_debug = 0;

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(3000, (gpointer)exit, NULL);
				break;
		}
	}

	gtk_init(&argc, &argv);
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		gerr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
	}

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	canvas = gtk_drawing_area_new();
#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus     (canvas, true);
#endif
	gtk_widget_set_size_request  (canvas, 480, 320);
	gtk_widget_set_gl_capability (canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events        (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);

	scene = (AGlScene*)agl_actor__new_root(canvas);

	AGlActor* _container = agl_actor__add_child((AGlActor*)scene, container(NULL));
	AGlActor* a = agl_actor__add_child((AGlActor*)_container, actor_with_viewport(NULL));
	AGlActor* b = agl_actor__add_child((AGlActor*)_container, actor_with_viewport(NULL));
	AGlActor* c = agl_actor__add_child((AGlActor*)_container, actor_with_viewport(NULL));
	AGlActor* d = agl_actor__add_child((AGlActor*)_container, actor_with_viewport(NULL));
	int w = 50;
	int h = 50;
	int y = 25;
	a->region = (AGlfRegion){10, y, 10 + w, y + h};
	y += h + 20;

	// the leftmost part of the scrollable area is visible
	b->region = (AGlfRegion){10, y, 10 + w, y + h};
	b->scrollable = (AGliRegion){0, 0, w * 2, h};
	y += h + 20;

	// scroll one page to the right
	// - the visible region is unchanged, but the scrollable area has moved to the left
	c->region = (AGlfRegion){10, y, 10 + w, y + h};
	c->scrollable = (AGliRegion){-w, 0, w, h};
	y += h + 20;

	d->region = (AGlfRegion){10, y, 10 + w, y + h};
	d->scrollable = (AGliRegion){2 * w, 0, w, h}; // viewport does not overlap the region, so should be invisible

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);

	gtk_widget_show_all(window);

	gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		switch(event->keyval){
			case 61:
				break;
			case 45:
				break;
			case KEY_Left:
			case KEY_KP_Left:
				dbg(0, "left");
				break;
			case KEY_Right:
			case KEY_KP_Right:
				dbg(0, "right");
				break;
			case KEY_Up:
			case KEY_KP_Up:
				dbg(0, "up");
				break;
			case KEY_Down:
			case KEY_KP_Down:
				dbg(0, "down");
				break;
			case (char)'a':
				break;
			case GDK_KP_Enter:
				break;
			case 113:
				exit(EXIT_SUCCESS);
				break;
			case GDK_Delete:
				break;
			default:
				dbg(0, "%i", event->keyval);
				break;
		}
		return TRUE;
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), NULL);

	gboolean window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


static gboolean canvas_init_done = false;
static void
on_canvas_realise(GtkWidget* _canvas, gpointer user_data)
{
	PF;
	if(canvas_init_done) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	gl_initialised = true;

	canvas_init_done = true;

	/*
	int n_frames = waveform_get_n_frames(w1);

	WfSampleRegion region[] = {
		{0,            n_frames    },
		{0,            n_frames / 2},
		{n_frames / 4, n_frames / 16},
		{n_frames / 2, n_frames / 2},
	};

	uint32_t colours[4][2] = {
		{0x66eeffff, 0x0000ffff}, // blue
		{0xffffff77, 0x0000ffff}, // grey
		{0xffdd66ff, 0x0000ffff}, // orange
		{0x66ff66ff, 0x0000ffff}, // green
	};

	int i; for(i=0;i<G_N_ELEMENTS(a);i++){
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)(a[i] = wf_canvas_add_new_actor(wfc, w1)));

		wf_actor_set_region (a[i], &region[i]);
		wf_actor_set_colour (a[i], colours[i][0]);
		wf_actor_set_z      (a[i], i * dz);
	}
	*/

	on_allocate(canvas, &canvas->allocation, user_data);
}


static void
on_allocate(GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	((AGlActor*)scene)->region.x2 = allocation->width;
	((AGlActor*)scene)->region.y2 = allocation->height;

	if(!gl_initialised) return;
}


AGlActor*
container(WaveformActor* wf_actor)
{
	bool container_paint(AGlActor* actor)
	{
		agl_print(10,  10, 0, 0xaaaaaaff, "No viewport");
		agl_print(10,  80, 0, 0xaaaaaaff, "Viewport to right");
		agl_print(10, 150, 0, 0xaaaaaaff, "Viewport to left");
		agl_print(10, 220, 0, 0xaaaaaaff, "Outside viewport (invisible)");
		return true;
	}

	void container_set_state(AGlActor* actor)
	{
		//((PlainShader*)actor->program)->uniform.colour = 0xaaaaaaff;
	}

	AGlActor* actor = AGL_NEW(AGlActor,
#ifdef AGL_DEBUG_ACTOR
		.name = "Container",
#endif
		//.program = (AGlShader*)agl->shaders.plain,
		.region = {45, 0, 100, 100},
		.set_state = container_set_state,
		.paint = container_paint,
	);

	return actor;
}


AGlActor*
actor_with_viewport(WaveformActor* wf_actor)
{
	agl_set_font_string("Sans 10");

	bool viewport_paint(AGlActor* actor)
	{
		// coords here are relative to the start of the world region
		// -but we only want to draw inside the Region

		// viewable region
		int rx = -actor->scrollable.x1;
		int ry = -actor->scrollable.y1;
		agl_rect(rx, ry, agl_actor__width(actor), agl_actor__height(actor));

		// viewport outline
		((PlainShader*)actor->program)->uniform.colour = 0x6699ff99;
		agl_use_program(actor->program);
		agl_box(1, 0, 0, actor->scrollable.x2 - actor->scrollable.x1, actor->scrollable.y2 - actor->scrollable.y1);

		// normal contents, cropped to viewable region
		agl_enable_stencil(rx, ry, agl_actor__width(actor), agl_actor__height(actor));
		agl_print(2, 2, 0, 0xff9933ff, "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
		agl_disable_stencil();

		return true;
	}

	void viewport_set_state(AGlActor* actor)
	{
		agl_enable(AGL_ENABLE_BLEND);
		((PlainShader*)actor->program)->uniform.colour = 0x00ff0099;
	}

	AGlActor* actor = AGL_NEW(AGlActor,
#ifdef AGL_DEBUG_ACTOR
		.name = "Viewport",
#endif
		.program = (AGlShader*)agl->shaders.plain,
		//.region = {0, 0, 100, 100},
		.set_state = viewport_set_state,
		.paint = viewport_paint,
	);

	return actor;
}


