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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gdk/gdk.h>

#include "gdkglprivate.h"
#include "gdkgldrawable.h"
#include "gdkglx.h"
#include "gdkglcontext.h"

G_DEFINE_TYPE (GdkGLDrawable, gdk_gl_drawable, G_TYPE_OBJECT)


static void
gdk_gl_drawable_class_init (GdkGLDrawableClass *klass)
{
}

static void
gdk_gl_drawable_init (GdkGLDrawable *self)
{
}

/**
 * gdk_gl_drawable_get_current:
 *
 * Returns the current #GdkGLDrawable.
 *
 * Return value: (transfer none): the current #GdkGLDrawable or NULL if there is no current drawable.
 **/
GdkGLDrawable*
gdk_gl_drawable_get_current (void)
{
  GDK_GL_NOTE_FUNC ();

  GdkGLContext* glcontext = gdk_gl_context_get_current ();
  if (!glcontext)
    return NULL;

  return gdk_gl_context_get_gl_drawable (glcontext);
}
