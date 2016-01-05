/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2016 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __spinner_actor_h__
#define __spinner_actor_h__

typedef struct {
    AGlActor       actor;
	bool           spinning;
} WfSpinner;

AGlActor* wf_spinner       (WaveformActor*);
void      wf_spinner_start (WfSpinner*);
void      wf_spinner_stop  (WfSpinner*);

#endif
