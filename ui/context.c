/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | WaveformContext allow multiple actors to share audio related         |
 | properties such as sample-rate and samples-per-pixel                 |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#define __wf_private__
#define __wf_canvas_priv__

#include "config.h"
#include "agl/actor.h"
#include "agl/debug.h"
#include "transition/frameclock.h"
#include "wf/waveform.h"
#include "waveform/ui-utils.h"
#include "waveform/pixbuf.h"
#include "waveform/shader.h"
#include "waveform/texture_cache.h"
#include "waveform/ui-private.h"
#include "waveform/context.h"

static AGl* agl = NULL;

#define _g_source_remove0(S) {if(S) g_source_remove(S); S = 0;}

#undef is_sdl
#ifdef USE_SDL
#  define is_sdl(WFC) (WFC->root->type == CONTEXT_TYPE_SDL)
#else
#  define is_sdl(WFC) false
#endif

static void wf_context_class_init      (WaveformContextClass*);
static void wf_context_instance_init   (WaveformContext*);
static void wf_context_finalize        (GObject*);

#define TRACK_ACTORS // for debugging only.
#undef TRACK_ACTORS

#ifdef TRACK_ACTORS
static GList* actors = NULL;
#endif

static gpointer waveform_context_parent_class = NULL;
#define WAVEFORM_CONTEXT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_WAVEFORM_CONTEXT, WfContextPriv))


GType
waveform_context_get_type ()
{
	static volatile gsize waveform_context_type_id__volatile = 0;
	if (g_once_init_enter ((gsize*)&waveform_context_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (WaveformContextClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) wf_context_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (WaveformContext), 0, (GInstanceInitFunc) wf_context_instance_init, NULL };
		GType waveform_context_type_id;
		waveform_context_type_id = g_type_register_static (G_TYPE_OBJECT, "WaveformContext", &g_define_type_info, 0);
		g_once_init_leave (&waveform_context_type_id__volatile, waveform_context_type_id);
	}
	return waveform_context_type_id__volatile;
}


#if defined (WF_USE_TEXTURE_CACHE) && defined (USE_OPENGL)
static void
on_steal (WfTexture* tex)
{
	WaveformBlock* wb = &tex->wb;

	if(wb->block & WF_TEXTURE_CACHE_HIRES_NG_MASK){
		extern void hi_gl2_on_steal(WaveformBlock*, guint);
		hi_gl2_on_steal(wb, tex->id);
	}else{
		extern void med_lo_on_steal(WaveformBlock*, guint);
		med_lo_on_steal(wb, tex->id);
	}
}
#endif


static void
wf_context_class_init (WaveformContextClass* klass)
{
	waveform_context_parent_class = g_type_class_peek_parent (klass);

	G_OBJECT_CLASS (klass)->finalize = wf_context_finalize;

	g_signal_new ("dimensions_changed", TYPE_WAVEFORM_CONTEXT, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	g_signal_new ("zoom_changed", TYPE_WAVEFORM_CONTEXT, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	agl = agl_get_instance();

#if defined (WF_USE_TEXTURE_CACHE) && defined (USE_OPENGL)
	texture_cache_init();
	texture_cache_set_on_steal(on_steal);
#endif

	// testing...
	g_signal_new ("ready", TYPE_WAVEFORM_CONTEXT, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
wf_context_instance_init (WaveformContext* self)
{
}


static void
wf_context_init (WaveformContext* wfc, AGlActor* root)
{
	wfc->sample_rate = 44100;
	wfc->v_gain = 1.0;

	wfc->samples_per_pixel = wfc->sample_rate / 32.0; // 32 pixels for 1 second
	wfc->bpm = 120.0;

	wfc->zoom = agl_observable_new();
	wfc->zoom->value.f = 1.0;
	wfc->zoom->min.f = WF_CONTEXT_MIN_ZOOM;
	wfc->zoom->max.f = WF_CONTEXT_MAX_ZOOM;

	wfc->start_time = AGL_NEW(AGlObservable,
		.max.b = LONG_MAX,
	);

	wfc->priv = WF_NEW(WfContextPriv,
		.zoom = {
			.val.f        = &wfc->zoom->value.f,
			.start_val.f  = wfc->zoom->value.f,
			.target_val.f = wfc->zoom->value.f,
			.type         = WF_FLOAT
		},
		.samples_per_pixel = {
			.val.f        = &wfc->samples_per_pixel,
			.start_val.f  = wfc->samples_per_pixel,
			.target_val.f = wfc->samples_per_pixel,
			.type         = WF_FLOAT
		},
		.start = {
			.val.b        = &wfc->start_time->value.b,
			.start_val.b  = wfc->start_time->value.b,
			.target_val.b = wfc->start_time->value.b,
			.type         = WF_INT64
		}
	);
}


static WaveformContext*
waveform_context_construct (GType object_type)
{
	return (WaveformContext*)g_object_new(object_type, NULL);
}


extern void wf_gl_init (WaveformContext*, AGlActor*);

WaveformContext*
wf_context_new (AGlActor* root)
{
	PF;

	WaveformContext* wfc = waveform_context_construct(TYPE_WAVEFORM_CONTEXT);
	wfc->root = root;
	wf_context_init(wfc, root);
	wf_gl_init(wfc, root);

	return wfc;
}


#ifdef USE_SDL
WaveformContext*
wf_context_new_sdl (SDL_GLContext* context)
{
	PF;

	WaveformContext* wfc = waveform_context_construct(TYPE_WAVEFORM_CONTEXT);

	wfc->show_rms = true;

	AGlActor* a = wfc->root = agl_actor__new_root(CONTEXT_TYPE_SDL);
	wfc->root->root->gl.sdl.context = context;

	wf_context_init(wfc, a);
	wf_gl_init(wfc, a);

	return wfc;
}
#endif


static void
wf_context_finalize (GObject* obj)
{
	WaveformContext* wfc = WAVEFORM_CONTEXT (obj);

	wf_free(wfc->priv);

	G_OBJECT_CLASS (waveform_context_parent_class)->finalize (obj);
}


void
wf_context_free (WaveformContext* wfc)
{
	g_return_if_fail(wfc);
	PF;
	WfContextPriv* c = wfc->priv;

	_g_source_remove0(c->pending_init);
	_g_source_remove0(c->_queued);
	g_clear_pointer(&wfc->zoom, agl_observable_free);
	g_clear_pointer(&wfc->start_time, agl_observable_free);

	g_object_unref((GObject*)wfc);
}


/*
 *  The actor is owned by the context and will be freed on calling agl_actor__remove_child().
 *
 *  After adding a waveform to the context you can g_object_unref the waveform if
 *  You do not need to hold an additional reference.
 */
WaveformActor*
wf_context_add_new_actor (WaveformContext* wfc, Waveform* w)
{
	g_return_val_if_fail(wfc, NULL);

	WaveformActor* a = wf_actor_new(w, wfc);
#ifdef TRACK_ACTORS
	actors = g_list_append(actors, a);
#endif
	return a;
}


#ifndef USE_FRAME_CLOCK
	static bool wf_canvas_redraw(gpointer _canvas)
	{
		WaveformContext* wfc = _canvas;

		if (wfc->root->draw) wfc->root->draw(wfc->root, wfc->root->user_data);
		wfc->priv->_queued = false;

		return G_SOURCE_REMOVE;
	}
#endif

void
wf_context_queue_redraw (WaveformContext* wfc)
{
#ifdef USE_FRAME_CLOCK
	if (wfc->root->root->is_animating) {
#if 0 // this is not needed - draw is called via the frame_clock_connect callback
		if(wfc->draw) wfc->draw(wfc, wfc->draw_data);
#endif
	} else {
#if 0
		frame_clock_request_phase(GDK_FRAME_CLOCK_PHASE_PAINT);
#else
		frame_clock_request_phase(GDK_FRAME_CLOCK_PHASE_UPDATE);
#endif
	}

#else
	if (!wfc->priv->_queued)
		wfc->priv->_queued = g_timeout_add(CLAMP(WF_FRAME_INTERVAL - (wf_get_time() - wfc->priv->_last_redraw_time), 1, WF_FRAME_INTERVAL), wf_canvas_redraw, wfc);
#endif
}


#ifdef USE_CANVAS_SCALING
float
wf_context_get_zoom (WaveformContext* wfc)
{
	return wfc->scaled ? wfc->zoom->value.f : 0.0;
}


static void
set_zoom_on_animation_finished (WfAnimation* animation, gpointer _wfc)
{
	dbg(1, "wfc=%p", _wfc);
}


static void
wf_context_set_zoom_on_frame (WfAnimation* animation, int time)
{
	WaveformContext* wfc = animation->user_data;

	agl_observable_set_float(wfc->zoom, wfc->zoom->value.f);

	// note that everything under the context root is invalidated.
	// Any non-scalable items should be in a separate sub-graph
	agl_actor__invalidate_down (wfc->root);

	agl_actor__set_size((AGlActor*)wfc->root);
}


/*
 *  Allows the whole scene to be scaled as a single transition
 *  which is more efficient than scaling many individual waveforms.
 *
 *  Calling this function puts the canvas into 'scaled' mode.
 *
 *  Before using scaled mode, the user must set the base
 *  samples_per_pixel value.
 */
void
wf_context_set_zoom (WaveformContext* wfc, float zoom)
{
	wfc->scaled = true;

	dbg(1, "zoom=%.2f-->%.2f spp=%.2f", wfc->zoom->value.f, zoom, wfc->samples_per_pixel);

	AGL_DEBUG if(wfc->samples_per_pixel < 0.001) pwarn("spp too low: %f", wfc->samples_per_pixel);

	zoom = CLAMP(zoom, WF_CONTEXT_MIN_ZOOM, WF_CONTEXT_MAX_ZOOM);

	if(!wfc->root->root->enable_animations){
		agl_observable_set_float(wfc->zoom, zoom);
		agl_actor__invalidate(wfc->root);
		return;
	}

	if (zoom == wfc->zoom->value.f) {
		return;
	}

	g_signal_emit_by_name(wfc, "zoom-changed");

	wfc->priv->zoom.target_val.f = zoom;

	WfAnimation* animation = wf_animation_new(set_zoom_on_animation_finished, wfc);
	animation->on_frame = wf_context_set_zoom_on_frame;

	GList* animatables = g_list_prepend(NULL, &wfc->priv->zoom);
	wf_transition_add_member(animation, animatables);

	wf_animation_start(animation);
}
#endif


/*
 *  wf_context_set_scale is similar to wf_context_set_zoom but
 *  does not use the zoom property because sometimes it is more
 *  convenient to specify the number of samples per pixel directly.
 */
void
wf_context_set_scale (WaveformContext* wfc, float samples_per_px)
{
	#define WF_CONTEXT_MAX_SAMPLES_PER_PIXEL 1000000.0

	samples_per_px = CLAMP(samples_per_px, 1.0, WF_CONTEXT_MAX_SAMPLES_PER_PIXEL);

	if (samples_per_px == wfc->samples_per_pixel) {
		return;
	}

	if (!wfc->root->root->enable_animations) {
		wfc->samples_per_pixel = samples_per_px;
		agl_actor__invalidate_down (wfc->root);
		if (wfc->root->parent)
			agl_actor__invalidate(wfc->root->parent);
		return;
	}
	g_signal_emit_by_name(wfc, "zoom-changed");

	WfAnimation* animation = wf_animation_new(NULL, wfc);
	animation->on_frame = wf_context_set_zoom_on_frame;

	wfc->priv->samples_per_pixel.target_val.f = samples_per_px;
	wf_transition_add_member(animation, g_list_prepend(NULL, &wfc->priv->samples_per_pixel));

	wf_animation_start(animation);
}


static void
wf_context_set_start_on_frame (WfAnimation* animation, int time)
{
	WaveformContext* wfc = animation->user_data;

	agl_observable_set_float(wfc->start_time, wfc->start_time->value.b);

	agl_actor__invalidate_down (wfc->root);
}


void
wf_context_set_start (WaveformContext* wfc, int64_t start)
{
	if (start == wfc->priv->start.target_val.b) return;

	WfAnimation* animation = wf_animation_new(NULL, wfc);
	animation->on_frame = wf_context_set_start_on_frame;

	wfc->priv->start.target_val.b = start;
	wf_transition_add_member(animation, g_list_prepend(NULL, &wfc->priv->start));

	wf_animation_start(animation);
}


void
wf_context_set_gain (WaveformContext* wfc, float gain)
{
	wfc->v_gain = gain;
	wf_context_queue_redraw(wfc);
}


float
wf_context_frame_to_x (WaveformContext* context, uint64_t frame)
{
	float pixels_per_sample = context->zoom->value.f / context->samples_per_pixel;
	return ((float)frame - (float)context->start_time->value.b) * pixels_per_sample;
}


uint64_t
wf_context_x_to_frame (WaveformContext* context, int x)
{
	return context->start_time->value.b + (float)x * context->samples_per_pixel / context->zoom->value.f;
}


const char*
wf_context_print_time (WaveformContext* wfc, int x)
{
	static char str[16] = "\0";

	int64_t frames = wf_context_x_to_frame(wfc, x);
	int _secsf = frames % (wfc->sample_rate * 60);

	int mins = frames / (wfc->sample_rate * 60);
	int secs = _secsf / wfc->sample_rate;
	int sub = (_secsf % wfc->sample_rate) * 1000 / wfc->sample_rate;
	snprintf(str, 15, "%02i:%02i:%03i", mins, secs, sub);

	return str;
}
