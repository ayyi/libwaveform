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
*/
#ifndef __waveform_actor_h__
#define __waveform_actor_h__
#include <gtk/gtkgl.h>
#include "transition/transition.h"
#include "canvas.h"
#include "peak.h"

#define HAVE_NON_SQUARE_TEXTURES // TODO change to runtime detection (indicated by AGL->have_npot_textures)

#define MULTILINE_SHADER
#undef MULTILINE_SHADER

typedef struct _actor_priv WfActorPriv;
typedef void    (*WaveformActorFn) (WaveformActor*, gpointer);

struct _waveform_actor {
	WaveformCanvas* canvas;   //TODO decide if this is a good idea or not. confusing but reduces fn args.
	Waveform*       waveform;
	WfSampleRegion  region;
	WfRectangle     rect;
	uint32_t        fg_colour;
	uint32_t        bg_colour;
	float           vzoom;     // vertical zoom. default 1.0
	float           z;         // render position on z-axis.

	WfActorPriv*    priv;
};

void            wf_actor_free                       (WaveformActor*);
void            wf_actor_set_region                 (WaveformActor*, WfSampleRegion*);
void            wf_actor_set_rect                   (WaveformActor*, WfRectangle*);
void            wf_actor_set_colour                 (WaveformActor*, uint32_t fg_colour, uint32_t bg_colour);
void            wf_actor_set_z                      (WaveformActor*, float);
void            wf_actor_set_full                   (WaveformActor*, WfSampleRegion*, WfRectangle*, int time, WaveformActorFn, gpointer);
void            wf_actor_fade_out                   (WaveformActor*, WaveformActorFn, gpointer);
void            wf_actor_fade_in                    (WaveformActor*, void* /*WfAnimatable* */, float, WaveformActorFn, gpointer);
void            wf_actor_set_vzoom                  (WaveformActor*, float);
gboolean        wf_actor_paint                      (WaveformActor*);
void            wf_actor_get_viewport               (WaveformActor*, WfViewPort*);
float           wf_actor_frame_to_x                 (WaveformActor*, uint64_t);

#ifdef USE_TEST
// access to private data
GList*          wf_actor_get_transitions            (WaveformActor*);
#endif

#endif //__waveform_actor_h__
