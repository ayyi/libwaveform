/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform                                     |
 | https://github.com/ayyi/libwaveform                                  |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
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
#include "agl/behaviours/follow.h"
#include "waveform/actor.h"
#include "waveform/ui-utils.h"
#include "waveform/shader.h"
#include "ui/actors/spp.h"

static AGl* agl = NULL;



#ifdef USE_GTK
		static bool on_middle_click (GtkWidget* widget, GdkEventButton* event, gpointer _spp)
		{
			SppActor* spp = (SppActor*)_spp;
			WaveformActor* wf_actor = spp->wf_actor;

			if(event->button == 2){
				AGliPt p = agl_actor__find_offset((AGlActor*)_spp);
				int x = (int)event->x - p.x;
				int64_t samples = x * (wf_actor->context->scaled
					? wf_actor->context->samples_per_pixel
					: wf_actor->region.len / agl_actor__width((AGlActor*)wf_actor)
				);
				wf_spp_actor_set_time((SppActor*)_spp, (1000 * samples) / wf_actor->context->sample_rate);
				return true;
			}
			return false;
		}
#endif

	static void spp_actor__init (AGlActor* actor)
	{
#ifdef USE_GTK
		if (!((SppActor*)actor)->text_colour) ((SppActor*)actor)->text_colour = wf_get_gtk_base_color(actor->root->gl.gdk.widget, GTK_STATE_NORMAL, 0xaa);
#endif

#ifdef USE_GTK
		g_signal_connect((gpointer)actor->root->gl.gdk.widget, "button-release-event", G_CALLBACK(on_middle_click), actor);
#endif

		agl_actor__set_size(actor->parent);
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


static bool
spp_actor__paint (AGlActor* actor)
{
	SppActor* spp = (SppActor*)actor;
	WaveformActor* a = spp->wf_actor;
	if (!a || !a->context) return false;

	if (spp->time != WF_SPP_TIME_NONE) {
		float width = 1.0;
		if (agl->use_shaders) {
			width = cursor.uniform.width;
		}

		int64_t frame = ((int64_t)spp->time) * a->context->sample_rate / 1000;
		float x = floorf(wf_context_frame_to_x(a->context, frame) - (width - 1.0));
		agl_rect(
			x, 0,
			width, agl_actor__height(actor)
		);

		agl_set_font_string("Roboto 16");
		char s[16] = {0,};
		snprintf(s, 15, "%02i:%02i:%03i", (spp->time / 1000) / 60, (spp->time / 1000) % 60, spp->time % 1000);
		// FIXME background is not opaque unless bg is ffffff
		agl_print_with_background(-actor->scrollable.x1, 0, 0, spp->text_colour, 0x000000ff, s);
		agl_set_font_string("Roboto 10");
	}

	return true;
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
			.paint = spp_actor__paint,
		},
		.wf_actor = wf_actor,
		.time = WF_SPP_TIME_NONE
	);

	AGlBehaviour* f = agl_actor__add_behaviour((AGlActor*)spp, follow());
	((FollowBehaviour*)f)->to_follow = (AGlActor*)wf_actor;

	return (AGlActor*)spp;
}


#ifdef HAVE_GLIB_2_76
static void
#else
static gboolean
#endif
check_playback (gpointer _spp)
{
	SppActor* spp = _spp;

	spp->play_timeout = 0;
	agl_actor__invalidate((AGlActor*)spp);

#ifndef HAVE_GLIB_2_76
	return G_SOURCE_REMOVE;
#endif
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
#ifdef HAVE_GLIB_2_76
	spp->play_timeout = g_timeout_add_once(100, check_playback, spp);
#else
	spp->play_timeout = g_timeout_add(100, check_playback, spp);
#endif

	agl_actor__invalidate((AGlActor*)spp);
}

