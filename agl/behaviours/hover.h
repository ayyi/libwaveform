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
#ifndef __hover_behaviour_h__
#define __hover_behaviour_h__

#include "glib.h"
#include "agl/actor.h"
#include "agl/behaviour.h"

AGlBehaviourClass* hover_get_class ();

AGlBehaviour* hover_behaviour ();

#endif
