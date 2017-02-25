/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2016-2017 Tim Orford <tim@orford.org>                  |
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "waveform/shader.h"
#include "waveform/actors/debug.h"

static AGl* agl = NULL;


AGlActor*
wf_debug_actor(WaveformActor* _)
{
	agl = agl_get_instance();

	void debug_actor__init(AGlActor* actor)
	{
	}

	/*
	void debug_actor__set_state(AGlActor* actor)
	{
		DebugActor* view = (DebugActor*)actor;
		WaveformActor* a = view->wf_actor;
		if(!a->canvas) return;

	}
	*/

	bool debug_actor__paint(AGlActor* actor)
	{
		DebugActor* view = (DebugActor*)actor;

		AGlActor* target = view->target;
		if(target && target->fbo){
			agl_textured_rect(view->target->fbo->texture, 0, 0, actor->region.x2, actor->region.y2, NULL);
			if(!target->cache.valid) dbg(0, "** cache invalid");
		}

		return true;
	}

	void debug_actor__set_size(AGlActor* actor)
	{
	}

	DebugActor* view = AGL_NEW(DebugActor,
		.actor = {
#ifdef AGL_DEBUG_ACTOR
			.name = "Debug",
#endif
			.program = (AGlShader*)&cursor,
			.init = debug_actor__init,
			//.set_state = debug_actor__set_state,
			.set_size = debug_actor__set_size,
			.paint = debug_actor__paint,
		},
	);

	return (AGlActor*)view;
}


/*
 *  Set the current playback position in milliseconds
 */
void
wf_debug_actor_set_actor(DebugActor* actor, AGlActor* target)
{
	DebugActor* view = (DebugActor*)actor;
	g_return_if_fail(actor);

	view->target = target;
}

