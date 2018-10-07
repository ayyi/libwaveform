/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2018 Tim Orford <tim@orford.org>                  |
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include "transition/frameclock.h"
#include "agl/actor.h"
#include "agl/ext.h"
#include "waveform/waveform.h"
#include "waveform/shader.h"

#define RADIUS 16

static AGl* agl = NULL;

AGlFBO* fbo = NULL;
static float rotation = 0;


	static void spinner__init(AGlActor* actor)
	{
		if(!fbo){
			fbo = agl_fbo_new(2 * RADIUS, 2 * RADIUS, 0, 0);
			agl_draw_to_fbo(fbo) {
				glClearColor(1.0, 1.0, 1.0, 0.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				agl->shaders.plain->uniform.colour = 0x1133bbff;
				agl_use_program((AGlShader*)agl->shaders.plain);
				glTranslatef(RADIUS, RADIUS, 0);
				int i; for(i=0;i<160;i++){
					agl->shaders.plain->uniform.colour = 0x1133bb00 + i * 0xff / 160;
					agl->shaders.plain->shader.set_uniforms_();
					agl_rect(0, RADIUS * 3 / 4, 1, RADIUS / 4);
					glRotatef(-2, 0, 0, 1);
				}
			} agl_end_draw_to_fbo;
		}
	}

	static void spinner__set_state(AGlActor* actor)
	{
		if(agl->use_shaders){
			((AlphaMapShader*)actor->program)->uniform.fg_colour = 0xffffff99;
		}
	}

	static bool spinner__paint(AGlActor* actor)
	{
		WfSpinner* spinner = (WfSpinner*)actor;

		if(!spinner->spinning) return true; // TODO

		glPushMatrix();
		glTranslatef(RADIUS, RADIUS, 0.0 - 0);
		glRotatef(rotation, 0.0, 0.0, 1.0);
		agl_textured_rect(fbo->texture,
			-RADIUS,
			-RADIUS,
			2 * RADIUS,
			2 * RADIUS,
			NULL
		);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(RADIUS, RADIUS, 0.0 - 0);
		glRotatef(-2.5 * rotation, 0.0, 0.0, 1.0);
		glScalef(-1.0, 1.0, 1.0);
		agl_textured_rect(fbo->texture,
			RADIUS,
			RADIUS,
			-2 * RADIUS,
			-2 * RADIUS,
			NULL
		);
		glPopMatrix();

		rotation += 2.0;

		return true;
	}

AGlActor*
wf_spinner(WaveformActor* wf_actor)
{
	agl = agl_get_instance();

	WfSpinner* spinner = g_new0(WfSpinner, 1);
	*spinner = (WfSpinner){
		.actor = {
			.name = "Spinner",
			.program = (AGlShader*)agl->shaders.alphamap,
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
	};

	return (AGlActor*)spinner;
}


	static void on_update(GdkFrameClock* clock, void* spinner)
	{
		agl_actor__invalidate((AGlActor*)spinner);
	}
void
wf_spinner_start(WfSpinner* spinner)
{
	g_return_if_fail(spinner);

	if(spinner->spinning) return;

	spinner->spinning = true;

	frame_clock_connect(G_CALLBACK(on_update), spinner);
	frame_clock_begin_updating();
}


void
wf_spinner_stop(WfSpinner* spinner)
{
	if(!spinner->spinning) return;

	spinner->spinning = false;

	frame_clock_disconnect(NULL, spinner);
	frame_clock_end_updating();

	agl_actor__invalidate((AGlActor*)spinner);
}


