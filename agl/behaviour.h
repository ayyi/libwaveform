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
typedef bool          (*AGlBehaviourDraw)  (AGlBehaviour*, AGlActor*, AGlActorPaint);

typedef struct
{
    AGlBehaviourNew      new;
    AGlBehaviourFn       free;
    AGlBehaviourInit     init;
    AGlBehaviourDraw     draw;
} AGlBehaviourClass;

struct _AGlBehaviour
{
    AGlBehaviourClass* klass;
};

#define agl_behaviour_init(B, A) ((B)->klass->init((B), A))

#endif
