/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include <sys/time.h>
#include "wf/waveform.h"
#include "agl/actor.h"
#include "agl/shader.h"
#include "waveform/plain.h"

static AGl* agl = NULL;


	static void plain_set_state(AGlActor* actor)
	{
		if(agl->use_shaders){
			((PlainShader*)actor->program)->uniform.colour = actor->colour;
		}else{
			glColor4f(0.4, 1.0, 0.4, 1.0);
		}
	}

	static bool plain_paint(AGlActor* actor)
	{
		agl_rect(
			0,
			0,
			agl_actor__width(actor),
			agl_actor__height(actor)
		);

		return true;
	}

AGlActor*
plain_actor(WaveformActor* view)
{
	agl = agl_get_instance();

	AGlActor* actor = WF_NEW(AGlActor,
		.name = "plain",
		.region = {
			.x2 = 1, .y2 = 1 // must have size else will not be rendered
		},
		.set_state = plain_set_state,
		.paint = plain_paint,
		.program = (AGlShader*)agl->shaders.plain,
	);

	return actor;
}


