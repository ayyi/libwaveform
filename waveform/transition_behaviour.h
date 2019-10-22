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

typedef struct
{
    int dummy;
} AGlBehaviour;

typedef struct
{
    AGlBehaviour behaviour;
    WfAnimatable animatable;
} TransitionBehaviour;

WfAnimation* transition_behaviour_set_f (TransitionBehaviour*, AGlActor*, float, WaveformActorFn, gpointer);
WfAnimation* transition_behaviour_set_i64 (TransitionBehaviour*, AGlActor*, int64_t, WaveformActorFn, gpointer);

#endif
