/**
* +----------------------------------------------------------------------+
* | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
* | copyright (C) 2016-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __agl_selectable_h__
#define __agl_selectable_h__

#include "glib.h"
#include "agl/observable.h"
#include "agl/actor.h"
#include "agl/behaviour.h"

typedef struct {
   AGlBehaviour   behaviour;
   Observable*    observable;
   ObservableFn   on_select;
} SelectBehaviour;

AGlBehaviour* selectable      ();

#endif
