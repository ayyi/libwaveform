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

  ---------------------------------------------------------------

  WaveformActor draws a Waveform object onto a shared opengl drawable.

  The rendering process depends on magnification and hardwave capabilities,
  but for at STD resolution with shaders available, and both USE_FBO and
  'multipass' are defined, the method for each block is as follows:

  1- when the waveform is loaded, the peakdata is copied to 1d textures.
  2- on a horizontal zoom change, an fbo is created that maps the 1d peakdata
     to a 2d map using the 'peak_shader'. This texture is independent of
     colour and vertical-zoom.
  3- on each expose, a single rectangle is drawn. The Vertical shader is used
     to apply vertical convolution to the 2d texture.

  animated transitions:

  WaveformActor has internal support for animating the following properties:
  region start and end, start and end position onscreen (zoom is derived
  from these) and opacity.

  Where an external animation framework is available, it should be used
  in preference, eg Clutter (TODO).

  ---------------------------------------------------------------

  TODO

  fbos do not use the texture cache

*/
#define __actor_c__
#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "agl/ext.h"
#include "transition/transition.h"
#include "waveform/typedefs.h"
#include "waveform/utils.h"
#include "waveform/gl_utils.h"
#include "waveform/peak.h"
#include "waveform/audio.h"
#include "waveform/texture_cache.h"
#include "waveform/alphabuf.h"
#include "waveform/fbo.h"
#include "waveform/actor.h"
static AGl* agl = NULL;
#ifdef USE_FBO
#define multipass
#endif
																												extern int n_loads[4096];

#define HIRES_NONSHADER_TEXTURES // work in progress
                                 // because of the issue with missing peaks with reduced size textures without shaders, this option is possibly unwanted.
#undef HIRES_NONSHADER_TEXTURES

/*
	TODO LOD / Mipmapping

	For both the shader and non-shader case, it would be useful to have
	mipmapping to have proper visibility of short term peaks and/or to reduce
	the number of expensive (oversampled) texture lookups. Unfortunately
	Opengl mipmaps cannot be used as we only want detail to be reduced on
	the x axis and not the y, so a custom solution is needed.

	This is now fully implemented for the shader case at HI_RES but still
	needs to be done for MED and LOW.

 */
#define USE_MIPMAPPING
#undef USE_MIPMAPPING

#undef RECT_ROUNDING

typedef struct {
   int first;
   int last;
} BlockRange;

typedef struct {
   double start, end;
} TextureRange;

typedef struct {
   double start, wid;
} QuadExtent;

typedef struct _RenderInfo RenderInfo;
typedef struct _Renderer Renderer;

struct _actor_priv
{
	float           opacity;     // derived from background colour

	struct Animatable {
		WfAnimatable  start;     // (int) region
		WfAnimatable  len;       // (int) region
		WfAnimatable  rect_left; // (float)
		WfAnimatable  rect_len;  // (float)
		WfAnimatable  z;         // (float)
		WfAnimatable  opacity;   // (float)
	}               animatable;
	GList*          transitions; // list of WfAnimation*

	gulong          peakdata_ready_handler;
	gulong          dimensions_changed_handler;
	gulong          use_shaders_changed_handler;

	// cached values used for rendering. cleared when rect/region/viewport changed.
	struct _RenderInfo {
		bool           valid;
		Renderer*      renderer;
		WfViewPort     viewport;
		WfSampleRegion region;
		WfRectangle    rect;
		double         zoom;
		Mode           mode;
		int            samples_per_texture;
		int            n_blocks;
		float          peaks_per_pixel;
		float          peaks_per_pixel_i;
		int            first_offset;
		double         first_offset_px;
		double         block_wid;
		int            region_start_block;
		int            region_end_block;
		BlockRange     viewport_blocks;
	}               render_info;
};


typedef void    (*WaveformActorNewFn)       (WaveformActor*);
typedef void    (*WaveformActorPreRenderFn) (Renderer*, WaveformActor*);
typedef void    (*WaveformActorBlockFn)     (Renderer*, WaveformActor*, int b);
typedef bool    (*WaveformActorRenderFn)    (Renderer*, WaveformActor*, int b, bool is_first, bool is_last, double x);
typedef void    (*WaveformActorFreeFn)      (Renderer*, Waveform*);

struct _Renderer
{
	Mode                     mode;

	WaveformActorNewFn       new;
	WaveformActorBlockFn     load_block;
	WaveformActorPreRenderFn pre_render;
	WaveformActorRenderFn    render_block;
	WaveformActorFreeFn      free;
};

typedef struct
{
	Renderer                 renderer;
	WfSampleRegion           block_region;
} HiRenderer;

typedef struct
{
	Renderer                 renderer;
	WfSampleRegion           block_region_v_hi;
} VHiRenderer;

static double wf_actor_samples2gl(double zoom, uint32_t n_samples);

static inline float get_peaks_per_pixel_i    (WaveformCanvas*, WfSampleRegion*, WfRectangle*, int mode);
static inline float get_peaks_per_pixel      (WaveformCanvas*, WfSampleRegion*, WfRectangle*, int mode);
static inline void _draw_block               (float tex_start, float tex_pct, float x, float y, float width, float height, float gain);
static inline void _draw_block_from_1d       (float tex_start, float tex_pct, float x, float y, float width, float height, int tsize);

#ifdef USE_FBO
static AglFBO* fbo_test = NULL;
#endif

typedef struct _a
{
	guchar positive[WF_PEAK_TEXTURE_SIZE * 16];
	guchar negative[WF_PEAK_TEXTURE_SIZE * 16];
} IntBufHi;

typedef void (MakeTextureData)(Waveform*, int ch, IntBufHi*, int blocknum);
static MakeTextureData
	make_texture_data_hi;

struct _draw_mode
{
	char             name[4];
	int              resolution;
	int              texture_size;      // mostly applies to 1d textures. 2d textures have non-square issues.
	MakeTextureData* make_texture_data; // might not be needed after all
	Renderer*        renderer;
} modes[N_MODES] = {
	{"V_LO", 16384, WF_PEAK_TEXTURE_SIZE,      NULL},
	{"LOW",   1024, WF_PEAK_TEXTURE_SIZE,      NULL},
	{"MED",    256, WF_PEAK_TEXTURE_SIZE,      NULL},
	{"HI",      16, WF_PEAK_TEXTURE_SIZE * 16, NULL}, // texture size chosen so that blocks are the same as in medium res
	{"V_HI",     1, WF_PEAK_TEXTURE_SIZE,      NULL},
};
#define HI_RESOLUTION modes[MODE_HI].resolution
#define RES_MED modes[MODE_MED].resolution
typedef struct { int lower; int upper; } ModeRange;
#define HI_MIN_TIERS 3 // equivalent to resolution of 1:16

static inline Mode get_mode                  (double zoom);
static ModeRange   mode_range                (WaveformActor*);

static void   wf_actor_load_texture1d        (Waveform*, Mode, WfGlBlock*, int b);
static void   wf_actor_load_texture2d        (WaveformActor*, Mode, int t, int b);

#if defined (USE_FBO) && defined (multipass)
static void   block_to_fbo                   (WaveformActor*, int b, WfGlBlock*, int resolution);
#endif

static bool   wf_actor_get_quad_dimensions   (WaveformActor*, int b, bool is_first, bool is_last, double x, TextureRange*, double* tex_x, double* block_wid, int border, int multiplier);
static int    wf_actor_get_n_blocks          (Waveform*, Mode);

#include "waveform/renderer_ng.c"
#include "waveform/med_res.c"
#include "waveform/hi_res_gl2.c"
#include "waveform/hi_res.c"
#include "waveform/v_hi_res.c"
#include "waveform/v_low_res.c"

static void   wf_actor_canvas_finalize_notify(gpointer, GObject*);
static void   wf_actor_start_transition      (WaveformActor*, GList* /* WfAnimatable* */, AnimationFn, gpointer);
static void   wf_actor_on_animation_finished (WfAnimation*, gpointer);
static void  _wf_actor_load_missing_blocks   (WaveformActor*);
static void   wf_actor_on_use_shaders_change ();
#if 0
static int    wf_actor_get_first_visible_block(WfSampleRegion*, double zoom, WfRectangle*, WfViewPort*);
#endif
static void  _wf_actor_get_viewport_max       (WaveformActor*, WfViewPort*);


static void
wf_actor_class_init()
{
	get_gl_extensions();
	agl = agl_get_instance();

	modes[MODE_HI].make_texture_data = make_texture_data_hi;

	modes[MODE_V_LOW].renderer = v_lo_renderer_new();
	modes[MODE_LOW].renderer = &lo_renderer;
	modes[MODE_MED].renderer = med_renderer_new();
	modes[MODE_HI].renderer = hi_renderer_new();
	modes[MODE_V_HI].renderer = (Renderer*)&v_hi_renderer;

	wf_actor_on_use_shaders_change();
}


WaveformActor*
wf_actor_new(Waveform* w, WaveformCanvas* wfc)
{
	dbg(2, "%s-------------------------%s", "\x1b[1;33m", "\x1b[0;39m");

	g_return_val_if_fail(wfc, NULL);

	uint32_t get_frame(int time)
	{
		return 0;
	}

	if(!modes[MODE_LOW].renderer) wf_actor_class_init();

	waveform_get_n_frames(w);
	if(!w->renderable) return NULL;

	WaveformActor* a = g_new0(WaveformActor, 1);
	a->canvas = wfc;
	a->priv = g_new0(WfActorPriv, 1);
	WfActorPriv* _a = a->priv;

	a->vzoom = 1.0;
	wf_actor_set_colour(a, 0xffffffff, 0x000000ff);
	a->waveform = g_object_ref(w);

	_a->animatable = (struct Animatable){
		.start = (WfAnimatable){
			.model_val.i = (uint32_t*)&a->region.start, //TODO add uint64_t to union
			.start_val.i = a->region.start,
			.val.i       = a->region.start,
			.type        = WF_INT
		},
		.len = (WfAnimatable){
			.model_val.i = &a->region.len,
			.start_val.i = a->region.len,
			.val.i       = a->region.len,
			.type        = WF_INT
		},
		.rect_left = (WfAnimatable){
			.model_val.f = &a->rect.left,
			.start_val.f = a->rect.left,
			.val.f       = a->rect.left,
			.type        = WF_FLOAT
		},
		.rect_len = (WfAnimatable){
			.model_val.f = &a->rect.len,
			.start_val.f = a->rect.len,
			.val.f       = a->rect.len,
			.type        = WF_FLOAT
		},
		.z = (WfAnimatable){
			.model_val.f = &a->z,
			.start_val.f = a->z,
			.val.f       = a->z,
			.type        = WF_FLOAT
		},
		.opacity = (WfAnimatable){
			.model_val.f = &_a->opacity,
			.start_val.f = _a->opacity,
			.val.f       = _a->opacity,
			.type        = WF_FLOAT
		}
	};

#ifdef WF_DEBUG
	g_strlcpy(_a->animatable.start.name,     "start",     16);
	g_strlcpy(_a->animatable.len.name,       "len",       16);
	g_strlcpy(_a->animatable.rect_left.name, "rect_left", 16);
	g_strlcpy(_a->animatable.rect_len.name,  "rect_len",  16);
#endif

	void _wf_actor_on_peakdata_available(Waveform* waveform, int block, gpointer _actor)
	{
		// because there can be many actors showing the same waveform
		// this can be called multiple times, but the texture must only
		// be updated once.
		// if the waveform has changed, the existing data must be cleared first.
		//
		// Even though low res renderers do not use the audio directly, they must handle this signal
		// in case that the audio has changed (old textures must have previously been cleared).
		// TODO file changes need better testing.

		WaveformActor* a = _actor;
		dbg(1, "block=%i", block);

		ModeRange mode = mode_range(a);
		int upper = MAX(mode.lower, mode.upper);
		int lower = MIN(mode.lower, mode.upper);
		int m; for(m=lower; m<=upper; m+=MAX(1, upper - lower)){
			Renderer* renderer = modes[m].renderer;
			call(renderer->load_block, renderer, a, m == MODE_LOW ? (block / WF_PEAK_STD_TO_LO) : block);
		}
		if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);

	}
	_a->peakdata_ready_handler = g_signal_connect (w, "peakdata-ready", (GCallback)_wf_actor_on_peakdata_available, a);

	void wf_actor_on_dimensions_changed(WaveformCanvas* wfc, gpointer _actor)
	{
		PF;
		WaveformActor* a = _actor;
		a->priv->render_info.valid = false;
	}
	_a->dimensions_changed_handler = g_signal_connect((gpointer)a->canvas, "dimensions-changed", (GCallback)wf_actor_on_dimensions_changed, a);

	void on_use_shaders_changed(WaveformCanvas* wfc, gpointer _actor)
	{
		WaveformActor* a = _actor;

		int m; for(m=0;m<N_MODES;m++){
			call(modes[m].renderer->free, modes[m].renderer, a->waveform);
		}
		a->priv->render_info.valid = false;

		wf_actor_on_use_shaders_change();

		waveform_load(a->waveform);
		_wf_actor_load_missing_blocks(a);
		if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
	}
	_a->use_shaders_changed_handler = g_signal_connect((gpointer)a->canvas, "use-shaders-changed", (GCallback)on_use_shaders_changed, a);

	g_object_weak_ref((GObject*)a->canvas, wf_actor_canvas_finalize_notify, a);
	return a;
}


void
wf_actor_free(WaveformActor* a)
{
	// Waveform data is shared so is not free'd here.

	PF;
	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	if(a->waveform){
		g_signal_handler_disconnect((gpointer)a->waveform, _a->peakdata_ready_handler);
		_a->peakdata_ready_handler = 0;
		g_signal_handler_disconnect((gpointer)a->canvas, _a->dimensions_changed_handler);
		_a->dimensions_changed_handler = 0;
		g_signal_handler_disconnect((gpointer)a->canvas, _a->use_shaders_changed_handler);
		_a->use_shaders_changed_handler = 0;

		g_object_weak_unref((GObject*)a->canvas, wf_actor_canvas_finalize_notify, a);

		waveform_unref0(a->waveform);
	}
	g_free(a->priv);
	g_free(a);
}


// temporary. only here because of private data
void
wf_actor_free_waveform(Waveform* waveform)
{
	int m; for(m=0;m<N_MODES;m++){
		if(waveform->priv->render_data[m]){
			call(modes[m].renderer->free, modes[m].renderer, waveform);
			waveform->priv->render_data[m] = NULL;
		}
	}
}


static void
wf_actor_canvas_finalize_notify(gpointer _actor, GObject* was)
{
	//should not get here. the weak_ref is removed in wf_actor_free.

	WaveformActor* a = (WaveformActor*)_actor;
	gwarn("actor should have been freed before canvas finalize. actor=%p", a);
}


void
wf_actor_set_region(WaveformActor* a, WfSampleRegion* region)
{
	g_return_if_fail(a);
	g_return_if_fail(region);
	WfActorPriv* _a = a->priv;
	dbg(1, "region_start=%Lu region_end=%Lu wave_end=%Lu", region->start, (uint64_t)(region->start + region->len), waveform_get_n_frames(a->waveform));
	if(!region->len){ gwarn("invalid region: len not set"); return; }
	if(region->start + region->len > waveform_get_n_frames(a->waveform)){ gwarn("invalid region: too long: %Lu len=%u n_frames=%Lu", region->start, region->len, waveform_get_n_frames(a->waveform)); return; }

	gboolean start = (region->start != a->region.start);
	gboolean end   = (region->len   != a->region.len);

	if(a->region.len < 2){
		a->region.len = _a->animatable.len.val.i = region->len; // dont animate on initial region set.
	}

	WfAnimatable* a1 = &a->priv->animatable.start;
	WfAnimatable* a2 = &a->priv->animatable.len;

	a->region = *region;

	if(!start && !end) return;

	if(a->rect.len > 0.00001) _wf_actor_load_missing_blocks(a); // this loads _current_ values, future values are loaded by the animator preview

	if(!a->canvas->draw || !a->canvas->enable_animations){
		a1->val.i = MAX(1, a->region.start);
		a2->val.i = MAX(1, a->region.len);

		_a->render_info.valid = false;
		if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
		return; //no animations
	}

	GList* animatables = start ? g_list_append(NULL, a1) : NULL;
	       animatables = end ? g_list_append(animatables, a2) : animatables;

	wf_actor_start_transition(a, animatables, NULL, NULL);
}


void
wf_actor_set_colour(WaveformActor* a, uint32_t fg_colour, uint32_t bg_colour)
{
	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	dbg(2, "0x%08x", fg_colour);
	a->fg_colour = fg_colour;
	a->bg_colour = bg_colour;

	_a->opacity = ((float)(fg_colour & 0xff)) / 0x100;
	_a->animatable.opacity.val.f = _a->opacity;
}


void
wf_actor_set_full(WaveformActor* a, WfSampleRegion* region, WfRectangle* rect, int transition_time, WaveformActorFn callback, gpointer user_data)
{
	PF;

	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;
	GList* animatables = NULL;

	gboolean is_new = a->rect.len == 0.0;
	//FIXME this definition is different to below
	gboolean animate = a->canvas->draw && a->canvas->enable_animations && !is_new;

	if(region){
		dbg(1, "region_start=%Lu region_end=%Lu wave_end=%Lu", region->start, (uint64_t)(region->start + region->len), waveform_get_n_frames(a->waveform));
		if(!region->len){ gwarn("invalid region: len not set"); return; }
		if(region->start + region->len > waveform_get_n_frames(a->waveform)){ gwarn("invalid region: too long: %Lu len=%u n_frames=%Lu", region->start, region->len, waveform_get_n_frames(a->waveform)); return; }

		WfAnimatable* a1 = &_a->animatable.start;
		WfAnimatable* a2 = &_a->animatable.len;

		if(a->region.len < 2){
			a->region.len = a2->val.i = region->len; //dont animate on initial region set.
		}

		bool start = (region->start != a->region.start);
		bool end   = (region->len   != a->region.len);

		a->region = *region;

		if(start || end){
						// TODO too early - set rect first.
			if(a->rect.len > 0.00001) _wf_actor_load_missing_blocks(a); // this loads _current_ values, future values are loaded by the animator preview

			if(!a->canvas->draw || !a->canvas->enable_animations){
				// no animations
				a1->val.i = MAX(1, a->region.start);
				a2->val.i = MAX(1, a->region.len);

				_a->render_info.valid = false;
				if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);

			}else{
				animatables = start ? g_list_append(NULL, a1) : NULL;
				animatables = end   ? g_list_append(animatables, a2) : animatables;
			}
		}
	}

	if(rect && !(rect->len == a->rect.len && rect->left == a->rect.left && rect->height == a->rect.height && rect->top == a->rect.top)){

		_a->render_info.valid = false;

		WfAnimatable* a3 = &_a->animatable.rect_left;
		WfAnimatable* a4 = &_a->animatable.rect_len;

		rect->len = MAX(1, rect->len);

		a->rect = *rect;

		dbg(2, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

				// TODO move to bottom
		if(!a->waveform->offline) _wf_actor_load_missing_blocks(a);

		if(animate){
			if(a3->start_val.f != *a3->model_val.f) animatables = g_list_prepend(animatables, a3);
			if(a4->start_val.f != *a4->model_val.f) animatables = g_list_prepend(animatables, a4);

		}else{
			a3->val.f = rect->left;
			_a->animatable.rect_left.start_val.f = rect->left;
			_a->animatable.rect_len.val.f = rect->len;
			_a->animatable.rect_len.start_val.f = rect->len;

			if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
		}
	}

											// TODO change sig of wf_actor_start_transition - all users just forward to original caller.
	typedef struct {
		WaveformActor*  actor;
		WaveformActorFn callback;
		gpointer        user_data;
	} C;
	C* c = g_new0(C, 1);
	c->actor = a;
	c->callback = callback;
	c->user_data = user_data;

	void _on_animation_finished(WfAnimation* animation, gpointer user_data)
	{
		PF0;
		g_return_if_fail(user_data);
		g_return_if_fail(animation);
		C* c = user_data;
		if(c->callback) c->callback(c->actor, c->user_data);
		g_free(c);
	}

	int len = wf_transition.length;
	wf_transition.length = transition_time;

	wf_actor_start_transition(a, animatables, _on_animation_finished, c);

	wf_transition.length = len;
}


static double
wf_actor_samples2gl(double zoom, uint32_t n_samples)
{
	//zoom is pixels per sample
	return n_samples * zoom;
}


#define FIRST_NOT_VISIBLE 10000
#define LAST_NOT_VISIBLE (-1)


#if 0
static int
wf_actor_get_first_visible_block(WfSampleRegion* region, double zoom, WfRectangle* rect, WfViewPort* viewport_px)
{
	// return the block number of the first block of the actor that is within the given viewport
	// or FIRST_NOT_VISIBLE if the actor is outside the viewport.

	if(rect->left > viewport_px->right) return FIRST_NOT_VISIBLE;

	int resolution = get_resolution(zoom);
	int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (resolution == 1024 ? WF_PEAK_STD_TO_LO : 1);

	double region_inset_px = wf_actor_samples2gl(zoom, region->start);
	double file_start_px = rect->left - region_inset_px;
	double block_wid = wf_actor_samples2gl(zoom, samples_per_texture);

	int region_start_block = region->start / samples_per_texture;
	int region_end_block = (region->start + region->len) / samples_per_texture;
	int b; for(b=region_start_block;b<=region_end_block-1;b++){ // stop before the last block
		int block_start_px = file_start_px + b * block_wid;
		double block_end_px = block_start_px + block_wid;
		dbg(3, "block_pos_px=%i", block_start_px);
		if(block_end_px >= viewport_px->left) return b;
	}
	{
		// check last block:
		double block_end_px = file_start_px + wf_actor_samples2gl(zoom, region->start + region->len);
		if(block_end_px >= viewport_px->left) return b;
	}

	dbg(1, "region outside viewport? vp_left=%.2f region_end=%.2f", viewport_px->left, file_start_px + region_inset_px + wf_actor_samples2gl(zoom, region->len));
	return FIRST_NOT_VISIBLE;
}
#endif


static BlockRange
wf_actor_get_visible_block_range(WfSampleRegion* region, WfRectangle* rect, double zoom, WfViewPort* viewport_px, int n_blocks)
{
	//the region, rect and viewport are passed explictly because different users require slightly different values during transitions.

	BlockRange range = {FIRST_NOT_VISIBLE, LAST_NOT_VISIBLE};

	Mode mode = get_mode(zoom);
	int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (
		mode == MODE_V_LOW
			? WF_MED_TO_V_LOW
			: mode == MODE_LOW ? WF_PEAK_STD_TO_LO : 1);

	double region_inset_px = wf_actor_samples2gl(zoom, region->start);
	double file_start_px = rect->left - region_inset_px;
	double block_wid = wf_actor_samples2gl(zoom, samples_per_texture);
	int region_start_block = region->start / samples_per_texture;

	int region_end_block; {
		float _end_block = ((float)(region->start + region->len)) / samples_per_texture;
		dbg(2, "%s region_start=%Li region_end=%i start_block=%i end_block=%.2f(%.i) n_peak_frames=%i n_blocks=%i", modes[mode].name, region->start, ((int)region->start) + region->len, region_start_block, _end_block, (int)ceil(_end_block), n_blocks * 256, n_blocks);

		// round _down_ as the block corresponds to the _start_ of a section.
		region_end_block = MIN(((int)_end_block), n_blocks - 1);
	}

	// find first block
	if(rect->left <= viewport_px->right){

		int b; for(b=region_start_block;b<=region_end_block-1;b++){ // stop before the last block
			int block_start_px = file_start_px + b * block_wid;
			double block_end_px = block_start_px + block_wid;
			dbg(3, "block_pos_px=%i", block_start_px);
			if(block_end_px >= viewport_px->left){
				range.first = b;
				goto next;
			}
		}
		// check last block:
		double block_end_px = file_start_px + wf_actor_samples2gl(zoom, region->start + region->len);
		if(block_end_px >= viewport_px->left){
			range.first = b;
			goto next;
		}

		dbg(1, "region outside viewport? vp_left=%.2f region_end=%.2f", viewport_px->left, file_start_px + region_inset_px + wf_actor_samples2gl(zoom, region->len));
		range.first = FIRST_NOT_VISIBLE;
	}

	next:

	// find last block
	if(rect->left <= viewport_px->right){
		range.last = MIN(range.first + WF_MAX_BLOCK_RANGE, region_end_block);

		g_return_val_if_fail(viewport_px->right - viewport_px->left > 0.01, range);

		//crop to viewport:
		int b; for(b=region_start_block;b<=range.last-1;b++){ //note we dont check the last block which can be partially outside the viewport
			float block_end_px = file_start_px + (b + 1) * block_wid;
			//dbg(1, " %i: block_px: %.1f --> %.1f", b, block_end_px - (int)block_wid, block_end_px);
			if(block_end_px > viewport_px->right) dbg(2, "end %i clipped by viewport at block %i. vp.right=%.2f block_end=%.1f", region_end_block, MAX(0, b/* - 1*/), viewport_px->right, block_end_px);
			if(block_end_px > viewport_px->right){
				range.last = MAX(0, b/* - 1*/);
				goto out;
			}

	#if 0
			if(rect.len > 0.0)
			if(file_start_px + (b) * block_wid > rect.left + rect.len){
				gerr("block too high: block_start=%.2f rect.len=%.2f", file_start_px + (b) * block_wid, rect.len);
				return b - 1;
			}
	#endif
		}

		if(file_start_px + wf_actor_samples2gl(zoom, region->start + region->len) < viewport_px->left){
			range.last = LAST_NOT_VISIBLE;
			goto out;
		}

		dbg(2, "end not outside viewport. vp_right=%.2f last=%i", viewport_px->right, region_end_block);
	}

	out: return range;
}


#if defined (USE_FBO) && defined (multipass)
static void
block_to_fbo(WaveformActor* a, int b, WfGlBlock* blocks, int resolution)
{
	// create a new fbo for the given block and render to it using the raw 1d peak data.

#ifdef WF_DEBUG
	g_return_if_fail(b < blocks->size);
#endif
	g_return_if_fail(!blocks->fbo[b]);
	blocks->fbo[b] = agl_fbo_new(256, 256, 0);
	{
		WaveformCanvas* wfc = a->canvas;
		WaveformActor* actor = a;
		WfGlBlock* textures = blocks;

		if(a->canvas->use_1d_textures){
			AglFBO* fbo = blocks->fbo[b];
			if(fbo){
				agl_draw_to_fbo(fbo) {
					WfColourFloat fg; wf_colour_rgba_to_float(&fg, actor->fg_colour);
					glClearColor(fg.r, fg.g, fg.b, 0.0); //background colour must be same as foreground for correct antialiasing
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

					{
						//set shader program
						PeakShader* peak_shader = wfc->priv->shaders.peak_nonscaling;
						peak_shader->uniform.n_channels = textures->peak_texture[WF_RIGHT].main ? 2 : 1;
						agl_use_program(&peak_shader->shader);
					}

					glEnable(GL_TEXTURE_1D);
					int c = 0;
					agl_texture_unit_use_texture(wfc->texture_unit[0], textures->peak_texture[c].main[b]);
					agl_texture_unit_use_texture(wfc->texture_unit[1], textures->peak_texture[c].neg[b]);
					if(a->waveform->priv->peak.buf[WF_RIGHT]){
						c = 1;
						agl_texture_unit_use_texture(a->canvas->texture_unit[2], textures->peak_texture[c].main[b]);
						agl_texture_unit_use_texture(a->canvas->texture_unit[3], textures->peak_texture[c].neg[b]);
					}

					//must introduce the overlap as early as possible in the pipeline. It is introduced during the copy from peakbuf to 1d texture.
					//-here only a 1:1 copy is needed.

					const double top = 0;
					const double bot = fbo->height;
					const double x1 = 0;
					const double x2 = fbo->width;

					const double src_start = 0;
					const double tex_pct = 1.0;

					glBegin(GL_QUADS);
					glMultiTexCoord2f(WF_TEXTURE0, src_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE1, src_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE2, src_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE3, src_start + 0.0,     0.0); glVertex2d(x1, top);
					glMultiTexCoord2f(WF_TEXTURE0, src_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE1, src_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE2, src_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE3, src_start + tex_pct, 0.0); glVertex2d(x2, top);
					glMultiTexCoord2f(WF_TEXTURE0, src_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE1, src_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE2, src_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE3, src_start + tex_pct, 1.0); glVertex2d(x2, bot);
					glMultiTexCoord2f(WF_TEXTURE0, src_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE1, src_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE2, src_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE3, src_start + 0.0,     1.0); glVertex2d(x1, bot);
					glEnd();
				} agl_end_draw_to_fbo;
#ifdef USE_FX
				//now process the first fbo onto the fx_fbo

				if(blocks->fx_fbo[b]) gwarn("%i: fx_fbo: expected empty", b);
				AglFBO* fx_fbo = blocks->fx_fbo[b] = agl_fbo_new(256, 256, 0);
				if(fx_fbo){
					dbg(1, "%i: rendering to fx fbo. from: id=%i texture=%u - to: texture=%u", b, fbo->id, fbo->texture, fx_fbo->texture);
					agl_draw_to_fbo(fx_fbo) {
						glClearColor(1.0, 1.0, 1.0, 0.0); //background colour must be same as foreground for correct antialiasing
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

						agl_use_texture(fbo->texture);

						BloomShader* shader = wfc->priv->shaders.vertical;
						shader->uniform.fg_colour = 0xffffffff;
						shader->uniform.peaks_per_pixel = 256; // peaks_per_pixel not used by this shader
						agl_use_program(&shader->shader);

						const double top = 0;
						const double bot = fbo->height;
						const double x1 = 0;
						const double x2 = fbo->width;
						const double tex_pct = 1.0; //TODO check
						glBegin(GL_QUADS);
						glTexCoord2d(0.0,     0.0); glVertex2d(x1, top);
						glTexCoord2d(tex_pct, 0.0); glVertex2d(x2, top);
						glTexCoord2d(tex_pct, 1.0); glVertex2d(x2, bot);
						glTexCoord2d(0.0,     1.0); glVertex2d(x1, bot);
						glEnd();
					} agl_end_draw_to_fbo;
				}
#endif
			} else gwarn("fbo not allocated");
		}
		gl_warn("fbo");
	}
}
#endif


void
wf_actor_get_viewport(WaveformActor* a, WfViewPort* viewport)
{
	WaveformCanvas* canvas = a->canvas;

	if(canvas->viewport) *viewport = *canvas->viewport;
	else {
		viewport->left   = a->priv->animatable.rect_left.val.f;
		viewport->top    = a->rect.top;
		viewport->right  = viewport->left + a->priv->animatable.rect_len.val.f;
		viewport->bottom = a->rect.top + a->rect.height;
	}
}


static void
wf_actor_on_animation_finished(WfAnimation* animation, gpointer user_data)
{
	WaveformActor* actor = user_data;
	g_return_if_fail(actor);
	g_return_if_fail(animation);
	WfActorPriv* _a = actor->priv;

	int l = g_list_length(_a->transitions);
	_a->transitions = g_list_remove(_a->transitions, animation);
	if(g_list_length(_a->transitions) != l - 1) gwarn("animation not removed. len=%i-->%i", l, g_list_length(_a->transitions));

#ifdef USE_FRAME_CLOCK
	if(!_a->transitions) actor->canvas->priv->is_animating = false;
#endif
}


	int get_min_n_tiers()
	{
		//int n_tiers_needed = WF_PEAK_RATIO / arr_samples_per_pix(arrange);
		return HI_MIN_TIERS;
	}

#if 0
	static int get_n_audio_blocks_loaded(Waveform* w)
	{
		WfAudioData* audio = w->audio_data;

		int n_loaded = 0;
		int n_blocks = waveform_get_n_audio_blocks(w);
		int b; for(b=0;b<n_blocks;b++){
			if(audio->buf16[b] && audio->buf16[b]->buf[0]) n_loaded++;
		}
		dbg(1, "n_loaded=%i", n_loaded);
		return n_loaded;
	}
#endif

static void
_wf_actor_get_viewport_max(WaveformActor* a, WfViewPort* viewport)
{
	//special version of get_viewport that gets the outer viewport for duration of the current animation.

	WaveformCanvas* canvas = a->canvas;

	if(canvas->viewport) *viewport = *canvas->viewport;
	else {
		float left_max = MAX(a->rect.left, a->priv->animatable.rect_left.val.f);
		float left_min = MIN(a->rect.left, a->priv->animatable.rect_left.val.f);

		viewport->left   = left_min;
		viewport->top    = a->rect.top;
		viewport->right  = left_max + a->rect.len;
		viewport->bottom = a->rect.top + a->rect.height;
	}
}


#define ZOOM_HI   (1.0/   16)
#define ZOOM_MED  (1.0/  256) // px_per_sample - transition point from std to hi-res mode.
#define ZOOM_LO   (1.0/ 4096) // px_per_sample - transition point from low-res to std mode.
#define ZOOM_V_LO (1.0/65536) // px_per_sample - transition point from v-low-res to low-res.

static inline Mode
get_mode(double zoom)
{
	return (zoom > ZOOM_HI)
		? MODE_V_HI
		: (zoom > ZOOM_MED)
			? MODE_HI
			: (zoom > ZOOM_LO)
				? MODE_MED
				: (zoom > ZOOM_V_LO)
					? MODE_LOW
					: MODE_V_LOW;  //TODO
}


static ModeRange
mode_range(WaveformActor* a)
{
	WfRectangle* rect = &a->rect;

	double zoom_end = rect->len / a->region.len;
	double zoom_start = a->priv->animatable.rect_len.val.f / a->priv->animatable.len.val.i;
	if(zoom_start == 0.0) zoom_start = zoom_end;

	return (ModeRange){get_mode(zoom_start), get_mode(zoom_end)};
}


static void
_wf_actor_allocate_hi(WaveformActor* a)
{
	/*

	How many textures do we need?
	16 * 60 * 60 / hour    => 5760 / hour
		-because this is relatively high, a hashtable is used instead of an array.

	caching options:
		- say that is inherently uncachable
		- have per-waveform texture cache
		- add to normal texture cache (will cause other stuff to be purged prematurely?)
		- add low-priority flag to regular texture cache
		- have separate low-priority texture cache       ****

	*/
	WfActorPriv* _a = a->priv;
	Waveform* w = a->waveform;
	g_return_if_fail(!w->offline);
	WfRectangle* rect = &a->rect;
	WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
	double zoom = rect->len / a->region.len;

#if 0
	// load _all_ blocks for the transition range
	// -works well at low-zoom and for smallish transitions
	// but can uneccesarily load too many blocks.

	WfAnimatable* start = &_a->animatable.start;
	WfSampleRegion region = {MIN(start->val.i, *start->model_val.i), _a->animatable.len.val.i};
	int first_block = wf_actor_get_first_visible_block(&region, zoom, rect, &viewport);
	//int last_block = get_last_visible_block(a, zoom, &viewport); -- looks like we might be able to do away with this now...?
	region.start = MAX(start->val.i, *_a->animatable.start.model_val.i);
	int last_block = wf_actor_get_last_visible_block(&region, rect, zoom, &viewport, a->waveform->textures);
	dbg(1, "%i--->%i", first_block, last_block);
	if(last_block - first_block > wf_audio_cache_get_size()) gwarn("too many blocks requested. increase cache size");

	int b;for(b=first_block;b<=last_block;b++){
		int n_tiers_needed = get_min_n_tiers();
		if(waveform_load_audio_async(a->waveform, b, n_tiers_needed)){
			((Renderer*)&hi_renderer)->load_block(&hi_renderer, a, b);
		}
	}
#else
	// load only the needed blocks - unfortunately is difficult to predict.

	WfAnimatable* start = &_a->animatable.start;
	WfSampleRegion region = {*start->model_val.i, *_a->animatable.len.model_val.i};
	BlockRange blocks = wf_actor_get_visible_block_range (&region, rect, zoom, &viewport, a->waveform->priv->n_blocks);

	int b;for(b=blocks.first;b<=blocks.last;b++){
		hi_request_block(a, b);
	}

#if 0 // audio requests are currently done in start_transition() but needs testing.
	bool is_new = a->rect.len == 0.0;
	bool animate = a->canvas->draw && a->canvas->enable_animations && !is_new;
	if(animate){
		// add blocks for transition
		// note that the animation doesnt run exactly on time so the position when redrawing cannot be precisely predicted.

		#define ANIMATION_LENGTH 300

		//FIXME duplicated from animator.c
		uint32_t transition_linear(WfAnimation* Xanimation, WfAnimatable* animatable, int time)
		{
			uint64_t len = ANIMATION_LENGTH;
			uint64_t t = time - 0;

			float time_fraction = MIN(1.0, ((float)t) / len);
			float orig_val   = animatable->type == WF_INT ? animatable->start_val.i : animatable->start_val.f;
			float target_val = animatable->type == WF_INT ? *animatable->model_val.i : *animatable->model_val.f;
			dbg(2, "%.2f orig=%.2f target=%.2f", time_fraction, orig_val, target_val);
			return  (1.0 - time_fraction) * orig_val + time_fraction * target_val;
		}

		int t; for(t=0;t<ANIMATION_LENGTH;t+=WF_FRAME_INTERVAL){
			uint32_t s = transition_linear(NULL, start, t);
			region.start = s;
			int b = wf_actor_get_first_visible_block(&region, zoom, rect, &viewport);
			dbg(2, "transition: %u %i", s, b);

			hi_request_block(a, b);
		}
	}
#endif
#endif
}


static void
_wf_actor_load_missing_blocks(WaveformActor* a)
{
	// during a transition, this will load the _currently_ visible blocks, not for the target.

	PF2;
	Waveform* w = a->waveform;
	WaveformPriv* _w = w->priv;

	WfRectangle* rect = &a->rect;
	double _zoom = rect->len / a->region.len;

	double zoom_start = a->priv->animatable.rect_len.val.f / a->priv->animatable.len.val.i;
	if(zoom_start == 0.0) zoom_start = _zoom;
	Mode mode1 = get_mode(zoom_start);
	Mode mode2 = get_mode(_zoom);
	dbg(2, "zoom=%.6f-->%.6f (%s --> %s)", zoom_start, _zoom, modes[mode1].name, modes[mode2].name);

	double zoom = MIN(_zoom, zoom_start); // use the zoom which uses the most blocks
																	//TODO can do better than this
	double zoom_max = MAX(_zoom, zoom_start);

	if(!_w->render_data[mode1]) call(modes[mode1].renderer->new, a);
	if(!_w->render_data[mode2]) call(modes[mode2].renderer->new, a);

	if(zoom_max >= ZOOM_MED){
		dbg(1, "HI-RES");
		if(!a->waveform->offline) _wf_actor_allocate_hi(a);
		else { mode1 = MAX(mode1, MODE_MED); mode2 = MAX(mode2, MODE_MED); } // fallback to lower res
	}

	if(mode1 == MODE_MED || mode2 == MODE_MED){
		dbg(1, "MED");
		WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
		double zoom_ = MAX(zoom, ZOOM_LO + 0.00000001);

		uint64_t start_max = MAX(a->region.start, a->priv->animatable.start.val.i);
		uint64_t start_min = MIN(a->region.start, a->priv->animatable.start.val.i);
		int len_max = start_max - start_min + MAX(a->region.len, a->priv->animatable.len.val.i);
		WfSampleRegion region = {start_min, len_max};

		WfRectangle rect_ = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};

		WfViewPort clippingport = viewport;
		float dl = a->region.start < a->priv->animatable.start.val.i ?-(a->region.start - a->priv->animatable.start.val.i) : 0.0; //panning left. make adjustment to prevent clipping
		float dr = a->region.start > a->priv->animatable.start.val.i ? (a->region.start - a->priv->animatable.start.val.i) : 0.0; //panning right. make adjustment to prevent clipping
		clippingport.left  = viewport.left  - dl;
		clippingport.right = viewport.right + dr;

		BlockRange viewport_blocks = wf_actor_get_visible_block_range (&region, &rect_, zoom_, &clippingport, _w->n_blocks);
		//dbg(0, "MED block range: %i --> %i", viewport_blocks.first, viewport_blocks.last);

		int b; for(b=viewport_blocks.first;b<=viewport_blocks.last;b++){
			modes[MODE_MED].renderer->load_block(modes[MODE_MED].renderer, a, b);
		}

#ifdef USE_FBO
		if(agl->use_shaders && !fbo_test) fbo_test = fbo_new_test();
#endif
	}

	if(mode1 == MODE_LOW || mode2 == MODE_LOW){
		// TODO low resolution doesnt have the same adjustments to region and viewport like STD mode does.
		// -this doesnt seem to be causing any problems though
		dbg(2, "LOW");
		WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
		//double zoom_ = MIN(zoom, ZOOM_LO - 0.0001);
		double zoom_ = MAX(zoom, ZOOM_V_LO + 0.00000001);

		if(!_w->render_data[MODE_LOW]){
//#warning check TEX_BORDER effect not multiplied in WF_PEAK_STD_TO_LO transformation
			_w->render_data[MODE_LOW] = (WaveformModeRender*)wf_texture_array_new(_w->num_peaks / (WF_PEAK_STD_TO_LO * WF_TEXTURE_VISIBLE_SIZE) + ((w->priv->num_peaks % (WF_PEAK_STD_TO_LO * WF_TEXTURE_VISIBLE_SIZE)) ? 1 : 0), w->n_channels);
		}

		WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
		WfRectangle rect_ = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};
		BlockRange viewport_blocks = wf_actor_get_visible_block_range (&region, &rect_, zoom_, &viewport, ((WfGlBlock*)_w->render_data[MODE_LOW])->size);
		//dbg(2, "LOW block range: %i --> %i", viewport_blocks.first, viewport_blocks.last);

		int b; for(b=viewport_blocks.first;b<=viewport_blocks.last;b++){
			lo_renderer.load_block(&lo_renderer, a, b);
		}
	}

	if(mode1 == MODE_V_LOW || mode2 == MODE_V_LOW){
		dbg(2, "V_LOW");
		Renderer* renderer = modes[MODE_V_LOW].renderer;
		WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
		double zoom_ = MIN(zoom, ZOOM_LO - 0.0001);

		WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
		WfRectangle rect_ = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};
		BlockRange viewport_blocks = wf_actor_get_visible_block_range (&region, &rect_, zoom_, &viewport, _w->render_data[MODE_V_LOW]->n_blocks);

		int b; for(b=viewport_blocks.first;b<=viewport_blocks.last;b++){
			renderer->load_block(renderer, a, b);
		}
	}

	gl_warn("");
}


void
wf_actor_set_rect(WaveformActor* a, WfRectangle* rect)
{
	g_return_if_fail(a);
	g_return_if_fail(rect);
	rect->len = MAX(1, rect->len);
	WfActorPriv* _a = a->priv;

	if(rect->len == a->rect.len && rect->left == a->rect.left && rect->height == a->rect.height && rect->top == a->rect.top) return;

	_a->render_info.valid = false;

	WfAnimatable* a1 = &_a->animatable.rect_left;
	WfAnimatable* a2 = &_a->animatable.rect_len;

#if 0 // no, is done by the animator
	a1->start_val.f = a->rect.left;
	a2->start_val.f = MAX(1, a->rect.len);
#endif

	gboolean is_new = a->rect.len == 0.0;
	gboolean animate = a->canvas->draw && a->canvas->enable_animations && !is_new;

	a->rect = *rect;

	dbg(2, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

	if(a->region.len && a->waveform->priv->num_peaks) _wf_actor_load_missing_blocks(a);

	if(animate){
		GList* animatables = NULL; //ownership is transferred to the WfAnimation.
		if(a1->start_val.f != *a1->model_val.f) animatables = g_list_prepend(animatables, a1);
		if(_a->animatable.rect_len.start_val.f != *_a->animatable.rect_len.model_val.f) animatables = g_list_prepend(animatables, &_a->animatable.rect_len);

#if 0
		GList* l = animatables;
		for(;l;l=l->next){
			WfAnimatable* animatable = l->data;
			animatable->val.i = a->canvas->draw
				? animatable->start_val.i
				: *animatable->model_val.i;
		}
#else
		// not setting val. using existing value.
#endif

#if 0  // done in wf_actor_start_transition
		GList* l = _a->transitions;
		for(;l;l=l->next){
			//only remove animatables we are replacing. others need to finish.
			//****** except we start a new animation!
			GList* k = animatables;
			for(;k;k=k->next){
				//dbg(0, "    l=%p l->data=%p", l, l->data);
				if(wf_animation_remove_animatable((WfAnimation*)l->data, (WfAnimatable*)k->data)) break;
			}
		}

		void allocate_on_frame(WfAnimation* animation, int time)
		{
			((WaveformActor*)animation->user_data)->priv->render_info.valid = false;
			wf_canvas_queue_redraw(((WaveformActor*)animation->user_data)->canvas);
		}
#endif
		if(animatables) wf_actor_start_transition(a, animatables, NULL, NULL);

	}else{
		*a1->model_val.f = a1->val.f = a1->start_val.f = rect->left;
		*a2->model_val.f = a2->val.f = a2->start_val.f = rect->len;

		if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
	}
}


void
wf_actor_set_z(WaveformActor* a, float z)
{
	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	if(z == a->z) return;

	//check if already animating
#if 0 // not needed, as old transitions are removed in wf_actor_start_transition
	{

		GList* l = _a->transitions;
		for(;l;l=l->next){
			WfAnimation* animation = l->data;
			GList* m = animation->members;
			for(;m;m=m->next){
				WfAnimActor* aa = m->data;
				GList* t = aa->transitions;
				for(;t;t=t->next){
					WfAnimatable* animatable = t->data;
					if(aa->actor == a && animatable == &_a->animatable.z) gwarn("already animating z. actor=%p", a);
				}
			}
		}
	}
#endif

	// the start value is taken from the current value, not the model value, to support overriding of previous transitions.
	WfAnimatable* animatable = &_a->animatable.z;
	if(animatable->val.f != *animatable->model_val.f) dbg(0, "*** transient value in effect. is currently animating?");
	animatable->start_val.f = animatable->val.f;
	a->z = z;

	if(animatable->start_val.f != *animatable->model_val.f){
		GList* animatables = g_list_prepend(NULL, animatable);
		wf_actor_start_transition(a, animatables, NULL, NULL);
	}
}


void
wf_actor_fade_out(WaveformActor* a, WaveformActorFn callback, gpointer user_data)
{
	// not yet sure if this fn will remain (see sig of _fade_in)

	//TODO for consistency we need to change the background colour, though also need to preserve it to fade in.
	//     -can we get away with just setting _a->opacity?

	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	if(_a->opacity == 0.0f) return;

	WfAnimatable* animatable = &_a->animatable.opacity;
	animatable->start_val.f = animatable->val.f;
	_a->opacity = 0.0f;

	GList* animatables = TRUE || (animatable->start_val.f != *animatable->model_val.f)
		? g_list_prepend(NULL, animatable)
		: NULL;

	typedef struct {
		WaveformActor*  actor;
		WaveformActorFn callback;
		gpointer        user_data;
	} C;
	C* c = g_new0(C, 1);
	c->actor = a;
	c->callback = callback;
	c->user_data = user_data;

	void _on_fadeout_finished(WfAnimation* animation, gpointer user_data)
	{
		PF;
		g_return_if_fail(user_data);
		g_return_if_fail(animation);
		C* c = user_data;
#if 0
		WfActorPriv* _a = actor->priv;
		if(animation->members){
			GList* m = animation->members;
			if(m){
				GList* t = ((WfAnimActor*)m->data)->transitions;
				if(!t) gwarn("animation has no transitions");
				for(;t;t=t->next){
					WfAnimatable* animatable = t->data;
					if(animatable == &_a->animatable.opacity) dbg(0, "opacity transition finished");
				}
			}
		}
#endif
		if(c->callback) c->callback(c->actor, c->user_data);
		g_free(c);
	}
	wf_actor_start_transition(a, animatables, _on_fadeout_finished, c);
}


// FIXME temporary signature
void
wf_actor_fade_in(WaveformActor* a, void* /* WfAnimatable* */ _animatable, float target, WaveformActorFn callback, gpointer user_data)
{
	PF;
	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	if(_a->opacity == 1.0f) return; //TODO interaction with fg_colour?

	WfAnimatable* animatable = &_a->animatable.opacity;
	animatable->start_val.f = animatable->val.f;
	_a->opacity = ((float)(a->fg_colour & 0xff)) / 256.0;

	GList* animatables = TRUE || (animatable->start_val.f != *animatable->model_val.f)
		? g_list_prepend(NULL, animatable)
		: NULL;

	typedef struct {
		WaveformActor*  actor;
		WaveformActorFn callback;
		gpointer        user_data;
	} C;
	C* c = g_new0(C, 1);
	c->actor = a;
	c->callback = callback;
	c->user_data = user_data;

	void _on_fadein_finished(WfAnimation* animation, gpointer user_data)
	{
		PF;
		g_return_if_fail(user_data);
		g_return_if_fail(animation);
		C* c = user_data;
		if(c->callback) c->callback(c->actor, c->user_data);
		g_free(c);
	}
	wf_actor_start_transition(a, animatables, _on_fadein_finished, c);
}


void
wf_actor_set_vzoom(WaveformActor* a, float vzoom)
{
	dbg(0, "vzoom=%.2f", vzoom);
	#define MAX_VZOOM 100.0
	g_return_if_fail(!(vzoom < 1.0 || vzoom > MAX_VZOOM));
	a->vzoom = vzoom;

	//TODO perhaps better to just the canvas gain instead? why would one need individual actor gain?
	wf_canvas_set_gain(a->canvas, vzoom);

	if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
}


static inline float
get_peaks_per_pixel_i(WaveformCanvas* wfc, WfSampleRegion* region, WfRectangle* rect, int mode)
{
	//eg: for 51200 frame sample 256pixels wide: n_peaks=51200/256=200, ppp=200/256=0.8

	float region_width_px = wf_canvas_gl_to_px(wfc, rect->len);
	if(mode == MODE_HI) region_width_px /= 16; //this gives the correct result but dont know why.
	float peaks_per_pixel = ceil(((float)region->len / WF_PEAK_TEXTURE_SIZE) / region_width_px);
	dbg(2, "region_width_px=%.2f peaks_per_pixel=%.2f (%.2f)", region_width_px, peaks_per_pixel, ((float)region->len / WF_PEAK_TEXTURE_SIZE) / region_width_px);
	if(mode == MODE_LOW) peaks_per_pixel /= WF_PEAK_STD_TO_LO;
	return mode == MODE_V_LOW
		? peaks_per_pixel / WF_MED_TO_V_LOW
		: peaks_per_pixel;
}


static inline float
get_peaks_per_pixel(WaveformCanvas* wfc, WfSampleRegion* region, WfRectangle* rect, int mode)
{
	//as above but not rounded to nearest integer value

	float region_width_px = wf_canvas_gl_to_px(wfc, rect->len);
	if(mode == MODE_HI) region_width_px /= 16; //this gives the correct result but dont know why.
	float peaks_per_pixel = ((float)region->len / WF_PEAK_TEXTURE_SIZE) / region_width_px;
	if(mode == MODE_LOW) peaks_per_pixel /= 16;
	return peaks_per_pixel;
}


static inline void
_draw_block(float tex_start, float tex_pct, float tex_x, float top, float width, float height, float gain)
{
	// the purpose of this fn is to hide the effect of the texture border.
	// (******* no longer the case. TODO remove all text border stuff OUT of here - its too complicated - see _draw_block_from_1d in MODE_HI)
	// - the start point is offset by TEX_BORDER
	// - the tex_pct given is the one that would be used if there was no border (ie it has max value of 1.0), so the scaling needs to be modified also.

	//TODO it might be better to not have tex_pct as an arg, but use eg n_pix instead?

	tex_start += TEX_BORDER / 256.0;
	tex_pct *= (256.0 - 2.0 * TEX_BORDER) / 256.0;

	glTexCoord2d(tex_start + 0.0,     0.5 - 0.5/gain); glVertex2d(tex_x + 0.0,   top);
	glTexCoord2d(tex_start + tex_pct, 0.5 - 0.5/gain); glVertex2d(tex_x + width, top);
	glTexCoord2d(tex_start + tex_pct, 0.5 + 0.5/gain); glVertex2d(tex_x + width, top + height);
	glTexCoord2d(tex_start + 0.0,     0.5 + 0.5/gain); glVertex2d(tex_x + 0.0,   top + height);
}


static inline Renderer*
set_renderer(WaveformActor* actor)
{
	RenderInfo* r = &actor->priv->render_info;

	Renderer* renderer = modes[r->mode].renderer;
	dbg(2, "%s", modes[r->mode].name);

	return renderer;
}


static inline bool
calc_render_info(WaveformActor* actor)
{
	WaveformCanvas* wfc = actor->canvas;
	Waveform* w = actor->waveform; 
	WfActorPriv* _a = actor->priv;
	WaveformPriv* _w = w->priv;
	RenderInfo* r  = &actor->priv->render_info;

	wf_actor_get_viewport(actor, &r->viewport);

	r->region = (WfSampleRegion){_a->animatable.start.val.i, _a->animatable.len.val.i};
	static gboolean region_len_warning_done = false;
	if(!region_len_warning_done && !r->region.len){ region_len_warning_done = true; gwarn("zero region length"); }

	r->rect = (WfRectangle){_a->animatable.rect_left.val.f, actor->rect.top, _a->animatable.rect_len.val.f, actor->rect.height};
	g_return_val_if_fail(r->rect.len, false);

	r->zoom = r->rect.len / r->region.len;
	r->mode = get_mode(r->zoom);

	if(!_w->render_data[r->mode]) return false;
	r->n_blocks = _w->render_data[r->mode]->n_blocks;
	r->samples_per_texture = WF_SAMPLES_PER_TEXTURE * (r->mode == MODE_V_LOW ? WF_MED_TO_V_LOW : r->mode == MODE_LOW ? WF_PEAK_STD_TO_LO : 1);

																						// why we need this?
	r->region_start_block   = r->region.start / r->samples_per_texture;
																						// FIXME this is calculated differently inside wf_actor_get_visible_block_range
	r->region_end_block     = (r->region.start + r->region.len) / r->samples_per_texture - (!((r->region.start + r->region.len) % r->samples_per_texture) ? 1 : 0);
	r->viewport_blocks = wf_actor_get_visible_block_range(&r->region, &r->rect, r->zoom, &r->viewport, r->n_blocks);

	if(r->viewport_blocks.last == LAST_NOT_VISIBLE && r->viewport_blocks.first == FIRST_NOT_VISIBLE){
		r->valid = true; // this prevents unnecesary recalculation but the RenderInfo is not really valid so _must_ be invalidated again before use.
		return false;
	}
	// ideally conditions which trigger this should be detected before rendering.
	g_return_val_if_fail(r->viewport_blocks.last >= r->viewport_blocks.first, false);

	if(r->region_end_block > r->n_blocks -1){ gwarn("region too long? region_end_block=%i n_blocks=%i region.len=%i", r->region_end_block, r->n_blocks, r->region.len); r->region_end_block = w->priv->n_blocks -1; }
#ifdef DEBUG
	dbg(2, "block range: region=%i-->%i viewport=%i-->%i", r->region_start_block, r->region_end_block, r->viewport_blocks.first, r->viewport_blocks.last);
	dbg(2, "rect=%.2f %.2f viewport=%.2f %.2f", r->rect.left, r->rect.len, r->viewport.left, r->viewport.right);
#endif

	r->first_offset = r->region.start % r->samples_per_texture;
	r->first_offset_px = wf_actor_samples2gl(r->zoom, r->first_offset);

	r->block_wid = wf_actor_samples2gl(r->zoom, r->samples_per_texture);

	r->peaks_per_pixel = get_peaks_per_pixel(wfc, &r->region, &r->rect, r->mode) / 1.0;
	r->peaks_per_pixel_i = get_peaks_per_pixel_i(wfc, &r->region, &r->rect, r->mode);

	r->renderer = set_renderer(actor);

	return r->valid = true;
}


bool
wf_actor_paint(WaveformActor* actor)
{
	// -must have called gdk_gl_drawable_gl_begin() first.
	// -it is assumed that all textures have been loaded.

	// note: there is some benefit in quantising the x positions (eg for subpixel consistency),
	// but to preserve relative actor positions it must be done at the canvas level.

	g_return_val_if_fail(actor, false);
	WaveformCanvas* wfc = actor->canvas;
	g_return_val_if_fail(wfc, false);
	WfActorPriv* _a = actor->priv;
	Waveform* w = actor->waveform; 
	RenderInfo* r  = &_a->render_info;
	if(!w->priv->num_peaks) return false;

	if(!wfc->draw) r->valid = false;

#ifdef RENDER_CACHE_HIT_STATS
	static int hits = 0;
	static int misses = 0;
	if(r->valid) hits++; else misses++;
	if(!((hits + misses) % 100)) dbg(1, "hit ratio: %.2f", ((float)hits) / ((float)misses));
#endif

	if(!r->valid){
		if(!calc_render_info(actor)) return false;

#ifdef WF_DEBUG
	}else{
		// temporary checks:

		WfRectangle rect = {_a->animatable.rect_left.val.f, actor->rect.top, _a->animatable.rect_len.val.f, actor->rect.height};

		WfSampleRegion region = (WfSampleRegion){_a->animatable.start.val.i, _a->animatable.len.val.i};
		double zoom = rect.len / region.len;
		if(zoom != r->zoom) gerr("valid should not be set: zoom %.3f %.3f", zoom, r->zoom);

		Mode mode = get_mode(r->zoom);
		if(mode != r->mode) gerr("valid should not be set: zoom %i %i", mode, r->mode);

		int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (mode == MODE_LOW ? WF_PEAK_STD_TO_LO : 1);
		int first_offset = region.start % samples_per_texture;
		if(first_offset != r->first_offset) gerr("valid should not be set: zoom %i %i", first_offset, r->first_offset);
#endif
	}

	bool inline render_block(Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x, Mode m, Mode* m_active)
	{
		if(m != *m_active){
			renderer->pre_render(renderer, actor);
			*m_active = m;
		}
		return renderer->render_block(renderer, actor, b, is_first, is_last, x);
	}

	double x = r->rect.left + (r->viewport_blocks.first - r->region_start_block) * r->block_wid - r->first_offset_px; // x is now the start of the first block (can be before part start when inset is present)
#ifdef RECT_ROUNDING
	double block_wid0 = r->block_wid;
	r->block_wid = round(r->block_wid);
	int i = 0;
	double x0 = x = round(x);
#endif

#ifdef DEBUG
	g_return_val_if_fail(WF_PEAK_BLOCK_SIZE == (WF_PEAK_RATIO * WF_PEAK_TEXTURE_SIZE), false); // temp check. we use a simplified loop which requires the two block sizes are the same
#endif
	bool render_ok = true;
	Mode m_active = N_MODES;
	bool is_first = true;
	int b; for(b=r->viewport_blocks.first;b<=r->viewport_blocks.last;b++){
		gboolean is_last = (b == r->viewport_blocks.last) || (b == r->n_blocks - 1); //2nd test is unneccesary?

		Mode m = r->mode;
		while((m < N_MODES) && !render_block(modes[m].renderer, actor, b, is_first, is_last, x, m, &m_active)){
			dbg(1, "%i: %sfalling through...%s %s-->%s", b, "\x1b[1;33m", wf_white, modes[m].name, modes[m - 1].name);
			// TODO pre_render not being set propery for MODE_HI due to use_shader settings.
			m--;
			if(m > N_MODES){
				render_ok = false;
				if(wf_debug) gwarn("render failed. no modes succeeded. mode=%i", r->mode); // not neccesarily an error. may simply be not ready.
			}
			if(!w->priv->render_data[m]) break;
		}
#ifdef RECT_ROUNDING
		i++;
		x = round(x0 + i * block_wid0);
		r->block_wid  = round(x0 + (i + 1) * block_wid0) - x; // block_wid is not constant when using rounding
#else
		x += r->block_wid;
#endif
		is_first = false;
	}

#if 0
#define DEBUG_BLOCKS
#ifdef DEBUG_BLOCKS
	float bot = r->rect.top + r->rect.height - 0.1;
	float top = r->rect.top + 0.1;
	x = r->rect.left + (r->viewport_blocks.first - r->region_start_block) * r->block_wid - r->first_offset_px; // x is now the start of the first block (can be before part start when inset is present)
	is_first = true;
	if(agl->use_shaders){
		agl->shaders.plain->uniform.colour = 0x6677ff77;
		agl_use_program((AGlShader*)agl->shaders.plain);
		glDisable(GL_TEXTURE_1D);
	}else{
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.0, 0.0, 1.0, 0.75);
	}
	for(b=r->viewport_blocks.first;b<=r->viewport_blocks.last;b++){
		glLineWidth(1);
		float x_ = x + 0.1; // added for intel 945.
		glBegin(GL_LINES);
			glVertex3f(x_,                top, 0);    glVertex3f(x_ + r->block_wid, top,             0);
			glVertex3f(x_ + r->block_wid, top, 0);    glVertex3f(x_ + r->block_wid, bot,             0);
			glVertex3f(x_,                bot, 0);    glVertex3f(x_ + r->block_wid, bot,             0);

			glVertex3f(x_,                top, 0);    glVertex3f(x_,                r->rect.top + 10,0);
			glVertex3f(x_,                bot, 0);    glVertex3f(x_,                bot - 10,        0);
		glEnd();

		x += r->block_wid;
		is_first = false;
	}
#endif // DEBUG_BLOCKS
#endif

	gl_warn("(@ paint end)");
	return render_ok;
}


/*
 *  Load all textures for the given block.
 *  There will be between 1 and 4 textures depending on shader/alphabuf mono/stero.
 */
static void
wf_actor_load_texture1d(Waveform* w, Mode mode, WfGlBlock* blocks, int blocknum)
{
	g_return_if_fail(mode == MODE_LOW);

	if(blocks) dbg(2, "%i: %i %i %i %i", blocknum,
		blocks->peak_texture[0].main[blocknum],
		blocks->peak_texture[0].neg[blocknum],
		blocks->peak_texture[1].main ? blocks->peak_texture[WF_RIGHT].main[blocknum] : 0,
		blocks->peak_texture[1].neg ? blocks->peak_texture[WF_RIGHT].neg[blocknum] : 0
	);

	struct _buf {
		guchar positive[WF_PEAK_TEXTURE_SIZE];
		guchar negative[WF_PEAK_TEXTURE_SIZE];
	} buf;

	void make_texture_data_med(Waveform* w, int ch, struct _buf* buf)
	{
		//copy peak data into a temporary buffer, translating from 16 bit to 8 bit
		// -src: peak buffer covering whole file (16bit)
		// -dest: temp buffer for single block (8bit) - will be loaded directly into a gl texture.

		#define B_SIZE (WF_PEAK_TEXTURE_SIZE - 2 * TEX_BORDER)
		WfPeakBuf* peak = &w->priv->peak;
		int n_blocks = w->n_frames / WF_SAMPLES_PER_TEXTURE + ((w->n_frames % WF_SAMPLES_PER_TEXTURE) ? 1 : 0);

		int stop = (blocknum == n_blocks - 1)
			? peak->size / WF_PEAK_VALUES_PER_SAMPLE + TEX_BORDER - B_SIZE * blocknum
			: WF_PEAK_TEXTURE_SIZE;
		int f = 0;
		if(blocknum == 0){
			for(f=0;f<TEX_BORDER;f++){
				buf->positive[f] = 0;
				buf->negative[f] = 0;
			}
		}
		for(;f<stop;f++){
			int i = (B_SIZE * blocknum + f - TEX_BORDER) * WF_PEAK_VALUES_PER_SAMPLE;

			buf->positive[f] =  peak->buf[ch][i  ] >> 8;
			buf->negative[f] = -peak->buf[ch][i+1] >> 8;
		}
		for(;f<WF_PEAK_TEXTURE_SIZE;f++){
			// could use memset here
			int i = (B_SIZE * blocknum + f - TEX_BORDER) * WF_PEAK_VALUES_PER_SAMPLE;
			buf->positive[f] = 0;
			buf->negative[f] = 0;
			if(i == peak->size) dbg(1, "end of peak: %i b=%i n_sec=%.3f", peak->size, blocknum, ((float)((WF_PEAK_TEXTURE_SIZE * blocknum + f) * WF_PEAK_RATIO))/44100);// break;
		}
	}

	void make_texture_data_low(Waveform* w, int ch, struct _buf* buf)
	{
		dbg(2, "b=%i", blocknum);

		WfPeakBuf* peak = &w->priv->peak;
		int f; for(f=0;f<WF_PEAK_TEXTURE_SIZE;f++){
			int i = ((WF_PEAK_TEXTURE_SIZE  - 2 * TEX_BORDER) * blocknum + f  - TEX_BORDER) * WF_PEAK_VALUES_PER_SAMPLE * WF_PEAK_STD_TO_LO;
			if(i >= peak->size) break;
			if(i < 0) continue;        //TODO change the loop start point instead (TEX_BORDER)

			WfPeakSample p = {0, 0};
			int j; for(j=0;j<WF_PEAK_STD_TO_LO;j++){
				int ii = i + WF_PEAK_VALUES_PER_SAMPLE * j;
				if(ii >= peak->size) break; // last item
				p.positive = MAX(p.positive, peak->buf[ch][ii    ]);
				p.negative = MIN(p.negative, peak->buf[ch][ii + 1]);
			}

			buf->positive[f] =  p.positive >> 8;
			buf->negative[f] = -p.negative >> 8;
		}
	}

	static IntBufHi buf_hi; //TODO shouldnt be static
	void make_data(Waveform* w, int c, struct _buf* buf)
	{
		make_texture_data_low(w, c, buf);
	}

	struct _d {
		int          tex_unit;
		int          tex_id;
		guchar*      buf;
	};

	void _load_texture(struct _d* d)
	{
		//copy the data to the hardware texture

		glActiveTexture(d->tex_unit);
glEnable(GL_TEXTURE_1D);
		glBindTexture(GL_TEXTURE_1D, d->tex_id);
		dbg (2, "loading texture1D... texture_id=%u", d->tex_id);
		gl_warn("gl error: bind failed: unit=%i buf=%p tid=%i", d->tex_unit, d->buf, d->tex_id);
		if(!glIsTexture(d->tex_id)) gwarn ("invalid texture: texture_id=%u", d->tex_id);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//
		//using MAG LINEAR smooths nicely but can reduce the opacity of peaks too much.
		//
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage1D(GL_TEXTURE_1D, 0, GL_ALPHA8, modes[mode].texture_size, 0, GL_ALPHA, GL_UNSIGNED_BYTE, d->buf);

		gl_warn("unit=%i buf=%p tid=%i", d->tex_unit, d->buf, d->tex_id);
	}

	int c;for(c=0;c<waveform_get_n_channels(w);c++){
		make_data(w, c, &buf);

		struct _d d[2] = {
			{WF_TEXTURE0 + 2 * c, blocks->peak_texture[c].main[blocknum], buf.positive},
			{WF_TEXTURE1 + 2 * c, blocks->peak_texture[c].neg [blocknum], buf.negative},
		};

		if(mode == MODE_HI){
			d[0].buf = buf_hi.positive;
			d[1].buf = buf_hi.negative;
		}

		int v; for(v=0;v<2;v++){
			_load_texture(&d[v]);
		}
		gl_warn("loading textures failed (lhs)");
	}

	glActiveTexture(WF_TEXTURE0);

	gl_warn("done");
}


static void
wf_actor_load_texture2d(WaveformActor* a, Mode mode, int texture_id, int blocknum)
{
	Waveform* w = a->waveform;
	dbg(1, "* %i: texture=%i", blocknum, texture_id);
	if(mode == MODE_MED){
		AlphaBuf* alphabuf = wf_alphabuf_new(w, blocknum, 1, false, TEX_BORDER);
		wf_canvas_load_texture_from_alphabuf(a->canvas, texture_id, alphabuf);
		wf_alphabuf_free(alphabuf);
	}
	else if(mode == MODE_HI){
		AlphaBuf* alphabuf = wf_alphabuf_new_hi(w, blocknum, 1, false, TEX_BORDER);
		wf_canvas_load_texture_from_alphabuf(a->canvas, texture_id, alphabuf);
		wf_alphabuf_free(alphabuf);
	}
}


static void
make_texture_data_hi(Waveform* w, int ch, IntBufHi* buf, int blocknum)
{
	//data is transformed from the Waveform hi-res peakbuf into IntBufHi* buf.

	dbg(1, "b=%i", blocknum);
	int texture_size = modes[MODE_HI].texture_size;
	Peakbuf* peakbuf = waveform_get_peakbuf_n(w, blocknum);
	int o = TEX_BORDER_HI; for(;o<texture_size;o++){
		int i = (o - TEX_BORDER_HI) * WF_PEAK_VALUES_PER_SAMPLE;
		if(i >= peakbuf->size){
			dbg(2, "end of peak: %i b=%i n_sec=%.3f", peakbuf->size, blocknum, ((float)((texture_size * blocknum + o) * WF_PEAK_RATIO))/44100); break;
		}

		short* p = peakbuf->buf[ch];
		buf->positive[o] =  p[i  ] >> 8;
		buf->negative[o] = -p[i+1] >> 8;
	}
#if 0
	int j; for(j=0;j<20;j++){
		printf("  %2i: %5i %5i %5u %5u\n", j, ((short*)peakbuf->buf[ch])[2*j], ((short*)peakbuf->buf[ch])[2*j +1], (guint)(buf->positive[j] * 0x100), (guint)(buf->negative[j] * 0x100));
	}
#endif
				/*
				//int k; for(k=0;k<20;k++){
				//	buf->positive[k] = 100;
				//}
				int k; for(k=0;k<100;k++){
					buf->positive[texture_size - 1 - k] = 80;
				}
				*/
}


/*
 *   Set initial values of animatables and start the transition.
 *
 *   The 'val' of each animatable is assumed to be already set. 'start_val' will be set here.
 *
 *   If the actor has any other transitions using the same animatable, these animatables
 *   are removed from that transition.
 *
 *   @param animatables - ownership of this list is transferred to the WfAnimation.
 */
static void
wf_actor_start_transition(WaveformActor* a, GList* animatables, AnimationFn done, gpointer user_data)
{
	// TODO handle the case here where animating is disabled.

	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	if(!a->canvas->enable_animations || !a->canvas->draw){ //if we cannot initiate painting we cannot animate.
		g_list_free(animatables);
		return;
	}

	// set initial value
	GList* l = animatables;
	for(;l;l=l->next){
		WfAnimatable* animatable = l->data;
		//animatable->val.i = ((WfAnimatable*)l->data)->start_val.i
		animatable->start_val.b = animatable->val.b;
	}

	typedef struct {
		WaveformActor* actor;
		AnimationFn    done;
		gpointer       user_data;
	} C;
	C* c = g_new0(C, 1);
	*c = (C){
		.actor = a,
		.done = done,
		.user_data = user_data
	};

	void on_frame(WfAnimation* animation, int time)
	{
		C* c = animation->user_data;
		c->actor->priv->render_info.valid = false; //strictly should only be invalidated when animating region and rect.
		wf_canvas_queue_redraw(c->actor->canvas);
	}

	void _on_animation_finished(WfAnimation* animation, gpointer user_data)
	{
		g_return_if_fail(user_data);
		g_return_if_fail(animation);
		C* c = user_data;

		if(c->done) c->done(animation, c->user_data);

		wf_actor_on_animation_finished(animation, c->actor);
		g_free(c);
	}

	l = _a->transitions;
	for(;l;l=l->next){
		// remove animatables we are replacing. let others finish.
		GList* k = animatables;
		for(;k;k=k->next){
			if(wf_animation_remove_animatable((WfAnimation*)l->data, (WfAnimatable*)k->data)) break;
		}
	}

	if(animatables){
		WfAnimation* animation = wf_animation_add_new(_on_animation_finished, c);
		animation->on_frame = on_frame;
		_a->transitions = g_list_append(_a->transitions, animation);
		wf_transition_add_member(animation, animatables);
		wf_animation_start(animation);
#ifdef USE_FRAME_CLOCK
		a->canvas->priv->is_animating = true;
#endif
	}

	// if neccesary load any additional audio needed by the transition.
	// - currently only changes to the region start are checked.
	{

		// TODO consider having each wf_animation_preview callback as a separate idle fn.
		void set_region_on_frame_preview(WfAnimation* animation, UVal val[], gpointer _c)
		{
			C* c = _c;
			WaveformActor* a = c->actor;
			WfActorPriv* _a = a->priv;

			GList* l = animation->members;
			for(;l;l=l->next){
				WfAnimActor* anim_actor = l->data;
				WfSampleRegion region = {*_a->animatable.start.model_val.i, *_a->animatable.len.model_val.i};
				GList* j = anim_actor->transitions;
				int i; for(i=0;j;j=j->next,i++){
					WfAnimatable* animatable = j->data;
					if(animatable == &_a->animatable.start){
						region.start = val[i].i;
					}
					else if(animatable == &_a->animatable.len){
						region.len = val[i].i;
					}

					double zoom = a->rect.len / region.len;
					int mode = get_mode(zoom);
					WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);

					BlockRange blocks = wf_actor_get_visible_block_range (&region, &a->rect, zoom, &viewport, wf_actor_get_n_blocks(a->waveform, mode));

					int b;for(b=blocks.first;b<=blocks.last;b++){
						if(mode >= MODE_HI){
							hi_request_block(a, b);
						}else{
							Renderer* renderer = modes[mode].renderer;
							if(a->waveform->priv->render_data[mode]) // can be unset during transitions that span more than one mode.
								call(renderer->load_block, renderer, a, b);
						}
					}
				}
			}
		}

		double zoom_start = a->priv->animatable.rect_len.val.f / a->priv->animatable.len.val.i;
		double zoom_end = a->rect.len / a->region.len;
		if(zoom_start == 0.0) zoom_start = zoom_end;
		GList* l = _a->transitions;
		for(;l;l=l->next){
			WfAnimation* animation = l->data;
			GList* j = animation->members;
			for(;j;j=j->next){
				WfAnimActor* anim_actor = j->data;
				GList* k = anim_actor->transitions;
				for(;k;k=k->next){
					WfAnimatable* animatable = k->data;
					if(animatable == &_a->animatable.start){
						wf_animation_preview(g_list_last(_a->transitions)->data, set_region_on_frame_preview, c);
					}
				}
			}
		}
	}
}


static int
wf_actor_get_n_blocks(Waveform* waveform, Mode mode)
{
	// better to use render_data[mode]->n_blocks
	// but this fn is useful if the render_data is not initialised

	WaveformPriv* w = waveform->priv;

	switch(mode){
		case MODE_V_LOW:
			return w->n_blocks / WF_MED_TO_V_LOW;
		case MODE_LOW:
			return w->n_blocks / WF_PEAK_STD_TO_LO;
		case MODE_MED:
		case MODE_HI:
		case MODE_V_HI:
			return w->n_blocks;
		default:
			break;
	}
	return -1;
}


static void
wf_actor_on_use_shaders_change()
{
	modes[MODE_HI].renderer = agl->use_shaders
		? (Renderer*)&hi_renderer_gl2
		: (Renderer*)&hi_renderer_gl1;

	modes[MODE_MED].renderer = agl->use_shaders
		? (Renderer*)&med_renderer_gl2
		: &med_renderer_gl1;

	lo_renderer = agl->use_shaders
		? lo_renderer_gl2
		: lo_renderer_gl1;

	modes[MODE_V_LOW].renderer = agl->use_shaders
		? (Renderer*)&v_lo_renderer_gl2
		: &v_lo_renderer_gl1;
}


#ifdef USE_TEST
GList*
wf_actor_get_transitions(WaveformActor* a)
{
	return a->priv->transitions;
}
#endif
