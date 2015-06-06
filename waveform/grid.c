/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

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
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/canvas.h"
#include "waveform/actor.h"
#include "waveform/grid.h"
																				// TODO draw over whole viewport not just the wfactor.

#if 0
struct _grid
{
	struct {
		WfAnimatable  start;
		WfAnimatable  len;
	}               animatable;
	WfAnimation     animation;
	guint           timer_id;
};
#endif

static bool grid_actor_paint(AGlActor*);


AGlActor*
grid_actor(WaveformActor* wf_actor)
{
	void grid_actor_size(AGlActor* actor)
	{
		actor->region = actor->parent->region;
	}

	void grid_actor_init(AGlActor* actor)
	{
#ifdef AGL_ACTOR_RENDER_CACHE
		actor->fbo = agl_fbo_new(actor->region.x2 - actor->region.x1, actor->region.y2 - actor->region.y1, 0, 0);
#endif
	}

	GridActor* grid = g_new0(GridActor, 1);
	grid->wf_actor = wf_actor;

	AGlActor* actor = (AGlActor*)grid;
#ifdef AGL_DEBUG_ACTOR
	actor->name = "grid";
#endif
	actor->init = grid_actor_init;
	actor->paint = grid_actor_paint;
	actor->set_size = grid_actor_size;
	return actor;
}


static bool
grid_actor_paint(AGlActor* _actor)
{
	//draw a vertical line every 1 second.

	//only the case of a canvas with a single Waveform is supported.

	AGl* agl = agl_get_instance();
	WaveformActor* actor = ((GridActor*)_actor)->wf_actor;
	Waveform* w = actor->waveform;
	WaveformCanvas* canvas = actor->canvas;

	g_return_if_fail(canvas);

	WfViewPort viewport; wf_actor_get_viewport(actor, &viewport);

	float h = viewport.bottom;
	float zoom = (viewport.right - viewport.left) / actor->region.len; // pixels per sample

	int interval = canvas->sample_rate * (zoom > 0.0002 ? 1 : zoom > 0.0001 ? 5 : 10);
	const int64_t region_end = actor->region.start + actor->region.len;

	int i = 0;
	uint64_t f = ((int)(actor->region.start / interval)) * interval;
	if(agl->use_shaders){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

															// TODO make fixed contrast with background
		agl->shaders.plain->uniform.colour = 0x1133bb55;
		agl_use_program((AGlShader*)agl->shaders.plain);

		for(; (f < region_end) && (i < 0xff); f += interval, i++){
			agl_rect_((AGlRect){wf_actor_frame_to_x(actor, f), 0, 1, h});
		}

		agl_set_font_string("Roboto 7");
		uint32_t colour = (actor->fg_colour & 0xffffff00) + (actor->fg_colour & 0x000000ff) * 0x44 / 0xff;
		char s[16] = {0,};
		int x_ = 0;
		uint64_t f = ((int)(actor->region.start / interval)) * interval;
		for(; (f < region_end) && (i < 0xff); f += interval, i++){
			int x = wf_actor_frame_to_x(actor, f) + 3;
			if(x - x_ > 60){
				if(w->n_frames / actor->canvas->sample_rate < 60)
					snprintf(s, 15, "%.1f", ((float)f) / actor->canvas->sample_rate);
				else{
					uint64_t mins = f / (60 * actor->canvas->sample_rate);
					snprintf(s, 15, "%Lu:%.1f", mins, ((float)f) / actor->canvas->sample_rate - 60 * mins);
				}
				agl_print(x, 0, 0, colour, s);
				x_ = x;
			}
		}
		agl_set_font_string("Roboto 10");
	}else{
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
		glLineWidth(1);
		glColor4f(0.5, 0.5, 1.0, 0.25);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for(; (f < region_end) && (i < 0xff); f += interval, i++){
			float x = wf_actor_frame_to_x(actor, f);

			glBegin(GL_LINES);
			glVertex3f(x, 0.0, 1);
			glVertex3f(x, h, 1);
			glEnd();
		}
	}
	return true;
}


