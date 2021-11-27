/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. https://www.ayyi.org          |
* | copyright (C) 2021-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

#define __wf_private__
#include "config.h"
#include "transition/transition.h"
#include "wf/debug.h"
#include "waveform/actor.h"
#include "waveform/invalidator.h"

static void invalidator_init (AGlBehaviour*, AGlActor*);
static void invalidator_free (AGlBehaviour*);

typedef struct
{
    AGlBehaviourClass class;
} InvalidatorClass;

static InvalidatorClass klass = {
	.class = {
		.init = invalidator_init,
		.free = invalidator_free,
	}
};


AGlBehaviourClass*
invalidator_get_class ()
{
	return (AGlBehaviourClass*)&klass;
}


void
invalidator_invalidate_item (Invalidator* invalidator, int item)
{
	invalidator->valid &= ~(1 << item);
	invalidator_queue_check (invalidator);
}


void
invalidator_queue_check (Invalidator* invalidator)
{
	gboolean invalidator_check (void* _invalidator)
	{
		Invalidator* invalidator = _invalidator;

		for (int i=0;i<invalidator->n_types;i++) {
			int bit = 1 << i;
			int valid = invalidator->valid & bit;
			if (!valid) {
				if (invalidator->resolve[i](invalidator)){
					invalidator->valid |= bit;
				} else {
					break;
				}
			}
		}

		invalidator->recheck_queue = 0;
		return G_SOURCE_REMOVE;
	}

	if (!invalidator->recheck_queue) {
		invalidator->recheck_queue = g_idle_add (invalidator_check, invalidator);
	}
}


static void
invalidator_init (AGlBehaviour* behaviour, AGlActor* actor)
{
}


static void
invalidator_free (AGlBehaviour* behaviour)
{
	Invalidator* invalidator = (Invalidator*)behaviour;

	if (invalidator->recheck_queue) g_source_remove(invalidator->recheck_queue);

	g_free(invalidator);
}
