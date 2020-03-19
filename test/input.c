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
# define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include "gdk/gdk.h"
#include "agl/ext.h"
#include "agl/debug.h"
#include "agl/behaviours/key.h"
#include "agl/text/text_input.h"
#define __glx_test__
#include "test/common2.h"

extern void on_window_resize (Display*, AGlWindow*, int, int);

static AGlRootActor* scene = NULL;

struct {
	AGlActor* input;
} layers = {0,};

static ActorKeyHandler
	quit;

ActorKey keys[] = {
	{XK_q,          quit},
	{XK_Escape,     quit},
};

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";


int
main (int argc, char *argv[])
{
	wf_debug = 0;

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

	scene = (AGlRootActor*)agl_actor__new_root_(CONTEXT_TYPE_GLX);

	AGlWindow* window = agl_make_window(dpy, "Text test", width, height, scene);
	XMapWindow(dpy, window->window);

	g_main_loop_new(NULL, true);

	agl_actor__add_child((AGlActor*)scene, layers.input = text_input(NULL));
	layers.input->region = (AGlfRegion){15, 15, .x2 = 380, .y2 = 120};
	text_input_set_text((TextInput*)layers.input, g_strdup("Hello world"));
	agl_observable_set(((TextInput*)layers.input)->font, 32);

	scene->selected = layers.input;

	#define KEYS(A) ((KeyBehaviour*)((AGlActor*)A)->behaviours[0])

	((AGlActor*)scene)->behaviours[0] = key_behaviour();
	KEYS(scene)->keys = &keys;
	key_behaviour_init(((AGlActor*)scene)->behaviours[0], (AGlActor*)scene);

	on_window_resize(NULL, window, width, height);

	event_loop(dpy);

	agl_window_destroy(dpy, &window);
	XCloseDisplay(dpy);

	return 0;
}


static bool
quit (AGlActor* user_data, GdkModifierType modifiers)
{
	extern bool running;
	running = false;
	return AGL_HANDLED;
}
