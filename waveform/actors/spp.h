/*
  copyright (C) 2012-2017 Tim Orford <tim@orford.org>

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
#ifndef __spp_actor_h__
#define __spp_actor_h__

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
