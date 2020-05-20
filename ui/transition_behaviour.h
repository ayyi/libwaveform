/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2019-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

#ifndef __transition_behaviour_h__
#define __transition_behaviour_h__

#include "agl/behaviour.h"

typedef struct
{
    AGlBehaviour behaviour;
    int          size;
    WfAnimatable animatables[];
} TransitionBehaviour;

typedef struct
{
    bool active;
    UVal value;
} TransitionValue;

typedef struct
{
    bool active;
    float value;
} TransitionValuef;

typedef struct
{
    bool active;
    int64_t value;
} TransitionValue64;


AGlBehaviourClass* transition_behaviour_get_class ();

WfAnimation* transition_behaviour_set      (TransitionBehaviour*, AGlActor*, TransitionValue[], WaveformActorFn, gpointer);
WfAnimation* transition_behaviour_set_f    (TransitionBehaviour*, AGlActor*, float, WaveformActorFn, gpointer);
WfAnimation* transition_behaviour_set_i64  (TransitionBehaviour*, AGlActor*, TransitionValue64[], WaveformActorFn, gpointer);

#endif
