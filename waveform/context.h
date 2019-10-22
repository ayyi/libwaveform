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
#ifndef __waveform_context_h__
#define __waveform_context_h__
#include "config.h"
#include <glib.h>
#include <glib-object.h>
#ifdef USE_SDL
#  include "SDL2/SDL.h"
#endif
#include "waveform/typedefs.h"
#include "waveform/shader.h"
#include "waveform/utils.h"

#define USE_CANVAS_SCALING 1
#define WF_CONTEXT_MIN_ZOOM 0.1
#define WF_CONTEXT_MAX_ZOOM 51200.0

#define TYPE_WAVEFORM_CONTEXT (waveform_context_get_type ())
#define WAVEFORM_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_WAVEFORM_CONTEXT, WaveformContext))
#define WAVEFORM_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_WAVEFORM_CONTEXT, WaveformContextClass))
#define IS_WAVEFORM_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_WAVEFORM_CONTEXT))
#define IS_WAVEFORM_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_WAVEFORM_CONTEXT))
#define WAVEFORM_CONTEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_WAVEFORM_CONTEXT, WaveformContextClass))

typedef struct _WfContextPriv WfContextPriv;

struct _WaveformContext {
	GObject        parent_instance;

	gboolean       show_rms;
	gboolean       use_1d_textures;

	AGlActor*      root;               // note the context root is not neccesarily the scenegraph root

	uint32_t       sample_rate;
	float          bpm;
	float          samples_per_pixel;  // application can specify the base sppx. Is multiplied by zoom to get the actual sppx.
	bool           scaled;             // scaled mode uses the WfContext time scale. Non-scaled mode uses only the actor rect and sample-region.
	float          zoom;
	int64_t        start_time;         // start time measured in frames.
	float          rotation;
	float          v_gain;

	struct {
		RulerShader*    ruler;
	}              shaders;

	WfContextPriv* priv;
	int           _draw_depth;
	AGlTextureUnit* texture_unit[4];
};

#ifdef __wf_canvas_priv__
struct _WfContextPriv {
	WfAnimatable  zoom;               // (float) samples_per_pixel
	WfAnimatable  samples_per_pixel;  // type float

#ifdef USE_FRAME_CLOCK
	guint64       _last_redraw_time;
#endif
	guint         _queued;
	guint         pending_init;
};
#endif

struct _WaveformContextClass {
	GObjectClass parent_class;
};

struct _WfViewPort { double left, top, right, bottom; };

WaveformContext* wf_context_new                       (AGlActor*);
#ifdef USE_SDL
WaveformContext* wf_context_new_sdl                   (SDL_GLContext*);
#endif
void             wf_context_free                      (WaveformContext*);

WaveformActor*   wf_canvas_add_new_actor              (WaveformContext*, Waveform*);

void             wf_context_set_viewport              (WaveformContext*, WfViewPort*);
void             wf_context_set_rotation              (WaveformContext*, float);
#ifdef USE_CANVAS_SCALING
float            wf_context_get_zoom                  (WaveformContext*);
void             wf_context_set_zoom                  (WaveformContext*, float);
#endif
void             wf_context_set_scale                 (WaveformContext*, float samples_per_px);
void             wf_context_set_gain                  (WaveformContext*, float);
void             wf_canvas_queue_redraw               (WaveformContext*);
void             wf_canvas_load_texture_from_alphabuf (WaveformContext*, int texture_id, AlphaBuf*);
float            wf_context_frame_to_x                (WaveformContext*, uint64_t);

#define wf_context_free0(A) (wf_context_free(A), A = NULL)

#endif
