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

  ----------------------------------------------------------------

  Song Position Pointer (cursor) actor
  ------------------------------------

  The time position can be set either by calling spp_actor_set_time()
  or by middle-clicking on the waveform.

*/
#define __wf_private__
#define __wf_canvas_priv__ // for scaled
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "waveform/shader.h"

static AGl* agl = NULL;


AGlActor*
spp_actor(WaveformActor* wf_actor)
{
	g_return_val_if_fail(wf_actor, NULL);

	agl = agl_get_instance();

	void spp_actor__init(AGlActor* actor)
	{
		if(!((SppActor*)actor)->text_colour) ((SppActor*)actor)->text_colour = wf_get_gtk_base_color(actor->root->widget, GTK_STATE_NORMAL, 0xaa);

		bool on_middle_click(GtkWidget* widget, GdkEventButton* event, gpointer _spp)
		{
			SppActor* spp = (SppActor*)_spp;
			WaveformActor* wf_actor = spp->wf_actor;

			if(event->button == 2){
				AGliPt p = agl_actor__find_offset((AGlActor*)_spp);
				int x = (int)event->x - p.x;
				int64_t samples = x * (wf_actor->canvas->priv->scaled
					? wf_actor->canvas->samples_per_pixel
					: wf_actor->region.len / wf_actor->rect.len
				);
				spp_actor_set_time((SppActor*)_spp, (1000 * samples) / wf_actor->canvas->sample_rate);
				return true;
			}
			return false;
		}
		g_signal_connect((gpointer)actor->root->widget, "button-release-event", G_CALLBACK(on_middle_click), actor);
	}

	void spp_actor__set_state(AGlActor* actor)
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

	bool spp_actor__paint(AGlActor* actor)
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
			agl_print(2, 2, 0, spp->text_colour, s);
			agl_set_font_string("Roboto 10");
		}
		return true;
	}

	void spp_actor__set_size(AGlActor* actor)
	{
		#define V_BORDER 0

		actor->region = (AGliRegion){
			.x1 = 0,
			.y1 = V_BORDER,
			.x2 = actor->parent->region.x2,
			.y2 = actor->parent->region.y2 - V_BORDER,
		};
	}

	SppActor* spp = g_new0(SppActor, 1);
	AGlActor* actor = (AGlActor*)spp;
#ifdef AGL_DEBUG_ACTOR
	actor->name = "SPP";
#endif
	actor->program = (AGlShader*)&cursor;
	actor->init = spp_actor__init;
	actor->set_state = spp_actor__set_state;
	actor->set_size = spp_actor__set_size;
	actor->paint = spp_actor__paint;

	spp->wf_actor = wf_actor;
	spp->time = WF_SPP_TIME_NONE;

	return actor;
}


/*
 *  Set the current playback position in milliseconds
 */
void
spp_actor_set_time(SppActor* spp, uint32_t time)
{
	g_return_if_fail(spp);

	bool check_playback(gpointer _spp)
	{
		SppActor* spp = _spp;

		spp->play_timeout = 0;
		gtk_widget_queue_draw(((AGlActor*)spp)->root->widget);

		return G_SOURCE_REMOVE;
	}

	spp->time = time;

	if(spp->play_timeout) g_source_remove(spp->play_timeout);
	spp->play_timeout = g_timeout_add(100, check_playback, spp);

	gtk_widget_queue_draw(((AGlActor*)spp)->root->widget);
}

