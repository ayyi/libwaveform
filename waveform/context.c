/*
  copyright (C) 2012-2019 Tim Orford <tim@orford.org>

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

  WaveformContext acts as a shared context for drawing multiple related Waveform Actors.

*/
#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef USE_GTK
#include <gtk/gtk.h>
#endif
#include <GL/gl.h>
#include <pango/pangofc-font.h>
#include <pango/pangofc-fontmap.h>
#include "agl/ext.h"
#include "agl/pango_render.h"
#include "transition/frameclock.h"
#include "waveform/waveform.h"
#include "waveform/alphabuf.h"
#include "waveform/shader.h"
#include "waveform/context.h"

static AGl* agl = NULL;

#define _g_source_remove0(S) {if(S) g_source_remove(S); S = 0;}

#undef is_sdl
#ifdef USE_SDL
#  define is_sdl(WFC) (WFC->root->type == CONTEXT_TYPE_SDL)
#else
#  define is_sdl(WFC) false
#endif

#ifdef USE_GTK
#define WAVEFORM_START_DRAW(wfc) \
	if(wfc->_draw_depth) gwarn("START_DRAW: already drawing"); \
	wfc->_draw_depth++; \
	if (actor_not_is_gtk(wfc->root->root) || \
		(wfc->_draw_depth > 1) || gdk_gl_drawable_make_current (wfc->root->root->gl.gdk.drawable, wfc->root->root->gl.gdk.context) \
		) {
#else
#define WAVEFORM_START_DRAW(wfc) \
	;
#endif

#ifdef USE_GTK
#define WAVEFORM_END_DRAW(wa) \
	wa->_draw_depth--; \
	if(wa->root->root->type == CONTEXT_TYPE_GTK){ \
		if(!wa->_draw_depth) ; \
	} \
	} else gwarn("!! gl_begin fail")
#else
#define WAVEFORM_END_DRAW(wa) \
	;
#endif

#define WAVEFORM_IS_DRAWING(wa) \
	(wa->_draw_depth > 0)

static void wf_context_init_gl         (WaveformContext*);
static void wf_context_class_init      (WaveformContextClass*);
static void wf_context_instance_init   (WaveformContext*);
static void wf_context_finalize        (GObject*);
static void wf_context_on_paint_update (GdkFrameClock*, void*);

extern PeakShader peak_shader, peak_nonscaling;
extern HiResShader hires_shader;
extern BloomShader horizontal;
extern BloomShader vertical;
extern AlphaMapShader tex2d, ass;
extern RulerShader ruler;
extern CursorShader cursor;

#define TRACK_ACTORS // for debugging only.
#undef TRACK_ACTORS

#ifdef TRACK_ACTORS
static GList* actors = NULL;
#endif

static gpointer waveform_context_parent_class = NULL;
#define WAVEFORM_CONTEXT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_WAVEFORM_CONTEXT, WfContextPriv))


GType
waveform_context_get_type()
{
	static volatile gsize waveform_context_type_id__volatile = 0;
	if (g_once_init_enter (&waveform_context_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (WaveformContextClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) wf_context_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (WaveformContext), 0, (GInstanceInitFunc) wf_context_instance_init, NULL };
		GType waveform_context_type_id;
		waveform_context_type_id = g_type_register_static (G_TYPE_OBJECT, "WaveformContext", &g_define_type_info, 0);
		g_once_init_leave (&waveform_context_type_id__volatile, waveform_context_type_id);
	}
	return waveform_context_type_id__volatile;
}


static void
wf_context_class_init(WaveformContextClass* klass)
{
	waveform_context_parent_class = g_type_class_peek_parent (klass);
	//g_type_class_add_private (klass, sizeof (WaveformContextPrivate));
	G_OBJECT_CLASS (klass)->finalize = wf_context_finalize;
	g_signal_new ("dimensions_changed", TYPE_WAVEFORM_CONTEXT, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	g_signal_new ("zoom_changed", TYPE_WAVEFORM_CONTEXT, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	agl = agl_get_instance();

	// testing...
	g_signal_new ("ready", TYPE_WAVEFORM_CONTEXT, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
wf_context_instance_init(WaveformContext* self)
{
}


		static bool wf_canvas_try_drawable(gpointer _wfc)
		{
			WaveformContext* wfc = _wfc;
			AGlScene* scene = wfc->root->root;

#ifdef USE_GTK
			if((scene->type == CONTEXT_TYPE_GTK) && !wfc->root->root->gl.gdk.drawable){
				return G_SOURCE_CONTINUE;
			}
#endif

			wf_context_init_gl(wfc);

			if(scene->draw) wf_canvas_queue_redraw(wfc);
			wfc->use_1d_textures = agl->use_shaders;

#ifdef USE_FRAME_CLOCK
			frame_clock_connect(G_CALLBACK(wf_context_on_paint_update), wfc);
#endif
			return wfc->priv->pending_init = G_SOURCE_REMOVE;
		}

static void
wf_context_init (WaveformContext* wfc, AGlActor* root)
{
	wfc->priv = g_new0(WfContextPriv, 1);

	wfc->sample_rate = 44100;
	wfc->v_gain = 1.0;
	wfc->texture_unit[0] = agl_texture_unit_new(WF_TEXTURE0);
	wfc->texture_unit[1] = agl_texture_unit_new(WF_TEXTURE1);
	wfc->texture_unit[2] = agl_texture_unit_new(WF_TEXTURE2);
	wfc->texture_unit[3] = agl_texture_unit_new(WF_TEXTURE3);

	wfc->samples_per_pixel = wfc->sample_rate / 32.0; // 32 pixels for 1 second
	wfc->zoom = 1.0;
	wfc->bpm = 120.0;

	wfc->priv->zoom = (WfAnimatable){
		.val.f        = &wfc->zoom,
		.start_val.f  = wfc->zoom,
		.target_val.f = wfc->zoom,
		.type         = WF_FLOAT
	};

	wfc->priv->samples_per_pixel = (WfAnimatable){
		.val.f        = &wfc->samples_per_pixel,
		.start_val.f  = wfc->samples_per_pixel,
		.target_val.f = wfc->samples_per_pixel,
		.type         = WF_FLOAT
	};

	wfc->shaders.ruler = &ruler;

	if(wfc->root){
		if(wf_canvas_try_drawable(wfc)) wfc->priv->pending_init = g_idle_add(wf_canvas_try_drawable, wfc);
	}
}


WaveformContext*
waveform_canvas_construct(GType object_type)
{
	WaveformContext* wfc = (WaveformContext*)g_object_new(object_type, NULL);
	return wfc;
}


WaveformContext*
wf_context_new(AGlActor* root)
{
	PF;

	WaveformContext* wfc = waveform_canvas_construct(TYPE_WAVEFORM_CONTEXT);
	wfc->root = root;
	wf_context_init(wfc, root);

	return wfc;
}


#ifdef USE_SDL
WaveformContext*
wf_context_new_sdl(SDL_GLContext* context)
{
	PF;

	WaveformContext* wfc = waveform_canvas_construct(TYPE_WAVEFORM_CONTEXT);

	wfc->show_rms = true;

	AGlActor* a = wfc->root = agl_actor__new_root_(CONTEXT_TYPE_SDL);
	wfc->root->root->gl.sdl.context = context;

	wf_context_init(wfc, a);

	return wfc;
}
#endif


static void
wf_context_finalize(GObject* obj)
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

#ifdef USE_FRAME_CLOCK
	frame_clock_disconnect(G_CALLBACK(wf_context_on_paint_update), wfc);
#endif

	_g_source_remove0(c->pending_init);
	_g_source_remove0(c->_queued);
	wf_context_finalize((GObject*)wfc);

	pango_gl_render_clear_caches();
}


static void
wf_context_init_gl(WaveformContext* wfc)
{
	PF;
#if 0
	if(agl->shaders.plain->shader.program){
		return;
	}
#endif

	if(!agl->pref_use_shaders){
		wfc->use_1d_textures = false;
#if 0
		WAVEFORM_START_DRAW(wfc) {
			if(wf_debug) printf("GL_RENDERER = %s\n", (const char*)glGetString(GL_RENDERER));
		} WAVEFORM_END_DRAW(wfc);
#endif
		return;
	}

	WAVEFORM_START_DRAW(wfc) {

		if(!wfc->root){
			agl_gl_init();
		}

		if(!agl->use_shaders){
			agl_use_program(NULL);
			wfc->use_1d_textures = false;
		}

	} WAVEFORM_END_DRAW(wfc);
}


#ifdef USE_FRAME_CLOCK
static void
wf_context_on_paint_update(GdkFrameClock* clock, void* _canvas)
{
	WaveformContext* wfc = _canvas;

	if(wfc->root->root->draw) wfc->root->root->draw(wfc->root->root, wfc->root->root->user_data);
	wfc->priv->_last_redraw_time = wf_get_time();
}
#endif


/*
 *  This will likely be removed. Instead just set scene->scrollable
 */
void
wf_context_set_viewport(WaveformContext* wfc, WfViewPort* _viewport)
{
	//@param viewport - optional.
	//                  Does not apply clipping.
	//                  units are not pixels, they are gl units.
	//                  setting viewport->left, viewport->top allows (application implemented) scrolling.

	g_return_if_fail(wfc);

	if(_viewport){
		((AGlActor*)wfc->root)->scrollable = (AGliRegion){
			.x1 = _viewport->left,
			.y1 = _viewport->top,
			.x2 = _viewport->right,
			.y2 = _viewport->bottom,
		};
	}

	if(wfc->root->root->draw) g_signal_emit_by_name(wfc, "dimensions-changed");
}


/*
 *  The actor is owned by the context and will be freed on calling wf_canvas_remove_actor().
 *
 *  After adding a waveform to the context you can g_object_unref the waveform if
 *  You do not need to hold an additional reference.
 */
WaveformActor*
wf_canvas_add_new_actor(WaveformContext* wfc, Waveform* w)
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

		if(wfc->root->draw) wfc->root->draw(wfc->root, wfc->root->user_data);
		wfc->priv->_queued = false;

		return G_SOURCE_REMOVE;
	}
#endif

void
wf_canvas_queue_redraw(WaveformContext* wfc)
{
#ifdef USE_FRAME_CLOCK
	if(wfc->root->root->is_animating){
#if 0 // this is not needed - draw is called via the frame_clock_connect callback
		if(wfc->draw) wfc->draw(wfc, wfc->draw_data);
#endif
	}else{
#if 0
		frame_clock_request_phase(GDK_FRAME_CLOCK_PHASE_PAINT);
#else
		frame_clock_request_phase(GDK_FRAME_CLOCK_PHASE_UPDATE);
#endif
	}

#else
	if(wfc->priv->_queued) return;

	wfc->priv->_queued = g_timeout_add(CLAMP(WF_FRAME_INTERVAL - (wf_get_time() - wfc->priv->_last_redraw_time), 1, WF_FRAME_INTERVAL), wf_canvas_redraw, wfc);
#endif
}


float
wf_canvas_gl_to_px(WaveformContext* wfc, float x)
{
	//convert from gl coords to screen pixels

	// TODO  _gl_to_px TODO where viewport not set.
#if 0
	if(!wfc->viewport) return x;

	//TODO move to resize handler?
	gint drawable_width_px, height;
	gdk_gl_drawable_get_size(gdk_gl_context_get_gl_drawable(gdk_gl_context_get_current()), &drawable_width_px, &height);

	float viewport_width = 256; //TODO
	if(wfc->viewport) viewport_width = wfc->viewport->right - wfc->viewport->left;

	float scale = drawable_width_px / viewport_width;
	return x * scale;
#else
	return x;
#endif
}


void
wf_canvas_load_texture_from_alphabuf(WaveformContext* wfc, int texture_name, AlphaBuf* alphabuf)
{
	//load the Alphabuf into the gl texture identified by texture_name.
	//-the user can usually free the Alphabuf afterwards as it is unlikely to be needed again.

	g_return_if_fail(alphabuf);
	g_return_if_fail(texture_name);

#ifdef USE_MIPMAPPING
	guchar* generate_mipmap(AlphaBuf* a, int level)
	{
		int r = 1 << level;
		int height = MAX(1, a->height / r);
		int width = agl->have & AGL_HAVE_NPOT_TEXTURES ? MAX(1, a->width / r) : height;
		guchar* buf = g_malloc(width * height);

		int y; for(y=0;y<height;y++){
			int x; for(x=0;x<width;x++){
				//TODO find max of all peaks, dont just use one.
				buf[width * y + x] = a->buf[a->width * y * r + x * r];
			}
		}
		return buf;
	}
#endif

	WAVEFORM_START_DRAW(wfc) {
		//note: gluBuild2DMipmaps is deprecated. instead use GL_GENERATE_MIPMAP (requires GL 1.4)

		glBindTexture(GL_TEXTURE_2D, texture_name);
		int width = agl->have & AGL_HAVE_NPOT_TEXTURES ? alphabuf->width : alphabuf->height;
		dbg (2, "copying texture... width=%i texture_id=%u", width, texture_name);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8, width, alphabuf->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, alphabuf->buf);

#ifdef USE_MIPMAPPING
		{
			int l; for(l=1;l<16;l++){
				guchar* buf = generate_mipmap(alphabuf, l);
				int width = (agl->have & AGL_HAVE_NPOT_TEXTURES ? alphabuf->width : alphabuf->height) / (1<<l);
				int width = alphabuf->height / (1<<l);
				glTexImage2D(GL_TEXTURE_2D, l, GL_ALPHA8, width, alphabuf->height/(1<<l), 0, GL_ALPHA, GL_UNSIGNED_BYTE, buf);
				wf_free(buf);
				int w = alphabuf->width / (1<<l);
				int h = alphabuf->height / (1<<l);
				if((w < 2) && (h < 2)) break;
			}
		}
#endif

#ifdef USE_MIPMAPPING
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
#else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
		// if TEX_BORDER is used, clamping will make no difference as we dont reach the edge of the texture.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //note using this stops gaps between blocks.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // prevent wrapping. GL_CLAMP_TO_EDGE uses the nearest texture value, and will not fade to the border colour like GL_CLAMP
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); //prevent wrapping

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		if(!glIsTexture(texture_name)) gwarn("texture not loaded! %i", texture_name);
	} WAVEFORM_END_DRAW(wfc);

	gl_warn("copy to texture");
}


void
wf_context_set_rotation(WaveformContext* wfc, float rotation)
{
	dbg(0, "TODO");
}


#ifdef USE_CANVAS_SCALING
float
wf_context_get_zoom(WaveformContext* wfc)
{
	return wfc->scaled ? wfc->zoom : 0.0;
}


	static void set_zoom_on_animation_finished(WfAnimation* animation, gpointer _wfc)
	{
		WaveformContext* wfc = _wfc;
		dbg(1, "wfc=%p", wfc);
	}

	static void wf_context_set_zoom_on_frame(WfAnimation* animation, int time)
	{
		WaveformContext* wfc = animation->user_data;

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
	// TODO should probably call agl_actor__start_transition

	wfc->scaled = true;
	dbg(1, "zoom=%f spp=%.2f", zoom, wfc->samples_per_pixel);

	AGL_DEBUG if(wfc->samples_per_pixel < 0.001) gwarn("spp too low: %f", wfc->samples_per_pixel);

	zoom = CLAMP(zoom, WF_CONTEXT_MIN_ZOOM, WF_CONTEXT_MAX_ZOOM);

	if(!wfc->root->root->enable_animations){
		wfc->zoom = zoom;
		agl_actor__invalidate(wfc->root);
		return;
	}

	// TODO move this into the animator xx
	if(zoom == wfc->zoom){
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

	if(!wfc->root->root->enable_animations){
		wfc->samples_per_pixel = samples_per_px;
		agl_actor__invalidate((AGlActor*)wfc->root);
		return;
	}
	g_signal_emit_by_name(wfc, "zoom-changed");

	WfAnimation* animation = wf_animation_new(NULL, wfc);
	animation->on_frame = wf_context_set_zoom_on_frame;

	wfc->priv->samples_per_pixel.target_val.f = samples_per_px;
	wf_transition_add_member(animation, g_list_prepend(NULL, &wfc->priv->samples_per_pixel));

	wf_animation_start(animation);
}


void
wf_context_set_gain(WaveformContext* wfc, float gain)
{
	wfc->v_gain = gain;
	wf_canvas_queue_redraw(wfc);
}


float
wf_context_frame_to_x (WaveformContext* context, uint64_t frame)
{
	float pixels_per_sample = context->zoom / context->samples_per_pixel;
	return (frame - context->start_time) * pixels_per_sample;
}


