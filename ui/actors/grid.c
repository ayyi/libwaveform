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
 | WaveformGrid draws timeline markers onto a shared opengl drawable.   |
 |                                                                      |
 | The scaling is controlled by the sample_rate and samples_per_pixel   |
 | properties of the WaveformContext.                                   |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include "agl/behaviours/cache.h"
#include "agl/behaviours/scrollable_h.h"
#include "agl/behaviours/follow.h"
#include "waveform/actor.h"
#include "waveform/grid.h"

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
	if (!vbo) {
		glGenBuffers(1, &vbo);
	}
}


static void
grid_actor_set_state (AGlActor* actor)
{
	agl_enable(AGL_ENABLE_BLEND);

	if (agl->use_shaders) {
		SET_PLAIN_COLOUR(agl->shaders.dotted, 0x55bb5566);
	}
}


AGlActor*
grid_actor (WaveformActor* wf_actor)
{
	g_return_val_if_fail(wf_actor, NULL);

	agl = agl_get_instance();

	GridActor* grid = agl_actor__new(GridActor,
		.actor = {
			.class = &actor_class,
			.colour = 0xffffffff,
			.init = grid_actor_init,
			.program = (AGlShader*)agl->shaders.dotted,
			.set_state = grid_actor_set_state,
			.paint = grid_actor_paint,
			.set_size = grid_actor_size,
			.behaviours = {
				cache_behaviour(),
			}
		},
		.wf_actor = wf_actor,
		.context = wf_actor->context,
	);

	AGlBehaviour* scrollable = agl_actor__find_behaviour((AGlActor*)wf_actor, scrollable_h_get_class());
	if (scrollable) {
		AGlBehaviour* f = agl_actor__add_behaviour((AGlActor*)grid, follow());
		((FollowBehaviour*)f)->to_follow = (AGlActor*)wf_actor;

		void grid_on_scroll (AGlObservable* o, AGlVal value, gpointer grid)
		{
			AGlActor* actor = grid;

			actor->scrollable = ((AGlActor*)((GridActor*)grid)->wf_actor)->scrollable;

			agl_actor__scroll_to (actor, (AGliPt){value.i, -1});
			agl_actor__invalidate(actor);
		}
		agl_observable_subscribe((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll, grid_on_scroll, grid);
	}

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
#else
	{
#endif
		zoom = agl_actor__width(actor) / grid->wf_actor->region.len;
	}

	int interval = context->sample_rate * (zoom > 0.005 ? 1 : zoom > 0.0002 ? 10 : zoom > 0.0001 ? 50 : zoom > 0.00001 ? 480 : 4800) / 10;

	const WfFrRange region = {
		.start = wf_context_x_to_frame (context, -actor->scrollable.x1),
		.end = context->scaled
			? wf_context_x_to_frame (context, agl_actor__width(actor) - actor->scrollable.x1)
			: grid->wf_actor->region.start + grid->wf_actor->region.len
	};

	int64_t f = ((int64_t)((region.start - 1)/ interval) + 1) * interval;
	if (agl->use_shaders) {
		int n = MIN(0x5f, (region.end - f) / interval + 1);
		if (n > 0) {
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
			glDrawArrays(GL_TRIANGLES, 0, n * AGL_V_PER_QUAD);
		}

		agl_set_font_string("Roboto 7");
		uint32_t colour = (actor->colour & 0xffffff00) + (actor->colour & 0x000000ff) * 0x88 / 0xff;
		char s[16] = {0,};
		int x_ = 0;
		uint64_t f = ((int64_t)(context->start_time->value.b / interval)) * interval;
		for (int i = 0; (f < region.end) && (i < 0xff); f += interval, i++) {
			int x = wf_context_frame_to_x(context, f) + 3;
			if (x > agl_actor__width(actor) - actor->scrollable.x1) break;
			if (x - x_ > 60) {
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
		agl_enable(AGL_ENABLE_BLEND);

		for (int i = 0; (f < region.end) && (i < 0xff); f += interval, i++) {
			float x = wf_context_frame_to_x(context, f);

			glBegin(GL_LINES);
			glVertex3f(x, 0.0, 1);
			glVertex3f(x, agl_actor__height(actor), 1);
			glEnd();
		}
	}
	return true;
}
