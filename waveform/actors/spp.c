/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
* | SONG POSITION POINTER (CURSOR) ACTOR                                 |
* | The time position can be set either by calling spp_actor_set_time()  |
* | or by middle-clicking on the waveform.                               |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "waveform/shader.h"

static AGl* agl = NULL;


#ifdef USE_GTK
		static bool on_middle_click(GtkWidget* widget, GdkEventButton* event, gpointer _spp)
		{
			SppActor* spp = (SppActor*)_spp;
			WaveformActor* wf_actor = spp->wf_actor;

			if(event->button == 2){
				AGliPt p = agl_actor__find_offset((AGlActor*)_spp);
				int x = (int)event->x - p.x;
				int64_t samples = x * (wf_actor->canvas->scaled
					? wf_actor->canvas->samples_per_pixel
					: wf_actor->region.len / agl_actor__width((AGlActor*)wf_actor)
				);
				wf_spp_actor_set_time((SppActor*)_spp, (1000 * samples) / wf_actor->canvas->sample_rate);
				return true;
			}
			return false;
		}
#endif

	static void spp_actor__init(AGlActor* actor)
	{
#ifdef USE_GTK
		if(!((SppActor*)actor)->text_colour) ((SppActor*)actor)->text_colour = wf_get_gtk_base_color(actor->root->gl.gdk.widget, GTK_STATE_NORMAL, 0xaa);
#endif

#ifdef USE_GTK
		g_signal_connect((gpointer)actor->root->gl.gdk.widget, "button-release-event", G_CALLBACK(on_middle_click), actor);
#endif
	}

	static void spp_actor__set_state(AGlActor* actor)
	{
		SppActor* spp = (SppActor*)actor;
		WaveformActor* a = spp->wf_actor;
		if(!a->canvas) return;

		if(spp->time != WF_SPP_TIME_NONE){
			if(agl->use_shaders){
				cursor.uniform.colour = 0x00ff00ff;
				cursor.uniform.width = spp->play_timeout
					? MAX(2.0, (WF_ACTOR_PX_PER_FRAME(a) / a->canvas->sample_rate) * (1 << 24) * 4)
					: 2.0;
			}else{
				glColor4f(0.0, 1.0, 0.0, 1.0);
				agl_enable(!AGL_ENABLE_TEXTURE_2D | !AGL_ENABLE_BLEND);
			}
		}
	}

	static bool spp_actor__paint(AGlActor* actor)
	{
		SppActor* spp = (SppActor*)actor;
		WaveformActor* a = spp->wf_actor;
		if(!a || !a->canvas) return false;

		if(spp->time != WF_SPP_TIME_NONE){
#if 0
			glStringMarkerGREMEDY(3, "SPP");
#endif
			float width = 1.0;
			if(agl->use_shaders){
				width = cursor.uniform.width;
			}

			int64_t frame = ((int64_t)spp->time) * a->canvas->sample_rate / 1000;
			float x = floorf(wf_actor_frame_to_x(a, frame) - (width - 1.0));
			glTranslatef(x, 0, 0); // TODO should be done using actor position instead.
			agl_rect(
				0, 0,
				width, actor->region.y2 - actor->region.y1
			);
			glTranslatef(-x, 0, 0);

			agl_set_font_string("Roboto 16");
			char s[16] = {0,};
			snprintf(s, 15, "%02i:%02i:%03i", (spp->time / 1000) / 60, (spp->time / 1000) % 60, spp->time % 1000);
			// FIXME background is not opaque unless bg is ffffff
			agl_print_with_background(0, 0, 0, spp->text_colour, 0x000000ff, s);
			agl_set_font_string("Roboto 10");
		}
		return true;
	}

	static void spp_actor__set_size(AGlActor* actor)
	{
		#define V_BORDER 0

		actor->region = (AGlfRegion){
			.x1 = 0,
			.y1 = V_BORDER,
			.x2 = actor->parent->region.x2,
			.y2 = actor->parent->region.y2 - V_BORDER,
		};
	}

AGlActor*
wf_spp_actor(WaveformActor* wf_actor)
{
	g_return_val_if_fail(wf_actor, NULL);

	agl = agl_get_instance();

	SppActor* spp = AGL_NEW(SppActor,
		.actor = {
#ifdef AGL_DEBUG_ACTOR
			.name = "SPP",
#endif
			.program = (AGlShader*)&cursor,
			.init = spp_actor__init,
			.set_state = spp_actor__set_state,
			.set_size = spp_actor__set_size,
			.paint = spp_actor__paint,
		},
		.wf_actor = wf_actor,
		.time = WF_SPP_TIME_NONE
	);

	return (AGlActor*)spp;
}


	static bool check_playback(gpointer _spp)
	{
		SppActor* spp = _spp;

		spp->play_timeout = 0;
		agl_actor__invalidate((AGlActor*)spp);

		return G_SOURCE_REMOVE;
	}

/*
 *  Set the current playback position in milliseconds
 */
void
wf_spp_actor_set_time(SppActor* spp, uint32_t time)
{
	g_return_if_fail(spp);

	spp->time = time;

	if(spp->play_timeout) g_source_remove(spp->play_timeout);
	spp->play_timeout = g_timeout_add(100, check_playback, spp);

	agl_actor__invalidate((AGlActor*)spp);
}

