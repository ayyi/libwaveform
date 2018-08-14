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

#ifndef __gdk_gl_window_h__
#define __gdk_gl_window_h__

#include <gdk/gdkgltypes.h>
#include <gdk/gdk.h>
#include <gdk/gdkglx.h>
#include <gdk/gdkgldrawable.h>

G_BEGIN_DECLS

typedef struct _GdkGLWindow GdkGLWindow;
typedef struct _GdkGLWindowClass GdkGLWindowClass;

#define GDK_TYPE_GL_WINDOW            (gdk_gl_window_get_type ())
#define GDK_GL_WINDOW(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), GDK_TYPE_GL_WINDOW, GdkGLWindow))
#define GDK_GL_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GDK_TYPE_GL_WINDOW, GdkGLWindowClass))
#define GDK_IS_GL_WINDOW(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), GDK_TYPE_GL_WINDOW))
#define GDK_IS_GL_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDK_TYPE_GL_WINDOW))
#define GDK_GL_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GDK_TYPE_GL_WINDOW, GdkGLWindowClass))

struct _GdkGLWindow
{
    GdkDrawable parent_instance;

    GdkDrawable* drawable;        /* Associated GdkWindow */

    /* GLXWindow glxwindow; */
    Window glxwindow;

    GdkGLConfig *glconfig;

    guint is_destroyed : 1;
};

struct _GdkGLWindowClass
{
    GdkDrawableClass parent_class;
};

GType        gdk_gl_window_get_type         (void);

/*
 * attrib_list is currently unused. This must be set to NULL or empty
 * (first attribute of None). See GLX 1.3 spec.
 */
GdkGLWindow* gdk_gl_window_new              (GdkGLConfig*, GdkWindow*, const int* attrib_list);
GdkWindow*   gdk_gl_window_get_window       (GdkGLWindow*);

/*
 * OpenGL extension to GdkWindow
 */

GdkGLWindow* gdk_window_set_gl_capability   (GdkWindow*, GdkGLConfig*, const int* attrib_list);
void         gdk_window_unset_gl_capability (GdkWindow*);
gboolean     gdk_window_is_gl_capable       (GdkWindow*);
GdkGLWindow* gdk_window_get_gl_window       (GdkWindow*);

#define      gdk_window_get_gl_drawable(window) \
	GDK_GL_DRAWABLE (gdk_window_get_gl_window (window))

GdkGLConfig* gdk_gl_window_get_gl_config        (GdkGLDrawable*);
void         gdk_gl_window_swap_buffers         (GdkGLDrawable*);
gboolean     gdk_gl_window_make_context_current (GdkGLDrawable*, GdkGLContext*);

G_END_DECLS

#endif
