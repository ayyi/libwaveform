/*
  copyright (C) 2012-2016 Tim Orford <tim@orford.org>

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

  ---------------------------------------------------------------

  WaveformGrid draws timeline markers onto a shared opengl drawable.

*/
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include "waveform/waveform.h"
#include "waveform/context.h"
#include "waveform/actors/grid.h"
																				// TODO draw over whole viewport not just the wfactor.
typedef struct {
    AGlActor         actor;
    WaveformActor*   wf_actor;
    WaveformContext* context;
    uint32_t         fg_colour;
} GridActor;

static AGl* agl = NULL;

static bool grid_actor_paint(AGlActor*);


AGlActor*
grid_actor(WaveformActor* wf_actor)
{
	g_return_val_if_fail(wf_actor, NULL);

	agl = agl_get_instance();

	void grid_actor_size(AGlActor* actor)
	{
		actor->region = actor->parent->region;
	}

	void grid_actor_init(AGlActor* actor)
	{
		if(agl->use_shaders){
			agl_create_program(&((GridActor*)actor)->context->shaders.ruler->shader);
		}
#ifdef AGL_ACTOR_RENDER_CACHE
		actor->fbo = agl_fbo_new(actor->region.x2 - actor->region.x1, actor->region.y2 - actor->region.y1, 0, 0);
#endif
	}

	GridActor* grid = g_new0(GridActor, 1);
	*grid = (GridActor){
		.actor = {
#ifdef AGL_DEBUG_ACTOR
			.name = "grid",
#endif
			.init = grid_actor_init,
			.paint = grid_actor_paint,
			.set_size = grid_actor_size,
		},
		.wf_actor = wf_actor,
		.context = wf_actor->canvas,
		.fg_colour = wf_actor->fg_colour,
	};

	return (AGlActor*)grid;
}


static bool
grid_actor_paint(AGlActor* actor)
{
	//draw a vertical line every 1 second.

	//only the case of a canvas with a single Waveform is supported.

	GridActor* grid = (GridActor*)actor;
	WaveformContext* context = grid->context;

	g_return_val_if_fail(context, false);
	if(!context->sample_rate) return false; // eg if file not loaded

	float zoom = 0; // pixels per sample
#ifdef USE_CANVAS_SCALING
	float _zoom = wf_context_get_zoom(context);
	if(_zoom > 0.0){
		zoom = _zoom / context->samples_per_pixel;
	}else{
#endif
		WfViewPort viewport; wf_actor_get_viewport(grid->wf_actor, &viewport);
		zoom = (viewport.right - viewport.left) / grid->wf_actor->region.len;
#ifdef USE_CANVAS_SCALING
	}
#endif

	int interval = context->sample_rate * (zoom > 0.0002 ? 1 : zoom > 0.0001 ? 5 : zoom > 0.00001 ? 48 : 480);
	const int64_t region_end = context->scaled
		? context->start_time + agl_actor__width(actor) * context->samples_per_pixel * context->zoom
		: grid->wf_actor->region.start + grid->wf_actor->region.len;

	int i = 0;
	uint64_t f = ((int)(context->start_time / interval)) * interval;
	if(agl->use_shaders){
		agl_enable(AGL_ENABLE_BLEND | !AGL_ENABLE_TEXTURE_2D);

															// TODO make fixed contrast with background
		agl->shaders.plain->uniform.colour = 0x1133bb55;
		agl_use_program((AGlShader*)agl->shaders.plain);

		for(; (f < region_end) && (i < 0xff); f += interval, i++){
			agl_rect_((AGlRect){wf_context_frame_to_x(context, f), 0, 1, agl_actor__height(actor)});
		}

		agl_set_font_string("Roboto 7");
		uint32_t colour = (grid->fg_colour & 0xffffff00) + (grid->fg_colour & 0x000000ff) * 0x44 / 0xff;
		char s[16] = {0,};
		int x_ = 0;
		uint64_t f = ((int)(context->start_time / interval)) * interval;
		for(; (f < region_end) && (i < 0xff); f += interval, i++){
			int x = wf_context_frame_to_x(context, f) + 3;
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
	}else{
		glDisable(GL_TEXTURE_1D);
		//glDisable(GL_TEXTURE_2D);
		glLineWidth(1);
		glColor4f(0.5, 0.5, 1.0, 0.25);
		agl_enable(AGL_ENABLE_BLEND | !AGL_ENABLE_TEXTURE_2D);

		for(; (f < region_end) && (i < 0xff); f += interval, i++){
			float x = wf_context_frame_to_x(context, f);

			glBegin(GL_LINES);
			glVertex3f(x, 0.0, 1);
			glVertex3f(x, agl_actor__height(actor), 1);
			glEnd();
		}
	}
	return true;
}


