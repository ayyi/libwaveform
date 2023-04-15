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
 |   Demonstration of the libwaveform WaveformActor interface
 |   showing a 3d presentation of a list of waveforms.
 |
 */

#include "config.h"
#include <getopt.h>
#include "agl/x11.h"
#include "agl/event.h"
#include "waveform/actor.h"
#include "test/common2.h"

#define WAV "mono_0:10.wav"

#define WIDTH 256.
#define HEIGHT 256.
#define VBORDER 8

float rotate[3] = {30.0, 30.0, 30.0};
float isometric_rotation[3] = {35.264f, 45.0f, 0.0f};

WaveformContext* wfc           = NULL;
AGlScene*       scene          = NULL;
WaveformActor*  a[4]           = {NULL,};
float           zoom           = 1.0;
float           dz             = 20.0;

static AGlActorPaint wrapped = NULL;


static AGlActor* rotator           (WaveformActor*);
static void      start_zoom        (float target_zoom);
static void      forward           ();
static void      backward          ();
static void      toggle_animate    ();

static const struct option long_options[] = {
	{ "autoquit", 0, NULL, 'q' },
};

static const char* const short_options = "q";


bool
_paint (AGlActor* actor)
{
	WfAnimatable* z = wf_actor_get_z((WaveformActor*)actor);

	glTranslatef(0, 0, *z->val.f);
	wrapped(actor);
	glTranslatef(0, 0, -*z->val.f);

	return true;
}


int
run (int argc, char *argv[])
{
	set_log_handlers();

	wf_debug = 0;

	AGlWindow* window = agl_window ("Group", -1, -1, WIDTH, HEIGHT, 0);
	AGlActor* root = (AGlActor*)window->scene;
	scene = window->scene;
	window->scene->selected = root;

	wfc = wf_context_new(root);

	char* filename = find_wav(WAV);
	g_assert(filename);
	Waveform* wav = waveform_load_new(filename);
	g_free(filename);

	agl_actor__add_child(root, rotator(NULL));

	int n_frames = waveform_get_n_frames(wav);

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

	AGlActor* rotator = root->children->data;
	for (int i=0;i<G_N_ELEMENTS(a);i++) {
		agl_actor__add_child(rotator, (AGlActor*)(a[i] = wf_context_add_new_actor(wfc, wav)));

		wrapped = ((AGlActor*)a[i])->paint;
		((AGlActor*)a[i])->paint = _paint;

		g_free(((AGlActor*)a[i])->name);
		((AGlActor*)a[i])->name = g_strdup_printf("Waveform %i", i);

		wf_actor_set_region (a[i], &region[i]);
		wf_actor_set_colour (a[i], colours[i][0]);
		wf_actor_set_z      (a[i], -i * dz, NULL, NULL);
	}

	bool on_event (AGlActor* actor, AGlEvent* event, AGliPt xy)
	{
		AGlEventKey* e = (AGlEventKey*)event;

		switch (event->type) {
			case AGL_KEY_PRESS:
				switch (e->keyval) {
					case 61:
						start_zoom(zoom * 1.5);
						break;
					case 45:
						start_zoom(zoom / 1.5);
						break;
					case XK_Up:
					case XK_KP_Up:
						dbg(0, "up");
						forward();
						break;
					case XK_Down:
					case XK_KP_Down:
						dbg(0, "down");
						backward();
						break;
					case (char)'a':
						toggle_animate();
						break;
					default:
						dbg(0, "%i", e->keyval);
						break;
				}
				return AGL_HANDLED;
			default:
				break;
		}
		return AGL_NOT_HANDLED;
	}
	root->on_event = on_event;

	void set_size (AGlActor* scene)
	{
		AGlActor* rotator = scene->children->data;
		rotator->region = rotator->parent->region;

		start_zoom(zoom);
	}
	root->set_size = set_size;

	g_main_loop_run (agl_main_loop_new());

	agl_window_destroy (&window);
	XCloseDisplay (dpy);

	return EXIT_SUCCESS;
}


AGlActor*
rotator (WaveformActor* wf_actor)
{
	void rotator_set_state (AGlActor* actor)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(rotate[0], 1.0f, 0.0f, 0.0f);
		glRotatef(rotate[1], 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.0f, -1.0f);
	}

	AGlActor* actor = agl_actor__new(AGlActor,
		.name = g_strdup("Rotator"),
		.set_state = rotator_set_state,
	);

	return actor;
}


static void
set_position (int i, int j)
{
	#define Y_FACTOR 0.0f //0.5f //currently set to zero to simplify changing stack order

	if (a[i]) wf_actor_set_rect(a[i], &(WfRectangle) {
		40.0,
		((float)j) * HEIGHT * Y_FACTOR / 4 + 10.0f,
		WIDTH * zoom,
		HEIGHT / 4 * 0.95
	});
}


static void
start_zoom (float target_zoom)
{
	// When zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	PF;
	zoom = MAX(0.1, target_zoom);

	for(int i=0;i<G_N_ELEMENTS(a);i++) set_position(i, i);
}


/*
 *  Move all actors forward
 *  Fade out the front actor, then move it to the back
 */
static void
forward ()
{
	void fade_out_done (WaveformActor* actor, gpointer user_data)
	{
		gboolean fade_out_really_done (WaveformActor* actor)
		{
			AGlScene* scene = ((AGlActor*)actor)->root;
			AGlActor* rotator = ((AGlActor*)scene)->children->data;

			scene->enable_animations = false;
			wf_actor_set_z(actor, - 3 * dz, NULL, NULL);
			scene->enable_animations = true;

			// move front element to back (becomes first element)
			GList* front = g_list_last(rotator->children);
			rotator->children = g_list_remove_link(rotator->children, front);
			front->next = rotator->children;
			rotator->children ->prev = front;
			rotator->children = front;

			wf_actor_fade_in(actor, 1.0f, NULL, NULL);

			return G_SOURCE_REMOVE;
		}
		g_idle_add((GSourceFunc)fade_out_really_done, actor);
	}

	AGlActor* rotator = ((AGlActor*)scene)->children->data;
	float z = - 2 * dz; // actors have to be drawn from back to front
	for (GList* l=rotator->children;l;l=l->next) {
		WaveformActor* a = l->data;
		wf_actor_set_z(a, z, NULL, NULL);
		z += dz;
	}

	wf_actor_fade_out(g_list_last(rotator->children)->data, fade_out_done, NULL);
}


/*
 *  Move all actors backward
 *  Fade out the back actor, then move it to the front
 */
static void
backward ()
{
	void fade_out_done (WaveformActor* actor, gpointer user_data)
	{
		gboolean fade_out_really_done (WaveformActor* actor)
		{
			AGlScene* scene = ((AGlActor*)actor)->root;
			AGlActor* rotator = ((AGlActor*)scene)->children->data;

			scene->enable_animations = false;
			wf_actor_set_z(actor, 0, NULL, NULL);
			scene->enable_animations = true;

			GList* first = rotator->children;
			g_assert(actor == first->data);
			rotator->children = g_list_remove_link(rotator->children, rotator->children);
			GList* last = g_list_last(rotator->children);
			last->next = first;
			first->prev = last;

			wf_actor_fade_in(actor, 1.0f, NULL, NULL);

			return G_SOURCE_REMOVE;
		}
		g_idle_add((GSourceFunc)fade_out_really_done, actor);
	}

	AGlActor* rotator = ((AGlActor*)scene)->children->data;
	float z = - 4 * dz; // actors have to be drawn from back to front
	for (GList* l=rotator->children;l;l=l->next) {
		WaveformActor* a = l->data;
		wf_actor_set_z(a, z, NULL, NULL);
		z += dz;
	}
	wf_actor_fade_out(rotator->children->data, fade_out_done, NULL);
}


static void
toggle_animate ()
{
	PF;

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
				forward();
			}
		}
		frame++;
		return G_SOURCE_CONTINUE;
	}
	g_timeout_add(50, on_idle, NULL);
}


#include "test/_x11.c"
