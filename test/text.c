/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2020-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*/
#include "config.h"
#include <getopt.h>
#ifndef USE_EPOXY
# define GLX_GLXEXT_PROTOTYPES
#endif
#include "gdk/gdk.h"
#include "agl/ext.h"
#include "agl/debug.h"
#include "agl/text/text_node.h"
#include "waveform/debug.h"
#define __glx_test__
#include "test/common2.h"

extern void on_window_resize (Display*, AGlWindow*, int, int);

static AGlRootActor* scene = NULL;

struct {
	AGlActor* text1;
	AGlActor* text2;
} layers = {0,};

static KeyHandler
	nav_up,
	nav_down,
	zoom_in,
	zoom_out;

Key keys[] = {
	{XK_Up,          nav_up},
	{XK_Down,        nav_down},
	{XK_equal,       zoom_in},
	{XK_KP_Add,      zoom_in},
	{XK_minus,       zoom_out},
	{XK_KP_Subtract, zoom_out},
	{XK_Escape,      (KeyHandler*)exit},
	{XK_q,           (KeyHandler*)exit},
};

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";


int
main (int argc, char* argv[])
{
	wf_debug = 1;

	const int width = 400, height = 200;

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(3000, (gpointer)exit, NULL);
				break;
		}
	}

	Display* dpy = XOpenDisplay(NULL);
	if (!dpy) {
		printf("Error: couldn't open display %s\n", XDisplayName(NULL));
		return -1;
	}

	AGlWindow* window = agl_make_window(dpy, "Text test", width, height);
	scene = window->scene;
	XMapWindow(dpy, window->window);

	g_main_loop_new(NULL, true);

	agl_actor__add_child((AGlActor*)scene, layers.text1 = text_node(NULL));
	text_node_set_text((TextNode*)layers.text1, g_strdup("H"));
	((TextNode*)layers.text1)->font.size = 12;
	layers.text1->colour = 0xff9900ff;

	agl_actor__add_child((AGlActor*)scene, layers.text2 = text_node(NULL));
	text_node_set_text((TextNode*)layers.text2, g_strdup("XeXe"));
	((TextNode*)layers.text2)->font.size = 120;
	layers.text2->colour = 0xff0000ff;

	AGlActor* debug_actor = wf_debug_actor (NULL);
	debug_actor->region = (AGlfRegion){145., 5., 195., 65.};
	((DebugActor*)debug_actor)->target = layers.text1;
	agl_actor__add_child((AGlActor*)scene, debug_actor);

	debug_actor = wf_debug_actor (NULL);
	debug_actor->region = (AGlfRegion){200., 5., 250., 65.};
	((DebugActor*)debug_actor)->target = layers.text2;
	agl_actor__add_child((AGlActor*)scene, debug_actor);

	void set_size (AGlActor* actor)
	{
		layers.text1->region = (AGlfRegion){.x2 = 60, .y2 = 30};

		// set a small size to demonstrate clipping
		layers.text2->region = (AGlfRegion){20, 40, .x2 = 120, .y2 = 180};
	}

	layers.text1->set_size = set_size;

	on_window_resize(NULL, window, width, height);

	add_key_handlers(keys);

	event_loop(dpy);

	agl_window_destroy(dpy, &window);
	XCloseDisplay(dpy);

	return 0;
}


static void
nav_up (gpointer user_data)
{
}


static void
nav_down (gpointer user_data)
{
}


static void
zoom_in (gpointer user_data)
{
}


static void
zoom_out (gpointer user_data)
{
}
