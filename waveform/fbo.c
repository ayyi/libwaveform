/*
  copyright (C) 2013-2015 Tim Orford <tim@orford.org>

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
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "agl/ext.h"
#include "waveform/utils.h"
#include "waveform/gl_utils.h"
#include "waveform/peak.h"
#include "waveform/texture_cache.h"
#include "waveform/canvas.h"
#include "waveform/actor.h"
#include "waveform/fbo.h"

extern BloomShader vertical;

#ifdef USE_FBO


AGlFBO*
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

	GLuint _wf_create_background_rgba()
	{
		//create an alpha-map gradient texture for use as background

		glEnable(GL_TEXTURE_2D);

		int width = 256;
		int height = 256;
		int rowstride = 256 * 4;
		char* pbuf = g_new0(char, width * height * 4);
		int y; for(y=0;y<height;y++){
			int x; for(x=0;x<width;x++){
				*(pbuf + y * rowstride + 4 * x    ) = ((x+y) * 0xff) / (width * 2);
				*(pbuf + y * rowstride + 4 * x + 1) = ((x+y) * 0xff) / (width * 2);
				*(pbuf + y * rowstride + 4 * x + 2) = ((x+y) * 0xff) / (width * 2);
				*(pbuf + y * rowstride + 4 * x + 3) = ((x+y) * 0xff) / (width * 2);

				if(x == 64 || x == 65 || x == 66 || x == 67){
					*(pbuf + y * rowstride + 4 * x + 1) = 0;
					*(pbuf + y * rowstride + 4 * x + 2) = 0;
					*(pbuf + y * rowstride + 4 * x + 3) = 0;
				}
			}
		}

		GLuint bg_textures;
		glGenTextures(1, &bg_textures);
		if(glGetError() != GL_NO_ERROR){ gerr ("couldnt create bg_texture."); return 0; }
		dbg(2, "bg_texture=%i", bg_textures);

		int pixel_format = GL_RGBA;
		glBindTexture  (GL_TEXTURE_2D, bg_textures);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);
		if(glGetError() != GL_NO_ERROR) gwarn("gl error binding bg texture!");

		g_free(pbuf);

		return bg_textures;
	}
	//AGlFBO* fbo = agl_fbo_new(256, 256, _wf_create_background(), 0);
	AGlFBO* fbo = agl_fbo_new(256, 256, _wf_create_background_rgba(), 0);
	return fbo;
}


#if 0
static AGlFBO* fbo0 = NULL;

void
fbo_print(WaveformActor* actor, int x, int y, double scale, uint32_t colour, int alpha)
{
	Waveform* w = actor->waveform;
	WfGlBlock* textures = w->textures;
	unsigned texture = textures->peak_texture[WF_LEFT].main[0];

					glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
	agl_enable(AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);

	if(!fbo0) fbo0 = agl_fbo_new(256, 256, 0, 0);

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
			agl_enable(AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);
		} end_draw_to_fbo;
	}

	void draw_waveform_to_fbo(Waveform* w)
	{
		draw_to_fbo(fbo0) {
			glClearColor(0.0, 0.0, 0.0, 1.0); //background colour must be same as foreground for correct antialiasing
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			agl_enable(AGL_ENABLE_TEXTURE_2D);
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
			agl_use_program(actor->canvas, &vertical.shader);
		}
		glColor4f(1.0, 1.0, 1.0, alpha / 256.0);
		agl_enable(AGL_ENABLE_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, fbo0->texture);
		if(!glIsTexture(fbo0->texture)) gwarn("not texture");
//TODO why have to disable blending?
glDisable(GL_BLEND);
//		agl_enable(AGL_ENABLE_BLEND | AGL_ENABLE_TEXTURE_2D);
//glBindTexture(GL_TEXTURE_2D, 1);
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

		agl_use_program_(actor->canvas, NULL);
	}
	gl_warn("gl error");
}
#endif
#endif //use_fbo

