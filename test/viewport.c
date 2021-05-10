/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2021 Tim Orford <tim@orford.org>                  |
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

#include "config.h"
#include <getopt.h>
#include <sys/time.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#include <gdk/gdkkeysyms.h>
#include "agl/gtk.h"
#include "agl/shader.h"
#include "agl/behaviours/cache.h"
#include "waveform/actor.h"
#include "test/common2.h"

struct
{
	int timeout;
} app;

#define GL_WIDTH 256.0
#define GL_HEIGHT 256.0
#define VBORDER 8

GtkWidget* canvas   = NULL;
AGlScene*  scene    = NULL;
gpointer   tests[]  = {};

static AGl* agl = NULL;

static AGlActor* container           (WaveformActor*);
static AGlActor* actor_with_viewport (WaveformActor*);
static void      on_canvas_realise   (GtkWidget*, gpointer);
static void      on_allocate         (GtkWidget*, GtkAllocation*, gpointer);

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";


int
main (int argc, char* argv[])
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

	GdkGLConfig* glconfig = NULL;
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		perr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
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

	// repeat the above 4 but enable caching
	AGlActor* e = agl_actor__add_child((AGlActor*)_container, actor_with_viewport(NULL));
	e->behaviours[0] = cache_behaviour();
	AGlActor* f = agl_actor__add_child((AGlActor*)_container, actor_with_viewport(NULL));
	f->behaviours[0] = cache_behaviour();
	AGlActor* g = agl_actor__add_child((AGlActor*)_container, actor_with_viewport(NULL));
	g->behaviours[0] = cache_behaviour();

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
	d->scrollable = (AGliRegion){w + 10, 0, .x2 = 2 * w + 10, h}; // viewport does not overlap the region, so content should be invisible

	{
		int y = 25;

		y += h + 20;

		// the leftmost part of the scrollable area is visible
		e->region = (AGlfRegion){210, y, 210 + w, y + h};
		e->scrollable = (AGliRegion){0, 0, w * 2, h};
		y += h + 20;

		// scroll one page to the right
		f->region = (AGlfRegion){210, y, 210 + w, y + h};
		f->scrollable = (AGliRegion){-w, 0, w, h};
		y += h + 20;

		g->region = (AGlfRegion){210, y, 210 + w, y + h};
		g->scrollable = (AGliRegion){w + 10, 0, 2 * w + 10, h};
		y += h + 20;
	}

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);

	gtk_widget_show_all(window);

	gboolean key_press (GtkWidget* widget, GdkEventKey* event, gpointer user_data)
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

	gboolean window_on_delete (GtkWidget* widget, GdkEvent* event, gpointer user_data)
	{
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


static void
on_canvas_realise (GtkWidget* _canvas, gpointer user_data)
{
	PF;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	on_allocate(canvas, &canvas->allocation, user_data);
}


static void
on_allocate (GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	((AGlActor*)scene)->region.x2 = allocation->width;
	((AGlActor*)scene)->region.y2 = allocation->height;
}


AGlActor*
container (WaveformActor* wf_actor)
{
	bool container_paint (AGlActor* actor)
	{
		agl_print(10,  10, 0, 0xaaaaaaff, "No viewport");
		agl_print(10,  80, 0, 0xaaaaaaff, "Viewport to right");
		agl_print(10, 150, 0, 0xaaaaaaff, "Viewport to left");
		agl_print(10, 220, 0, 0xaaaaaaff, "Contents outside viewport (invisible)");

		agl_print(210, 10, 0, 0xaaaaaaff, "With caching");

		return true;
	}

	void container_set_state (AGlActor* actor)
	{
		//((PlainShader*)actor->program)->uniform.colour = 0xaaaaaaff;
	}

	AGlActor* actor = AGL_NEW(AGlActor,
		.name = "Container",
		//.program = (AGlShader*)agl->shaders.plain,
		.region = {45, 0, 100, 100},
		.set_state = container_set_state,
		.paint = container_paint,
	);

	return actor;
}


AGlActor*
actor_with_viewport (WaveformActor* wf_actor)
{

	bool viewport_paint (AGlActor* actor)
	{
		agl_set_font_string("Sans 10");

		// In order for coords to be independant of scroll position,
		// coords here are relative to the start of the whole scrollable area
		// -but we only want to draw inside the viewport (actor->region)

		// viewable region
		int rx = -actor->scrollable.x1;
		int ry = -actor->scrollable.y1;
		agl_rect(rx, ry, agl_actor__width(actor), agl_actor__height(actor));

		// outline of whole scrollable region
		agl_set_colour_uniform (&actor->program->uniforms[PLAIN_COLOUR], 0x6699ff99);
		agl_box(1, 0, 0, actor->scrollable.x2 - actor->scrollable.x1, actor->scrollable.y2 - actor->scrollable.y1);

		// normal contents, cropped to viewable region
		if(actor->behaviours[0]){
			agl_push_clip(rx * 0, ry * 0, agl_actor__width(actor), agl_actor__height(actor));
		}else{
			// temporary while glTranslatef is still being used
			AGliPt offset = agl_actor__find_offset(actor);
			agl_push_clip(offset.x + rx, offset.y + ry, agl_actor__width(actor), agl_actor__height(actor));
		}
		agl_print(2, 2, 0, 0xff9933ff, "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
		agl_pop_clip();

		return true;
	}

	void viewport_set_state (AGlActor* actor)
	{
		agl_enable(AGL_ENABLE_BLEND);
		SET_PLAIN_COLOUR(agl->shaders.plain, 0x00ff0099);
	}

	AGlActor* actor = agl_actor__new(AGlActor,
		.name = "Viewport",
		.program = agl->shaders.plain,
		.set_state = viewport_set_state,
		.paint = viewport_paint,
	);

	return actor;
}
