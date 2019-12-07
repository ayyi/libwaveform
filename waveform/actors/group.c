/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "group.h"

static AGlActorClass actor_class = {0, "Group", (AGlActorNew*)group_actor};

AGlActor*
group_actor(WaveformActor* wf_actor)
{
	AGlActor* actor = agl_actor__new(AGlActor);
	actor->name = actor_class.name;

	return actor;
}

