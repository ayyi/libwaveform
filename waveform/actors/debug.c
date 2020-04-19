/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2016-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
* | DEBUG ACTOR                                                          |
* | Shows the raw fbo texture of another actor.                          |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include "agl/actor.h"
#include "agl/behaviours/border.h"
#include "wf/debug.h"
#include "waveform/shader.h"
#include "waveform/actors/debug.h"

static AGl* agl = NULL;

AGlActor* wf_debug_actor (AGlActor*);

static AGlActorClass actor_class = {0, "Debug", (AGlActorNew*)wf_debug_actor};


AGlActorClass*
debug_node_get_class ()
{
	static bool init_done = false;

	if(!init_done){
		agl = agl_get_instance();

		agl_actor_class__add_behaviour(&actor_class, border_get_class());

		init_done = true;
	}

	return &actor_class;
}


static void
debug_actor__init (AGlActor* actor)
{
	agl_set_font_string("Roboto 10"); // initialise the pango context
}


static bool
debug_actor__paint (AGlActor* actor)
{
#ifdef AGL_ACTOR_RENDER_CACHE
	DebugActor* view = (DebugActor*)actor;

	AGlActor* target = view->target;
	if(target && target->fbo){
		agl_set_font_string("Roboto 7");

		agl_print(2, 2, 0, 0xffffff77, "%i %ix%i", target->fbo->id, target->fbo->width, target->fbo->height);
		if(target->cache.valid)
			agl_print(2, 10, 0, 0xffffff77, "VALID");
		else
			agl_print(2, 10, 0, 0x5555ffaa, "INVALID");

		agl->shaders.texture->uniform.fg_colour = 0xffffffff;
		agl_use_program((AGlShader*)agl->shaders.texture);
		agl_textured_rect(view->target->fbo->texture,
			0, 10,
			agl_actor__width(actor),
			agl_actor__height(actor) - 10,
			&(AGlQuad){0.0, 1.0, 1.0, 0.0}
		);
	}
#endif

	return true;
}


static void
debug_actor__set_size (AGlActor* actor)
{
}


AGlActor*
wf_debug_actor (AGlActor* _)
{
	debug_node_get_class();

	DebugActor* view = agl_actor__new(DebugActor,
		.actor = {
			.class  = &actor_class,
			.name = actor_class.name,
			.program = (AGlShader*)&cursor,
			.init = debug_actor__init,
			.set_size = debug_actor__set_size,
			.paint = debug_actor__paint,
		},
	);

	return (AGlActor*)view;
}


void
wf_debug_actor_set_actor (DebugActor* actor, AGlActor* target)
{
	DebugActor* view = (DebugActor*)actor;
	g_return_if_fail(actor);

	view->target = target;

	agl_actor__invalidate((AGlActor*)actor);
}

