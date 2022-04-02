/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform                                     |
 | https://github.com/ayyi/libwaveform                                  |
 | copyright (C) 2012-2022 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 | SONG POSITION POINTER (CURSOR) ACTOR                                 |
 | The time position can be set either by calling spp_actor_set_time()  |
 | or by middle-clicking on the waveform.                               |
 +----------------------------------------------------------------------+
 |
 */

#define __wf_private__

#include "config.h"
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include "agl/event.h"
#include "waveform/actor.h"
#include "waveform/ui-utils.h"
#include "waveform/shader.h"
#include "ui/actors/spp.h"

static AGl* agl = NULL;

static void spp_actor__set_size (AGlActor*);



static void spp_actor__init (AGlActor* actor)
{
#if 0 // TODO use theme object?
#ifdef USE_GTK
		if (!((SppActor*)actor)->text_colour) ((SppActor*)actor)->text_colour = wf_get_gtk_base_color(actor->root->gl.gdk.widget, GTK_STATE_NORMAL, 0xaa);
#endif
#endif
}


static void
spp_actor__set_state (AGlActor* actor)
{
	SppActor* spp = (SppActor*)actor;
	WaveformActor* a = spp->wf_actor;
	if (!a->context) return;

	if (spp->time != WF_SPP_TIME_NONE) {
		((AGlUniformUnion*)&cursor.shader.uniforms[0])->value.i[0] = 0x00ff00ff;
		cursor.uniform.width = spp->play_timeout
			? MAX(2.0, (WF_ACTOR_PX_PER_FRAME(a) / a->context->sample_rate) * (1 << 24) * 4)
			: 2.0;
	}
}


static float
get_x (AGlActor* actor)
{
	SppActor* spp = (SppActor*)actor;
	WaveformActor* a = spp->wf_actor;

	int64_t frame = ((int64_t)spp->time) * a->context->sample_rate / 1000;
	return floorf(wf_actor_frame_to_x(a, frame) - (cursor.uniform.width - 1.0));
}


static bool
spp_actor__paint (AGlActor* actor)
{
	SppActor* spp = (SppActor*)actor;
	WaveformActor* a = spp->wf_actor;
	if (!a || !a->context) return false;

	if (spp->time != WF_SPP_TIME_NONE) {

		agl_rect(
			get_x(actor), 0,
			cursor.uniform.width, agl_actor__height(actor)
		);

		agl_set_font_string("Roboto 16");
		char s[16] = {0,};
		snprintf(s, 15, "%02i:%02i:%03i", (spp->time / 1000) / 60, (spp->time / 1000) % 60, spp->time % 1000);
		// FIXME background is not opaque unless bg is ffffff
		agl_print_with_background(0, 0, 0, spp->text_colour, 0x000000ff, s);
		agl_set_font_string("Roboto 10");
	}

	return true;
}


static bool
spp_event (AGlActor* actor, AGlEvent* event, AGliPt xy)
{
	SppActor* spp = (SppActor*)actor;
	WaveformActor* wf_actor = spp->wf_actor;

	switch (event->type) {
		case AGL_BUTTON_PRESS:
			float x = get_x(actor);
			switch (event->button.button) {
				case 1:
					{
						if (xy.x > x - 2 && xy.x < x + 2) {
							// drag
						}
					}
					break;
				case 3: {
					AGliPt p = agl_actor__find_offset(actor);
					int xa = (int)xy.x - p.x;
					int64_t samples = xa * (wf_actor->context->scaled
						? wf_actor->context->samples_per_pixel
						: wf_actor->region.len / agl_actor__width((AGlActor*)wf_actor)
					);
					wf_spp_actor_set_time(spp, (1000 * samples) / wf_actor->context->sample_rate);

					return AGL_HANDLED;
				}
				default:
					break;
			}
			break;
		default:
			break;
	}
	return AGL_NOT_HANDLED;
}


AGlActor*
wf_spp_actor (WaveformActor* wf_actor)
{
	g_return_val_if_fail(wf_actor, NULL);

	agl = agl_get_instance();

	SppActor* spp = agl_actor__new(SppActor,
		.actor = {
			.name = g_strdup("SPP"),
			.program = (AGlShader*)&cursor,
			.init = spp_actor__init,
			.set_state = spp_actor__set_state,
			.set_size = spp_actor__set_size,
			.paint = spp_actor__paint,
			.on_event = spp_event,
		},
		.wf_actor = wf_actor,
		.time = WF_SPP_TIME_NONE
	);

	return (AGlActor*)spp;
}


static void
spp_actor__set_size (AGlActor* actor)
{
	#define V_BORDER 0

	actor->region = (AGlfRegion){
		.x1 = 0,
		.y1 = V_BORDER,
		.x2 = actor->parent->region.x2,
		.y2 = actor->parent->region.y2 - V_BORDER,
	};
}


static gboolean
check_playback (gpointer _spp)
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
wf_spp_actor_set_time (SppActor* spp, uint32_t time)
{
	g_return_if_fail(spp);

	spp->time = time;

	if (spp->play_timeout) g_source_remove(spp->play_timeout);
	spp->play_timeout = g_timeout_add(100, check_playback, spp);

	agl_actor__invalidate((AGlActor*)spp);
}

