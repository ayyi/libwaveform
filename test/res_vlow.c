/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform                                     |
 | https://github.com/ayyi/libwaveform                                  |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <getopt.h>
#include "agl/x11.h"
#include "agl/behaviours/simple_key.h"
#include "agl/behaviours/fullsize.h"
#include "actors/background.h"
#include "wf/waveform.h"
#include "waveform/actor.h"
#define __glx_test__
#include "test/common2.h"

struct {
	WaveformActor* wa;
} layers = {0,};

static AGlKeyHandler
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
	{ "help", 0, NULL, 'h' },
};

static const char* const short_options = "qh";


int
main (int argc, char* argv[])
{
	int width = 2, height = 160;

	int opt;
	while ((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch (opt) {
			case 'h':
				printf("Usage:\n");
				printf("  res_vlow [options]\n");
				printf("Options:\n");
				printf("  -help                   Print this information\n");
				return 0;
			case 'q':
				g_timeout_add(3000, (gpointer)exit, NULL);
				break;
		}
	}

	AGlWindow* window = agl_window("Waveform v-low resolution test", -1, -1, width, height, 0);

	agl_actor__add_child((AGlActor*)window->scene, background_actor(NULL));

	g_autofree char* filename = find_wav("large1.wav");
	Waveform* w = waveform_load_new(filename);

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
