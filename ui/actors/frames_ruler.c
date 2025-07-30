/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2013-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | FramesRuler                                                          |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include "agl/behaviours/cache.h"
#include "wf/waveform.h"
#include "waveform/actor.h"
#include "waveform/context.h"

extern RulerFramesShader frames_ruler;

typedef struct {
    AGlActor         actor;
    WaveformContext* context;
} FramesRulerActor;

static AGl* agl = NULL;

static bool frames_ruler_actor_paint (AGlActor*);


static void
frames_ruler_actor_size (AGlActor* actor)
{
	actor->region.x2 = agl_actor__width(actor->parent);
}


static void
frames_ruler_set_state (AGlActor* actor)
{
	#define samples_per_beat(C) (C->sample_rate / (C->bpm / 60.0))

	RulerFramesShader* shader = &frames_ruler;
	FramesRulerActor* ruler = (FramesRulerActor*)actor;
	WaveformContext* context = ruler->context;

	float num = context->samples_per_pixel / (samples_per_beat(context) * context->zoom->value.f);
	int exponent = (int)log10(num);
	float quantizedValue = pow(10, exponent);
	if (num / quantizedValue > 5.) quantizedValue *= 10.f;

	float pixels_per_unit = 120.f / (num / quantizedValue);
	float zoom = context->zoom->value.f / context->samples_per_pixel;
	float n_subs = 2.f;
	if (zoom < 0.00015 && zoom > 0.00007) {
		// each unit is one minute. 6 sub-divisions of 10s each
		n_subs = 6.f;
	} else if (zoom < 0.0008 && zoom > 0.00015) {
		// change unit from 6s to 10s
		pixels_per_unit *= 10. / 6.;
	} else if (zoom < .006 && zoom > .0015) {
		pixels_per_unit *= 10. / 6.;
	} else if (zoom < .01 && zoom > .006) {
		pixels_per_unit *= 5. / 6.;
	} else if (zoom >= .01) {
		pixels_per_unit *= 5. / 6.;
	}

	shader->uniform.fg_colour = 0xffffff7f;
	shader->uniform.samples_per_pixel = context->samples_per_pixel;
	shader->uniform.pixels_per_unit = pixels_per_unit;
	shader->uniform.n_subs = n_subs;
	shader->uniform.viewport_left = -wf_context_frame_to_x(context, 0);
}


AGlActor*
frames_ruler_actor (WaveformActor* wf_actor)
{
	g_return_val_if_fail(wf_actor, NULL);

	agl = agl_get_instance();

	return (AGlActor*)agl_actor__new(FramesRulerActor,
		.actor = {
			.name = strdup("ruler"),
			.program = (AGlShader*)&frames_ruler,
			.set_state = frames_ruler_set_state,
			.paint = frames_ruler_actor_paint,
			.set_size = frames_ruler_actor_size,
			.behaviours = {
				cache_behaviour(),
			}
		},
		.context = wf_actor->context
	);
}


static bool
frames_ruler_actor_paint (AGlActor* actor)
{
#if 0 // shader debugging
	{
		float smoothstep(float edge0, float edge1, float x)
		{
			float t = CLAMP((x - edge0) / (edge1 - edge0), 0.0, 1.0);
			return t * t * (3.0 - 2.0 * t);
		}

		float pixels_per_beat = 1.0 / wfc->priv->shaders.ruler->uniform.beats_per_pixel;
		dbg(0, "ppb=%.2f", pixels_per_beat);
		int x; for(x=0;x<30;x++){
			float m = (x * 100) % ((int)pixels_per_beat * 100);
			float m_ = x - pixels_per_beat * floor(x / pixels_per_beat);
			printf("  %.2f %.2f %.2f\n", m / 100, m_, smoothstep(0.0, 0.5, m_));
		}
	}
#endif

	agl_quad (-actor->scrollable.x1, 0., actor->region.x2, agl_actor__height(actor));

	return true;
}
