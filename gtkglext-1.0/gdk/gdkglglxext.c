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
 * This is a generated file.  Please modify "gen-gdkglglxext-c.pl".
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gdkglx.h"
#include "gdk/gdkglprivate.h"
#include "gdkglglxext.h"


/*
 * GLX_MESA_pixmap_colormap
 */

static GdkGL_GLX_MESA_pixmap_colormap _procs_GLX_MESA_pixmap_colormap = {
  (GdkGLProc_glXCreateGLXPixmapMESA) -1
};

/* glXCreateGLXPixmapMESA */
GdkGLProc
gdk_gl_get_glXCreateGLXPixmapMESA (void)
{
  if (_procs_GLX_MESA_pixmap_colormap.glXCreateGLXPixmapMESA == (GdkGLProc_glXCreateGLXPixmapMESA) -1)
    _procs_GLX_MESA_pixmap_colormap.glXCreateGLXPixmapMESA = (GdkGLProc_glXCreateGLXPixmapMESA) gdk_gl_get_proc_address ("glXCreateGLXPixmapMESA");

  GDK_GL_NOTE (MISC, g_message (" - gdk_gl_get_glXCreateGLXPixmapMESA () - %s", (_procs_GLX_MESA_pixmap_colormap.glXCreateGLXPixmapMESA) ? "supported" : "not supported"));

  return (GdkGLProc) (_procs_GLX_MESA_pixmap_colormap.glXCreateGLXPixmapMESA);
}

/* Get GLX_MESA_pixmap_colormap functions */
GdkGL_GLX_MESA_pixmap_colormap *
gdk_gl_get_GLX_MESA_pixmap_colormap (GdkGLConfig *glconfig)
{
  static gint supported = -1;

  if (supported == -1) {
    supported = gdk_x11_gl_query_glx_extension (glconfig, "GLX_MESA_pixmap_colormap");

    if (supported) {
      supported &= (gdk_gl_get_glXCreateGLXPixmapMESA () != NULL);
    }
  }

  GDK_GL_NOTE (MISC, g_message (" - gdk_gl_get_GLX_MESA_pixmap_colormap () - %s", (supported) ? "supported" : "not supported"));

  if (!supported)
    return NULL;

  return &_procs_GLX_MESA_pixmap_colormap;
}

/*
 * GLX_MESA_release_buffers
 */

static GdkGL_GLX_MESA_release_buffers _procs_GLX_MESA_release_buffers = {
  (GdkGLProc_glXReleaseBuffersMESA) -1
};

/* glXReleaseBuffersMESA */
GdkGLProc
gdk_gl_get_glXReleaseBuffersMESA (void)
{
  if (_procs_GLX_MESA_release_buffers.glXReleaseBuffersMESA == (GdkGLProc_glXReleaseBuffersMESA) -1)
    _procs_GLX_MESA_release_buffers.glXReleaseBuffersMESA = (GdkGLProc_glXReleaseBuffersMESA) gdk_gl_get_proc_address ("glXReleaseBuffersMESA");

  GDK_GL_NOTE (MISC, g_message (" - gdk_gl_get_glXReleaseBuffersMESA () - %s", (_procs_GLX_MESA_release_buffers.glXReleaseBuffersMESA) ? "supported" : "not supported"));

  return (GdkGLProc) (_procs_GLX_MESA_release_buffers.glXReleaseBuffersMESA);
}

/* Get GLX_MESA_release_buffers functions */
GdkGL_GLX_MESA_release_buffers *
gdk_gl_get_GLX_MESA_release_buffers (GdkGLConfig *glconfig)
{
  static gint supported = -1;

  if (supported == -1) {
    supported = gdk_x11_gl_query_glx_extension (glconfig, "GLX_MESA_release_buffers");

    if (supported) {
      supported &= (gdk_gl_get_glXReleaseBuffersMESA () != NULL);
    }
  }

  GDK_GL_NOTE (MISC, g_message (" - gdk_gl_get_GLX_MESA_release_buffers () - %s", (supported) ? "supported" : "not supported"));

  if (!supported)
    return NULL;

  return &_procs_GLX_MESA_release_buffers;
}

