#ifndef __waveform_canvas_h__
#define __waveform_canvas_h__
#include "waveform/typedefs.h"

typedef struct _vp { double left, top, right, bottom; } WfViewPort; 

struct _waveform_canvas {
	gboolean       show_rms;
	gboolean       use_shaders;

	void           (*draw)(WaveformCanvas*, gpointer);
	gpointer       draw_data;

	GdkGLContext*  gl_context;
	GdkGLDrawable* gl_drawable;
	WfViewPort*    viewport;
	uint32_t       sample_rate;
	float          rotation;
	float          v_gain;

	int           _draw_depth;
	int           _program;
};

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
void            wf_actor_load_texture_from_alphabuf (WaveformCanvas*, int texture_id, AlphaBuf*);
void            wf_canvas_use_program               (WaveformCanvas*, int);

#define wf_canvas_free0(A) (wf_canvas_free(A), A = NULL)

#endif //__waveform_canvas_h__
