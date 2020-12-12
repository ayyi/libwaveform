/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
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

#ifndef __spp_actor_h__
#define __spp_actor_h__

#include "agl/actor.h"

typedef struct {
    AGlActor       actor;
	WaveformActor* wf_actor;     // The WfActor is needed to find positions when the WfContext is in non-scaled mode
	uint32_t       text_colour;
    uint32_t       time;         // milliseconds (maximum of 1193 hours)
    uint32_t       play_timeout;
} SppActor;

AGlActor* wf_spp_actor          (WaveformActor*);
void      wf_spp_actor_set_time (SppActor*, uint32_t);

#define WF_SPP_TIME_NONE UINT32_MAX

#endif
