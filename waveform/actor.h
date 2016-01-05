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
*/
#ifndef __waveform_actor_h__
#define __waveform_actor_h__
#include <gtk/gtkgl.h>
#include "transition/transition.h"
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "waveform/context.h"
#include "waveform/actors/group.h"
#include "waveform/actors/background.h"
#include "waveform/actors/ruler.h"
#include "waveform/actors/grid.h"
#include "waveform/actors/spp.h"
#include "waveform/actors/spinner.h"

#define HAVE_NON_SQUARE_TEXTURES // TODO change to runtime detection (indicated by AGL->have_npot_textures)

#define MULTILINE_SHADER
#undef MULTILINE_SHADER

typedef struct _actor_priv WfActorPriv;
typedef void    (*WaveformActorFn) (WaveformActor*, gpointer);

struct _WaveformActor {
	AGlActor        actor;
	WaveformCanvas* canvas;
	Waveform*       waveform;
	WfSampleRegion  region;
	WfRectangle     rect;
	uint32_t        fg_colour;
	float           vzoom;     // vertical zoom. default 1.0
	float           z;         // render position on z-axis.

	WfActorPriv*    priv;
};

void            wf_actor_free                       (WaveformActor*);
void            wf_actor_set_waveform               (WaveformActor*, Waveform*, WaveformActorFn, gpointer);
void            wf_actor_set_waveform_sync          (WaveformActor*, Waveform*);
void            wf_actor_set_region                 (WaveformActor*, WfSampleRegion*);
void            wf_actor_set_rect                   (WaveformActor*, WfRectangle*);
void            wf_actor_set_colour                 (WaveformActor*, uint32_t fg_colour);
void            wf_actor_set_z                      (WaveformActor*, float);
void            wf_actor_set_full                   (WaveformActor*, WfSampleRegion*, WfRectangle*, int time, WaveformActorFn, gpointer);
void            wf_actor_fade_out                   (WaveformActor*, WaveformActorFn, gpointer);
void            wf_actor_fade_in                    (WaveformActor*, void* /*WfAnimatable* */, float, WaveformActorFn, gpointer);
void            wf_actor_set_vzoom                  (WaveformActor*, float);
void            wf_actor_get_viewport               (WaveformActor*, WfViewPort*);
float           wf_actor_frame_to_x                 (WaveformActor*, uint64_t);
void            wf_actor_clear                      (WaveformActor*);

#define         WF_ACTOR_PX_PER_FRAME(A) (A->rect.len / A->region.len)

#endif //__waveform_actor_h__
