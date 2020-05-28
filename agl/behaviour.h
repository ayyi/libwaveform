/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2019-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

#ifndef __agl_behaviour_h__
#define __agl_behaviour_h__

#include "agl/actor.h"

typedef struct _AGlBehaviour AGlBehaviour;

typedef AGlBehaviour* (*AGlBehaviourNew)   ();
typedef void          (*AGlBehaviourFn)    (AGlBehaviour*);
typedef void          (*AGlBehaviourInit)  (AGlBehaviour*, AGlActor*);
typedef void          (*AGlBehaviourLayout)(AGlBehaviour*, AGlActor*);
typedef bool          (*AGlBehaviourDraw)  (AGlBehaviour*, AGlActor*, AGlActorPaint);
typedef bool          (*AGlBehaviourEvent) (AGlBehaviour*, AGlActor*, GdkEvent*);

typedef struct
{
    AGlBehaviourNew      new;
    AGlBehaviourFn       free;
    AGlBehaviourInit     init;
    AGlBehaviourLayout   layout;
    AGlBehaviourDraw     draw;
    AGlBehaviourEvent    event;
} AGlBehaviourClass;

struct _AGlBehaviour
{
    AGlBehaviourClass* klass;
};

#define agl_behaviour_init(B, A) ((B)->klass->init((B), A))
#define agl_behaviour_event(B, A, E) ((B)->klass->event((B), A, E))

#define behaviour_foreach(A) \
			for(int i = 0; i < AGL_ACTOR_N_BEHAVIOURS; i++){ \
				AGlBehaviour* behaviour = A->behaviours[i]; \
				if(!behaviour) \
					break;

#endif
