/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform                                     |
 | https://github.com/ayyi/libwaveform                                  |
 | copyright (C) 2012-2023 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <getopt.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#ifndef USE_EPOXY
# define GLX_GLXEXT_PROTOTYPES
#endif
#include "agl/ext.h"
#include "agl/x11.h"
#include "agl/behaviours/simple_key.h"
#include "agl/behaviours/fullsize.h"
#include "actors/background.h"
#define __wf_private__
#include "wf/waveform.h"
#include "waveform/actor.h"
#define __glx_test__
#include "test/common2.h"

static GLboolean print_info = GL_FALSE;

#define BENCHMARK
#define NUL '\0'

extern PFNGLXGETFRAMEUSAGEMESAPROC get_frame_usage;

struct {
	WaveformActor* wa;
} layers = {0,};

static KeyHandler
	nav_left,
	nav_right,
	nav_up,
	nav_down,
	zoom_in,
	zoom_out;

AGlKey keys[] = {
	{XK_Left,        nav_left},
	{XK_Right,       nav_right},
	{XK_Up,          nav_up},
	{XK_Down,        nav_down},
	{XK_equal,       zoom_in},
	{XK_KP_Add,      zoom_in},
	{XK_minus,       zoom_out},
	{XK_KP_Subtract, zoom_out},
};

static const struct option long_options[] = {
	{ "autoquit", 0, NULL, 'q' },
};

static const char* const short_options = "q";


int
main (int argc, char* argv[])
{
	int width = 400, height = 160;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-info") == 0) {
			print_info = GL_TRUE;
		}
		else if (strcmp(argv[i], "-swap") == 0 && i + 1 < argc) {
			//swap_interval = atoi( argv[i+1] );
			//do_swap_interval = GL_TRUE;
			i++;
		}
		else if (strcmp(argv[i], "-forcegetrate") == 0) {
			/* This option was put in because some DRI drivers don't support the
			 * full GLX_OML_sync_control extension, but they do support
			 * glXGetMscRateOML.
			 */
			//force_get_rate = GL_TRUE;
		}
		else if (strcmp(argv[i], "-help") == 0) {
			printf("Usage:\n");
			printf("  glx [options]\n");
			printf("Options:\n");
			printf("  -help                   Print this information\n");
			printf("  -info                   Display GL information\n");
			printf("  -swap N                 Swap no more than once per N vertical refreshes\n");
			printf("  -forcegetrate           Try to use glXGetMscRateOML function\n");
			return 0;
		}
	}

	int opt;
	while ((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch (opt) {
			case 'q':
				g_timeout_add(3000, (gpointer)exit, NULL);
				break;
		}
	}

	AGlWindow* window = agl_window("Waveform glx test", -1, -1, width, height, 0);

	agl_actor__add_child((AGlActor*)window->scene, background_actor(NULL));

	char* filename = find_wav("mono_0:10.wav");
	Waveform* w = waveform_load_new(filename);
	g_free(filename);

	WaveformContext* wfc = wf_context_new((AGlActor*)window->scene);
	wfc->samples_per_pixel = waveform_get_n_frames(w) / 400.0;

	layers.wa = (WaveformActor*)agl_actor__add_child((AGlActor*)window->scene, (AGlActor*)wf_context_add_new_actor(wfc, w));
	((AGlActor*)layers.wa)->colour = 0xaaaaaaff;
	agl_actor__add_behaviour((AGlActor*)layers.wa, fullsize());

	wf_actor_set_region(layers.wa, &(WfSampleRegion){0, 441000});

	#define KEYS(A) ((SimpleKeyBehaviour*)((AGlActor*)A)->behaviours[0])
	AGlActor* root = (AGlActor*)window->scene;
	window->scene->selected = root;
	root->behaviours[0] = simple_key_behaviour();
	KEYS(root)->keys = &keys;
	simple_key_behaviour_init(root->behaviours[0], root);

	GMainLoop* mainloop = agl_main_loop_new();
	g_main_loop_run(mainloop);

	agl_window_destroy(&window);
	XCloseDisplay(dpy);

	return 0;
}


static void
nav_left (gpointer user_data)
{
	WaveformContext* wfc = layers.wa->context;

	int64_t n_frames_visible = agl_actor__width(((AGlActor*)layers.wa)) * wfc->samples_per_pixel / wfc->zoom->value.f;

	int64_t start_frame = CLAMP(
		wfc->start_time->value.b - n_frames_visible / 10,
		0,
		(int64_t)(waveform_get_n_frames(layers.wa->waveform) - MAX(10, n_frames_visible))
	);

	wf_context_set_start(wfc, start_frame);

	wf_actor_set_region(layers.wa, &(WfSampleRegion){
		start_frame,
		n_frames_visible
	});
}


static void
nav_right (gpointer user_data)
{
	WaveformContext* wfc = layers.wa->context;

	int64_t n_frames_visible = agl_actor__width(((AGlActor*)layers.wa)) * wfc->samples_per_pixel / wfc->zoom->value.f;

	int64_t start_frame = CLAMP(
		wfc->start_time->value.b + n_frames_visible / 10,
		0,
		(int64_t)(waveform_get_n_frames(layers.wa->waveform) - MAX(10, n_frames_visible))
	);

	wf_context_set_start(wfc, start_frame);

	wf_actor_set_region(layers.wa, &(WfSampleRegion){
		start_frame,
		n_frames_visible
	});
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
	PF0;
	WaveformContext* wfc = layers.wa->context;
	wf_context_set_zoom(wfc, (wfc->scaled ? wf_context_get_zoom(wfc) : 1.0) * 1.5);
}


static void
zoom_out (gpointer user_data)
{
	PF0;
	WaveformContext* wfc = layers.wa->context;
	wf_context_set_zoom(wfc, (wfc->scaled ? wf_context_get_zoom(wfc) : 1.0) / 1.5);
}


