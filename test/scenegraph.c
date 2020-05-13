/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
# define GLX_GLXEXT_PROTOTYPES
#include "gdk/gdk.h"
#include "agl/ext.h"
#define __wf_private__ // for dbg
#include "waveform/waveform.h"
#include "agl/actor.h"
#include "waveform/actors/background.h"
#include "waveform/actors/plain.h"
#define __glx_test__
#include "test/common2.h"

#undef SHOW_2ND_CHILD
#define SHOW_2ND_CHILD

uint32_t colour0 = 0xff0000ff; // bg
uint32_t colour1 = 0x7fff007f;
#ifdef SHOW_2ND_CHILD
uint32_t colour2 = 0x00ff003f;
#endif

extern char* basename(const char*);

static GLboolean print_info = GL_FALSE;

static AGlActor* cache_actor   (void*);
static AGlActor* text_actor    (void*);
static AGlActor* group         (void*);
static AGlActor* cached_group  (void*);
#ifdef SHOW_2ND_CHILD
AGlActor* plain2_actor         (void*);
#endif

static void scene_needs_redraw (AGlScene* scene, gpointer _){ scene->gl.glx.needs_draw = True; }

static AGlScene* scene = NULL;
struct {AGlActor *bg, *grp, *l1, *l2, *g2, *ga2, *gb2, *text; } layers = {0,};

static KeyHandler
	toggle_cache,
	nav_up,
	nav_down,
	zoom_in,
	zoom_out;

Key keys[] = {
	{XK_c,           toggle_cache},
	{XK_Up,          nav_up},
	{XK_Down,        nav_down},
	{XK_equal,       zoom_in},
	{XK_KP_Add,      zoom_in},
	{XK_minus,       zoom_out},
	{XK_KP_Subtract, zoom_out},
};

static const struct option long_options[] = {
	{ "help",             0, NULL, 'h' },
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "nh";

static const char* const usage =
	"Usage: %s [OPTIONS]\n\n"
	"\n"
	"Options:\n"
	"  --help\n"
	"  -info                   Display GL information\n"
	"  -swap N                 Swap no more than once per N vertical refreshes\n"
	"  -forcegetrate           Try to use glXGetMscRateOML function\n"
	"\n";


int
main (int argc, char *argv[])
{
	int width = 300, height = 300;

	int i; for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-info") == 0) {
			print_info = GL_TRUE;
		}
		else if (strcmp(argv[i], "-swap") == 0 && i + 1 < argc) {
//			swap_interval = atoi( argv[i+1] );
//			do_swap_interval = GL_TRUE;
			i++;
		}
		else if (strcmp(argv[i], "-forcegetrate") == 0) {
			/* This option was put in because some DRI drivers don't support the
			 * full GLX_OML_sync_control extension, but they do support
			 * glXGetMscRateOML.
			 */
//			force_get_rate = GL_TRUE;
		}
	}

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'h':
				printf(usage, basename(argv[0]));
				exit(EXIT_SUCCESS);
				break;
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

	scene = (AGlScene*)agl_actor__new_root_(CONTEXT_TYPE_GLX);
	scene->draw = scene_needs_redraw;

	AGlWindow* window = agl_make_window(dpy, "waveformscenegraphtest", width, height, scene);

	g_main_loop_new(NULL, true);

	{
		agl_actor__add_child((AGlActor*)scene, layers.grp = group(NULL));
		layers.grp->region = (AGlfRegion){10, 22, 60, 72};

		agl_actor__add_child((AGlActor*)scene, layers.l2 = plain_actor(NULL));
		layers.l2->colour = 0x9999ff99;
		layers.l2->region = (AGlfRegion){40, 52, 90, 102};

		// now show the same 2 squares again, but wrapped in a caching actor

		agl_actor__add_child((AGlActor*)scene, layers.g2 = cached_group(NULL));
		layers.g2->region = (AGlfRegion){10, 72, 60, 122};

		//------------------------------

		// 2nd pair of blocks, now with a background

		//agl_actor__add_child((AGlActor*)scene, layers.bg = background_actor(NULL));
		agl_actor__add_child((AGlActor*)scene, layers.bg = plain_actor(NULL));
		layers.bg->colour = colour0;
		layers.bg->region = (AGlfRegion){100, 0, 300, 300};

		agl_actor__add_child((AGlActor*)scene, layers.ga2 = group(NULL));
		layers.ga2->region = (AGlfRegion){110, 22, 160, 72};

		agl_actor__add_child((AGlActor*)scene, layers.gb2 = cached_group(NULL));
		layers.gb2->region = (AGlfRegion){110, 72, 160, 122};

		//------------------------------

		agl_actor__add_child((AGlActor*)scene, layers.text = text_actor(NULL));
		layers.text->region = (AGlfRegion){0, 0, 90, 218};
	}

	add_key_handlers(keys);

	event_loop(dpy);

	agl_window_destroy(dpy, &window);
	XCloseDisplay(dpy);

	return 0;
}


static AGlActor*
text_actor (void* _)
{
	bool text_paint (AGlActor* actor)
	{
		agl_print(10, 7, 0, 0xffffffff, "You should see 2 overlapping squares");
		agl_print(10, 108, 0, 0xffffffff, "The two squares below are from an fbo cache.");
		agl_print(10, 122, 0, 0xffffffff, "They should be the same as above");

		return true;
	}

	void text_init (AGlActor* actor)
	{
		agl_set_font_string("Roboto 8");
	}

	return AGL_NEW(AGlActor,
		.name = "Text",
		.init = text_init,
		.paint = text_paint,
	);
}


static AGlActor*
cache_actor (void* _)
{
	void cache_set_state (AGlActor* actor)
	{
	}

	bool cache_paint (AGlActor* actor)
	{
		return true;
	}

	void cache_init (AGlActor* actor)
	{
#ifdef AGL_ACTOR_RENDER_CACHE
		actor->fbo = agl_fbo_new(agl_actor__width(actor), agl_actor__height(actor), 0, 0);
		actor->cache.enabled = true;
#endif
	}

	return AGL_NEW(AGlActor,
		.name = "Cache",
		.init = cache_init,
		.paint = cache_paint,
		.set_state = cache_set_state
	);
}


#if 0
static int
_red (uint32_t rgba, uint32_t b)
{
	double _r = (rgba & 0xff000000) >> 24;
	double _a = (rgba & 0x000000ff);

	double _a2 = (b & 0x000000ff);

	return (int)(_r * (_a - _a2) / 256.0);
}
#endif


static bool
read_values (gpointer _)
{
	AGlActor* actor = layers.gb2;
	AGlFBO* fbo = actor->fbo;
	guchar data[128] = {0,};

	printf("cache=%i %i\n", actor->cache.enabled, actor->cache.valid);
	printf("0x%x\n", (int)(0.389376 * 256));
	dbg(0, "bgc: %08x", colour0);
	dbg(0, "fg1: %08x", colour1);
#ifdef SHOW_2ND_CHILD
	dbg(0, "fg2: %08x", colour2);
#endif

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo->id);
	glReadPixels(25, fbo->width / 2, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, data);
	//uint32_t expected = _red(colour1, colour2) + _red(colour2, 0);
	//dbg(0, "fbe: %02x%02x%02x%02x", expected, 0, 0, 0);
	dbg(0, "fbo: %02x%02x%02x%02x", (int)data[0], (int)data[1], (int)data[2], (int)data[3]);

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	glReadPixels(130, 180, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, data);
	dbg(0, "chd: %02x%02x%02x%02x %s", (int)data[0], (int)data[1], (int)data[2], (int)data[3], actor->cache.valid ? "<-- brighter when from fbo" : "");

	glReadPixels(130, 250, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, data);
	dbg(0, "dir: %02x%02x%02x%02x", (int)data[0], (int)data[1], (int)data[2], (int)data[3]);

	return G_SOURCE_REMOVE;
}


static void
toggle_cache (gpointer user_data)
{
	if(layers.g2){
		layers.g2->cache.enabled = !layers.g2->cache.enabled;
		agl_actor__invalidate(layers.g2);
	}

	layers.gb2->cache.enabled = !layers.gb2->cache.enabled;
	agl_actor__invalidate(layers.gb2);

	g_timeout_add(50, read_values, NULL);
}


static void
nav_up (gpointer user_data)
{
	PF0;
}


static void
nav_down (gpointer user_data)
{
	PF0;
}


static void
zoom_in (gpointer user_data)
{
}


static void
zoom_out (gpointer user_data)
{
}


#ifdef SHOW_2ND_CHILD
	static void plain2_set_state(AGlActor* actor)
	{
		((PlainShader*)actor->program)->uniform.colour = actor->colour;
	}

	static bool plain2_paint(AGlActor* actor)
	{
		agl_rect(
			0,
			0,
			agl_actor__width(actor),
			agl_actor__height(actor)
		);

		return true;
	}

AGlActor*
plain2_actor (void* view)
{
	AGlActor* actor = WF_NEW(AGlActor,
		.name = "plain",
		.region = {
			.x2 = 1, .y2 = 1 // must have size else will not be rendered
		},
		.set_state = plain2_set_state,
		.paint = plain2_paint,
		.program = (AGlShader*)agl_get_instance()->shaders.plain,
	);

	return actor;
}
#endif


AGlActor*
group (void* _)
{
	AGlActor* g = group_actor(NULL);

	AGlActor* a;
	agl_actor__add_child(g, a = plain_actor(NULL));
	a->colour = colour1;
	//layers.l1->region = (AGliRegion){10, 22, 60, 72};
	a->region = (AGlfRegion){0, 0, 50, 50};

#ifdef SHOW_2ND_CHILD
	AGlActor* b;
	agl_actor__add_child(g, b = plain_actor(NULL));
	b->colour = colour2;
	//layers.l1b->region = (AGliRegion){10, 22, 60, 72};
	b->region = (AGlfRegion){0, 0, 50, 50};
#endif

	return g;
}


AGlActor*
cached_group (void* _)
{
	AGlActor* g = cache_actor(NULL);
	g->region = (AGlfRegion){10, 72, 60, 122};

	AGlActor* a = plain_actor(NULL);
	agl_actor__add_child(g, a);
	a->colour = colour1;
	a->region = (AGlfRegion){0, 0, 50, 50};

#ifdef SHOW_2ND_CHILD
	AGlActor* b = plain2_actor(NULL);
	agl_actor__add_child(g, b);
	b->colour = colour2;
	b->region = (AGlfRegion){0, 0, 50, 50};
#endif

#if 0
	agl_actor__add_child(layers.l3, layers.l5 = plain_actor(NULL));
	layers.l5->colour = 0x9999ff99;
	layers.l5->region = (AGliRegion){30, 30, 80, 80};
#endif

	return g;
}
