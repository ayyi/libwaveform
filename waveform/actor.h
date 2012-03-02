/*
  copyright (C) 2012 Tim Orford <tim@orford.org>

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
#include "canvas.h"
#include "peak.h"

#define WF_SAMPLES_PER_TEXTURE (WF_PEAK_RATIO * WF_PEAK_TEXTURE_SIZE)

#define HAVE_NON_SQUARE_TEXTURES //TODO add runtime detection

typedef struct
{
	float left;
	float top;
	float len;
	float height;
} WfRectangle;

typedef struct _actor_priv WfActorPriv;

struct _waveform_actor {
	WaveformCanvas* canvas;   //TODO decide if this is a good idea or not. confusing but reduces fn args.
	Waveform*       waveform;
	WfSampleRegion  region;
	WfRectangle     rect;
	uint32_t        fg_colour;
	uint32_t        bg_colour;

	WfActorPriv*    priv;
};

void            wf_actor_free                             (WaveformActor*);
void            wf_actor_set_region                       (WaveformActor*, WfSampleRegion*);
void            wf_actor_set_colour                       (WaveformActor*, uint32_t fg_colour, uint32_t bg_colour);
void            wf_actor_allocate                         (WaveformActor*, WfRectangle*);
void            wf_actor_paint                            (WaveformActor*);
void            wf_actor_paint_hi                         (WaveformActor*); //tmp

#endif //__waveform_actor_h__
