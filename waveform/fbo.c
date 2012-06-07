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
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "waveform/utils.h"
#include "waveform/gl_utils.h"
#include "waveform/peak.h"
#include "waveform/texture_cache.h"
#include "waveform/shaderutil.h"
#include "waveform/gl_ext.h"
#include "waveform/canvas.h"
#include "waveform/actor.h"
#include "waveform/fbo.h"

extern BloomShader horizontal, vertical;

#ifdef USE_FBO

WfFBO* fbo0 = NULL;

static GLuint make_fbo(GLuint texture);

WfFBO*
fbo_new(GLuint texture)
{
	//if texture is zero, a new texture will be created.

	GLuint make_texture(int size)
	{
					glEnable(GL_TEXTURE_2D);
					glActiveTexture(GL_TEXTURE0);
					glEnable(GL_TEXTURE_2D);
		dbg(2, "creating fbo texture...");

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB, fbo0->width/2, fbo0->height/2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, TextureLevel);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, TextureLevel);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		return texture;
	}

	WfFBO* fbo = g_new0(WfFBO, 1);
	fbo->width = 256;
	fbo->height = 256;
	fbo->texture = texture ? texture : make_texture(fbo->width);
	fbo->id = make_fbo(fbo->texture);
	dbg(1, "fb=%i texture=%i", fbo->id, fbo->texture);

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo->id);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) gwarn("framebuffer incomplete");

	/*
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, fbo0->width, fbo0->height);

	//draw something into the fbo
	glClearColor(0.0, 1.0, 0.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glColor3f(0, 1, 0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	float s = 256.0;
	glBegin(GL_POLYGON);
	glVertex2f(-s,  0.0);
	glVertex2f( 0.0, -s);
	glVertex2f( s,  0.0);
	glVertex2f( 0.0,  s);
	glEnd();

	glPopAttrib(); //restore viewport
	*/

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); // rebind the normal framebuffer

	gl_warn("fbo_new");
	return fbo;
}


void
fbo_free(WfFBO* fbo)
{
	glDeleteTextures(1, &fbo->texture);
	fbo->texture = 0;
}


static GLuint
make_fbo(GLuint texture)
{

					glActiveTexture(GL_TEXTURE0);
					glEnable(GL_TEXTURE_2D);
	dbg(2, "generating framebuffer...");
	g_return_val_if_fail(glGenFramebuffers, 0);
	GLuint fb;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fb);

	dbg(2, "attaching texture %u to fbo...", texture);
	int texture_level = 0;
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture, texture_level);

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); // rebind the normal framebuffer

	return fb;
}


WfFBO*
fbo_new_test()
{
	GLuint _wf_create_background()
	{
		//create an alpha-map gradient texture for use as background

		glEnable(GL_TEXTURE_2D);

		int width = 256;
		int height = 256;
		char* pbuf = g_new0(char, width * height);
		int y; for(y=0;y<height;y++){
			int x; for(x=0;x<width;x++){
				*(pbuf + y * width + x) = ((x+y) * 0xff) / (width * 2);
			}
		}

		GLuint bg_textures;
		glGenTextures(1, &bg_textures);
		if(glGetError() != GL_NO_ERROR){ gerr ("couldnt create bg_texture."); return 0; }
		dbg(2, "bg_texture=%i", bg_textures);

		int pixel_format = GL_ALPHA;
		glBindTexture  (GL_TEXTURE_2D, bg_textures);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);
		if(glGetError() != GL_NO_ERROR) gwarn("gl error binding bg texture!");

		g_free(pbuf);

		return bg_textures;
	}
	WfFBO* fbo = fbo_new(_wf_create_background());
	return fbo;
}


void fbo_print(WaveformActor* actor, int x, int y, double scale, uint32_t colour, int alpha)
{
	Waveform* w = actor->waveform;
	WfGlBlock* textures = w->textures;
	unsigned texture = textures->peak_texture[WF_LEFT].main[0];

					glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(!fbo0) fbo0 = fbo_new(0);

	void draw_test_to_fbo()
	{
		draw_to_fbo(fbo0) {
			//ColourFloat fc; colour_rgba_to_float(&fc, colour);
	//		glBindTexture(GL_TEXTURE_2D, fbo0->texture);

			//glClearColor(fc.r, fc.g, fc.b, 0.0); //background colour must be same as foreground for correct antialiasing
			glClearColor(0.0, 0.0, 0.0, 1.0); //background colour must be same as foreground for correct antialiasing
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			//glClear(GL_COLOR_BUFFER_BIT);
			//TODO move the rgba_to_argb conversion into gl_pango_print.
			//gl_pango_print(0, 0, 0, colour_rgba_to_argb(colour), text);

			glDisable(GL_TEXTURE_2D);
			glColor4f(1.0, 1.0, 0.0, 1.0);
			glDisable(GL_BLEND);
			glColor4f(1.0, 0.0, 0.0, 1.0);
	#if 0
			glBegin(GL_POLYGON);
			float s = 100.0;
			glVertex2f(-s,  0.0);
			glVertex2f( 0.0, -s);
			glVertex2f( s,  0.0);
			glVertex2f( 0.0,  s);
			glEnd();
	#endif
				glBegin(GL_POLYGON);
				glVertex2f( 50.0,  50.0);
				glVertex2f(100.0,  50.0);
				glVertex2f(100.0, 100.0);
				glVertex2f( 50.0, 100.0);
				glEnd();
			glEnable(GL_TEXTURE_2D);
					glEnable(GL_BLEND);
		} end_draw_to_fbo;
	}

	void draw_waveform_to_fbo(Waveform* w)
	{
		draw_to_fbo(fbo0) {
			glClearColor(0.0, 0.0, 0.0, 1.0); //background colour must be same as foreground for correct antialiasing
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture);
			double top = 0;
			double bot = fbo0->height;
			double x1 = 0;
			double x2 = fbo0->width;
			glBegin(GL_QUADS);
			glTexCoord2d(1.0, 1.0); glVertex2d(x2, top);
			glTexCoord2d(0.0, 1.0); glVertex2d(x1, top);
			glTexCoord2d(0.0, 0.0); glVertex2d(x1, bot);
			glTexCoord2d(1.0, 0.0); glVertex2d(x2, bot);
			glEnd();
		} end_draw_to_fbo;
	}
	draw_waveform_to_fbo(w); //TODO dont do on every expose

	//put the fbo onto the screen
	{
		if(true){
			wf_canvas_use_program(actor->canvas, vertical.shader.program);
		}
		glColor4f(1.0, 1.0, 1.0, alpha / 256.0);
						glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, fbo0->texture);
		if(!glIsTexture(fbo0->texture)) gwarn("not texture");
//TODO why have to disable blending?
glDisable(GL_BLEND);
//glBindTexture(GL_TEXTURE_2D, 1);
//		glEnable(GL_BLEND);
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double top = y;
		double bot = y + fbo0->height * scale;
		double x1 = x;
		double x2 = x + fbo0->width * scale;
		glBegin(GL_QUADS);
		glTexCoord2d(1.0, 1.0); glVertex2d(x2, top);
		glTexCoord2d(0.0, 1.0); glVertex2d(x1, top);
		glTexCoord2d(0.0, 0.0); glVertex2d(x1, bot);
		glTexCoord2d(1.0, 0.0); glVertex2d(x2, bot);
		glEnd();
/*
*/
		wf_canvas_use_program(actor->canvas, 0);
	}
	gl_warn("gl error");
}
#endif //use_fbo

