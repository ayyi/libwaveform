/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://ayyi.org              |
 | copyright (C) 2013-2021 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | WaveformLabels draws text over the ruler showing minutes and seconds |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#define __wf_private__

#include "config.h"
#include "agl/behaviours/cache.h"
#include "waveform/actor.h"
#include "waveform/labels.h"

typedef struct {
    AGlActor         actor;
    WaveformContext* context;
} LabelsActor;

static AGl* agl = NULL;

static bool labels_actor_paint (AGlActor*);


static void
labels_actor_init (AGlActor* actor)
{
	if (agl->use_shaders) {
		agl_create_program(&((LabelsActor*)actor)->context->shaders.ruler->shader);
	}
}


static void
labels_actor_size (AGlActor* actor)
{
}


AGlActor*
labels_actor (WaveformContext* context)
{
	g_return_val_if_fail(context, NULL);

	agl = agl_get_instance();

	LabelsActor* labels = AGL_NEW(LabelsActor,
		.actor = {
			.name = "labels",
			.init = labels_actor_init,
			.paint = labels_actor_paint,
			.set_size = labels_actor_size,
			.behaviours = {
				cache_behaviour(),
			}
		},
		.context = context,
	);

	return (AGlActor*)labels;
}


static bool
labels_actor_paint (AGlActor* actor)
{
#ifdef USE_CANVAS_SCALING
	LabelsActor* labels = (LabelsActor*)actor;
	WaveformContext* context = labels->context;

	g_return_val_if_fail(context, false);
	if(!context->sample_rate) return false; // eg if file not loaded

	float zoom = 0; // pixels per sample
	float _zoom = wf_context_get_zoom(context);
	if (_zoom > 0.0) {
		zoom = _zoom / context->samples_per_pixel;
	} else {
		zoom = 1.0 / context->samples_per_pixel;
	}

	int interval = context->sample_rate * (zoom > 0.0002 ? 1 : zoom > 0.0001 ? 5 : zoom > 0.00001 ? 48 : 480);
	const int64_t region_end = context->scaled
		? context->start_time->value.b + agl_actor__width(actor) * context->samples_per_pixel * context->zoom->value.f
		: context->start_time->value.b + agl_actor__width(actor) * context->samples_per_pixel;

	int i = 0;
	uint64_t f = ((int)(context->start_time->value.b / interval)) * interval;

	agl_set_font_string("Roboto 7");
	char s[16] = {0,};
	int x_ = 0;
	for (; (f < region_end) && (i < 0xff); f += interval, i++) {
		int x = wf_context_frame_to_x(context, f) + 3;
		if (x - x_ > 60) {
			uint64_t mins = f / (60 * context->sample_rate);
			snprintf(s, 15, "%"PRIi64":%.1f", mins, ((float)f) / context->sample_rate - 60 * mins);
			agl_print(x, 0, 0, actor->colour, s);
			x_ = x;
		}
	}
	agl_set_font_string("Roboto 10");
#endif
	return true;
}
