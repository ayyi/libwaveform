/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

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

#ifndef USE_FRAME_CLOCK
#define WF_FRAME_INTERVAL ((uint64_t)40)
#endif

typedef struct
{
   int length;
}
WfTransitionGlobal;
#ifndef __wf_transition_c__
extern WfTransitionGlobal wf_transition;
#endif

typedef enum {WF_INT, WF_INT64, WF_FLOAT} WfPropType;

typedef union {
   uint32_t  i;
   int64_t   b;
   float     f;
} UVal;

typedef union {
   uint32_t* i;
   int64_t*  b;
   float*    f;
} UValp;

typedef struct _animatable_property
{
	UVal  val;
	UVal  start_val;
	UValp model_val; // target (end) value
	//union {uint32_t* i; float* f;} min;
	WfPropType type;
#ifdef WF_DEBUG
	char       name[16];
#endif
} WfAnimatable;

// following refactoring this is no longer neccesary
typedef struct _anim_actor
{
	GList*         transitions; // list of WfAnimatable*
#ifdef WF_DEBUG
	char           name[16];
#endif
} WfAnimActor;

typedef struct _animation WfAnimation;
typedef void  (*AnimationFn) (WfAnimation*, gpointer);
typedef void  (*AnimationFrameFn) (WfAnimation*, int time);
typedef void  (*AnimationValueFn) (WfAnimation*, UVal[], gpointer);

struct _animation
{
   uint32_t    length;
   uint64_t    start;
   uint64_t    end;
   uint32_t    (*frame_i)  (WfAnimation*, WfAnimatable*, int time); // easing fn
   float       (*frame_f)  (WfAnimation*, WfAnimatable*, int time); // easing fn
   GList*      members;                                             // list of WfAnimActor*  -- subject to change
//#ifndef USE_FRAME_CLOCK
   guint       timer;
//#endif
   AnimationFrameFn on_frame;                                       // caller can run redraw fn.
   AnimationFn on_finish;                                           // caller can free stuff in this callback
   gpointer    user_data;

   guint       id;                                                  // for debugging only
   guint       timeout;                                             // for debugging only
};

WfAnimation* wf_animation_add_new           (AnimationFn, gpointer);
void         wf_transition_add_member       (WfAnimation*, GList* animatables);
void         wf_animation_remove            (WfAnimation*);
gboolean     wf_animation_remove_animatable (WfAnimation*, WfAnimatable*);
void         wf_animation_start             (WfAnimation*);
void         wf_animation_preview           (WfAnimation*, AnimationValueFn, gpointer);

#endif //__wf_animator_h__
