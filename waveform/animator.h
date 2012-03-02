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
*/
#ifndef __wf_animator_h__
#define __wf_animator_h__

#ifdef __wf_private__

typedef enum {WF_INT, WF_FLOAT} WfPropType;

typedef struct _animatable_property
{
	union {uint32_t  i; float  f;} val;
	union {uint32_t  i; float  f;} start_val;
	union {uint32_t* i; float* f;} model_val;
	//union {uint32_t* i; float* f;} min;
	WfPropType type;
} WfAnimatable;

typedef struct _blah
{
	WaveformActor* actor;
	GList*         transitions; // list of WfAnimatable*
	//void           (*actor_free)(WaveformActor*); //the actor can free stuff in this callback
} Blah;

typedef struct _animation WfAnimation;
struct _animation
{
	uint64_t  start;
	uint64_t  end;
	uint32_t  (*frame_i)(WfAnimation*, WfAnimatable*, int time);
	float     (*frame_f)(WfAnimation*, WfAnimatable*, int time);
	GList*    members;    // list of Blah*
	guint     timer;
	void      (*on_finish)(WaveformActor*, WfAnimation*); //the actor can free stuff in this callback
};

WfAnimation* wf_animation_add_new           (void (*on_finished)(WaveformActor*, WfAnimation*));
void         wf_transition_add_member       (WfAnimation*, WaveformActor*, GList* animatables);
void         wf_animation_remove            (WfAnimation*);
void         wf_animation_remove_animatable (WfAnimation*, WfAnimatable*);
void         wf_animation_start             (WfAnimation*);

#endif

#endif //__wf_animator_h__
