/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
* | SPINNER VIEW                                                         |
* | Display a rotating spinner icon while waiting for a waveform to load |
* +----------------------------------------------------------------------+
*
*/

#define __wf_private__

#include "config.h"
#include <stdio.h>
#include <math.h>
#include "transition/frameclock.h"
#include "agl/actor.h"
#include "agl/ext.h"
#include "waveform/spinner.h"

#define RADIUS 16
#define SCALE 2.

extern RotatableShader rotatable;

static AGl* agl = NULL;
static AGlFBO* fbo = NULL;
static float rotation = 0;

static bool spinner__paint     (AGlActor*);
static void spinner__set_state (AGlActor*);


static void
spinner__init (AGlActor* actor)
{
	if(!fbo){
		unsigned int vbo;
		glGenBuffers(1, &vbo);

		fbo = agl_fbo_new(2 * RADIUS * SCALE, 2 * RADIUS * SCALE, 0, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		agl_draw_to_fbo(fbo) {
			#define N 50
			#define V_PER_QUAD 4

			AGlQuadVertex vertices[1];

			agl_enable(AGL_ENABLE_BLEND);
			glBlendEquation (GL_MAX);

			glClearColor(1.0, 1.0, 1.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			agl_use_program(agl->shaders.plain);
			agl_scale (agl->shaders.plain, RADIUS * SCALE, RADIUS * SCALE);
			agl_translate_abs (agl->shaders.plain, 0, 0);

			void set_line (AGlQuadVertex (*v)[], AGlVertex p0, AGlVertex p1)
			{
				AGlVertex normal = {
					(p0.y - p1.y) / 4.,
					(p1.x - p0.x) / 4.
				};

				#define THICKNESS 8.
				#define P (THICKNESS / (RADIUS * SCALE) + 0.5)

				(*v)[0] = (AGlQuadVertex){
					(AGlVertex){p0.x - normal.x * P, p0.y - normal.y * P},
					(AGlVertex){p0.x + normal.x * P, p0.y + normal.y * P},

					(AGlVertex){p1.x + normal.x, p1.y + normal.y},
					(AGlVertex){p1.x - normal.x, p1.y - normal.y},
				};
			}

			glBindBuffer (GL_ARRAY_BUFFER, vbo);
			glEnableVertexAttribArray (0);
			glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

			float r = 0.;
			for (int i = 0; i < N; i++, r += M_PI / 28.){
				agl_set_colour_uniform (&agl->shaders.plain->uniforms[PLAIN_COLOUR], 0xffffff00  + i * 0xff / N);

				#define R (RADIUS * SCALE)
				set_line (&vertices,
					(AGlVertex){R + cos(r) * (R - THICKNESS), R + sin(r) * (R - THICKNESS)},
					(AGlVertex){R + R * cos(r), R + R * sin(r)}
				);
				glBufferData (GL_ARRAY_BUFFER, sizeof(AGlQuadVertex), vertices, GL_STATIC_DRAW);
				glDrawArrays(GL_QUADS, 0, V_PER_QUAD);
			}

			glBlendEquation (GL_FUNC_ADD);
		} agl_end_draw_to_fbo;

		glDeleteBuffers (1, &vbo);
	}
}


AGlActor*
wf_spinner (WaveformActor* wf_actor)
{
	agl = agl_get_instance();

	return (AGlActor*)AGL_NEW(WfSpinner,
		.actor = {
			.name = "Spinner",
			.program = (AGlShader*)&rotatable,
			.init = spinner__init,
			.set_state = spinner__set_state,
			.paint = spinner__paint,
			.region = {
				.x1 = 5,
				.y1 = 28,
				.x2 = 10 + 2 * RADIUS,
				.y2 = 28 + 2 * RADIUS,
			}
		}
	);
}


static void
spinner__set_state (AGlActor* actor)
{
	if(agl->use_shaders){
		((AGlUniformUnion*)&actor->program->uniforms[ROTATABLE_COLOUR])->value.i[0] = 0xffffff99;
		actor->program->uniforms[2].value[0] = rotation;
		actor->program->uniforms[3].value[0] = RADIUS;
		actor->program->uniforms[3].value[1] = RADIUS;
	}
}


static bool
spinner__paint (AGlActor* actor)
{
	WfSpinner* spinner = (WfSpinner*)actor;

	if(!spinner->spinning) return true; // TODO

	agl_textured_rect(fbo->texture,
		0.,
		0.,
		2. * RADIUS,
		2. * RADIUS,
		NULL
	);

	rotation += M_PI / 60.;

	return true;
}


static void
on_frame_clock (GdkFrameClock* clock, void* spinner)
{
	agl_actor__invalidate((AGlActor*)spinner);
}


void
wf_spinner_start (WfSpinner* spinner)
{
	g_return_if_fail(spinner);

	if(spinner->spinning) return;

	spinner->spinning = true;

	frame_clock_connect(G_CALLBACK(on_frame_clock), spinner);
	frame_clock_begin_updating();
}


void
wf_spinner_stop (WfSpinner* spinner)
{
	if(!spinner->spinning) return;

	spinner->spinning = false;

	frame_clock_disconnect(NULL, spinner);
	frame_clock_end_updating();

	agl_actor__invalidate((AGlActor*)spinner);
}
