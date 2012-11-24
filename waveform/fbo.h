#ifndef __wf_fbo_h__
#define __wf_fbo_h__
#include "agl/fbo.h"

AglFBO* fbo_new      (guint texture);
void    fbo_free     (AglFBO*);
AglFBO* fbo_new_test ();

#define draw_to_fbo(F) \
	glMatrixMode(GL_PROJECTION); \
	glPushMatrix(); \
	glLoadIdentity(); \
	glOrtho (0, F->width, F->height, 0, 10.0, -100.0); \
	glMatrixMode(GL_MODELVIEW); \
	glPushMatrix(); \
	glLoadIdentity(); \
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, F->id); \
	glPushAttrib(GL_VIEWPORT_BIT); \
	glViewport(0, 0, F->width, F->height);
#define end_draw_to_fbo \
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); \
	glPopAttrib(); /*restore viewport */ \
	glMatrixMode(GL_PROJECTION); \
	glPopMatrix(); \
	glMatrixMode(GL_MODELVIEW); \
	glPopMatrix();


#endif //__wf_fbo_h__
