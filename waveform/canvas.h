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
#ifndef __waveform_canvas_h__
#define __waveform_canvas_h__
#include "waveform/typedefs.h"
#include "waveform/shader.h"

typedef struct _wf_canvas_priv WfCanvasPriv;

struct _waveform_canvas {
	gboolean       show_rms;
	//gboolean       use_shaders; // is now global
	gboolean       use_1d_textures;

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
	int           _program;
	guint         _queued;
	TextureUnit*   texture_unit[4];
};

#ifdef __wf_canvas_priv__
struct _wf_canvas_priv {
	struct {
		PeakShader*     peak;
		HiResShader*    hires;
		BloomShader*    vertical;
		BloomShader*    horizontal;
		AlphaMapShader* tex2d;
		AlphaMapShader* tex2d_b;
		RulerShader*    ruler;
	}              shaders;
};
#endif

struct _vp { double left, top, right, bottom; }; 

WaveformCanvas* wf_canvas_new                       (GdkGLContext*, GdkGLDrawable*);
WaveformCanvas* wf_canvas_new_from_widget           (GtkWidget*);
void            wf_canvas_free                      (WaveformCanvas*);
void            wf_canvas_set_use_shaders           (WaveformCanvas*, gboolean);
void            wf_canvas_set_viewport              (WaveformCanvas*, WfViewPort*);
void            wf_canvas_set_share_list            (WaveformCanvas*);
void            wf_canvas_set_rotation              (WaveformCanvas*, float);
WaveformActor*  wf_canvas_add_new_actor             (WaveformCanvas*, Waveform*);
void            wf_canvas_remove_actor              (WaveformCanvas*, WaveformActor*);
void            wf_canvas_queue_redraw              (WaveformCanvas*);
void            wf_canvas_load_texture_from_alphabuf(WaveformCanvas*, int texture_id, AlphaBuf*);
void            wf_canvas_use_program               (WaveformCanvas*, int);
#ifdef __gl_h_
void            wf_canvas_use_program_              (WaveformCanvas*, WfShader*);
#endif

#define wf_canvas_free0(A) (wf_canvas_free(A), A = NULL)

#endif //__waveform_canvas_h__
