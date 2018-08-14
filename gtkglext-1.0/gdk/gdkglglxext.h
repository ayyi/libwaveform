/* GdkGLExt - OpenGL Extension to GDK
 * Copyright (C) 2002-2004  Naofumi Yasufuku
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
 */

/*
 * This is a generated file.  Please modify "gen-gdkglglxext-h.pl".
 */

#ifndef __GDK_GL_GLXEXT_H__
#define __GDK_GL_GLXEXT_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <gdk/gdkglquery.h>
#include <gdk/gdkglconfig.h>

G_BEGIN_DECLS

#ifndef GLX_VERSION_1_3
typedef struct __GLXFBConfigRec *GLXFBConfig;
typedef XID GLXFBConfigID;
typedef XID GLXContextID;
typedef XID GLXWindow;
typedef XID GLXPbuffer;
#endif

#undef __glxext_h_
#undef GLX_GLXEXT_VERSION
#include <gdk/glext/glxext.h>
#include <gdk/glext/glxext-extra.h>

/*
 * GLX_MESA_pixmap_colormap
 */

/* glXCreateGLXPixmapMESA */
typedef GLXPixmap ( * GdkGLProc_glXCreateGLXPixmapMESA) (Display *dpy, XVisualInfo *visual, Pixmap pixmap, Colormap cmap);
GdkGLProc    gdk_gl_get_glXCreateGLXPixmapMESA (void);
#define      gdk_gl_glXCreateGLXPixmapMESA(proc, dpy, visual, pixmap, cmap) \
  ( ((GdkGLProc_glXCreateGLXPixmapMESA) (proc)) (dpy, visual, pixmap, cmap) )

/* proc struct */

typedef struct _GdkGL_GLX_MESA_pixmap_colormap GdkGL_GLX_MESA_pixmap_colormap;

struct _GdkGL_GLX_MESA_pixmap_colormap
{
  GdkGLProc_glXCreateGLXPixmapMESA glXCreateGLXPixmapMESA;
};

GdkGL_GLX_MESA_pixmap_colormap *gdk_gl_get_GLX_MESA_pixmap_colormap (GdkGLConfig *glconfig);

/*
 * GLX_MESA_release_buffers
 */

/* glXReleaseBuffersMESA */
typedef Bool ( * GdkGLProc_glXReleaseBuffersMESA) (Display *dpy, GLXDrawable drawable);
GdkGLProc    gdk_gl_get_glXReleaseBuffersMESA (void);
#define      gdk_gl_glXReleaseBuffersMESA(proc, dpy, drawable) \
  ( ((GdkGLProc_glXReleaseBuffersMESA) (proc)) (dpy, drawable) )

/* proc struct */

typedef struct _GdkGL_GLX_MESA_release_buffers GdkGL_GLX_MESA_release_buffers;

struct _GdkGL_GLX_MESA_release_buffers
{
  GdkGLProc_glXReleaseBuffersMESA glXReleaseBuffersMESA;
};

GdkGL_GLX_MESA_release_buffers *gdk_gl_get_GLX_MESA_release_buffers (GdkGLConfig *glconfig);

G_END_DECLS

#endif
