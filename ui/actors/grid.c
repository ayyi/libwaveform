/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
* +----------------------------------------------------------------------+
* | WaveformGrid draws timeline markers onto a shared opengl drawable.   |
* +----------------------------------------------------------------------+
* | The scaling is controlled by the sample_rate and samples_per_pixel   |
* | properties of the WaveformContext.                                   |
* +----------------------------------------------------------------------+
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "waveform/actor.h"
#include "waveform/grid.h"

#define V_PER_QUAD 4

typedef struct {
    AGlActor         actor;
    WaveformActor*   wf_actor;
    WaveformContext* context;
} GridActor;

static AGl* agl;
static unsigned int vbo;

static bool grid_actor_paint (AGlActor*);
static void grid_actor_free  (AGlActor*);

static AGlActorClass actor_class = {0, "Grid", (AGlActorNew*)grid_actor, grid_actor_free};


static void
grid_actor_size (AGlActor* actor)
{
	actor->region = actor->parent->region;
}


static void
grid_actor_init (AGlActor* actor)
{
	if(agl->use_shaders){
		agl_create_program(&((GridActor*)actor)->context->shaders.ruler->shader);
	}
#ifdef AGL_ACTOR_RENDER_CACHE
	actor->fbo = agl_fbo_new(actor->region.x2 - actor->region.x1, actor->region.y2 - actor->region.y1, 0, 0);
#endif

	if (!vbo) {
		glGenBuffers(1, &vbo);
	}
}


static void
grid_actor_set_state (AGlActor* actor)
{
	agl_enable(AGL_ENABLE_BLEND | !AGL_ENABLE_TEXTURE_2D);

	if(agl->use_shaders){
		SET_PLAIN_COLOUR(agl->shaders.dotted, 0x55bb5566);
	}
}


AGlActor*
grid_actor (WaveformActor* wf_actor)
{
	g_return_val_if_fail(wf_actor, NULL);

	agl = agl_get_instance();

	GridActor* grid = AGL_NEW(GridActor,
		.actor = {
			.class = &actor_class,
			.name = actor_class.name,
			.colour = 0xffffffff,
			.init = grid_actor_init,
			.program = (AGlShader*)agl->shaders.dotted,
			.set_state = grid_actor_set_state,
			.paint = grid_actor_paint,
			.set_size = grid_actor_size,
		},
		.wf_actor = wf_actor,
		.context = wf_actor->context,
	);

	return (AGlActor*)grid;
}


static void
grid_actor_free (AGlActor* actor)
{
	glDeleteBuffers (1, &vbo);
}


static bool
grid_actor_paint (AGlActor* actor)
{
	GridActor* grid = (GridActor*)actor;
	WaveformContext* context = grid->context;

	g_return_val_if_fail(context, false);
	if (!context->sample_rate) return false; // eg if file not loaded

	float zoom = 0; // pixels per sample
#ifdef USE_CANVAS_SCALING
	float _zoom = wf_context_get_zoom(context);
	if (_zoom > 0.0) {
		zoom = _zoom / context->samples_per_pixel;
	} else {
#endif
		WfViewPort viewport; wf_actor_get_viewport(grid->wf_actor, &viewport);
		zoom = (viewport.right - viewport.left) / grid->wf_actor->region.len;
#ifdef USE_CANVAS_SCALING
	}
#endif

	int interval = context->sample_rate * (zoom > 0.005 ? 1 : zoom > 0.0002 ? 10 : zoom > 0.0001 ? 50 : zoom > 0.00001 ? 480 : 4800) / 10;

	const int64_t region_end = context->scaled
		? context->start_time + agl_actor__width(actor) * context->samples_per_pixel / context->zoom->value.f
		: grid->wf_actor->region.start + grid->wf_actor->region.len;

	uint64_t f = ((int)(context->start_time / interval)) * interval;
	if (agl->use_shaders) {
		int n = MIN(0x5f, (region_end - f) / interval);
		if (f < context->start_time) f += interval;
		AGlQuadVertex vertices[n];

		for (int i = 0; i < n; f += interval, i++) {
			float x = wf_context_frame_to_x(context, f);
			agl_set_quad (&vertices, i,
				(AGlVertex){x, 0.},
				(AGlVertex){x + 2., agl_actor__height(actor)}
			);
		}

		glBindBuffer (GL_ARRAY_BUFFER, vbo);
		glBufferData (GL_ARRAY_BUFFER, sizeof(AGlQuadVertex) * n, vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray (0);
		glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glDrawArrays(GL_QUADS, 0, n * V_PER_QUAD);

		agl_set_font_string("Roboto 7");
		uint32_t colour = (actor->colour & 0xffffff00) + (actor->colour & 0x000000ff) * 0x88 / 0xff;
		char s[16] = {0,};
		int x_ = 0;
		uint64_t f = ((int)(context->start_time / interval)) * interval;
		for (int i = 0; (f < region_end) && (i < 0xff); f += interval, i++) {
			int x = wf_context_frame_to_x(context, f) + 3;
			if(x > agl_actor__width(actor)) break;
			if(x - x_ > 60){
#if 0
				if(w->n_frames / actor->canvas->sample_rate < 60)
					snprintf(s, 15, "%.1f", ((float)f) / actor->canvas->sample_rate);
				else{
#endif
					uint64_t mins = f / (60 * context->sample_rate);
					snprintf(s, 15, "%"PRIi64":%.1f", mins, ((float)f) / context->sample_rate - 60 * mins);
#if 0
				}
#endif
				agl_print(x, 0, 0, colour, s);
				x_ = x;
			}
		}
		agl_set_font_string("Roboto 10");
	} else {
		glDisable(GL_TEXTURE_1D);
		glLineWidth(1);
		glColor4f(0.5, 0.5, 1.0, 0.25);
		agl_enable(AGL_ENABLE_BLEND | !AGL_ENABLE_TEXTURE_2D);

		for (int i = 0; (f < region_end) && (i < 0xff); f += interval, i++) {
			float x = wf_context_frame_to_x(context, f);

			glBegin(GL_LINES);
			glVertex3f(x, 0.0, 1);
			glVertex3f(x, agl_actor__height(actor), 1);
			glEnd();
		}
	}
	return true;
}
