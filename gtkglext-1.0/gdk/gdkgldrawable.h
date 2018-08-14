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

#ifndef __GDK_GL_DRAWABLE_H__
#define __GDK_GL_DRAWABLE_H__

#include <gdk/gdkgltypes.h>

G_BEGIN_DECLS

typedef struct _GdkGLDrawable GdkGLDrawable;
typedef struct _GdkGLDrawableClass GdkGLDrawableClass;

#define GDK_TYPE_GL_DRAWABLE             (gdk_gl_drawable_get_type ())
#define GDK_GL_DRAWABLE(inst)            (G_TYPE_CHECK_INSTANCE_CAST ((inst), GDK_TYPE_GL_DRAWABLE, GdkGLDrawable))
#define GDK_GL_DRAWABLE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), GDK_TYPE_GL_DRAWABLE, GdkGLDrawableClass))
#define GDK_IS_GL_DRAWABLE(inst)         (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GDK_TYPE_GL_DRAWABLE))
#define GDK_IS_GL_DRAWABLE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), GDK_TYPE_GL_DRAWABLE))
#define GDK_GL_DRAWABLE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GDK_TYPE_GL_DRAWABLE, GdkGLDrawableClass))

struct _GdkGLDrawable
{
  GObject parent_instance;
};

struct _GdkGLDrawableClass
{
  GObjectClass parent_class;
};

#define gdk_gl_drawable_make_current           gdk_gl_window_make_context_current
#define gdk_gl_drawable_swap_buffers(DRAWABLE) gdk_gl_window_swap_buffers(DRAWABLE)

GType          gdk_gl_drawable_get_type           (void);
GdkGLDrawable* gdk_gl_drawable_get_current        (void);

G_END_DECLS

#endif
