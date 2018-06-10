/*
  copyright (C) 2012-2018 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "group.h"

static AGlActorClass actor_class = {0, "Group", (AGlActorNew*)group_actor};

AGlActor*
group_actor(WaveformActor* wf_actor)
{
	AGlActor* actor = agl_actor__new();
	actor->name = actor_class.name;

	return actor;
}

