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

#pragma once

#include "agl/behaviour.h"

typedef struct _Invalidator Invalidator;

typedef bool (*InvalidatorResolve) (Invalidator*);

struct _Invalidator
{
    AGlBehaviour       behaviour;
    int                n_types;
    int                valid;      // bitfield, one field for each of the types
    void*              user_data;
	guint              recheck_queue;
	InvalidatorResolve resolve[];
};

AGlBehaviourClass*
     invalidator_get_class       ();

void invalidator_invalidate_item (Invalidator*, int);
void invalidator_queue_check     (Invalidator*);
