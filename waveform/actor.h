/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __waveform_actor_h__
#define __waveform_actor_h__
#include "transition/transition.h"
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "waveform/context.h"
#include "waveform/actors/group.h"
#include "waveform/actors/background.h"
#include "waveform/actors/ruler.h"
#include "waveform/actors/grid.h"
#include "waveform/actors/labels.h"
#include "waveform/actors/spp.h"
#include "waveform/actors/spinner.h"

#define MULTILINE_SHADER
#undef MULTILINE_SHADER

typedef struct _actor_priv WfActorPriv;
typedef void    (*WaveformActorFn) (WaveformActor*, gpointer);

struct _WaveformActor {
	AGlActor        actor;
	WaveformContext* canvas;
	Waveform*       waveform;
	WfSampleRegion  region;
	uint32_t        fg_colour;
	float           vzoom;     // vertical zoom. default 1.0
	float           z;         // render position on z-axis.

	WfActorPriv*    priv;
};

AGlActorClass* wf_actor_get_class            ();

void           wf_actor_set_waveform         (WaveformActor*, Waveform*, WaveformActorFn, gpointer);
void           wf_actor_set_waveform_sync    (WaveformActor*, Waveform*);
void           wf_actor_set_region           (WaveformActor*, WfSampleRegion*);
void           wf_actor_set_rect             (WaveformActor*, WfRectangle*);
void           wf_actor_set_colour           (WaveformActor*, uint32_t fg_colour);
void           wf_actor_set_full             (WaveformActor*, WfSampleRegion*, WfRectangle*, int time, WaveformActorFn, gpointer);
void           wf_actor_fade_out             (WaveformActor*, WaveformActorFn, gpointer);
void           wf_actor_fade_in              (WaveformActor*, float, WaveformActorFn, gpointer);
void           wf_actor_set_vzoom            (WaveformActor*, float);
WfAnimatable*  wf_actor_get_z                (WaveformActor*);
void           wf_actor_set_z                (WaveformActor*, float, WaveformActorFn, gpointer);
void           wf_actor_get_viewport         (WaveformActor*, WfViewPort*);
float          wf_actor_frame_to_x           (WaveformActor*, uint64_t);
void           wf_actor_clear                (WaveformActor*);

#define        WF_ACTOR_PX_PER_FRAME(A) (agl_actor__width((AGlActor*)A) / A->region.len)

#endif
