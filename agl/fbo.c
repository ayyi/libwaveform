/*
  copyright (C) 2012-2013 Tim Orford <tim@orford.org>

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
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <glib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include "agl/utils.h"
#include "agl/ext.h"
#include "agl/fbo.h"

//TODO, perhaps just remove custom debugging messages...
extern void wf_debug_printf (const char* func, int level, const char* format, ...);
#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define dbg(A, B, ...) wf_debug_printf(__func__, A, B, ##__VA_ARGS__)

static GLuint make_fbo(GLuint texture);


AglFBO*
agl_fbo_new(int width, int height, GLuint texture)
{
	//if texture is zero, a new texture will be created.

	GLuint make_texture(guint size)
	{
					glEnable(GL_TEXTURE_2D);
					glActiveTexture(GL_TEXTURE0);
					glEnable(GL_TEXTURE_2D);
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, TextureLevel);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, TextureLevel);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		return texture;
	}

	AglFBO* fbo = g_new0(AglFBO, 1);
	fbo->width = width;
	fbo->height = height;
	fbo->texture = texture ? texture : make_texture(agl_power_of_two(MAX(width, height)));
	fbo->id = make_fbo(fbo->texture);
	dbg(2, "fb=%i texture=%i", fbo->id, fbo->texture);

	/*
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo->id);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) gwarn("framebuffer incomplete");

	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, fbo->width, fbo->height);

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

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); // rebind the normal framebuffer
	*/

	gl_warn("fbo_new");
	return fbo;
}


void
agl_fbo_free(AglFBO* fbo)
{
	g_return_if_fail(fbo);

	glDeleteTextures(1, &fbo->texture);
	glDeleteFramebuffers(1, &fbo->id);
	g_free(fbo);
}


static GLuint
make_fbo(GLuint texture)
{

					//glActiveTexture(GL_TEXTURE0);
					//glEnable(GL_TEXTURE_2D);
	dbg(2, "generating framebuffer...");
	g_return_val_if_fail(glGenFramebuffers, 0);
	GLuint fb;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fb);

	dbg(2, "attaching texture %u to fbo...", texture);
	int texture_level = 0;
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture, texture_level);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) gwarn("framebuffer incomplete: 0x%04x", status);

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); // rebind the normal framebuffer

	return fb;
}


