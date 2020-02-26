/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2020-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __agl_border_h__
#define __agl_border_h__

#include "glib.h"
#include "agl/actor.h"

typedef struct {
   AGlBehaviour   behaviour;
} BorderBehaviour;

AGlBehaviourClass* border_get_class ();

AGlBehaviour* border ();

#endif
