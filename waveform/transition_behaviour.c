/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "transition/transition.h"
#include "waveform/actor.h"
#include "waveform/transition_behaviour.h"

typedef struct {
	WaveformActor*  actor;
	WaveformActorFn callback;
	gpointer       user_data;
} C;


static void
on_transition_finished (WfAnimation* animation, gpointer user_data)
{
	PF;
	g_return_if_fail(user_data);
	g_return_if_fail(animation);
	C* c = user_data;
	if(c->callback) c->callback(c->actor, c->user_data);
	g_free(c);
}


// TODO move this up to AGlActor
// TODO This only handles one animatable :-(
WfAnimation*
transition_behaviour_set_f (TransitionBehaviour* behaviour, AGlActor* actor, float val, WaveformActorFn callback, gpointer user_data)
{
	WfAnimatable* animatable = &behaviour->animatable;

	if(val == animatable->target_val.f){
		if(callback){
			callback((WaveformActor*)actor, user_data);
		}
		return NULL;
	}

	animatable->target_val.f = val;

	GList* animatables = TRUE || (animatable->start_val.f != animatable->target_val.f)
		? g_list_prepend(NULL, animatable)
		: NULL;

	return agl_actor__start_transition(actor, animatables, on_transition_finished, AGL_NEW(C,
		.actor = (WaveformActor*)actor,
		.callback = callback,
		.user_data = user_data
	));
}


WfAnimation*
transition_behaviour_set_i64 (TransitionBehaviour* behaviour, AGlActor* actor, int64_t val, WaveformActorFn callback, gpointer user_data)
{
	WfAnimatable* animatable = &behaviour->animatable;

	if(val == animatable->target_val.b){
		if(callback){
			callback((WaveformActor*)actor, user_data);
		}
		return NULL;
	}

	animatable->target_val.b = val;

	GList* animatables = TRUE || (animatable->start_val.b != animatable->target_val.b)
		? g_list_prepend(NULL, animatable)
		: NULL;

	return agl_actor__start_transition(actor, animatables, on_transition_finished, AGL_NEW(C,
		.actor = (WaveformActor*)actor,
		.callback = callback,
		.user_data = user_data
	));
}
