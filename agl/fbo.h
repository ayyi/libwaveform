#ifndef __agl_fbo_h__
#define __agl_fbo_h__

typedef struct _fbo AglFBO;

struct _fbo {
	guint id;
	guint texture;
	int   width;      // the size that is being used. the texture may be bigger.
	int   height;
};

AglFBO* agl_fbo_new      (int width, int height, guint texture);
void    agl_fbo_free     (AglFBO*);
void    agl_fbo_set_size (AglFBO*, int width, int height);

#define agl_draw_to_fbo(F) \
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
#define agl_end_draw_to_fbo \
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); \
	glPopAttrib(); /*restore viewport */ \
	glMatrixMode(GL_PROJECTION); \
	glPopMatrix(); \
	glMatrixMode(GL_MODELVIEW); \
	glPopMatrix();

#endif
