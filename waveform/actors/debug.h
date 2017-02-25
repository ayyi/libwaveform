/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2016-2017 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __debug_actor_h__
#define __debug_actor_h__

typedef struct {
    AGlActor    actor;
	AGlActor*   target;
} DebugActor;

AGlActor* wf_debug_actor           (WaveformActor*);
void      wf_debug_actor_set_actor (DebugActor*, AGlActor*);

#endif
