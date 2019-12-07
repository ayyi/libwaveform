/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

/*
  WaveformActor draws a Waveform object onto a shared opengl drawable.

  [ **** The description below is out of date **** ]

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
#include <sys/time.h>
#ifdef USE_GTK
#include <gtk/gtk.h>
#endif
#include <GL/gl.h>
#include "agl/ext.h"
#include "transition/transition.h"
#include "waveform/waveform.h"
#include "waveform/audio.h"
#include "waveform/texture_cache.h"
#include "waveform/alphabuf.h"
#include "waveform/fbo.h"
#include "waveform/transition_behaviour.h"
#include "waveform/actor.h"

#define _g_signal_handler_disconnect0(A, H) (H = (g_signal_handler_disconnect((gpointer)A, H), 0))

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

	For the non-shader case, it would be useful to have
	mipmapping to have proper visibility of short term peaks and/or to reduce
	the number of expensive (oversampled) texture lookups. Unfortunately
	Opengl mipmaps cannot be used as we only want detail to be reduced on
	the x axis and not the y, so a custom solution is needed.

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

typedef struct {
    AGlActorClass parent;
} WfActorClass;

static void   wf_actor_free  (AGlActor*);
static bool   wf_actor_paint (AGlActor*);

static WfActorClass actor_class = {{0, "Waveform", (AGlActorNew*)wf_actor_new, wf_actor_free}};

typedef enum
{
	REGION = 0,
	RECT,
	Z,
	OPACITY,
	N_BEHAVIOURS
} Behaviours;

#define START(A) (((TransitionBehaviour*)(A)->behaviours[REGION])->animatables[0])
#define LEN(A) (((TransitionBehaviour*)(A)->behaviours[REGION])->animatables[1])
#define LEFT(A) (((TransitionBehaviour*)A->behaviours[RECT])->animatables[0])
#define RIGHT(A) (((TransitionBehaviour*)A->behaviours[RECT])->animatables[1])
#define OPACITY(A) (((TransitionBehaviour*)A->behaviours[OPACITY])->animatables[0])

typedef void    (*WaveformActorTransitionFn) (WaveformActor*, WfAnimatable*);

typedef struct
{
    TransitionBehaviour       behaviours;
    WfAnimatable              flex_space[2]; // alocate space for the flex array
    WaveformActorTransitionFn on_start;
} WfWaveformActorSizeTransitions;

struct _actor_priv
{
	float           opacity;     // derived from background colour

	struct {
		gulong      peakdata_ready;
		gulong      dimensions_changed;
		gulong      zoom_changed;
	}               handlers;
	AMPromise*      peakdata_ready;

	guint           load_render_data_queue;

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
#if 0
		float          peaks_per_pixel_i;
#endif
		int            first_offset;
		double         first_offset_px;
		double         block_wid;
		int            region_start_block;
		int            region_end_block;
		BlockRange     viewport_blocks;
		bool           cropped;
	}               render_info;
};


typedef void    (*WaveformActorNewFn)       (WaveformActor*);
typedef bool    (*WaveformActorPreRenderFn) (Renderer*, WaveformActor*);
typedef void    (*WaveformActorBlockFn)     (Renderer*, WaveformActor*, int b);
typedef bool    (*WaveformActorRenderFn)    (Renderer*, WaveformActor*, int b, bool is_first, bool is_last, double x);
typedef void    (*WaveformActorFreeFn)      (Renderer*, Waveform*);
#ifdef USE_TEST
typedef bool    (*WaveformActorTestFn)      (Renderer*, WaveformActor*);
#endif

struct _Renderer
{
	Mode                     mode;

	WaveformActorNewFn       new;
	WaveformActorBlockFn     load_block;
	WaveformActorPreRenderFn pre_render;
	WaveformActorRenderFn    render_block;
	WaveformActorFreeFn      free;
#ifdef USE_TEST
	WaveformActorTestFn      is_not_blank;
#endif
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

static double wf_actor_samples2gl (double zoom, uint32_t n_samples);

#if 0
static inline float get_peaks_per_pixel_i    (WaveformContext*, WfSampleRegion*, WfRectangle*, int mode);
#endif
static inline float get_peaks_per_pixel      (WaveformContext*, WfSampleRegion*, WfRectangle*, int mode);
static inline void _draw_block               (float tex_start, float tex_pct, float x, float y, float width, float height, float gain);
static inline void _draw_block_from_1d       (float tex_start, float tex_pct, float x, float y, float width, float height, int tsize);

typedef struct
{
	guchar positive[WF_PEAK_TEXTURE_SIZE * 16];
	guchar negative[WF_PEAK_TEXTURE_SIZE * 16];
} IntBufHi;

typedef void (MakeTextureData)(Waveform*, int ch, IntBufHi*, int blocknum);

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
#define HI_MIN_TIERS 4 // equivalent to resolution of 1:16

static inline Mode get_mode                  (double zoom);
static ModeRange   mode_range                (WaveformActor*);

#if 0
static void   wf_actor_load_texture1d        (Waveform*, Mode, WfGlBlock*, int b);
#endif
static void   wf_actor_load_texture2d        (WaveformActor*, Mode, int t, int b);

#if defined (USE_FBO) && defined (multipass)
#if 0
static void   block_to_fbo                   (WaveformActor*, int b, WfGlBlock*, int resolution);
#endif
#endif

static bool   wf_actor_get_quad_dimensions   (WaveformActor*, int b, bool is_first, bool is_last, double x, TextureRange*, double* tex_x, double* block_wid, int border, int multiplier);
static int    wf_actor_get_n_blocks          (Waveform*, Mode);
static void   wf_actor_connect_waveform      (WaveformActor*);
static void   wf_actor_disconnect_waveform   (WaveformActor*);

#include "waveform/renderer/ng.c"
#include "waveform/renderer/res_med.c"
#include "waveform/renderer/res_lo.c"
#include "waveform/renderer/res_hi_gl2.c"
#include "waveform/renderer/res_hi.c"
#include "waveform/renderer/res_v_hi.c"
#include "waveform/renderer/res_v_low.c"

static void  wf_actor_waveform_finalize_notify (gpointer, GObject*);
static void  wf_actor_on_size_transition_start (WaveformActor*, WfAnimatable*);
static void  wf_actor_queue_load_render_data   (WaveformActor*);
static void _wf_actor_load_missing_blocks      (WaveformActor*);
static void  wf_actor_on_use_shaders_change    ();
#if 0
static int    wf_actor_get_first_visible_block(WfSampleRegion*, double zoom, WfRectangle*, WfViewPort*);
#endif
static void  _wf_actor_get_viewport_max        (WaveformActor*, WfViewPort*);


static void
size_transition_set (TransitionBehaviour* behaviour, WaveformActor* actor, TransitionValue values[], WaveformActorFn callback, gpointer user_data)
{
	transition_behaviour_set(behaviour, (AGlActor*)actor, values, callback, user_data);
	wf_actor_on_size_transition_start(actor, NULL);
}


AGlActorClass*
wf_actor_get_class ()
{
	return (AGlActorClass*)&actor_class;
}


static void
wf_actor_class_init()
{
	agl = agl_get_instance();

	modes[MODE_HI].make_texture_data = make_texture_data_hi;

	modes[MODE_V_LOW].renderer = v_lo_renderer_new();
	modes[MODE_LOW].renderer = lo_renderer_new();
	modes[MODE_MED].renderer = med_renderer_new();
	modes[MODE_HI].renderer = hi_renderer_new();
	modes[MODE_V_HI].renderer = (Renderer*)&v_hi_renderer;

	wf_actor_on_use_shaders_change();
}


	static void _wf_actor_invalidate(AGlActor* actor)
	{
		WaveformActor* a = (WaveformActor*)actor;
		a->priv->render_info.valid = false; //strictly should only be invalidated when animating region and rect.
		wf_canvas_queue_redraw(a->canvas);
	}

	static void wf_actor_on_dimensions_changed(WaveformContext* wfc, gpointer _actor)
	{
		PF;
		WaveformActor* a = _actor;
		a->priv->render_info.valid = false;
	}

	static void wf_actor_on_zoom_changed(WaveformContext* wfc, gpointer _actor)
	{
		wf_actor_queue_load_render_data((WaveformActor*)_actor);
	}

		static void wf_actor_init_load_done(Waveform* w, GError* error, gpointer _actor)
		{
			AGlActor* actor = _actor;
			WaveformActor* a = _actor;

			a->canvas->sample_rate = a->waveform->samplerate;

			if(agl_actor__width(actor) > 0.0){
				_wf_actor_load_missing_blocks(a);
				agl_actor__invalidate((AGlActor*)a);
			}
			if(((AGlActor*)a)->root->draw) wf_canvas_queue_redraw(a->canvas);
		}

	static void wf_actor_on_init(AGlActor* actor)
	{
		WaveformActor* a = (WaveformActor*)actor;

		a->priv->render_info.valid = false;

		a->canvas->use_1d_textures = agl->use_shaders;

		wf_actor_on_use_shaders_change();

		if(a->waveform && !a->waveform->priv->peak.size) waveform_load(a->waveform, wf_actor_init_load_done, actor);
	}

/*
 *	Graph layout handler
 *	No animations are done here, they must be requested explictly
 */
static void
wf_actor_set_size (AGlActor* actor)
{
	WaveformActor* a = (WaveformActor*)actor;
	WfActorPriv* _a = ((WaveformActor*)actor)->priv;

	WfAnimatable* right = &RIGHT(actor);
	if(!right->target_val.f){
		LEFT(actor).target_val.f = actor->region.x1;
		right->target_val.f = actor->region.x2;
	}

	RenderInfo* ri = &_a->render_info;
	if(ri->valid){
		if(ri->rect.left != actor->region.x1 || ri->rect.len != agl_actor__width(actor)){
			agl_actor__invalidate(actor);
			_wf_actor_load_missing_blocks(a);
		}
	}else{
		if(agl_actor__width(actor) && a->waveform){
			agl_actor__invalidate(actor);
			_wf_actor_load_missing_blocks(a);
		}
	}
}


/*
 *  Normally called by wf_context_add_new_actor.
 */
WaveformActor*
wf_actor_new (Waveform* w, WaveformContext* wfc)
{
	dbg(2, "%s-------------------------%s", "\x1b[1;33m", "\x1b[0;39m");

	g_return_val_if_fail(wfc, NULL);

	if(!modes[MODE_LOW].renderer) wf_actor_class_init();

	if(w){
		waveform_get_n_frames(w);
		if(!w->renderable) return NULL;
	}

	WaveformActor* a = WF_NEW(WaveformActor,
		.actor = {
			.class = (AGlActorClass*)&actor_class,
			.name = "Waveform",
			.init = wf_actor_on_init,
			.paint = wf_actor_paint,
			.invalidate = _wf_actor_invalidate,
			.set_size = wf_actor_set_size,
		},
		.canvas = wfc,
		.priv = g_new0(WfActorPriv, 1),
		.vzoom = 1.0,
		.waveform = w ? g_object_ref(w) : NULL,
	);

	WfActorPriv* _a = a->priv;
	AGlActor* actor = (AGlActor*)a;

	wf_actor_set_colour(a, 0xffffffff);

	// implement flex array with size 2 (used only for allocation)
	typedef struct
	{
		TransitionBehaviour parent;
		WfAnimatable animatables[2];
	} TransitionBehaviourLen2;

	typedef struct
	{
		TransitionBehaviourLen2  behaviours;
		WaveformActorTransitionFn on_start;
	} WfWaveformActorSizeTransitionsLen2;

	// implement flex array with size 1 (used only for allocation)
	typedef struct
	{
		TransitionBehaviour parent;
		WfAnimatable animatables[2];
	} TransitionsBehaviourLen1;

	actor->behaviours[REGION] = (AGlBehaviour*)AGL_NEW(WfWaveformActorSizeTransitionsLen2,
		.behaviours = {
			.parent = {
				.behaviour = {
					.klass = transition_behaviour_get_class()
				},
				.size = 2,
			},
			.animatables = {
				{
					.val.b = &a->region.start,
					.type = WF_INT64,
#ifdef WF_DEBUG
					.name = "region-start"
#endif
				},
				{
					.val.b = &a->region.len,
					.type = WF_INT64,
#ifdef WF_DEBUG
					.name = "region-len"
#endif
				}
			}
		},
		.on_start = wf_actor_on_size_transition_start
	);

	actor->behaviours[RECT] = (AGlBehaviour*)AGL_NEW(WfWaveformActorSizeTransitionsLen2,
		.behaviours = {
			.parent = {
				.behaviour = {
					.klass = transition_behaviour_get_class()
				},
				.size = 2,
			},
			.animatables = {
				{
					.val.f = &actor->region.x1,
					.type = WF_FLOAT,
#ifdef WF_DEBUG
				.name = "rect-left"
#endif
				},
				{
					.val.f = &actor->region.x2,
					.type = WF_FLOAT,
#ifdef WF_DEBUG
					.name = "rect-right"
#endif
				}
			}
		},
		.on_start = wf_actor_on_size_transition_start
	);

	actor->behaviours[Z] = (AGlBehaviour*)AGL_NEW(TransitionsBehaviourLen1,
		.parent = {
			.behaviour = {
				.klass = transition_behaviour_get_class()
			},
			.size = 1
		},
		.animatables = {
			{
				.val.f = &a->z,
				.type = WF_FLOAT,
#ifdef WF_DEBUG
				.name = "z"
#endif
			}
		}
	);

	actor->behaviours[OPACITY] = (AGlBehaviour*)AGL_NEW(TransitionsBehaviourLen1,
		.parent = {
			.behaviour = {
				.klass = transition_behaviour_get_class()
			},
			.size = 1
		},
		.animatables = {
			{
				.val.f = &_a->opacity,
				.type = WF_FLOAT
#ifdef WF_DEBUG
				.name = "opacity"
#endif
			}
		}
	);

	_a->peakdata_ready = am_promise_new(a);

	if(w) wf_actor_connect_waveform(a);

	_a->handlers.dimensions_changed = g_signal_connect((gpointer)a->canvas, "dimensions-changed", (GCallback)wf_actor_on_dimensions_changed, a);
	_a->handlers.zoom_changed = g_signal_connect((gpointer)a->canvas, "zoom-changed", (GCallback)wf_actor_on_zoom_changed, a);

	return a;
}


static void
_wf_actor_on_peakdata_available (Waveform* waveform, int block, gpointer _actor)
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

	agl_actor__invalidate((AGlActor*)a);

	ModeRange mode = mode_range(a);
	int upper = MAX(mode.lower, mode.upper);
	int lower = MIN(mode.lower, mode.upper);
	int m; for(m=lower; m<=upper; m+=MAX(1, upper - lower)){
		Renderer* renderer = modes[m].renderer;
		call(renderer->load_block, renderer, a, m == MODE_LOW ? (block / WF_PEAK_STD_TO_LO) : block);
	}
	if(((AGlActor*)a)->root && ((AGlActor*)a)->root->draw) wf_canvas_queue_redraw(a->canvas);
}


static void
wf_actor_connect_waveform (WaveformActor* a)
{
	WfActorPriv* _a = a->priv;
	g_return_if_fail(!_a->handlers.peakdata_ready);

	_a->handlers.peakdata_ready = g_signal_connect (a->waveform, "hires-ready", (GCallback)_wf_actor_on_peakdata_available, a);

	g_object_weak_ref((GObject*)a->waveform, wf_actor_waveform_finalize_notify, a);
}


static void
wf_actor_disconnect_waveform (WaveformActor* a)
{
	WfActorPriv* _a = a->priv;
	g_return_if_fail(_a->handlers.peakdata_ready);

	_g_signal_handler_disconnect0(a->waveform, _a->handlers.peakdata_ready);

	g_object_weak_unref((GObject*)a->waveform, wf_actor_waveform_finalize_notify, a);
}


/*
 *  This would not normally be called by the user.
 *  Freeing the actor can be triggered by calling
 *  agl_actor__remove_child.
 */
static void
wf_actor_free (AGlActor* actor)
{
	// Waveform data is shared so is not free'd here.

	PF;
	g_return_if_fail(actor);
	WaveformActor* a = (WaveformActor*)actor;
	WfActorPriv* _a = a->priv;

	GList* l = actor->transitions;
	for(;l;l=l->next) wf_animation_remove((WfAnimation*)l->data);
	g_list_free0(actor->transitions);

	if(_a->load_render_data_queue) g_source_remove(_a->load_render_data_queue);

	if(a->waveform){
		wf_actor_disconnect_waveform(a);
		_g_signal_handler_disconnect0(a->canvas, _a->handlers.dimensions_changed);
		_g_signal_handler_disconnect0(a->canvas, _a->handlers.zoom_changed);

		// if the waveform has no more users, the finalise notify will run which will clear the render data
		waveform_unref0(a->waveform);
	}

	g_free0(a->priv);

#if 0 // no, cannot call this because it calls the free function
	if(actor->parent) agl_actor__remove_child(actor->parent, actor);
#endif
}


void
waveform_free_render_data(Waveform* waveform)
{
	int m; for(m=0;m<N_MODES;m++){
		if(waveform->priv->render_data[m]){
			call(modes[m].renderer->free, modes[m].renderer, waveform);
			waveform->priv->render_data[m] = NULL;
		}
	}
}


static void
wf_actor_waveform_finalize_notify (gpointer _actor, GObject* was)
{
	waveform_free_render_data(((WaveformActor*)_actor)->waveform);
}


	typedef struct { WaveformActor* actor; WaveformActorFn callback; gpointer user_data; } C2;

	static void wf_actor_set_waveform_done (Waveform* w, GError* error, gpointer _c)
	{
		C2* c = (C2*)_c;
		PF;

		if(waveform_get_n_frames(w)){
			c->actor->canvas->sample_rate = c->actor->waveform->samplerate;
			wf_actor_queue_load_render_data(c->actor);
		}

		if(c->callback) c->callback(c->actor, c->user_data);

		g_free(c);
	}

void
wf_actor_set_waveform (WaveformActor* a, Waveform* waveform, WaveformActorFn callback, gpointer user_data)
{
	g_return_if_fail(a);
	PF;

	waveform_get_n_frames(waveform);
	if(!waveform->renderable) return;

	agl_actor__invalidate((AGlActor*)a);

	if(a->waveform){
		wf_actor_clear(a);
		wf_actor_disconnect_waveform(a);
		g_object_unref(a->waveform);
	}
	a->waveform = g_object_ref(waveform);
	wf_actor_set_region(a, &(WfSampleRegion){0, waveform->n_frames});

	wf_actor_connect_waveform(a);

	waveform_load(
		a->waveform,
		wf_actor_set_waveform_done,
		WF_NEW(C2, .actor = a, .callback = callback, .user_data = user_data)
	);
}


void
wf_actor_set_waveform_sync (WaveformActor* a, Waveform* waveform)
{
	g_return_if_fail(a);

	waveform_get_n_frames(waveform);
	if(!waveform->renderable) return;

	agl_actor__invalidate((AGlActor*)a);

	if(a->waveform){
		wf_actor_clear(a);
		wf_actor_disconnect_waveform(a);
		g_object_unref(a->waveform);
	}
	a->waveform = g_object_ref(waveform);
	wf_actor_connect_waveform(a);

	waveform_load_sync(a->waveform);

	if(waveform_get_n_frames(a->waveform)){
		a->canvas->sample_rate = a->waveform->samplerate;
		wf_actor_queue_load_render_data(a);
		wf_actor_set_region(a, &(WfSampleRegion){0, waveform_get_n_frames(a->waveform)});
	}
}


void
wf_actor_set_region (WaveformActor* a, WfSampleRegion* region)
{
	g_return_if_fail(a);
	g_return_if_fail(a->waveform);
	g_return_if_fail(region);
	AGlActor* actor = (AGlActor*)a;
	WfActorPriv* _a = a->priv;
	AGlScene* scene = actor->root;
	dbg(1, "region_start=%"PRIi64" region_end=%"PRIi64" wave_end=%Lu", region->start, region->start + region->len, waveform_get_n_frames(a->waveform));
	if(!region->len && a->waveform->n_channels){ gwarn("invalid region: len not set"); return; }
	if(region->start > waveform_get_n_frames(a->waveform)){ gwarn("invalid region: start out of range: %"PRIi64" > %"PRIi64"", region->start, waveform_get_n_frames(a->waveform)); return; }
	if(region->start + region->len > waveform_get_n_frames(a->waveform)){ gwarn("invalid region: too long: %"PRIi64" len=%"PRIi64" n_frames=%"PRIi64, region->start, region->len, waveform_get_n_frames(a->waveform)); return; }

	region->len = MAX(1, region->len);

	bool start = (region->start != a->region.start);
	bool end   = (region->len   != a->region.len);

	WfAnimatable* a1 = &START(actor);

	if(a->region.len < 2){
		a->region.len = a1->target_val.b = region->len; // dont animate on initial region set.
	}

	if(!start && !end) return;

	if(agl_actor__width(actor) > 0.00001) wf_actor_queue_load_render_data(a);

	if(!(scene && scene->draw) || !scene->enable_animations){
		a->region = *region;
		START(actor).target_val.b = region->start;
		LEN(actor).target_val.b = region->len;

		_a->render_info.valid = false;
		if(scene && scene->draw) wf_canvas_queue_redraw(a->canvas);
		return; // no animations
	}

	size_transition_set(
		(TransitionBehaviour*)actor->behaviours[REGION],
		a,
		(TransitionValue[]){
			{start, .value.b = region->start},
			{end, .value.b = region->len}
		},
		NULL, NULL
	);
}


void
wf_actor_set_colour (WaveformActor* a, uint32_t fg_colour)
{
	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	dbg(2, "0x%08x", fg_colour);
	a->fg_colour = fg_colour;

	_a->opacity = ((float)(fg_colour & 0xff)) / 0x100;

	agl_actor__invalidate((AGlActor*)a);
}


// temporary while removing wfactor->rect
#define AGL_ACTOR__SET_REGION_FROM_RECT(ACTOR, RECT) \
	ACTOR->region = (AGlfRegion){ \
		.x1 = (RECT)->left, \
		.y1 = (RECT)->top, \
		.x2 = (RECT)->left + (RECT)->len, \
		.y2 = (RECT)->top + (RECT)->height, \
	}

#define WF_ACTOR_GET_RECT(A, RECT) \
	*RECT = (WfRectangle){ \
		.left = ((AGlActor*)A)->region.x1, \
		.top = ((AGlActor*)A)->region.y1, \
		.len = agl_actor__width(((AGlActor*)A)), \
		.height = agl_actor__height(((AGlActor*)A)), \
	}
#define WF_ACTOR_RECT(A) \
	{A->region.x1, A->region.y1, agl_actor__width(A), agl_actor__height(A)}

typedef struct {
	WaveformActor*  actor;
	WaveformActorFn callback;
	gpointer        user_data;
} C;


static void
_on_set_full_finished (WfAnimation* animation, gpointer user_data)
{
	PF;
	g_return_if_fail(user_data);
	g_return_if_fail(animation);
	C* c = user_data;

	if(c->callback) c->callback(c->actor, c->user_data);

	g_free(c);
}


void
wf_actor_set_full (WaveformActor* a, WfSampleRegion* region, WfRectangle* rect, int transition_time, WaveformActorFn callback, gpointer user_data)
{
	// Currently this does not use the TransitionBehaviours because it creates a transition with up to 4 members
	PF;

	g_return_if_fail(a);
	AGlActor* actor = (AGlActor*)a;
	AGlScene* scene = actor->root;
	WfActorPriv* _a = a->priv;
	GList* animatables = NULL;

	bool is_new = agl_actor__width(actor) == 0;
	//FIXME this definition is different to below
	bool animate = scene->draw && scene->enable_animations && !is_new;

	if(region){
		dbg(1, "region_start=%Lu region_end=%Lu wave_end=%Lu", region->start, (uint64_t)(region->start + region->len), waveform_get_n_frames(a->waveform));
		if(!region->len){ gwarn("invalid region: len not set"); return; }
		if(region->start + region->len > waveform_get_n_frames(a->waveform)){ gwarn("invalid region: too long: %"PRIi64" len=%"PRIi64" n_frames=%"PRIi64, region->start, region->len, waveform_get_n_frames(a->waveform)); return; }

		region->len = MAX(1, region->len);

		WfAnimatable* a1 = &START(actor);
		WfAnimatable* a2 = &LEN(actor);

		if(a->region.len < 2){
			a->region.len = a2->target_val.b = region->len; //dont animate on initial region set.
		}

		bool start = (region->start != a->region.start);
		bool end   = (region->len   != a->region.len);

		if(start || end){
						// TODO too early - set rect first.
			if(agl_actor__width(actor) > 0.00001) _wf_actor_load_missing_blocks(a); // this loads _current_ values, future values are loaded by the animator preview

			if(!scene->draw || !scene->enable_animations){
				// no animations
				a->region = *region;

				_a->render_info.valid = false;
				if(scene->draw) wf_canvas_queue_redraw(a->canvas);

			}else{
				animatables = start ? g_list_append(NULL, a1) : NULL;
				animatables = end   ? g_list_append(animatables, a2) : animatables;
			}
		}
	}

	float len = agl_actor__width(actor);
	float height = agl_actor__height(actor);
	if(rect && !(rect->len == len && rect->left == actor->region.x1 && rect->height == height && rect->top == actor->region.y1)){

		_a->render_info.valid = false;

		WfAnimatable* a3 = &LEFT(actor);
		WfAnimatable* a4 = &RIGHT(actor);

		rect->len = MAX(1, rect->len);

		AGL_ACTOR__SET_REGION_FROM_RECT(actor, rect);

		dbg(2, "rect: %.2f --> %.2f", actor->region.x1, actor->region.x2);

		if(animate){
			if(a3->target_val.f != rect->left) animatables = g_list_prepend(animatables, a3);
			if(a4->target_val.f != rect->len) animatables = g_list_prepend(animatables, a4);
		}else{
			*a3->val.f = actor->region.x1;
			a3->start_val.f = a3->target_val.f = actor->region.x1;
			*a4->val.f = agl_actor__width(actor);
			a4->start_val.f = a4->target_val.f = agl_actor__width(actor);

			if(scene->draw) wf_canvas_queue_redraw(a->canvas);
		}

		if(!a->waveform->offline) _wf_actor_load_missing_blocks(a);
	}

	int tlen = wf_transition.length;
	wf_transition.length = transition_time;

	agl_actor__start_transition(actor, animatables, _on_set_full_finished, AGL_NEW(C,
		.actor = a,
		.callback = callback,
		.user_data = user_data
	));
	wf_actor_on_size_transition_start(a, NULL);

	wf_transition.length = tlen;
}


static double
wf_actor_samples2gl (double zoom, uint32_t n_samples)
{
	// zoom is pixels per sample

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
wf_actor_get_visible_block_range (WfSampleRegion* region, WfRectangle* rect, double zoom, WfViewPort* viewport_px, int n_blocks)
{
	// The region, rect and viewport are passed explictly because different users require slightly
	// different values during transitions.

	BlockRange range = {FIRST_NOT_VISIBLE, LAST_NOT_VISIBLE};

	Mode mode = get_mode(zoom);
	int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (
		mode == MODE_V_LOW
			? WF_MED_TO_V_LOW
			: mode == MODE_LOW ? WF_PEAK_STD_TO_LO : 1);

	double region_inset_px = wf_actor_samples2gl(zoom, region->start);
	double file_start_px = rect->left - region_inset_px;
	double block_wid = wf_actor_samples2gl(zoom, samples_per_texture);
	BlockRange region_blocks = {region->start / samples_per_texture, -1};
	{
		//float _end_block = ((float)(region->start + region->len - 1)) / samples_per_texture;
		float _end_block = ((float)(region->start + region->len)) / samples_per_texture;
		dbg(2, "%s region=%"PRIi64"-->%"PRIi64" blocks=%i-->%.2f(%.i) n_peak_frames=%i tot_blocks=%i", modes[mode].name, region->start, region->start + region->len, region_blocks.first, _end_block, (int)ceil(_end_block), n_blocks * 256, n_blocks);

		// round _down_ as the block corresponds to the _start_ of a section.
		region_blocks.last = MIN(((int)_end_block), n_blocks - 1);
	}

	// find first block
	if(rect->left <= viewport_px->right){

		int b; for(b=region_blocks.first;b<=region_blocks.last-1;b++){ // stop before the last block
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
		range.last = MIN(range.first + WF_MAX_BLOCK_RANGE, region_blocks.last);

		g_return_val_if_fail(viewport_px->right - viewport_px->left > 0.01, range);

		//crop to viewport:
		int b; for(b=region_blocks.first;b<=range.last-1;b++){ //note we dont check the last block which can be partially outside the viewport
			float block_end_px = file_start_px + (b + 1) * block_wid;
			//dbg(1, " %i: block_px: %.1f --> %.1f", b, block_end_px - (int)block_wid, block_end_px);
			if(block_end_px > viewport_px->right) dbg(2, "end %i clipped by viewport at block %i. vp.right=%.2f block_end=%.1f", region_blocks.last, MAX(0, b/* - 1*/), viewport_px->right, block_end_px);
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

		dbg(2, "end not outside viewport. vp_right=%.2f last=%i", viewport_px->right, region_blocks.last);
	}

	out: return range;
}


#if defined (USE_FBO) && defined (multipass)
// this is only used in gl-1 mode with the obsolete use_1d_textures options.
#if 0
static void
block_to_fbo(WaveformActor* a, int b, WfGlBlock* blocks, int resolution)
{
	// create a new fbo for the given block and render to it using the raw 1d peak data.

#ifdef WF_DEBUG
	g_return_if_fail(b < blocks->size);
#endif
	g_return_if_fail(!blocks->fbo[b]);
	blocks->fbo[b] = agl_fbo_new(256, 256, 0, 0);
	{
		WaveformContext* wfc = a->canvas;
		WaveformActor* actor = a;
		WfGlBlock* textures = blocks;

		if(a->canvas->use_1d_textures){
			AGlFBO* fbo = blocks->fbo[b];
			if(fbo){
				agl_draw_to_fbo(fbo) {
					AGlColourFloat fg; wf_colour_rgba_to_float(&fg, actor->fg_colour);
					glClearColor(fg.r, fg.g, fg.b, 0.0); // background colour must be same as foreground for correct antialiasing
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
				AGlFBO* fx_fbo = blocks->fx_fbo[b] = agl_fbo_new(256, 256, 0, 0);
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
#endif


/*
 *   Returns the instantaneous positions of the top-left and bottom-right corners of the actor.
 *   These points may lie outside of the canvas viewport.
 */
void
wf_actor_get_viewport(WaveformActor* a, WfViewPort* viewport)
{
	AGlActor* actor = (AGlActor*)a;
#if 0
	WfActorPriv* _a = a->priv;
	WaveformContext* canvas = a->canvas;

	if(canvas->viewport) *viewport = *canvas->viewport;
	else {
#else
	{
#endif
		viewport->left   = actor->region.x1;
		viewport->top    = actor->region.y1;
		viewport->right  = actor->region.x2;
		viewport->bottom = actor->region.y2;
	}
}


float
wf_actor_frame_to_x (WaveformActor* actor, uint64_t frame)
{
	//WfActorPriv* a = actor->priv;
	#define PIXELS_PER_SAMPLE (agl_actor__width((AGlActor*)actor) / actor->region.len)

	return ((float)frame - actor->region.start) * PIXELS_PER_SAMPLE + ((AGlActor*)actor)->region.x1;
}


/*
 *  Remove render data. Used for example if the render context has changed.
 *  Note that the render data is shared by all users of the waveform.
 *  The render data must be removed by a WaveformActor, it cannot be done by the Waveform itself.
 */
void
wf_actor_clear (WaveformActor* actor)
{
	// note that we cannot clear outstanding requests for loading audio as we
	// dont track who made the requests.

	WfActorPriv* a = actor->priv;
	Waveform* w = actor->waveform;

	if(a->load_render_data_queue){
		g_source_remove(a->load_render_data_queue);
		a->load_render_data_queue = 0;
	}

	int m; for(m=0;m<N_MODES;m++){
		WaveformModeRender** r = &w->priv->render_data[m];
		if(*r){
			if(modes[m].renderer->free){
				modes[m].renderer->free(modes[m].renderer, w);
			}
#ifdef DEBUG
			else gwarn("cannot free render data. mode=%s", modes[m].name);
#endif
			*r = 0;
		}
	}
}


#if 0
	int get_min_n_tiers()
	{
		return HI_MIN_TIERS;
	}
#endif

static void
_wf_actor_get_viewport_max (WaveformActor* a, WfViewPort* viewport)
{
	//special version of get_viewport that gets the outer viewport for duration of the current animation.

	AGlActor* actor = (AGlActor*)a;

	float left_max = MAX(actor->region.x1, LEFT(actor).target_val.f);
	float left_min = MIN(actor->region.x1, LEFT(actor).target_val.f);

	*viewport = (WfViewPort){
		.left   = left_min,
		.top    = actor->region.y1,
		.right  = left_max + agl_actor__width(actor),
		.bottom = actor->region.y2
	};
}


// px_per_sample values for each mode:
#define ZOOM_HI   (1.0/   16) // transition point from hi-res to v-hi-res mode.
#define ZOOM_MED  (1.0/  256) // transition point from std to hi-res mode.
#define ZOOM_LO   (1.0/ 4096) // transition point from low-res to std mode.
#define ZOOM_V_LO (1.0/65536) // transition point from v-low-res to low-res.

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
mode_range (WaveformActor* a)
{
	AGlActor* actor = (AGlActor*)a;

	float t1 = LEFT(actor).target_val.f;
	float t2 = RIGHT(actor).target_val.f;
	float target = t2 - t1;

	double zoom_end = ((float)agl_actor__width(actor)) / a->region.len;
	double zoom_start = target / a->region.len;
	if(zoom_start == 0.0) zoom_start = zoom_end;

	return (ModeRange){get_mode(zoom_start), get_mode(zoom_end)};
}


static void
_wf_actor_allocate_hi (WaveformActor* a)
{
	/*

	How many textures do we need?
	16 * 60 * 60 / hour    => 5760 / hour
		-because this is relatively high, a hashtable is used instead of an array.

	Hi-res mode uses a separate low-priority texture cache.

	*/
	//WfActorPriv* _a = a->priv;
	Waveform* w = a->waveform;
	g_return_if_fail(!w->offline);
	WfRectangle rect; WF_ACTOR_GET_RECT(a, &rect);
	WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
#ifdef USE_CANVAS_SCALING
	// currently only blocks for the final target zoom are loaded, not for any transitions
	double zoom = a->canvas->scaled ? a->canvas->zoom / a->canvas->samples_per_pixel : ((float)agl_actor__width(((AGlActor*)a))) / a->region.len;
#else
	double zoom = ((float)agl_actor__width(((AGlActor*)a))) / a->region.len;
#endif

#if 0
	// load _all_ blocks for the transition range
	// -works well at low-zoom and for smallish transitions
	// but can uneccesarily load too many blocks.

	WfAnimatable* start = &_a->animatable.start;
	WfSampleRegion region = {MIN(start->val.i, *start->model_val.i), _a->animatable.len.val.b};
	int first_block = wf_actor_get_first_visible_block(&region, zoom, rect, &viewport);
	//int last_block = get_last_visible_block(a, zoom, &viewport); -- looks like we might be able to do away with this now...?
	region.start = MAX(start->val.i, *_a->animatable.start.model_val.i);
	int last_block = wf_actor_get_last_visible_block(&region, rect, zoom, &viewport, a->waveform->textures);
	dbg(1, "%i--->%i", first_block, last_block);
	if(last_block - first_block > wf_audio_cache_get_size()) gwarn("too many blocks requested. increase cache size");

	int b;for(b=first_block;b<=last_block;b++){
		int n_tiers_needed = get_min_n_tiers();
		if(waveform_load_audio(a->waveform, b, n_tiers_needed, NULL, NULL)){
			((Renderer*)&hi_renderer)->load_block(&hi_renderer, a, b);
		}
	}
#else
	// load only the needed blocks - unfortunately is difficult to predict.

	//WfAnimatable* start = &_a->animatable.start;
	//WfSampleRegion region = {*START->model_val.b, *_a->animatable.len.model_val.b};
	BlockRange blocks = wf_actor_get_visible_block_range (&a->region, &rect, zoom, &viewport, a->waveform->priv->n_blocks);

	int b;for(b=blocks.first;b<=blocks.last;b++){
		hi_request_block(a, b);
	}

#if 0 // audio requests are currently done in start_transition() but needs testing.
	bool is_new = a->rect.len == 0.0;
	bool animate = a->canvas->draw && scene->enable_animations && !is_new;
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


	static bool load_render_data(gpointer _a)
	{
		WaveformActor* a = _a;
		AGlScene* scene = ((AGlActor*)a)->root;

		if(!a->waveform->priv->num_peaks) return G_SOURCE_CONTINUE; // TODO use peakdata-ready instead

		_wf_actor_load_missing_blocks(a); // this loads _current_ values, future values are loaded by the animator preview

		// because this is asynchronous, any caches may consist of an empty render.
		agl_actor__invalidate((AGlActor*)a);

		if(scene && scene->draw) wf_canvas_queue_redraw(a->canvas);
		a->priv->load_render_data_queue = 0;

		return G_SOURCE_REMOVE;
	}

static void
wf_actor_queue_load_render_data(WaveformActor* a)
{
	WfActorPriv* _a = a->priv;

	if(!_a->load_render_data_queue) _a->load_render_data_queue = g_timeout_add(10, load_render_data, a);
}


static void
_wf_actor_load_missing_blocks (WaveformActor* a)
{
	// during a transition, this will load the _currently_ visible blocks, not for the target.
	// (animation_preview is used to load blocks for the whole animation)
	// TODO so this means that if zooming _in_, blocks are loaded which are not needed

	inline WfViewPort get_clipping_port (WaveformActor* a)
	{
		// when animating region (ie panning) the viewport needs to be adjusted to prevent wf_actor_get_visible_block_range from clipping the range

		WfAnimatable* start = &START((AGlActor*)a);

		WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
		WfViewPort clippingport = viewport;
		float dl = start->target_val.b < start->start_val.b ?-(start->target_val.b - *start->val.b) : 0.0; // panning left. make adjustment to prevent clipping
		float dr = start->target_val.b > start->start_val.b ? (start->target_val.b - *start->val.b) : 0.0; // panning right. make adjustment to prevent clipping
		clippingport.left  = viewport.left  - dl;
		clippingport.right = viewport.right + dr;

		g_return_val_if_fail(clippingport.right >= clippingport.left, clippingport);

		return clippingport;
	}

	PF2;
	AGlActor* actor = (AGlActor*)a;
	Waveform* w = a->waveform;
	WaveformPrivate* _w = w->priv;

	WfdRange _zoom =
#ifdef USE_CANVAS_SCALING
		a->canvas->scaled ? (WfdRange){
			.start = a->canvas->zoom / a->canvas->samples_per_pixel,
			.end = a->canvas->priv->zoom.target_val.f / a->canvas->samples_per_pixel,
		} :
#endif
		(WfdRange){
			.start = agl_actor__width(actor) / a->region.len,
			.end = (RIGHT(actor).target_val.f - LEFT(actor).target_val.f) / LEN(actor).target_val.b,
	};

	if(_zoom.start == 0.0) _zoom.start = _zoom.end;
	Mode mode1 = get_mode(_zoom.start);
	Mode mode2 = get_mode(_zoom.end);
	dbg(2, "zoom=%.6f-->%.6f (%s --> %s)", _zoom.start, _zoom.end, modes[mode1].name, modes[mode2].name);

	double zoom = MIN(_zoom.end, _zoom.start); // use the zoom which uses the most blocks
																	//TODO can do better than this
	double zoom_max = MAX(_zoom.end, _zoom.start);

	Mode mode[] = {
		MIN(mode1, mode2),
		MAX(mode1, mode2)
	};

	for(int i=mode[0];i<=mode[1];i++)
		if(!_w->render_data[i])
			call(modes[i].renderer->new, a);

	if(zoom_max >= ZOOM_MED){
		dbg(1, "HI-RES");
		if(!a->waveform->offline)
			_wf_actor_allocate_hi(a);
		else {
			// fallback to lower res
			mode[0] = MAX(mode[0], MODE_MED);
			mode[1] = MAX(mode[1], MODE_MED);
		}
	}

	if(mode[0] <= MODE_MED && mode[1] >= MODE_MED){
		dbg(1, "MED");
		double zoom_ = MAX(zoom, ZOOM_LO + 0.00000001);

		uint64_t start_max = MAX(a->region.start, START(actor).target_val.b);
		uint64_t start_min = MIN(a->region.start, START(actor).target_val.b);
		int len_max = start_max - start_min + MAX(a->region.len, LEN(actor).target_val.b);
		WfSampleRegion region = {start_min, len_max};

		WfRectangle rect_ = {actor->region.x1, actor->region.y1, agl_actor__width(actor), agl_actor__height(actor)};

		WfViewPort clippingport = get_clipping_port(a);

		BlockRange viewport_blocks = wf_actor_get_visible_block_range (&region, &rect_, zoom_, &clippingport, _w->n_blocks);
		//dbg(0, "MED block range: %i --> %i", viewport_blocks.first, viewport_blocks.last);

		int b; for(b=viewport_blocks.first;b<=viewport_blocks.last;b++){
			modes[MODE_MED].renderer->load_block(modes[MODE_MED].renderer, a, b);
		}
	}

	if(mode[0] <= MODE_LOW && mode[1] >= MODE_LOW){
		// TODO low resolution doesnt have the same adjustments to region and viewport like STD mode does.
		// -this doesnt seem to be causing any problems though
		dbg(2, "LOW");
		Renderer* renderer = modes[MODE_LOW].renderer;
		//double zoom_ = MIN(zoom, ZOOM_LO - 0.0001);
		double zoom_ = MAX(zoom, ZOOM_V_LO + 0.00000001);

		uint64_t start_max = MAX(a->region.start, START(actor).target_val.b);
		uint64_t start_min = MIN(a->region.start, START(actor).target_val.b);
		int len_max = start_max - start_min + MAX(a->region.len, LEN(actor).target_val.b);
		WfSampleRegion region = {start_min, len_max};

		WfRectangle rect_ = {actor->region.x1, actor->region.y1, agl_actor__width(actor), agl_actor__height(actor)};

		WfViewPort clippingport = get_clipping_port(a);

		BlockRange viewport_blocks = wf_actor_get_visible_block_range (&region, &rect_, zoom_, &clippingport, wf_actor_get_n_blocks(w, MODE_LOW));
		//dbg(2, "LOW block range: %i --> %i", viewport_blocks.first, viewport_blocks.last);

		int b; for(b=viewport_blocks.first;b<=viewport_blocks.last;b++){
			renderer->load_block(renderer, a, b);
		}
	}

	if(mode[0] <= MODE_V_LOW && mode[1] >= MODE_V_LOW){
		dbg(2, "V_LOW");
		Renderer* renderer = modes[MODE_V_LOW].renderer;
		double zoom_ = MIN(zoom, ZOOM_LO - 0.0001);

		uint64_t start_max = MAX(a->region.start, START(actor).target_val.b);
		uint64_t start_min = MIN(a->region.start, START(actor).target_val.b);
		int len_max = start_max - start_min + MAX(a->region.len, LEN(actor).target_val.b);
		WfSampleRegion region = {start_min, len_max};

		WfRectangle rect_ = {actor->region.x1, actor->region.y1, agl_actor__width(actor), agl_actor__height(actor)};

		WfViewPort clippingport = get_clipping_port(a);

		BlockRange viewport_blocks = wf_actor_get_visible_block_range (&region, &rect_, zoom_, &clippingport, _w->render_data[MODE_V_LOW]->n_blocks);

		int b; for(b=viewport_blocks.first;b<=viewport_blocks.last;b++){
			renderer->load_block(renderer, a, b);
		}
	}

	gl_warn("");
}


										// private to AGlActor
										typedef struct {
											AGlActor*      actor;
											AnimationFn    done;
											gpointer       user_data;
										} C4;
void
wf_actor_set_rect (WaveformActor* a, WfRectangle* rect)
{
	g_return_if_fail(a);
	g_return_if_fail(rect);
	rect->len = MAX(1.0, rect->len);
	WfActorPriv* _a = a->priv;
	AGlActor* actor = (AGlActor*)a;
	AGlScene* scene = actor->root;

	AGL_DEBUG if(rect->len == agl_actor__width(actor) && rect->left == actor->region.x1 && rect->height == agl_actor__height(actor) && rect->top == actor->region.y1) dbg(1, "unchanged");
#if 0
	if(rect->len == a->rect.len && rect->left == a->rect.left && rect->height == a->rect.height && rect->top == a->rect.top) return;
#else
	// the renderer check is to ensure an initial draw. perhaps there is a better method.
	if(_a->render_info.renderer && rect->len == agl_actor__width(actor) && rect->left == actor->region.x1 && rect->height == agl_actor__height(actor) && rect->top == actor->region.y1) return;
#endif

	// TODO this test fails if we are called twice in quick succession because the valid flag is cleared in the first call
	bool had_full_render = _a->render_info.valid && !_a->render_info.cropped;

	_a->render_info.valid = false;

	WfAnimatable* a1 = &LEFT(actor);
	WfAnimatable* a2 = &RIGHT(actor);

	bool is_new = agl_actor__width(actor) == 0.0;
	bool animate = scene && scene->enable_animations && !is_new;
	bool left_changed = rect->left != a1->target_val.f;
	bool len_changed = rect->len != a2->target_val.f;
	bool have_full_render = had_full_render && !len_changed;

	if(agl_actor__width(actor) < 1 || agl_actor__height(actor) < 1){
		AGL_ACTOR__SET_REGION_FROM_RECT(actor, rect);
	}

	dbg(2, "rect: %i --> %i", actor->region.x1, actor->region.x2);

	if(a->region.len && !have_full_render && a->waveform->priv->num_peaks) wf_actor_queue_load_render_data(a);

	if(animate){
		if(left_changed || len_changed){
			size_transition_set(
				(TransitionBehaviour*)actor->behaviours[RECT],
				a,
				(TransitionValue[]){
					{left_changed, .value.f = rect->left},
					{len_changed, .value.f = rect->left + rect->len},
				},
				NULL,
				NULL
			);
		}

	}else{
		*a1->val.f = a1->target_val.f = a1->start_val.f = rect->left;
		*a2->val.f = a2->target_val.f = a2->start_val.f = rect->left + rect->len;

		if(scene && scene->draw) wf_canvas_queue_redraw(a->canvas);
	}
}


/*
 *  Function may be removed
 */
WfAnimatable*
wf_actor_get_z (WaveformActor* a)
{
	return &((TransitionBehaviour*)((AGlActor*)a)->behaviours[Z])->animatables[0];
}


void
wf_actor_set_z (WaveformActor* a, float z, WaveformActorFn callback, gpointer user_data)
{
	g_return_if_fail(a);

	transition_behaviour_set_f((TransitionBehaviour*)((AGlActor*)a)->behaviours[Z], (AGlActor*)a, z, callback, user_data);
}


#if 0
	static void _on_fadeout_finished(WfAnimation* animation, gpointer user_data)
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
#endif

void
wf_actor_fade_out (WaveformActor* a, WaveformActorFn callback, gpointer user_data)
{
	// not yet sure if this fn will remain (see sig of _fade_in)

	//TODO for consistency we need to change the background colour, though also need to preserve it to fade in.
	//     -can we get away with just setting _a->opacity?

	g_return_if_fail(a);

#if 1
	transition_behaviour_set_f((TransitionBehaviour*)((AGlActor*)a)->behaviours[OPACITY], (AGlActor*)a, 0.0, callback, user_data);
#else
	transition_behaviour_set(
		(TransitionBehaviour*)_a->behaviours[OPACITY],
		(AGlActor*)a,
		(TransitionValue[]){
			{true, .value.f = 0.0},
		},
		callback,
		user_data
	);
#endif
}


void
wf_actor_fade_in (WaveformActor* a, float target, WaveformActorFn callback, gpointer user_data)
{
	transition_behaviour_set(
		(TransitionBehaviour*)((AGlActor*)a)->behaviours[OPACITY],
		(AGlActor*)a,
		(TransitionValue[]){
			{true, .value.f = 1.0}
		},
		callback,
		user_data
	);
}


void
wf_actor_set_vzoom (WaveformActor* a, float vzoom)
{
	dbg(1, "vzoom=%.2f", vzoom);

	#define MAX_VZOOM 100.0
	a->vzoom = CLAMP(vzoom, 1.0, MAX_VZOOM);

	wf_context_set_gain(a->canvas, a->vzoom);

	agl_actor__invalidate((AGlActor*)a);
}


#if 0
static inline float
get_peaks_per_pixel_i(WaveformContext* wfc, WfSampleRegion* region, WfRectangle* rect, int mode)
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
#endif


static inline float
get_peaks_per_pixel(WaveformContext* wfc, WfSampleRegion* region, WfRectangle* rect, int mode)
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
calc_render_info (WaveformActor* actor)
{
	AGlActor* a = (AGlActor*)actor;
	WaveformContext* wfc = actor->canvas;
	Waveform* w = actor->waveform; 
	WaveformPrivate* _w = w->priv;
	RenderInfo* r  = &actor->priv->render_info;

	// This check was added because it appears to prevent corruption
	// but it is not clear why we are trying to render a waveform that has not been loaded
	if(!waveform_get_n_frames(w)) return false;

	wf_actor_get_viewport(actor, &r->viewport);

	r->region = (WfSampleRegion){actor->region.start, MIN(actor->region.len, w->n_frames)};
	static bool region_len_warning_done = false;
	if(!region_len_warning_done && !r->region.len){ region_len_warning_done = true; gwarn("zero region length"); }

	if(r->region.start + r->region.len > w->n_frames){
		// happens during transitions
		dbg(1, "bad region adjusted: %Lu / %Lu", r->region.start + r->region.len, w->n_frames);
		r->region.len = w->n_frames - r->region.start;
	}

	r->rect = (WfRectangle){a->region.x1, 0.0, agl_actor__width(a), agl_actor__height(a)};
	g_return_val_if_fail(r->rect.len, false);

	r->zoom = wfc->scaled
		? wfc->zoom / (wfc->samples_per_pixel)
		: r->rect.len / r->region.len;
	r->mode = get_mode(r->zoom);

	if(!_w->render_data[r->mode]) return false;
	r->n_blocks = _w->render_data[r->mode]->n_blocks;
	r->samples_per_texture = WF_SAMPLES_PER_TEXTURE * (r->mode == MODE_V_LOW ? WF_MED_TO_V_LOW : r->mode == MODE_LOW ? WF_PEAK_STD_TO_LO : 1);

																						// why we need this?
	r->region_start_block = r->region.start / r->samples_per_texture;

																						// FIXME this is calculated differently inside wf_actor_get_visible_block_range
	r->region_end_block = (r->region.start + r->region.len) / r->samples_per_texture - (!((r->region.start + r->region.len) % r->samples_per_texture) ? 1 : 0);
	r->viewport_blocks = wf_actor_get_visible_block_range(&r->region, &r->rect, r->zoom, &r->viewport, r->n_blocks);

	if(r->viewport_blocks.last == LAST_NOT_VISIBLE && r->viewport_blocks.first == FIRST_NOT_VISIBLE){
		r->valid = true; // this prevents unnecessary recalculation but the RenderInfo is not really valid so _must_ be invalidated again before use.
		return false;
	}
	// ideally conditions which trigger this should be detected before rendering.
	g_return_val_if_fail(r->viewport_blocks.last >= r->viewport_blocks.first, false);

	if(r->region_end_block > r->n_blocks -1){ gwarn("region too long? region_end_block=%i n_blocks=%i region.len=%"PRIi64, r->region_end_block, r->n_blocks, r->region.len); r->region_end_block = w->priv->n_blocks -1; }
#ifdef DEBUG
	dbg(2, "block range: region=%i-->%i viewport=%i-->%i", r->region_start_block, r->region_end_block, r->viewport_blocks.first, r->viewport_blocks.last);
	dbg(2, "rect=%.2f %.2f viewport=%.2f %.2f", r->rect.left, r->rect.len, r->viewport.left, r->viewport.right);
#endif

	if(r->viewport_blocks.last != r->region_end_block || r->viewport_blocks.first != r->region_start_block){
		// if not cropped by viewport we can avoid having to recheck for missing blocks during translations.
		r->cropped = true;
	}

	r->first_offset = r->region.start % r->samples_per_texture;
	r->first_offset_px = wf_actor_samples2gl(r->zoom, r->first_offset);

	r->block_wid = wf_actor_samples2gl(r->zoom, r->samples_per_texture);

	r->peaks_per_pixel = get_peaks_per_pixel(wfc, &r->region, &r->rect, r->mode) / 1.0;
#if 0
	r->peaks_per_pixel_i = get_peaks_per_pixel_i(wfc, &r->region, &r->rect, r->mode);
#endif

	r->renderer = set_renderer(actor);

	return r->valid = true;
}


static bool
wf_actor_paint (AGlActor* _actor)
{
	// -it is assumed that all textures have been loaded.

	// note: there is some benefit in quantising the x positions (eg for subpixel consistency),
	// but to preserve relative actor positions it must be done at the canvas level.

	WaveformActor* actor = (WaveformActor*)_actor;
	g_return_val_if_fail(actor, false);
	WaveformContext* wfc = actor->canvas;
	g_return_val_if_fail(wfc, false);
	WfActorPriv* _a = actor->priv;
	Waveform* w = actor->waveform; 
	RenderInfo* r  = &_a->render_info;
	if(!w || !w->priv->num_peaks) return false;

	g_return_val_if_fail(actor->region.start < actor->waveform->n_frames, false);

	if(!_actor->root || !_actor->root->draw) r->valid = false;

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

		WfRectangle rect = {_a->animatable.rect_left.val.f, _actor->region.y1, _a->animatable.rect_right.val.f, agl_actor__height(_actor)};

		WfSampleRegion region = (WfSampleRegion){_a->animatable.start.val.b, _a->animatable.len.val.b};
		double zoom = rect.len / region.len;
		if(zoom != r->zoom) gerr("valid should not be set: zoom %.3f %.3f", zoom, r->zoom);

		Mode mode = get_mode(r->zoom);
		if(mode != r->mode) gerr("valid should not be set: zoom %i %i", mode, r->mode);

		int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (mode == MODE_LOW ? WF_PEAK_STD_TO_LO : 1);
		int first_offset = region.start % samples_per_texture;
		if(first_offset != r->first_offset) gerr("valid should not be set: zoom %i %i", first_offset, r->first_offset);
#endif
	}

#ifdef SHOW_RENDERER_STATUS
	{
		Renderer* renderer = modes[MODE_MED].renderer;
		AGlShader* shader = ((NGRenderer*)renderer)->shader;
		HiResNGWaveform* data = (HiResNGWaveform*)w->priv->render_data[renderer->mode];
		dbg(1, "renderer status: MED data=%p shader=%p", data, shader);

		renderer = modes[MODE_LOW].renderer;
		shader = ((NGRenderer*)renderer)->shader;
		data = (HiResNGWaveform*)w->priv->render_data[renderer->mode];
		dbg(1, "renderer status: LO data=%p shader=%p", data, shader);
	}
#endif

	bool inline render_block(Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x, Mode m, Mode* m_active)
	{
		if(m != *m_active){
			if(!renderer->pre_render(renderer, actor))
				return false;
			*m_active = m;
		}
		return renderer->render_block(renderer, actor, b, is_first, is_last, x);
	}

	double x = (r->viewport_blocks.first - r->region_start_block) * r->block_wid - r->first_offset_px; // x is now the start of the first block (can be before part start when inset is present)
#ifdef RECT_ROUNDING
	double block_wid0 = r->block_wid;
	r->block_wid = round(r->block_wid);
	int i = 0;
	double x0 = x = round(x);
#endif

#ifdef DEBUG
	g_return_val_if_fail(WF_PEAK_BLOCK_SIZE == (WF_PEAK_RATIO * WF_PEAK_TEXTURE_SIZE), false); // temp check. we use a simplified loop which requires the two block sizes are the same
#endif

#if 0
	// Currently, because using the z property is not common, users have to do the translation themselves
	glTranslatef(0, 0, actor->priv->animatable.z.val.f);
#endif

	bool render_ok = true;
	Mode m_active = N_MODES;
	bool is_first = true;
	int b; for(b=r->viewport_blocks.first;b<=r->viewport_blocks.last;b++){
		bool is_last = (b == r->viewport_blocks.last) || (b == r->n_blocks - 1); //2nd test is unneccesary?

		Mode m = r->mode;
		// I think there is an optimisation to do here.
		// Pre-render should not be done for each block
		// but only when changing mode. Once we have fallen through
		// to a lower mode, it seems like we will not go back to a higher mode.
		while((m < N_MODES) && !render_block(modes[m].renderer, actor, b, is_first, is_last, x, m, &m_active)){
			dbg(1, "%i: %sfalling through...%s %s-->%s", b, "\x1b[1;33m", wf_white, modes[m].name, modes[m - 1].name);
			// TODO pre_render not being set propery for MODE_HI due to use_shader settings.
			// TODO render_info not correct when falling through. Is set for the higher mode.
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
		r->block_wid = round(x0 + (i + 1) * block_wid0) - x; // block_wid is not constant when using rounding
#else
		x += r->block_wid;
#endif
		is_first = false;
	}

#if 0
	glTranslatef(0, 0, -actor->priv->animatable.z.val.f);
#endif

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
		agl_enable(AGL_ENABLE_BLEND | !AGL_ENABLE_TEXTURE_2D);
		glColor4f(1.0, 0.0, 1.0, 0.75);
	}
	for(b=r->viewport_blocks.first;b<=r->viewport_blocks.last;b++){
		glLineWidth(1);
		float x_ = x + 0.1; // added for intel 945.
		glBegin(GL_LINES);
		{
			// lines are cropped by the rect to avoid confusion with multiple waveforms
			glVertex3f(MAX(x_, r->rect.left), top, 0); glVertex3f(MIN(x_ + r->block_wid, r->rect.left + r->rect.len), top, 0);
			if(x_ + r->block_wid <= r->rect.left + r->rect.len){
			glVertex3f(x_ + r->block_wid, top, 0);    glVertex3f(x_ + r->block_wid, bot,             0);
			}
			glVertex3f(x_,                bot, 0);    glVertex3f(x_ + r->block_wid, bot,             0);

			if(x_ >= r->rect.left){
			glVertex3f(x_,                top, 0);    glVertex3f(x_,                r->rect.top + 10,0);
			}
			glVertex3f(x_,                bot, 0);    glVertex3f(x_,                bot - 10,        0);
		}
		glEnd();

		// add a small marker every 32 blocks
		if(!(b % 32)){
			if(agl->use_shaders){
				agl->shaders.plain->uniform.colour = 0xff7733aa;
				agl->shaders.plain->shader.set_uniforms_();
			}
			glBegin(GL_LINES);
				glVertex3f(x_+ 1,         top, 0);    glVertex3f(x_ + 1,            r->rect.top + 10,0);
				glVertex3f(x_+ 1,         bot, 0);    glVertex3f(x_ + 1,            bot - 10,        0);
			glEnd();
			if(agl->use_shaders){
				agl->shaders.plain->uniform.colour = 0x6677ff77;
				agl->shaders.plain->shader.set_uniforms_();
			}
		}

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
#if 0
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
#endif


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


	typedef struct {
		WaveformActor* actor;
		AnimationFn    done;
		gpointer       user_data;
	} C3;

		// TODO consider having each wf_animation_preview callback as a separate idle fn.
		static void set_region_on_frame_preview (WfAnimation* animation, UVal val[], gpointer _c)
		{
			WaveformActor* a = _c;
			AGlActor* actor = (AGlActor*)a;

			GList* l = animation->members;
			for(;l;l=l->next){
				WfAnimActor* anim_actor = l->data;
				WfSampleRegion region = {START(actor).target_val.b, LEN(actor).target_val.b};
				GList* j = anim_actor->transitions;
				int i; for(i=0;j;j=j->next,i++){
					WfAnimatable* animatable = j->data;
					if(animatable == &START(actor)){
						region.start = val[i].b;
					}
					else if(animatable == &LEN(actor)){
						region.len = val[i].b;
					}

#ifdef USE_CANVAS_SCALING
					double zoom = a->canvas->scaled ? wf_context_get_zoom(a->canvas) : agl_actor__width(actor) / region.len;
#else
					double zoom = _a->rect.len / region.len;
#endif

					int mode = get_mode(zoom);
					WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);

					BlockRange blocks = wf_actor_get_visible_block_range (&region, &(WfRectangle)WF_ACTOR_RECT(actor), zoom, &viewport, wf_actor_get_n_blocks(a->waveform, mode));

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

/*
 *  load resources that will be needed over the course of the transition
 */
static void
wf_actor_on_size_transition_start (WaveformActor* a, WfAnimatable* Xanimatable)
{
	AGlActor* actor = (AGlActor*)a;

	// if neccesary load any additional audio needed by the transition.
	// - currently only changes to the region start are checked.
	{

		double zoom_start = agl_actor__width(actor) / a->region.len;
		double zoom_end = RIGHT(actor).target_val.f / a->region.len;
		if(zoom_start == 0.0) zoom_start = zoom_end;
		GList* l = actor->transitions;
		for(;l;l=l->next){
			WfAnimation* animation = l->data;
			GList* j = animation->members;
			for(;j;j=j->next){
				WfAnimActor* anim_actor = j->data;
				GList* k = anim_actor->transitions;
				for(;k;k=k->next){
					WfAnimatable* animatable = k->data;
												#if 0
					if(animatable == &_a->animatable.start){
						wf_animation_preview(g_list_last(actor->transitions)->data, set_region_on_frame_preview, a);
					}
												#endif
					if(animatable == &RIGHT(actor)){
						wf_animation_preview(g_list_last(actor->transitions)->data, set_region_on_frame_preview, a);
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

	WaveformPrivate* w = waveform->priv;

	switch(mode){
		case MODE_V_LOW:
			return w->n_blocks / WF_MED_TO_V_LOW + (w->n_blocks % WF_MED_TO_V_LOW ? 1 : 0);
		case MODE_LOW:
			return w->n_blocks / WF_PEAK_STD_TO_LO + (w->n_blocks % WF_PEAK_STD_TO_LO ? 1 : 0);
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

	modes[MODE_LOW].renderer = agl->use_shaders
		? (Renderer*)&lo_renderer_gl2
		: &lo_renderer_gl1;

	modes[MODE_V_LOW].renderer = agl->use_shaders
		? (Renderer*)&v_lo_renderer_gl2
		: &v_lo_renderer_gl1;
}


#ifdef USE_TEST
bool
wf_actor_test_is_not_blank(WaveformActor* a)
{
	RenderInfo* r = &a->priv->render_info;
	Renderer* renderer = r->renderer;
	g_return_val_if_fail(renderer, false);

	if(renderer->is_not_blank){
		if(renderer->is_not_blank(renderer, a)){
			dbg(0, "is not blank");
		}else{
			dbg(0, "is blank");
			return false;
		}
	}

	return true;
}

#endif
