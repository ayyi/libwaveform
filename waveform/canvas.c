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

  ---------------------------------------------------------------

  WaveformActor draws a Waveform object onto a shared opengl drawable.

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
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/texture_cache.h"
#include "waveform/shaderutil.h"
#include "waveform/gl_utils.h"
#include "waveform/actor.h"
#include "waveform/canvas.h"
#include "waveform/alphabuf.h"
#include "waveform/shader.h"
#include "waveform/gl_ext.h"

#define WAVEFORM_START_DRAW(wa) \
	if(wa->_draw_depth) gwarn("START_DRAW: already drawing"); \
	wa->_draw_depth++; \
	if ((wa->_draw_depth > 1) || gdk_gl_drawable_gl_begin (wa->gl_drawable, wa->gl_context)) {
#define WAVEFORM_END_DRAW(wa) \
	wa->_draw_depth--; \
	if(!wa->_draw_depth) gdk_gl_drawable_gl_end(wa->gl_drawable); \
	} else gwarn("!! gl_begin fail")
#define WAVEFORM_IS_DRAWING(wa) \
	(wa->_draw_depth > 0)

static void   wf_canvas_init_gl (WaveformCanvas*);

extern PeakShader peak_shader;
extern HiResShader hires_shader;
extern BloomShader horizontal, vertical;
extern AlphaMapShader tex2d, tex2d_b;
extern RulerShader ruler;

#define TRACK_ACTORS // for debugging only.
#ifdef TRACK_ACTORS
GList* actors = NULL;
#endif


static void
wf_canvas_init(WaveformCanvas* wfc)
{
	wfc->priv = g_new0(WfCanvasPriv, 1);

	wfc->use_shaders = wf_get_instance()->pref_use_shaders;
	wfc->sample_rate = 44100;
	wfc->v_gain = 1.0;
	wfc->texture_unit[0] = texture_unit_new(WF_TEXTURE0);
	wfc->texture_unit[1] = texture_unit_new(WF_TEXTURE1);
	wfc->texture_unit[2] = texture_unit_new(WF_TEXTURE2);
	wfc->texture_unit[3] = texture_unit_new(WF_TEXTURE3);
	wfc->use_1d_textures = wfc->use_shaders;
	if(wfc->use_shaders) wf_canvas_init_gl(wfc);
	//memset(&wfc->peak_shader, 0, sizeof(PeakShader));
}


WaveformCanvas*
wf_canvas_new(GdkGLContext* gl_context, GdkGLDrawable* gl_drawable)
{
	PF;

	if(!glAttachShader){
		wf_actor_init();
		get_gl_extensions();
	}

	WaveformCanvas* wfc = g_new0(WaveformCanvas, 1);
	wfc->show_rms = true;
	wfc->gl_context = gl_context;
	wfc->gl_drawable = gl_drawable;
	wf_canvas_init(wfc);
#ifdef WF_USE_TEXTURE_CACHE
#if 0 // dont want to generate textures if this is not the first canvas. textures will be generated on demand anyway
	texture_cache_gen();
#endif
#endif
	return wfc;
}


WaveformCanvas*
wf_canvas_new_from_widget(GtkWidget* widget)
{
	GdkGLDrawable* gl_drawable = gtk_widget_get_gl_drawable(widget);
	if(!gl_drawable){
		dbg(2, "cannot get drawable");
		return NULL;
	}

	WaveformCanvas* wfc = g_new0(WaveformCanvas, 1);
	wfc->gl_drawable = gl_drawable; 
	dbg(2, "got drawable");
	wfc->gl_context = gtk_widget_get_gl_context(widget);
	//int t; for(t=0;t<2;t++) wfc->texture_unit[t] = texture_unit_new(WF_TEXTURE0);
	wf_canvas_init(wfc);
	return wfc;
}


void
wf_canvas_free (WaveformCanvas* wfc)
{
	PF;
	if(wfc->_queued){ g_source_remove(wfc->_queued); wfc->_queued = false; }
	//if(wfc->priv->peak_shader) g_free(wfc->priv->peak_shader);
	g_free(wfc->priv);
	g_free(wfc);
}


/*
 *  Currently, if an application has multiple canvases and they share Waveforms,
 *  each canvas must have the same use_shaders setting.
 */
void
wf_canvas_set_use_shaders(WaveformCanvas* wfc, gboolean val)
{
	PF;
	wf_get_instance()->pref_use_shaders = val;

	if(wfc){
		if(!val){
			wf_canvas_use_program_(wfc, NULL);
			wfc->use_1d_textures = false;
		}
		wfc->use_shaders = val;
	}
}


static void
wf_canvas_init_gl(WaveformCanvas* wfc)
{
	if(!glAttachShader) wf_actor_init();

	WfCanvasPriv* priv = wfc->priv;

	if(priv->shaders.peak){ gwarn("already done"); return; }

	WAVEFORM_START_DRAW(wfc) {

		if(wf_get_instance()->pref_use_shaders && !shaders_supported()){
			printf("gl shaders not supported. expect reduced functionality.\n");
			wf_canvas_use_program_(wfc, NULL);
			wfc->use_shaders = false;
			wfc->use_1d_textures = false;
		}
		printf("GL_RENDERER = %s\n", (const char*)glGetString(GL_RENDERER));

		if(wfc->use_shaders){
			wf_shaders_init();
			wfc->priv->shaders.peak = &peak_shader;
			wfc->priv->shaders.hires = &hires_shader;
			wfc->priv->shaders.vertical = &vertical;
			wfc->priv->shaders.horizontal = &horizontal;
			wfc->priv->shaders.tex2d = &tex2d;
			wfc->priv->shaders.tex2d_b = &tex2d_b;
			wfc->priv->shaders.ruler = &ruler;
		}

	} WAVEFORM_END_DRAW(wfc);
}


void
wf_canvas_set_viewport(WaveformCanvas* wfc, WfViewPort* _viewport)
{
	//@param viewport - optional. Used to optimise drawing where some of the rect lies outside the viewport. Does not apply clipping.
	//                  *** new policy *** units are not pixels, they are gl units.
	//                  TODO clarify: can only be omitted if canvas displays only one region?
	//                                ... no, not true. dont forget, the display is not set here, we only store the viewport property.

	g_return_if_fail(wfc);

	if(_viewport){
		if(!wfc->viewport) wfc->viewport = g_new(WfViewPort, 1);
		memcpy(wfc->viewport, _viewport, sizeof(WfViewPort));
		dbg(1, "x: %.2f --> %.2f", wfc->viewport->left, wfc->viewport->right);
	}else{
		dbg(1, "viewport=NULL");
		if(wfc->viewport){ g_free(wfc->viewport); wfc->viewport = NULL; }
	}
}


WaveformActor*
wf_canvas_add_new_actor(WaveformCanvas* wfc, Waveform* w)
{
	g_return_val_if_fail(wfc, NULL);

	g_object_ref(w);

	WaveformActor* a = wf_actor_new(w);
	a->canvas = wfc;
#ifdef TRACK_ACTORS
	actors = g_list_append(actors, a);
#endif
	return a;
}


void
wf_canvas_remove_actor(WaveformCanvas* wfc, WaveformActor* actor)
{
	PF;
	Waveform* w = actor->waveform;

	wf_actor_free(actor);
#ifdef TRACK_ACTORS
	if(!g_list_find(actors, actor)) gwarn("actor not found! %p", actor);
	actors = g_list_remove(actors, actor);
	if(actors) dbg(0, "n_actors=%i", g_list_length(actors));
#endif

	if(w) g_object_unref(w);
}


void
wf_canvas_queue_redraw(WaveformCanvas* wfc)
{
	if(wfc->_queued) return;

	gboolean wf_canvas_redraw(gpointer _canvas)
	{
		WaveformCanvas* wfc = _canvas;
		if(wfc->draw) wfc->draw(wfc, wfc->draw_data);
		wfc->_queued = false;
		return IDLE_STOP;
	}

	wfc->_queued = g_idle_add(wf_canvas_redraw, wfc);
}


float
wf_canvas_gl_to_px(WaveformCanvas* wfc, float x)
{
	//convert from gl coords to screen pixels

	#warning _gl_to_px TODO where viewport not set.
	if(!wfc->viewport) return x;

	//TODO move to resize handler?
	gint drawable_width_px, height;
	gdk_gl_drawable_get_size(gdk_gl_context_get_gl_drawable(gdk_gl_context_get_current()), &drawable_width_px, &height);

	float viewport_width = 256; //TODO
	if(wfc->viewport) viewport_width = wfc->viewport->right - wfc->viewport->left;

	float scale = drawable_width_px / viewport_width;
	return x * scale;
}


void
wf_canvas_load_texture_from_alphabuf(WaveformCanvas* wa, int texture_name, AlphaBuf* alphabuf)
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

	WAVEFORM_START_DRAW(wa) {
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //note using this stops gaps between blocks.
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); //prevent wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //prevent wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); //prevent wrapping

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		if(!glIsTexture(texture_name)) gwarn("texture not loaded! %i", texture_name);
	} WAVEFORM_END_DRAW(wa);
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


void
wf_canvas_use_program(WaveformCanvas* wfc, int program)
{
	//deprecated. use fn below.

	if(wfc->use_shaders && (program != wfc->_program)){
		dbg(2, "%i", program);
		glUseProgram(wfc->_program = program);
	}
}


void
wf_canvas_use_program_(WaveformCanvas* wfc, WfShader* shader)
{
	int program = shader ? shader->program : 0;

	if(wfc->use_shaders && (program != wfc->_program)){
		dbg(2, "%i", program);
		glUseProgram(wfc->_program = program);

		//TODO do for all shaders
		if(shader == (WfShader*)&peak_shader){
			//peak_shader.set_uniforms(peaks_per_pixel, rect.top, bottom, actor->fg_colour, n_channels);
		}else{
			if(shader) shader->set_uniforms_();
		}
	}
}


