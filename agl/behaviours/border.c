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
/*
 *   This behaviour applies clipping to the actor.
 *   It is not normally needed if using fbo caching
 */
#include "config.h"
#undef USE_GTK
#include "agl/debug.h"
#include "agl/text/renderer.h"
#include "agl/text/roundedrect.h"
#include "agl/shader.h"
#include "border.h"

void border_init (AGlBehaviour*, AGlActor*);
bool border_draw (AGlBehaviour*, AGlActor*, AGlActorPaint);

static AGlBehaviourClass klass = {
	.new = border,
	.init = border_init,
	.draw = border_draw
};


AGlBehaviourClass*
border_get_class ()
{
	return &klass;
}


AGlBehaviour*
border ()
{
	return (AGlBehaviour*)AGL_NEW(BorderBehaviour,
		.behaviour = {
			.klass = &klass,
		}
	);
}


void
border_init (AGlBehaviour* behaviour, AGlActor* actor)
{
}


bool
border_draw (AGlBehaviour* behaviour, AGlActor* actor, AGlActorPaint wrapped)
{
	AGl* agl = agl_get_instance();

	agl->shaders.plain->uniform.colour = 0x11bb33aa;
	agl_use_program((AGlShader*)agl->shaders.plain);
	agl_box(1, 0., 0., agl_actor__width(actor) - 1, agl_actor__height(actor) - 1);

	bool good = wrapped (actor);

	return good;
}
