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
#include "clip.h"

void clip_init (AGlBehaviour*, AGlActor*);
bool clip_draw (AGlBehaviour*, AGlActor*, AGlActorPaint);

static AGlBehaviourClass klass = {
	.new = clip,
	.init = clip_init,
	.draw = clip_draw
};


AGlBehaviourClass*
clip_get_class ()
{
	return &klass;
}


AGlBehaviour*
clip ()
{
	ClipBehaviour* a = AGL_NEW(ClipBehaviour,
		.behaviour = {
			.klass = &klass,
		}
	);

	return (AGlBehaviour*)a;
}


void
clip_init (AGlBehaviour* behaviour, AGlActor* actor)
{
}


bool
clip_draw (AGlBehaviour* behaviour, AGlActor* actor, AGlActorPaint wrapped)
{
	ops_push_clip (
		renderer.current_builder,
		&AGL_ROUNDED_RECT_INIT(
			0,
			0,
			actor->region.x2,
			actor->region.y2
		)
	);

	bool good = wrapped (actor);

	ops_pop_clip (builder());

	return good;
}
