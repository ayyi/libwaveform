/**
* +----------------------------------------------------------------------+
* | copyright (C) 2013-2015 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __agl_fbo_h__
#define __agl_fbo_h__

typedef enum {
	AGL_FBO_HAS_STENCIL = 1,
} AGlFBOFlags;

struct _AGlFBO {
	guint       id;
	guint       texture;
	int         width;      // the size that is being used. the texture may be bigger.
	int         height;
	AGlFBOFlags flags;
};

AGlFBO* agl_fbo_new      (int width, int height, guint texture, AGlFBOFlags);
void    agl_fbo_free     (AGlFBO*);
void    agl_fbo_set_size (AGlFBO*, int width, int height);

#define agl_fbo_free0(var) (var = (agl_fbo_free(var), NULL))

#define agl_draw_to_fbo(F) \
	glMatrixMode(GL_PROJECTION); \
	glPushMatrix(); \
	glLoadIdentity(); \
	glOrtho (0, F->width, F->height, 0, 10.0, -100.0); \
	\
	glMatrixMode(GL_MODELVIEW); \
	glPushMatrix(); \
	glLoadIdentity(); \
	\
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, F->id); \
	glPushAttrib(GL_VIEWPORT_BIT); \
	glViewport(0, 0, F->width, F->height);

#define agl_end_draw_to_fbo \
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); \
	glPopAttrib(); /*restore viewport */ \
	\
	glMatrixMode(GL_PROJECTION); \
	glPopMatrix(); \
	\
	glMatrixMode(GL_MODELVIEW); \
	glPopMatrix();

#define AGL_MAX_FBO_WIDTH 2048 // TODO

#endif

