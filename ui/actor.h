/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2013-2023 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#ifndef __waveform_actor_h__
#define __waveform_actor_h__

#include "transition/transition.h"
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "waveform/context.h"

typedef struct _WfActorPriv WfActorPriv;
typedef void    (*WaveformActorFn) (WaveformActor*, gpointer);

#ifdef DEBUG
typedef enum {
	RENDER_RESULT_OK = 0,
	RENDER_RESULT_LOADING,
	RENDER_RESULT_SIZE,
	RENDER_RESULT_NO_BLOCKS,
	RENDER_RESULT_BLOCK_RANGE,
	RENDER_RESULT_NO_AUDIO_DATA,
	RENDER_RESULT_BAD
} RenderResult;
#endif

struct _WaveformActor {
	AGlActor         actor;
	WaveformContext* context;
	Waveform*        waveform;
	WfSampleRegion   region;
	float            vzoom;     // vertical zoom. default 1.0
	float            z;         // render position on z-axis.

	WfActorPriv*     priv;
#ifdef DEBUG
	RenderResult     render_result;
#endif
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
