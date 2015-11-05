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

  WaveformCanvas acts as a shared context for drawing multiple related Waveform Actors.

*/
#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <pango/pangofc-font.h>
#include <pango/pangofc-fontmap.h>
#include "agl/ext.h"
#include "agl/pango_render.h"
#include "waveform/waveform.h"
#include "waveform/gl_utils.h"
#include "waveform/canvas.h"
#include "waveform/alphabuf.h"
#include "waveform/shader.h"
#include "transition/frameclock.h"

static AGl* agl = NULL;

#undef is_sdl
#ifdef USE_SDL
#  define is_sdl(WFC) (WFC->root->type == CONTEXT_TYPE_SDL)
#else
#  define is_sdl(WFC) false
#endif

#define WAVEFORM_START_DRAW(wfc) \
	if(wfc->_draw_depth) gwarn("START_DRAW: already drawing"); \
	wfc->_draw_depth++; \
	if (actor_not_is_gtk(wfc->root) || \
		(wfc->_draw_depth > 1) || gdk_gl_drawable_gl_begin (wfc->root->gl.gdk.drawable, wfc->root->gl.gdk.context) \
		) {

#define WAVEFORM_END_DRAW(wa) \
	wa->_draw_depth--; \
	if(wa->root->type == CONTEXT_TYPE_GTK){ \
		if(!wa->_draw_depth) gdk_gl_drawable_gl_end(wa->root->gl.gdk.drawable); \
	} \
	} else gwarn("!! gl_begin fail")

#define WAVEFORM_IS_DRAWING(wa) \
	(wa->_draw_depth > 0)

static void wf_canvas_init_gl        (WaveformCanvas*);
static void wf_context_class_init    (WaveformContextClass*);
static void wf_context_instance_init (WaveformCanvas*);
static void wf_canvas_finalize       (GObject*);

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
GList* actors = NULL;
#endif

static gpointer waveform_context_parent_class = NULL;
#define WAVEFORM_CONTEXT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_WAVEFORM_CONTEXT, WfContextPriv))


GType
waveform_context_get_type()
{
	static volatile gsize waveform_context_type_id__volatile = 0;
	if (g_once_init_enter (&waveform_context_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (WaveformContextClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) wf_context_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (WaveformCanvas), 0, (GInstanceInitFunc) wf_context_instance_init, NULL };
		GType waveform_context_type_id;
		waveform_context_type_id = g_type_register_static (G_TYPE_OBJECT, "WaveformCanvas", &g_define_type_info, 0);
		g_once_init_leave (&waveform_context_type_id__volatile, waveform_context_type_id);
	}
	return waveform_context_type_id__volatile;
}


static void
wf_context_class_init(WaveformContextClass* klass)
{
	waveform_context_parent_class = g_type_class_peek_parent (klass);
	//g_type_class_add_private (klass, sizeof (WaveformContextPrivate));
	G_OBJECT_CLASS (klass)->finalize = wf_canvas_finalize;
	g_signal_new ("dimensions_changed", TYPE_WAVEFORM_CONTEXT, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	agl = agl_get_instance();
}


static void
wf_context_instance_init(WaveformCanvas* self)
{
}


#ifdef USE_FRAME_CLOCK
static void wf_canvas_on_paint_update(GdkFrameClock* clock, void* _canvas)
{
	WaveformCanvas* wfc = _canvas;

	if(wfc->root->draw) wfc->root->draw(wfc->root, wfc->root->user_data);
	wfc->priv->_last_redraw_time = wf_get_time();
}
#endif


static void
wf_canvas_init(WaveformCanvas* wfc, AGlRootActor* root)
{
	wfc->priv = g_new0(WfContextPriv, 1);

	wfc->enable_animations = true;
	wfc->blend = true;
	wfc->sample_rate = 44100;
	wfc->v_gain = 1.0;
	wfc->texture_unit[0] = agl_texture_unit_new(WF_TEXTURE0);
	wfc->texture_unit[1] = agl_texture_unit_new(WF_TEXTURE1);
	wfc->texture_unit[2] = agl_texture_unit_new(WF_TEXTURE2);
	wfc->texture_unit[3] = agl_texture_unit_new(WF_TEXTURE3);

	wfc->samples_per_pixel = wfc->sample_rate / 32.0; // 32 pixels for 1 second
	wfc->zoom = 1.0;

	wfc->priv->zoom = (WfAnimatable){
		.model_val.f = &wfc->zoom,
		.start_val.f = wfc->zoom,
		.val.f       = wfc->zoom,
		.type        = WF_FLOAT
	};

	wfc->shaders.ruler = &ruler;

	if(wfc->root){

		bool wf_canvas_try_drawable(gpointer _wfc)
		{
			WaveformCanvas* wfc = _wfc;

			if((wfc->root->type == CONTEXT_TYPE_GTK) && !wfc->root->gl.gdk.drawable){
				return G_SOURCE_CONTINUE;
			}

			wf_canvas_init_gl(wfc);

			if(wfc->root->draw) wf_canvas_queue_redraw(wfc);
			wfc->use_1d_textures = agl->use_shaders;

#ifdef USE_FRAME_CLOCK
			frame_clock_connect(G_CALLBACK(wf_canvas_on_paint_update), wfc);
#endif
			return G_SOURCE_REMOVE;
		}
		if(wf_canvas_try_drawable(wfc)) wfc->priv->pending_init = g_idle_add(wf_canvas_try_drawable, wfc);
	}
}


WaveformCanvas*
waveform_canvas_construct(GType object_type)
{
	WaveformCanvas* wfc = (WaveformCanvas*)g_object_new(object_type, NULL);
	return wfc;
}


WaveformCanvas*
wf_canvas_new(AGlRootActor* root)
{
	PF;

	WaveformCanvas* wfc = waveform_canvas_construct(TYPE_WAVEFORM_CONTEXT);
	wfc->root = root;
	wf_canvas_init(wfc, root);
	return wfc;
}


#ifdef USE_SDL
WaveformCanvas*
wf_canvas_new_sdl(SDL_GLContext* context)
{
	PF;

	WaveformCanvas* wfc = waveform_canvas_construct(TYPE_WAVEFORM_CONTEXT);

	wfc->show_rms = true;

	AGlRootActor* a = wfc->root = (AGlScene*)agl_actor__new_root_(CONTEXT_TYPE_SDL);
	wfc->root->gl.sdl.context = context;

	wf_canvas_init(wfc, a);

	return wfc;
}
#endif


static void
wf_canvas_finalize(GObject* obj)
{
	WaveformCanvas* wfc = WAVEFORM_CONTEXT (obj);

	g_free(wfc->priv);

	G_OBJECT_CLASS (waveform_context_parent_class)->finalize (obj);
}


void
wf_canvas_free (WaveformCanvas* wfc)
{
	g_return_if_fail(wfc);
	PF;
	WfContextPriv* c = wfc->priv;

#ifdef USE_FRAME_CLOCK
	frame_clock_disconnect(G_CALLBACK(wf_canvas_on_paint_update), wfc);
#endif

	if(c->pending_init){ g_source_remove(c->pending_init); c->pending_init = 0; }
	if(c->_queued){ g_source_remove(c->_queued); c->_queued = false; }
	wf_canvas_finalize((GObject*)wfc);

	pango_gl_render_clear_caches();
}


static void
wf_canvas_init_gl(WaveformCanvas* wfc)
{
	PF;
	if(agl->shaders.plain->shader.program){
#ifdef DEBUG
		gwarn("already done");
#endif
		return;
	}

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

		agl_gl_init();

		if(!agl->use_shaders){
			agl_use_program(NULL);
			wfc->use_1d_textures = false;
		}

	} WAVEFORM_END_DRAW(wfc);
}


void
wf_canvas_set_viewport(WaveformCanvas* wfc, WfViewPort* _viewport)
{
	//@param viewport - optional. Used to optimise drawing where some of the rect lies outside the viewport.
	//                  Does not apply clipping.
	//                  units are not pixels, they are gl units.
	//                  setting viewport->left allows scrolling.
	//
	//                  TODO clarify: can only be omitted if canvas displays only one region?
	//                                ... no, not true. dont forget, the display is not set here, we only store the viewport property.

	g_return_if_fail(wfc);

#if 0
	if(_viewport){
		if(!wfc->viewport) wfc->viewport = g_new(WfViewPort, 1);
		*wfc->viewport = *_viewport;
		dbg(1, "x: %.2f --> %.2f", wfc->viewport->left, wfc->viewport->right);
	}else{
		dbg(1, "viewport=NULL");
		if(wfc->viewport) g_free0(wfc->viewport);
	}
#else
	if(_viewport){
		wfc->root->viewport = (AGlRect){
			.x = _viewport->left,
			.y = _viewport->top,
			.h = _viewport->right - _viewport->left,
			.w = _viewport->bottom - _viewport->top,
		};
	}
#endif

#if 0
	if(wfc->draw) g_signal_emit_by_name(wfc, "dimensions-changed");
#else
	if(wfc->root->draw) g_signal_emit_by_name(wfc, "dimensions-changed");
#endif
}


/*
 *  The actor is owned by the canvas and will be freed on calling wf_canvas_remove_actor()
 */
WaveformActor*
wf_canvas_add_new_actor(WaveformCanvas* wfc, Waveform* w)
{
	g_return_val_if_fail(wfc, NULL);

	if(w) g_object_ref(w);

	WaveformActor* a = wf_actor_new(w, wfc);
#ifdef TRACK_ACTORS
	actors = g_list_append(actors, a);
#endif
	return a;
}


void
wf_canvas_remove_actor(WaveformCanvas* wfc, WaveformActor* actor)
{
	g_return_if_fail(actor);
	PF;
	Waveform* w = actor->waveform;

	wf_actor_free(actor);
#ifdef TRACK_ACTORS
	if(!g_list_find(actors, actor)) gwarn("actor not found! %p", actor);
	actors = g_list_remove(actors, actor);
	if(actors) dbg(1, "n_actors=%i", g_list_length(actors));
#endif

	if(w) g_object_unref(w);
}



void
wf_canvas_queue_redraw(WaveformCanvas* wfc)
{
#ifdef USE_FRAME_CLOCK
	if(wfc->root->is_animating){
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

	gboolean wf_canvas_redraw(gpointer _canvas)
	{
		WaveformCanvas* wfc = _canvas;
		if(wfc->root->draw) wfc->root->draw(wfc->root, wfc->root->user_data);
		wfc->priv->_queued = false;
		return G_SOURCE_REMOVE;
	}
	wfc->priv->_queued = g_timeout_add(CLAMP(WF_FRAME_INTERVAL - (wf_get_time() - wfc->_last_redraw_time), 1, WF_FRAME_INTERVAL), wf_canvas_redraw, wfc);
#endif
}


float
wf_canvas_gl_to_px(WaveformCanvas* wfc, float x)
{
	//convert from gl coords to screen pixels

	#warning _gl_to_px TODO where viewport not set.
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
wf_canvas_load_texture_from_alphabuf(WaveformCanvas* wfc, int texture_name, AlphaBuf* alphabuf)
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
#ifdef HAVE_NON_SQUARE_TEXTURES
		int width  = MAX(1, a->width  / r);
#else
		int width = height;
#endif
		guchar* buf = g_malloc(width * height);
		int y; for(y=0;y<height;y++){
			int x; for(x=0;x<width;x++){
				//TODO find max of all peaks, dont just use one.
				buf[width * y + x] = a->buf[a->width * y * r + x * r];
			}
		}
		//dbg(0, "r=%i size=%ix%i", r, width, height);
		return buf;
	}
#endif

	WAVEFORM_START_DRAW(wfc) {
		//note: gluBuild2DMipmaps is deprecated. instead use GL_GENERATE_MIPMAP (requires GL 1.4)

		glBindTexture(GL_TEXTURE_2D, texture_name);
#ifdef HAVE_NON_SQUARE_TEXTURES
		int width = alphabuf->width;
#else
		int width = alphabuf->height;
#endif
		dbg (2, "copying texture... width=%i texture_id=%u", width, texture_name);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8, width, alphabuf->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, alphabuf->buf);

#ifdef USE_MIPMAPPING
		{
			int l; for(l=1;l<16;l++){
				guchar* buf = generate_mipmap(alphabuf, l);
#ifdef HAVE_NON_SQUARE_TEXTURES
				int width = alphabuf->width / (1<<l);
#else
				int width = alphabuf->height / (1<<l);
#endif
				glTexImage2D(GL_TEXTURE_2D, l, GL_ALPHA8, width, alphabuf->height/(1<<l), 0, GL_ALPHA, GL_UNSIGNED_BYTE, buf);
				g_free(buf);
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
wf_canvas_set_share_list(WaveformCanvas* wfc)
{
	dbg(0, "TODO");
}


void
wf_canvas_set_rotation(WaveformCanvas* wfc, float rotation)
{
	dbg(0, "TODO");
}


/*
 *  Allows the whole scene to be scaled as a single transition
 *  which is more efficient than scaling many individual waveforms.
 *
 *  Calling this function puts the canvas into 'scaled' mode.
 */
#ifdef USE_CANVAS_SCALING
void
wf_canvas_set_zoom(WaveformCanvas* wfc, float zoom)
{
	// TODO should probably call agl_actor__start_transition

	wfc->priv->scaled = true;
	dbg(0, "zoom=%f", zoom);

#ifdef TRACK_ACTORS
	if(!actors) return;

	void set_zoom_on_animation_finished(WfAnimation* animation, gpointer _wfc)
	{
		WaveformCanvas* wfc = _wfc;
		dbg(0, "wfc=%p", wfc);
	}

	void wf_canvas_set_zoom_on_frame(WfAnimation* animation, int time)
	{
		WaveformCanvas* wfc = animation->user_data;

#if 0 // invalidate only the waveform actors
		GList* l = animation->members;
		for(;l;l=l->next){
			WfAnimActor* member = l->data;
			GList* k = member->transitions;
			for(;k;k=k->next){
				WfAnimatable* animatable = k->data;
#ifdef TRACK_ACTORS
				GList* a = actors;
				for(;a;a=a->next){
					WaveformActor* actor = a->data;
					agl_actor__invalidate((AGlActor*)actor); // TODO can probably just invalidate the whole scene?
				}
#endif
			}
		}
#else
		agl_actor__invalidate((AGlActor*)wfc->root); // strictly speaking some non-scalable items should not be invalidated
#endif
	}

	wfc->zoom = zoom; // TODO clamp

	WfAnimation* animation = wf_animation_new(set_zoom_on_animation_finished, wfc);
	animation->on_frame = wf_canvas_set_zoom_on_frame;

	GList* animatables = g_list_prepend(NULL, &wfc->priv->zoom);
	wf_transition_add_member(animation, animatables);

	wf_animation_start(animation);
#endif
}
#endif


void
wf_canvas_set_gain(WaveformCanvas* wfc, float gain)
{
	wfc->v_gain = gain;
	wf_canvas_queue_redraw(wfc);
}


