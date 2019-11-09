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


typedef struct
{
    AGlBehaviourClass class;
} TransitionBehaviourClass;

static TransitionBehaviourClass klass = {
	.class = {
		.free = (AGlBehaviourFn)g_free,
	}
};


AGlBehaviourClass*
transition_behaviour_get_class ()
{
	return (AGlBehaviourClass*)&klass;
}


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
WfAnimation*
transition_behaviour_set_f (TransitionBehaviour* behaviour, AGlActor* actor, float val, WaveformActorFn callback, gpointer user_data)
{
	return transition_behaviour_set(
		behaviour,
		actor,
		(TransitionValue[]){{true, .value.f = val}},
		callback,
		user_data
	);
}


WfAnimation*
transition_behaviour_set (TransitionBehaviour* behaviour, AGlActor* actor, TransitionValue values[], WaveformActorFn callback, gpointer user_data)
{
	inline bool maybe_update (TransitionValue tv, WfAnimatable* animatable)
	{
		bool changed = false;

		switch(animatable->type){
			case WF_INT:
				if((changed = tv.value.i != animatable->target_val.i))
					animatable->target_val.i = tv.value.i;
				break;
			case WF_FLOAT:
				if((changed = tv.value.f != animatable->target_val.f))
					animatable->target_val.f = tv.value.f;
				break;
			case WF_INT64:
				if((changed = tv.value.b != animatable->target_val.b))
					animatable->target_val.b = tv.value.b;
				break;
			default:
				g_return_val_if_fail(false, false);
		}
		return changed;
	}

	GList* animatables = NULL;
	for(int i = 0; i < behaviour->size; i++){
		WfAnimatable* animatable = &behaviour->animatables[i];
		TransitionValue tv = values[i];
		if(tv.active){
			if(maybe_update(tv, animatable)){
				animatables = g_list_prepend(animatables, animatable);
			}
		}
	}

	if(!animatables){
		if(callback){
			callback((WaveformActor*)actor, user_data);
		}
		return NULL;
	}

	return agl_actor__start_transition(actor, animatables, on_transition_finished, AGL_NEW(C,
		.actor = (WaveformActor*)actor,
		.callback = callback,
		.user_data = user_data
	));
}


#if 0
WfAnimation*
transition_behaviours_set_f (TransitionBehaviour* behaviour, AGlActor* actor, TransitionValuef values[], WaveformActorFn callback, gpointer user_data)
{
	GList* animatables = NULL;
	for(int i = 0; i < behaviour->size; i++){
		WfAnimatable* animatable = &behaviour->animatables[i];
		TransitionValuef tv = values[i];
		if(tv.active){
			if(tv.value != animatable->target_val.f){
				animatable->target_val.f = tv.value;
				animatables = g_list_prepend(animatables, animatable);
			}
		}
	}

	if(!animatables){
		if(callback){
			callback((WaveformActor*)actor, user_data);
		}
		return NULL;
	}

	return agl_actor__start_transition(actor, animatables, on_transition_finished, AGL_NEW(C,
		.actor = (WaveformActor*)actor,
		.callback = callback,
		.user_data = user_data
	));
}
#endif


WfAnimation*
transition_behaviour_set_i64 (TransitionBehaviour* behaviour, AGlActor* actor, TransitionValue64 values[], WaveformActorFn callback, gpointer user_data)
{
	GList* animatables = NULL;
	for(int i = 0; i < behaviour->size; i++){
		WfAnimatable* animatable = &behaviour->animatables[i];
		TransitionValue64 tv = values[i];
		if(tv.active){
			if(tv.value != animatable->target_val.b){
				animatable->target_val.b = tv.value;
				animatables = g_list_prepend(animatables, animatable);
			}
		}
	}

	if(!animatables){
		if(callback){
			callback((WaveformActor*)actor, user_data);
		}
		return NULL;
	}

	return agl_actor__start_transition(actor, animatables, on_transition_finished, AGL_NEW(C,
		.actor = (WaveformActor*)actor,
		.callback = callback,
		.user_data = user_data
	));
}
