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
