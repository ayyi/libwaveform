/*
  copyright (C) 2012-2013 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  ---------------------------------------------------------------

  This provides a simple framework for animated transitions from one scaler value to another.

  The object being animated will own one or more WfAnimatable. The animatable
  contains a pointer to the property being animated. The object will then have
  two values for the property, the original (model) value, plus the instantaneous
  (animating) value.

  The canvas has a list of Transitions, each with a list of WfActor's.                     // TODO make very sure we are ok to remove this, why are we not currently using multiple actors per animation?
  For each actor there is a list of WfAnimatable's, each of which transitions
  from a start to an end value for a particular property such as the start point. 
  The WfAnimatable has a value type that must be either integer or float.

  Each WfAnimation has its own clock and finish-callback.
                           ---------                         <-- this will change

  Basic usage:
  1- create a property to animate (WfAnimatable).
  2- create an animation using wf_animation_add_new().
  3- add the animatable to the animation using wf_transition_add_member().
  4- start the animation using wf_animation_start().

  easing fns:
    by default, a linear easing fn is used. callers can provide their own if required
    (either int or float depending on the type of the property being animated).

  use cases:
	-what happens if a pan is requested while already panning?
		-this property is removed from first transition (should not have same
		 prop in 2 simultaneous Transitions), and a new one is started.
		 The new start value is taken from the current transient value, not the model value.
	-what happens if we start a zoom while in the middle of panning?
		(ie 2 independent transitions)
		-with a single fixed length Transition, the first op will slow down after second starts
		-parallel Transitions are needed.
			-what happens if 2nd op contains same property as first?

		In most of the examples given, zoom is faked by modifying the same
		properties as with panning (rect_left and rect_len), so in these cases,
		parallel transitions cannot be used so the result will not be optimal.
		However there is nothing to prevent real zoom being used by modifying
		the projection. The canvas projection is deliberately out of scope for
		Canvas and Actor interface. (The WaveformView widget does however take
		control of the projection.)

 */
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include "waveform/utils.h"
#include "transition/animator.h"

#define WF_DEBUG_ANIMATOR

extern void wf_canvas_queue_redraw (WaveformCanvas*);

static uint32_t transition_linear       (WfAnimation*, WfAnimatable*, int time);
static float    transition_linear_f     (WfAnimation*, WfAnimatable*, int time);

#ifdef WF_DEBUG_ANIMATOR
GList* animations = NULL;
guint idx = 0;
#endif

GList* transitions = NULL; // list of currently running transitions (type WfAnimation*).

#if 0
static char white    [16] = "\x1b[0;39m";
static char yellow   [16] = "\x1b[1;33m";
static char green    [16] = "\x1b[1;32m";
#endif

WfAnimation*
wf_animation_add_new(AnimationFn on_finished, gpointer user_data)
{
	WfAnimation* animation = g_new0(WfAnimation, 1);
	animation->length = 300;
	animation->on_finish = on_finished;
	animation->user_data = user_data;
	animation->frame_i = transition_linear;
	animation->frame_f = transition_linear_f;
#ifdef WF_DEBUG_ANIMATOR
	animation->id = idx++;
	animations = g_list_append(animations, animation);
#endif
	transitions = g_list_append(transitions, animation);

	return animation;
}


void
wf_transition_add_member(WfAnimation* animation, GList* animatables)
{
	//ownership of the animatables list is taken. Caller should not free it.

	g_return_if_fail(animation);
	g_return_if_fail(animatables);

	GList* l = transitions;
	for(;l;l=l->next){
		//only remove animatables we are replacing. others need to finish.
		GList* k = animatables;
		for(;k;k=k->next){
			if(wf_animation_remove_animatable((WfAnimation*)l->data, (WfAnimatable*)k->data)) break;
		}
	}

	WfAnimActor* member = g_new0(WfAnimActor, 1);
	member->transitions = animatables;
	animation->members = g_list_append(animation->members, member);
#ifdef WF_DEBUG
	g_strlcpy(member->name, name, 16);
#endif

	//TODO do this in animation_start instead.
	l = animatables;
	for(;l;l=l->next){
		WfAnimatable* a = l->data;
		a->start_val.f = a->val.f;
	}

	WfAnimatable* animatable = animatables->data;
	if(animatable->type == WF_INT)
		dbg(2, "start=%i end=%i", animatable->start_val.i, *animatable->model_val.i);
	else
		dbg(2, "start=%.2f end=%.2f", animatable->start_val.f, *animatable->model_val.f);
}


#ifdef NOT_USED
static int
wf_animation_count_animatables(WfAnimation* animation)
{
	int n = 0;
	GList* l = animation->members;
	for(;l;l=l->next){
		WfAnimActor* actor = l->data;
		GList* k = actor->transitions;
		for(;k;k=k->next){
			//WfAnimatable* animatable = k->data;
			//dbg(0, "     animatable=%p type=%i %p %.2f %s", animatable, animatable->type, animatable->model_val.f, *animatable->model_val.f, animatable->name);
			n++;
		}
	}
	return n;
}


static const char*
wf_animation_list_animatables(WfAnimation* animation)
{
	static char str[256];
	str[0] = '\0';

	GList* l = animation->members;
	for(;l;l=l->next){
		WfAnimActor* actor = l->data;
		GList* k = actor->transitions;
		for(;k;k=k->next){
			WfAnimatable* animatable = k->data;
			//dbg(0, "     animatable=%p type=%i %p %.2f %s", animatable, animatable->type, animatable->model_val.f, *animatable->model_val.f, animatable->name);
#ifdef WF_DEBUG
			g_strlcpy(str + strlen(str), animatable->name, 16);
			g_strlcpy(str + strlen(str), " ", 16);
#endif
		}
	}
	return str;
}
#endif


void
wf_animation_remove(WfAnimation* animation)
{
	GList* l = animation->members;
	for(;l;l=l->next){
		WfAnimActor* aa = l->data;
		animation->on_finish(animation, animation->user_data); //arg2 is unnecesary
		transitions = g_list_remove(transitions, animation);
		g_list_free0(aa->transitions);
		g_free(aa);
	}
	g_list_free0(animation->members);

	if(animation->timer) g_source_remove(animation->timer);
	animation->timer = 0;
#ifdef WF_DEBUG_ANIMATOR
	if(!g_list_find(animations, animation)){
		dbg(0, "*** animation not found ***");
		//return;
	}
	animations = g_list_remove(animations, animation);
#endif
	g_free(animation);
}


gboolean
wf_animation_remove_animatable(WfAnimation* animation, WfAnimatable* animatable)
{
	// returns true is the whole animation is removed.

#ifdef WF_DEBUG_ANIMATOR
	if(/*wf_debug &&*/ !g_list_find(animations, animation)){
		dbg(0, "*** animation not found %p ***", animation);
		//return false;
	}
#endif
	GList* m = animation->members;
	dbg(2, "animation=%p n_members=%i", animation, g_list_length(m));
//dbg(0, "looking for: %s", animatable->name);
//dbg(0, "animation=%p n_members=%i", animation, g_list_length(m));
	for(;m;m=m->next){
		WfAnimActor* actor = m->data;
		if(g_list_find(actor->transitions, animatable)){
			//remove the animatable from the old animation
			dbg(2, "       already animating: 'start'");
			animatable->start_val.i = animatable->val.i;

			if(!(actor->transitions = g_list_remove(actor->transitions, animatable))){
				wf_animation_remove(animation);
//dbg(0, "...animatable removed (%s) and animation removed <<", animatable->name);
				return true;
			}
//			break;
//dbg(0, "...animatable removed (%s) <<", animatable->name);
			return false;
		}
	}
//dbg(0, "%s not found <<", animatable->name);
	return false;
}


void
wf_animation_start(WfAnimation* animation)
{
	g_return_if_fail(animation);

	static GSourceFunc on_timeout;

	void print_animation()
	{
		GList* l = animation->members;
		dbg(0, "animation=%p n_members=%i", animation, g_list_length(l));
		for(;l;l=l->next){
			WfAnimActor* actor = l->data;
			GList* k = actor->transitions;
			dbg(0, "  actor=%p n_transitions=%i", actor, g_list_length(k));
			for(;k;k=k->next){
				WfAnimatable* animatable = k->data;
				dbg(0, "     animatable=%p type=%i %p %.2f", animatable, animatable->type, animatable->model_val.f, *animatable->model_val.f);
			}
		}
	}
	if(wf_debug > 1) print_animation();

	int count_animatables()
	{
		int n = 0;
		GList* l = animation->members;
		for(;l;l=l->next){
			GList* k = ((WfAnimActor*)l->data)->transitions;
			for(;k;k=k->next) n++;
		}
		return n;
	}

	if(!count_animatables()){
		wf_animation_remove(animation);
		//cannot call on_finish because there are no member actors - TODO no longer true
		return;
	}

	uint64_t _get_time()
	{
		struct timeval start;
		gettimeofday(&start, NULL);
		return start.tv_sec * 1000 + start.tv_usec / 1000;
	}

	animation->start = _get_time();
	animation->end   = animation->start + animation->length;

	gboolean wf_transition_frame(gpointer _animation)
	{
		uint64_t time = _get_time();

		WfAnimation* animation = _animation;
		GList* l = animation->members;
		for(;l;l=l->next){
			WfAnimActor* blah = l->data;

			GList* k = blah->transitions;
			if(!k) gwarn("actor member has no transitions");
			for(;k;k=k->next){
				WfAnimatable* animatable = k->data;
				if(animatable->type == WF_INT){
					animatable->val.i = animation->frame_i(animation, animatable, time);
				}else{
					animatable->val.f = animation->frame_f(animation, animatable, time);
				}
			}
		}
		animation->on_frame(animation, time); // user frame callback

		if(time > animation->end){
			wf_animation_remove(animation);
			return TIMER_STOP;
		}

		uint64_t step = (time - animation->start) / WF_FRAME_INTERVAL;
		uint64_t late = (time - animation->start) % WF_FRAME_INTERVAL;
		guint new_interval = CLAMP(WF_FRAME_INTERVAL - late, 1, WF_FRAME_INTERVAL);
		dbg(2, "step=%Lu late=%Lu new_interval=%u", step, late, new_interval);

		GSource* source = g_timeout_source_new(CLAMP(WF_FRAME_INTERVAL - late, 1, WF_FRAME_INTERVAL));
		g_source_set_callback(source, on_timeout, animation, NULL);
		g_source_set_priority(source, G_PRIORITY_HIGH);
		animation->timer = g_source_attach(source, NULL);

		return TIMER_STOP;
	}
	on_timeout = wf_transition_frame;
	if(wf_transition_frame(animation)){ // !!!!!! not sure it is safe to do this - members not added yet?
#if 0
		animation->timer = g_timeout_add(40, wf_transition_frame, animation);
#else
		GSource* source = g_timeout_source_new(WF_FRAME_INTERVAL);
		g_source_set_callback(source, wf_transition_frame, animation, NULL);
		g_source_set_priority(source, G_PRIORITY_HIGH);
		animation->timer = g_source_attach(source, NULL);
#endif
	}
}


static uint32_t
transition_linear(WfAnimation* animation, WfAnimatable* animatable, int time)
{
	uint64_t len = animation->end - animation->start;
	uint64_t t = time - animation->start;

	float time_fraction = MIN(1.0, ((float)t) / len);
	float orig_val   = animatable->type == WF_INT ? animatable->start_val.i : animatable->start_val.f;
	float target_val = animatable->type == WF_INT ? *animatable->model_val.i : *animatable->model_val.f;
	dbg(2, "%.2f orig=%.2f target=%.2f", time_fraction, orig_val, target_val);
	return  (1.0 - time_fraction) * orig_val + time_fraction * target_val;
}


static float
transition_linear_f(WfAnimation* animation, WfAnimatable* animatable, int time)
{
	uint64_t len = animation->end - animation->start;
	uint64_t t = time - animation->start;

	float time_fraction = MIN(1.0, ((float)t) / len);
	float orig_val   = animatable->start_val.f;
	float target_val = *animatable->model_val.f;
	dbg(2, "%.2f orig=%.2f target=%.2f", time_fraction, orig_val, target_val);
	return  (1.0 - time_fraction) * orig_val + time_fraction * target_val;
}


#if 0
static void print_animation(WfAnimation* animation)
{
	GList* l = animation->members;
	dbg(0, "animation=%p n_members=%i", animation, g_list_length(l));
	for(;l;l=l->next){
		WfAnimActor* actor = l->data;
		GList* k = actor->transitions;
		dbg(0, "  actor=%p n_transitions=%i %s", actor->actor, g_list_length(k), actor->name);
		for(;k;k=k->next){
			WfAnimatable* animatable = k->data;
			dbg(0, "     animatable=%p type=%i %p %.2f %s", animatable, animatable->type, animatable->model_val.f, *animatable->model_val.f, animatable->name);
		}
	}
}
#endif

