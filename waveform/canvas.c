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
//#include "gl_utils.h" //FIXME
extern void use_texture(int texture);
#include "waveform/actor.h"
#include "waveform/canvas.h"
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

extern Shader sh_main;
static UniformInfo uniforms[] = {
   {"tex1d",     1, GL_INT,   { 0, 0, 0, 0 }, -1}, // LHS +ve - 0 corresponds to glActiveTexture(GL_TEXTURE0);
   {"tex1d_neg", 1, GL_INT,   { 1, 0, 0, 0 }, -1}, // LHS -ve - 1 corresponds to glActiveTexture(GL_TEXTURE1);
   {"tex1d_3",   1, GL_INT,   { 2, 0, 0, 0 }, -1}, // RHS +ve GL_TEXTURE2
   {"tex1d_4",   1, GL_INT,   { 3, 0, 0, 0 }, -1}, // RHS -ve GL_TEXTURE3
   END_OF_UNIFORMS
};

static GLuint create_program         (WaveformCanvas*, Shader*, UniformInfo*);
static void   wf_canvas_init_shaders (WaveformCanvas*);


WaveformCanvas*
wf_canvas_new(GdkGLContext* gl_context, GdkGLDrawable* gl_drawable)
{
	PF0;

	if(!glAttachShader){
		wf_actor_init();
		get_gl_extensions();
	}

	WaveformCanvas* wfc = g_new0(WaveformCanvas, 1);
	wfc->show_rms = true;
	wfc->use_shaders = wf_get_instance()->pref_use_shaders;
	wfc->gl_context = gl_context;
	wfc->gl_drawable = gl_drawable;
	wfc->sample_rate = 44100;
	wfc->v_gain = 1.0;
#ifdef WF_USE_TEXTURE_CACHE
#if 0 // dont want to generate textures if this is not the first canvas. textures will be generated on demand anyway
	texture_cache_gen();
#endif
#endif
	if(wfc->use_shaders) wf_canvas_init_shaders(wfc);
	return wfc;
}


WaveformCanvas*
wf_canvas_new_from_widget(GtkWidget* widget)
{
	WaveformCanvas* wfc = g_new0(WaveformCanvas, 1);
	wfc->use_shaders = wf_get_instance()->pref_use_shaders;
	if(!(wfc->gl_drawable = gtk_widget_get_gl_drawable(widget))){
		dbg(2, "cannot get drawable");
		g_free(wfc);
		return NULL;
	}
	dbg(2, "success");
	wfc->gl_context = gtk_widget_get_gl_context(widget);
	wfc->sample_rate = 44100;
	wfc->v_gain = 1.0;
	if(wfc->use_shaders) wf_canvas_init_shaders(wfc);
	return wfc;
}


void
wf_canvas_free (WaveformCanvas* wfc)
{
	PF0;
	g_free(wfc);
}


void
wf_canvas_set_use_shaders(WaveformCanvas* wfc, gboolean val)
{
	PF0;
	wf_get_instance()->pref_use_shaders = val;

	if(wfc){
		if(!val) wf_canvas_use_program(wfc, 0);
		wfc->use_shaders = val;
	}
}


static void
wf_canvas_init_shaders(WaveformCanvas* wfc)
{
	if(!glAttachShader) wf_actor_init();

	if(sh_main.program){ gwarn("already done"); return; }

	WAVEFORM_START_DRAW(wfc) {

		if(wfc->use_shaders){
			create_program(wfc, &sh_main, uniforms);
		}

	} WAVEFORM_END_DRAW(wfc);

#if 0
	int n_peaks = 128;
	float peaks[n_peaks * 2];
	int i; for(i=0;i<n_peaks;i++){
		peaks[2 * i    ] = i;
		peaks[2 * i + 1] = i;
	}
	GLuint offsetLoc = glGetUniformLocation(sh_main.program, "peaks");
	dbg(0, "setting uniform... (peaks) loc=%i", offsetLoc);
	glUniform2fv(offsetLoc, n_peaks, peaks);
#endif
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

	WaveformActor* a = wf_actor_new(w);
	a->canvas = wfc;
	return a;
}


void
wf_canvas_remove_actor(WaveformCanvas* wfc, WaveformActor* actor)
{
	PF0;
	waveform_unref0(actor->waveform);
}


void
wf_canvas_queue_redraw(WaveformCanvas* wfc)
{
	static guint queued = false;
	//if(queued) gwarn("queued");
	if(queued) return;

	gboolean wf_canvas_redraw(gpointer _canvas)
	{
		WaveformCanvas* wfc = _canvas;
		if(wfc->draw) wfc->draw(wfc, wfc->draw_data);
		queued = false;
		return IDLE_STOP;
	}

	queued = g_idle_add(wf_canvas_redraw, wfc);
}


float
wf_canvas_gl_to_px(WaveformCanvas* wfc, float x)
{
	//convert from gl coords to screen pixels

	#warning _gl_to_px TODO where viewport not set.

	//TODO move to resize handler?
	gint drawable_width_px, height;
	gdk_gl_drawable_get_size(gdk_gl_context_get_gl_drawable(gdk_gl_context_get_current()), &drawable_width_px, &height);

	float viewport_width = 256; //TODO
	if(wfc->viewport) viewport_width = wfc->viewport->right - wfc->viewport->left;

	float scale = drawable_width_px / viewport_width;
	return x * scale;
}


void
wf_actor_load_texture_from_alphabuf(WaveformCanvas* wa, int texture_name, AlphaBuf* alphabuf)
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
				int width = alphabuf->width/(1<<l);
#else
				int width = alphabuf->height/(1<<l);
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


static GLuint
create_program(WaveformCanvas* wfc, Shader* sh, UniformInfo* uniforms)
{
	GLuint vert_shader = compile_shader_file(GL_VERTEX_SHADER, sh->vertex_file);
	GLuint frag_shader = compile_shader_file(GL_FRAGMENT_SHADER, sh->fragment_file);

	GLint status;
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE){
		printf("shader compile error! %i\n", status);
	}

	GLuint program = sh->program = link_shaders(vert_shader, frag_shader);
	dbg(2, "%u %u program=%u", vert_shader, frag_shader, program);

	wf_canvas_use_program(wfc, program);

	uniforms_init(program, uniforms);

	sh->uniforms = uniforms;

	return program;
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
	if(wfc->use_shaders && (program != wfc->_program)){
		dbg(2, "%i", program);
		glUseProgram(wfc->_program = program);
	}
}


