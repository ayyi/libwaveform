#ifndef __agl_fbo_h__
#define __agl_fbo_h__

typedef struct _fbo             AglFBO;

struct _fbo {
	guint id;
	guint texture;
	int   width;
	int   height;
};

#endif
