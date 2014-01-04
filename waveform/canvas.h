/*
  copyright (C) 2012-2013 Tim Orford <tim@orford.org>

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
#include <glib.h>
#include <glib-object.h>
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

	GdkGLContext*  gl_context;
	GdkGLDrawable* gl_drawable;
	WfViewPort*    viewport;
	uint32_t       sample_rate;
	float          rotation;
	float          v_gain;

	WfCanvasPriv*  priv;
	int           _draw_depth;
	guint         _queued;
	guint64       _last_redraw_time;
	TextureUnit*   texture_unit[4];
};

#ifdef __wf_canvas_priv__
struct _wf_canvas_priv {
	struct {
		PeakShader*     peak;
//#ifdef USE_FBO // cannot use this unless is really private
		PeakShader*     peak_nonscaling;
//#endif
		HiResShader*    hires;
		BloomShader*    vertical;
		BloomShader*    horizontal;
		AlphaMapShader* tex2d;
		RulerShader*    ruler;
		LinesShader*    lines;
	}              shaders;
};
#endif

struct _WaveformCanvasClass {
	GObjectClass parent_class;
};

struct _vp { double left, top, right, bottom; }; 

WaveformCanvas* wf_canvas_new                       (GdkGLContext*, GdkGLDrawable*);
WaveformCanvas* wf_canvas_new_from_widget           (GtkWidget*);
void            wf_canvas_free                      (WaveformCanvas*);
void            wf_canvas_set_use_shaders           (WaveformCanvas*, gboolean);
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
