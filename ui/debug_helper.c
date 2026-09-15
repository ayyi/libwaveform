/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 | Debug Helper                                                         |
 | ------------                                                         |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include "transition/transition.h"
#include "wf/debug.h"
#include "wf/private.h"
#include "waveform/ui-private.h"
#include "waveform/actor.h"
#include "waveform/debug_helper.h"

typedef struct
{
    AGlBehaviour behaviour;
	RenderResult previous_result;
	int          n_draw_calls;
} DebugHelper;

typedef struct
{
    AGlBehaviourClass class;
} DebugHelperClass;

void debug_helper_init (AGlBehaviour*, AGlActor*);
bool debug_helper_draw (AGlBehaviour*, AGlActor*, AGlActorPaint);

static DebugHelperClass klass = {
	.class = {
		.init = debug_helper_init,
		.draw = debug_helper_draw
	}
};


AGlBehaviourClass*
debug_helper_class ()
{
	return (AGlBehaviourClass*)&klass;
}


AGlBehaviour*
debug_helper ()
{
	return (AGlBehaviour*)AGL_NEW (DebugHelper,
		.behaviour= {
			.klass = &klass.class,
		}
	);
}


void
debug_helper_init (AGlBehaviour* behaviour, AGlActor* actor)
{
	typedef struct {
		AGlActor*     actor;
		AGlBehaviour* behaviour;
	} BehaviourClosure;

	gboolean debug_helper_init_timeout (void* user_data)
	{
		BehaviourClosure* c = user_data;
		extern void agl_actor__print_tree (AGlActor*);
		extern bool agl_actor__is_onscreen (AGlActor*);
		DebugHelper* dh = (DebugHelper*)c->behaviour;

		if (!agl_actor__is_onscreen(c->actor)) pwarn("OFFSCREEN");

		dbg(1, "after 2s: n_draw_calls=%i", dh->n_draw_calls);
		if (!dh->n_draw_calls)
			agl_actor__print_tree((AGlActor*)c->actor->root);

		g_free(c);
		return G_SOURCE_REMOVE;
	}
	g_timeout_add(2000, debug_helper_init_timeout, WF_NEW(BehaviourClosure, actor, behaviour));
}


bool
debug_helper_draw (AGlBehaviour* behaviour, AGlActor* actor, AGlActorPaint wrapped)
{
	WaveformActor* wf_actor = (WaveformActor*)actor;
	WaveformContext* wfc = wf_actor->context;

	bool result = wrapped (actor);

	((DebugHelper*)behaviour)->n_draw_calls++;

	if (wf_actor->render_result == ((DebugHelper*)behaviour)->previous_result) return result;

	double spp = wfc->scaled
		? wfc->zoom->value.f / wfc->samples_per_pixel
		: agl_actor__width(actor) / wf_actor->region.len;

	switch (wf_actor->render_result) {
		case RENDER_RESULT_LOADING:
			printf("RENDER_RESULT_LOADING\n");
			break;
		case RENDER_RESULT_NO_PROGRAM:
			printf("RENDER_RESULT_NO_PROGRAM\n");
			break;
		case RENDER_RESULT_NO_AUDIO_DATA:
			printf("RENDER_RESULT_NO_AUDIO_DATA\n");
			break;
		default:
			dbg(0, "%s %s result=%i zoom=%.2f spp=%.04f", result ? "ok" : "failed", wf_actor_print_mode(wf_actor), wf_actor->render_result, wfc->zoom->value.f, spp);
	}

	if (!result) {
		void agl_actor__print_tree (AGlActor*);
		agl_actor__print_tree(actor->parent);
	}

	((DebugHelper*)behaviour)->previous_result = wf_actor->render_result;

	return result;
}
