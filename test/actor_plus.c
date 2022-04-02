/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2022 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 |  Demonstration of the libwaveform WaveformActor interface
 |
 |  Similar to actor.c but with additional features, eg background, ruler.
 |
 */

#include "config.h"
#include <getopt.h>
#include "agl/x11.h"
#include "actors/background.h"
#include "actors/group.h"
#include "waveform/actor.h"
#include "waveform/ruler.h"
#include "test/common.h"

#define WAV "mono_0:10.wav"

#define GL_WIDTH 300.0
#define GL_HEIGHT 256.0
#define HBORDER 16
#define VBORDER 8

AGlRootActor*    scene    = NULL;
AGlActor*        group    = NULL;
WaveformContext* wfc      = NULL;
Waveform*        w1       = NULL;
WaveformActor*   a[]      = {NULL};
gpointer         tests[]  = {};

KeyHandler
	zoom_in,
	zoom_out,
	scroll_left,
	scroll_right,
	toggle_animate,
	quit;

AGlKey keys[] = {
	{KEY_Left,      scroll_left},
	{KEY_KP_Left,   scroll_left},
	{KEY_Right,     scroll_right},
	{KEY_KP_Right,  scroll_right},
	{61,            zoom_in},
	{45,            zoom_out},
	{XK_KP_Enter,  NULL},
	{(char)'<',    NULL},
	{(char)'>',    NULL},
	{(char)'a',    toggle_animate},
	{XK_Delete,    NULL},
	{113,           quit},
	{0},
};

static void start_zoom   (float target_zoom);
uint64_t    get_time     ();

static const struct option long_options[] = {
	{ "autoquit", 0, NULL, 'q' },
};

static const char* const short_options = "q";


int
run (int argc, char* argv[])
{
	wf_debug = 0;

	AGlWindow* window = agl_window ("Actor", -1, -1, 320, 160, 0);
	scene = window->scene;

	char* filename = find_wav(WAV);
	w1 = waveform_load_new(filename);
	g_free(filename);

	group = agl_actor__add_child((AGlActor*)scene, group_actor(a[0]));

	void group__set_size (AGlActor* group)
	{
		group->region = (AGlfRegion){
			.x1 = HBORDER,
			.y1 = VBORDER,
			.x2 = group->parent->region.x2 - HBORDER,
			.y2 = group->parent->region.y2 - VBORDER,
		};

		double width = agl_actor__width(group);//((AGlActor*)scene) - 2 * ((int)HBORDER);
		wfc->samples_per_pixel = waveform_get_n_frames(w1) / width;

		#define ruler_height 20.0

		for (int i=0;i<G_N_ELEMENTS(a);i++)
			if (a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
				0.0,
				ruler_height + 5 + i * ((AGlActor*)scene)->region.y2 / 2,
				width,
				agl_actor__height(group) / 2 * 0.95
			});

		start_zoom(wf_context_get_zoom(wfc));
	}
	group->set_size = group__set_size;

	wfc = wf_context_new(group);
	wf_context_set_zoom(wfc, 1.0);

	int n_frames = waveform_get_n_frames(w1);

	WfSampleRegion region[] = {
		{0,            n_frames    },
		{0,            n_frames / 2},
		{n_frames / 4, n_frames / 4},
		{n_frames / 2, n_frames / 2},
	};

	uint32_t colours[4][2] = {
		{0xffffff77, 0x0000ffff},
		{0x66eeffff, 0x0000ffff},
		{0xffdd66ff, 0x0000ffff},
		{0x66ff66ff, 0x0000ffff},
	};

	for (int i=0;i<G_N_ELEMENTS(a);i++) {
		agl_actor__add_child(group, (AGlActor*)(a[i] = wf_context_add_new_actor(wfc, w1)));

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	agl_actor__add_child(group, background_actor(a[0]));

	AGlActor* ruler = agl_actor__add_child(group, ruler_actor(a[0]));
	ruler->region = (AGlfRegion){0, 0, 0, 20};

	g_main_loop_run (agl_main_loop_new());

	agl_window_destroy (&window);
	XCloseDisplay (dpy);

	return EXIT_SUCCESS;
}


static void
start_zoom (float target_zoom)
{
	wf_context_set_zoom(wfc, target_zoom);
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


void
zoom_in (gpointer _)
{
	start_zoom(wf_context_get_zoom(wfc) * 1.5);
}


void
zoom_out (gpointer _)
{
	start_zoom(wf_context_get_zoom(wfc) / 1.5);
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
quit (gpointer _)
{
	exit(EXIT_SUCCESS);
}


uint64_t
get_time ()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


#include "test/_x11.c"
