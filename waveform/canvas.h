/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

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
#ifndef __waveform_canvas_h__
#define __waveform_canvas_h__
#include "config.h"
#include <glib.h>
#include <glib-object.h>
#ifdef USE_SDL
#  include "SDL2/SDL.h"
#endif
#include "waveform/typedefs.h"
#include "waveform/shader.h"

#define TYPE_WAVEFORM_CANVAS (waveform_canvas_get_type ())
#define WAVEFORM_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_WAVEFORM_CANVAS, WaveformCanvas))
#define WAVEFORM_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_WAVEFORM_CANVAS, WaveformCanvasClass))
#define IS_WAVEFORM_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_WAVEFORM_CANVAS))
#define IS_WAVEFORM_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_WAVEFORM_CANVAS))
#define WAVEFORM_CANVAS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_WAVEFORM_CANVAS, WaveformCanvasClass))

typedef struct _wf_canvas_priv WfCanvasPriv;

struct _waveform_canvas {
	GObject        parent_instance;

	gboolean       show_rms;
	gboolean       use_1d_textures;
	gboolean       enable_animations;
	gboolean       blend;              // true by default - set to false to increase performance if using without background (doesnt make much difference). Flag currently not honoured in all cases.

	void           (*draw)(WaveformCanvas*, gpointer);
	gpointer       draw_data;

	AGlRootActor*  root;

	WfViewPort*    viewport;
	uint32_t       sample_rate;
	float          rotation;
	float          v_gain;

	WfCanvasPriv*  priv;
	int           _draw_depth;
	AGlTextureUnit* texture_unit[4];
};

#ifdef __wf_canvas_priv__
struct _wf_canvas_priv {
	struct {
		PeakShader*     peak;
		PeakShader*     peak_nonscaling;
		HiResShader*    hires;
		HiResNGShader*  hires_ng;
		BloomShader*    vertical;
		BloomShader*    horizontal;
		RulerShader*    ruler;
		LinesShader*    lines;
		CursorShader*   cursor;
	}              shaders;
#ifdef USE_FRAME_CLOCK
	guint64       _last_redraw_time;
#endif
	guint         _queued;
	guint         pending_init;
};
#endif

struct _WaveformCanvasClass {
	GObjectClass parent_class;
};

struct _vp { double left, top, right, bottom; }; 

WaveformCanvas* wf_canvas_new                       (AGlRootActor*);
#ifdef USE_SDL
WaveformCanvas* wf_canvas_new_sdl                   (SDL_GLContext*);
#endif
void            wf_canvas_free                      (WaveformCanvas*);
void            wf_canvas_set_viewport              (WaveformCanvas*, WfViewPort*);
void            wf_canvas_set_share_list            (WaveformCanvas*);
void            wf_canvas_set_rotation              (WaveformCanvas*, float);
void            wf_canvas_set_gain                  (WaveformCanvas*, float);
WaveformActor*  wf_canvas_add_new_actor             (WaveformCanvas*, Waveform*);
void            wf_canvas_remove_actor              (WaveformCanvas*, WaveformActor*);
void            wf_canvas_queue_redraw              (WaveformCanvas*);
void            wf_canvas_load_texture_from_alphabuf(WaveformCanvas*, int texture_id, AlphaBuf*);

#define wf_canvas_free0(A) (wf_canvas_free(A), A = NULL)

#endif //__waveform_canvas_h__
