/*
  copyright (C) 2012 Tim Orford <tim@orford.org>

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

  This provides an optional private mini animation framework for WfActors.

  Properties that can be animated: start, end (zoom is derived from these)

  Where an external animation framework is available, it should be used
  in preference, eg Clutter (TODO).

  In cases where the canvas is shared with other objects, this animator
  cannot be used. The application must provide its own animator.

  -what happens if we start a zoom while in the middle of panning?
		-with a single fixed length Transition, the first op will slow down after second starts
		-parallel Transitions:
			-what happens if 2nd op contains same property as first?
				-probably ok to remove this prop from first op (cannot have same prop in 2 simultaneous Transitions).
				 Dont see any other option here.

	-ok, so the canvas owns a list of Transitions, each with a list of Actors.

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
#include "waveform/peak.h"
#include "waveform/actor.h"
#include "waveform/animator.h"

extern void wf_canvas_queue_redraw (WaveformCanvas*);

static uint32_t transition_linear       (WfAnimation*, WfAnimatable*, int time);
static float    transition_linear_f     (WfAnimation*, WfAnimatable*, int time);

GList* animations = NULL;


WfAnimation*
wf_animation_add_new(void (*on_finished)(WaveformActor*, WfAnimation*))
{
	WfAnimation* animation = g_new0(WfAnimation, 1);
	animation->on_finish = on_finished;
	animation->frame_i = transition_linear;
	animation->frame_f = transition_linear_f;
	animations = g_list_append(animations, animation);

	return animation;
}


void
wf_transition_add_member(WfAnimation* animation, WaveformActor* actor, GList* animatables)
{
	//ownership of the animatables list is taken. Caller should not free it.

	g_return_if_fail(animation);
	g_return_if_fail(actor);
	g_return_if_fail(!g_list_find(animation->members, actor));
	g_return_if_fail(animatables);

	Blah* blah = g_new0(Blah, 1);
	blah->actor = actor;
	blah->transitions = animatables;
	animation->members = g_list_append(animation->members, blah);

	WfAnimatable* animatable = animatables->data;
	if(animatable->type == WF_INT)
		dbg(2, "start=%i end=%i", animatable->start_val.i, *animatable->model_val.i);
	else
		dbg(2, "start=%.2f end=%.2f", animatable->start_val.f, *animatable->model_val.f);
}


void
wf_animation_remove(WfAnimation* animation)
{
	GList* l = animation->members;
	for(;l;l=l->next){
		Blah* blah = l->data;
		g_list_free0(blah->transitions);
		animation->on_finish(blah->actor, animation);
		g_free(blah);
	}
	g_list_free0(animation->members);

	if(animation->timer) g_source_remove(animation->timer);
	animation->timer = 0;
	animations = g_list_remove(animations, animation);
	g_free(animation);
}


void
wf_animation_remove_animatable(WfAnimation* animation, WfAnimatable* animatable)
{
	GList* m = animation->members;
	dbg(2, "  animation=%p n_members=%i", animation, g_list_length(m));
	for(;m;m=m->next){
		Blah* blah = m->data;
		if(g_list_find(blah->transitions, animatable)){
			//remove the animatable from the old animation
			dbg(2, "       already animating: 'start'");
			animatable->start_val.i = animatable->val.i;

			if(!(blah->transitions = g_list_remove(blah->transitions, animatable))) wf_animation_remove(animation);
			break;
		}
	}
}


void
wf_animation_start(WfAnimation* animation)
{
	g_return_if_fail(animation);

	void print_animation()
	{
		GList* l = animation->members;
		dbg(0, "animation=%p n_members=%i", animation, g_list_length(l));
		for(;l;l=l->next){
			Blah* blah = l->data;
			GList* k = blah->transitions;
			dbg(0, "  actor=%p n_transitions=%i", blah->actor, g_list_length(k));
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
			GList* k = ((Blah*)l->data)->transitions;
			for(;k;k=k->next) n++;
		}
		return n;
	}

	if(!count_animatables()){
		wf_animation_remove(animation);
		//cannot call on_finish because there are no member actors
		return;
	}

	uint64_t _get_time()
	{
		struct timeval start;
		gettimeofday(&start, NULL);
		return start.tv_sec * 1000 + start.tv_usec / 1000;
	}

	#define ANIMATION_LENGTH 300
	animation->start = _get_time();
	animation->end   = animation->start + ANIMATION_LENGTH;

	gboolean wf_transition_frame(gpointer _animation)
	{
		uint64_t time = _get_time();

		WfAnimation* animation = _animation;
		GList* l = animation->members;
		for(;l;l=l->next){
			Blah* blah = l->data;
			WaveformActor* a = blah->actor;

			GList* k = blah->transitions;
			if(!k) gwarn("actor member has no transitions");
			for(;k;k=k->next){
				WfAnimatable* animatable = k->data;
				if(animatable->type == WF_INT){
					animatable->val.i = animation->frame_i(animation, animatable, time);
					dbg(2, "actor=%p val=%u", a, animatable->val);
				}else{
					animatable->val.f = animation->frame_f(animation, animatable, time);
					dbg(2, "actor=%p val=%.2f", a, animatable->val.f);
				}
			}
			wf_canvas_queue_redraw(a->canvas);
		}

		if(time > animation->end){
			wf_animation_remove(animation);
			return TIMER_STOP;
		}
		return TIMER_CONTINUE;
	}
	if(wf_transition_frame(animation)){ // !!!!!! not sure it is safe to do this - members not added yet?
		animation->timer = g_timeout_add(40, wf_transition_frame, animation);
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


