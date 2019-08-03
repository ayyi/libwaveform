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

#include <gtk/gtk.h>

#include "gdkglprivate.h"
#include "gdkglconfig.h"
#include "gdkglpixmap.h"
#include "gdkglcontext.h"

G_DEFINE_TYPE (GdkGLPixmap, gdk_gl_pixmap, GDK_TYPE_GL_DRAWABLE)

static void
gdk_gl_pixmap_init (GdkGLPixmap *self)
{
}

static void
_gdk_gl_pixmap_destroy (GdkGLPixmap *glpixmap)
{
  GDK_GL_NOTE_FUNC_PRIVATE ();

  if (glpixmap->is_destroyed)
    return;

  Display *xdisplay = GDK_GL_CONFIG_XDISPLAY (glpixmap->glconfig);

  if (glpixmap->glxpixmap == glXGetCurrentDrawable()) {
    glXWaitGL ();

    GDK_GL_NOTE_FUNC_IMPL ("glXMakeCurrent");
    glXMakeCurrent (xdisplay, None, NULL);
  }

  GDK_GL_NOTE_FUNC_IMPL ("glXDestroyGLXPixmap");
  glXDestroyGLXPixmap (xdisplay, glpixmap->glxpixmap);

  glpixmap->glxpixmap = None;
  glpixmap->is_destroyed = TRUE;
}

static void
gdk_gl_pixmap_finalize (GObject *object)
{
  GdkGLPixmap *glpixmap = GDK_GL_PIXMAP (object);

  GDK_GL_NOTE_FUNC_PRIVATE ();

  _gdk_gl_pixmap_destroy (GDK_GL_PIXMAP (object));

  g_object_unref (G_OBJECT (glpixmap->glconfig));

  if (glpixmap->drawable)
    g_object_remove_weak_pointer (G_OBJECT (glpixmap->drawable), (gpointer *) &(glpixmap->drawable));

  G_OBJECT_CLASS (gdk_gl_pixmap_parent_class)->finalize (object);
}

static void
gdk_gl_pixmap_class_init (GdkGLPixmapClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  GDK_GL_NOTE_FUNC_PRIVATE ();

  object_class->finalize = gdk_gl_pixmap_finalize;
}

/*< private >*/
void
_gdk_gl_pixmap_get_size (GdkGLDrawable *gldrawable, gint *width, gint *height)
{
  g_return_if_fail (GDK_IS_GL_PIXMAP (gldrawable));

  GdkDrawable *real_drawable = ((GdkGLPixmap *) gldrawable)->drawable;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->get_size (real_drawable, width, height);
}

/**
 * gdk_gl_pixmap_get_pixmap:
 * @glpixmap: a #GdkGLPixmap.
 *
 * Returns the #GdkPixmap associated with @glpixmap.
 *
 * Notice that #GdkGLPixmap is not #GdkPixmap, but another
 * #GdkDrawable which have an associated #GdkPixmap.
 *
 * Return value: (transfer none): the #GdkPixmap associated with @glpixmap.
 **/
GdkPixmap *
gdk_gl_pixmap_get_pixmap (GdkGLPixmap *glpixmap)
{
  g_return_val_if_fail (GDK_IS_GL_PIXMAP (glpixmap), NULL);

  return GDK_PIXMAP (glpixmap->drawable);
}

/*
 * OpenGL extension to GdkPixmap
 */

static const gchar quark_gl_pixmap_string[] = "gdk-gl-pixmap-gl-pixmap";
static GQuark quark_gl_pixmap = 0;

/**
 * gdk_pixmap_set_gl_capability:
 * @pixmap: the #GdkPixmap to be used as the rendering area.
 * @glconfig: a #GdkGLConfig.
 * @attrib_list: (array) (allow-none): this must be set to NULL or empty (first attribute of None).
 *
 * Set the OpenGL-capability to the @pixmap.
 * This function creates a new #GdkGLPixmap held by the @pixmap.
 * attrib_list is currently unused. This must be set to NULL or empty
 * (first attribute of None).
 *
 * Return value: (transfer none): the #GdkGLPixmap used by the @pixmap if it is successful,
 *               NULL otherwise.
 **/
GdkGLPixmap *
gdk_pixmap_set_gl_capability (GdkPixmap *pixmap, GdkGLConfig *glconfig, const int *attrib_list)
{
  GDK_GL_NOTE_FUNC ();

  g_return_val_if_fail (GDK_IS_PIXMAP (pixmap), NULL);
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), NULL);

  if (quark_gl_pixmap == 0)
    quark_gl_pixmap = g_quark_from_static_string (quark_gl_pixmap_string);

  /* If already set */
  GdkGLPixmap *glpixmap = g_object_get_qdata (G_OBJECT (pixmap), quark_gl_pixmap);
  if (glpixmap)
    return glpixmap;

  /*
   * Create GdkGLPixmap
   */

  glpixmap = gdk_gl_pixmap_new (glconfig, pixmap, attrib_list);
  if (!glpixmap){
    g_warning ("cannot create GdkGLPixmap\n");
    return NULL;
  }

  g_object_set_qdata_full (G_OBJECT (pixmap), quark_gl_pixmap, glpixmap, (GDestroyNotify) g_object_unref);

  return glpixmap;
}

/**
 * gdk_pixmap_unset_gl_capability:
 * @pixmap: a #GdkPixmap.
 *
 * Unset the OpenGL-capability of the @pixmap.
 * This function destroys the #GdkGLPixmap held by the @pixmap.
 *
 **/
void
gdk_pixmap_unset_gl_capability (GdkPixmap *pixmap)
{
  GdkGLPixmap *glpixmap;

  GDK_GL_NOTE_FUNC ();

  if (quark_gl_pixmap == 0)
    quark_gl_pixmap = g_quark_from_static_string (quark_gl_pixmap_string);

  /*
   * Destroy OpenGL resources explicitly, then unref.
   */

  glpixmap = g_object_get_qdata (G_OBJECT (pixmap), quark_gl_pixmap);
  if (glpixmap == NULL)
    return;

  _gdk_gl_pixmap_destroy (glpixmap);

  g_object_set_qdata (G_OBJECT (pixmap), quark_gl_pixmap, NULL);
}

/**
 * gdk_pixmap_is_gl_capable:
 * @pixmap: a #GdkPixmap.
 *
 * Returns whether the @pixmap is OpenGL-capable.
 *
 * Return value: TRUE if the @pixmap is OpenGL-capable, FALSE otherwise.
 **/
gboolean
gdk_pixmap_is_gl_capable (GdkPixmap *pixmap)
{
  g_return_val_if_fail (GDK_IS_PIXMAP (pixmap), FALSE);

  return g_object_get_qdata (G_OBJECT (pixmap), quark_gl_pixmap) != NULL ? TRUE : FALSE;
}

/**
 * gdk_pixmap_get_gl_pixmap:
 * @pixmap: a #GdkPixmap.
 *
 * Returns the #GdkGLPixmap held by the @pixmap.
 *
 * Return value: (transfer none): the #GdkGLPixmap.
 **/
GdkGLPixmap *
gdk_pixmap_get_gl_pixmap (GdkPixmap *pixmap)
{
  g_return_val_if_fail (GDK_IS_PIXMAP (pixmap), NULL);

  return g_object_get_qdata (G_OBJECT (pixmap), quark_gl_pixmap);
}

GdkGLConfig *
gdk_gl_pixmap_get_gl_config (GdkGLDrawable *gldrawable)
{
  g_return_val_if_fail (GDK_IS_GL_PIXMAP(gldrawable), NULL);

  return GDK_GL_PIXMAP(gldrawable)->glconfig;
}

gboolean
gdk_gl_pixmap_make_context_current (GdkGLDrawable *draw, GdkGLContext  *glcontext)
{
  GLXContext glxcontext;

  g_return_val_if_fail (GDK_IS_GL_PIXMAP(draw), FALSE);
  g_return_val_if_fail (GDK_IS_GL_CONTEXT_IMPL_X11 (glcontext), FALSE);

  GdkGLConfig *glconfig = GDK_GL_PIXMAP (draw)->glconfig;
  GLXPixmap glxpixmap = GDK_GL_PIXMAP (draw)->glxpixmap;
  glxcontext = GDK_GL_CONTEXT_GLXCONTEXT (glcontext);

  if (glxpixmap == None || glxcontext == NULL)
    return FALSE;

  GDK_GL_NOTE (MISC,
    g_message (" -- Pixmap: screen number = %d",
      GDK_SCREEN_XNUMBER (gdk_drawable_get_screen (GDK_DRAWABLE (draw)))));
  GDK_GL_NOTE (MISC,
    g_message (" -- Pixmap: visual id = 0x%lx",
      GDK_VISUAL_XVISUAL (gdk_drawable_get_visual (GDK_DRAWABLE (draw)))->visualid));

  GDK_GL_NOTE_FUNC_IMPL ("glXMakeCurrent");

  if (!glXMakeCurrent (GDK_GL_CONFIG_XDISPLAY (glconfig), glxpixmap, glxcontext))
    {
      g_warning ("glXMakeCurrent() failed");
      _gdk_gl_context_set_gl_drawable (glcontext, NULL);
      /* currently unused. */
      /* _gdk_gl_context_set_gl_drawable_read (glcontext, NULL); */
      return FALSE;
    }

  _gdk_gl_context_set_gl_drawable (glcontext, draw);

  /* currently unused. */
#if 0
  GdkGLDrawable *read = draw;
  gdk_gl_context_set_gl_drawable_read (glcontext, read);
#endif

  if (_GDK_GL_CONFIG_AS_SINGLE_MODE (glconfig))
    {
      /* We do this because we are treating a double-buffered frame
         buffer as a single-buffered frame buffer because the system
         does not appear to export any suitable single-buffered
         visuals (in which the following are necessary). */
      glDrawBuffer (GL_FRONT);
      glReadBuffer (GL_FRONT);
    }

  GDK_GL_NOTE (MISC, _gdk_gl_print_gl_info ());

  return TRUE;
}

/**
 * gdk_gl_pixmap_new:
 * @glconfig: a #GdkGLConfig.
 * @pixmap: the #GdkPixmap to be used as the rendering area.
 * @attrib_list: (array) (allow-none): this must be set to NULL or empty (first attribute of None).
 *
 * Creates an off-screen rendering area.
 * attrib_list is currently unused. This must be set to NULL or empty
 * (first attribute of None). See GLX 1.3 spec.
 *
 * Return value: the new #GdkGLPixmap.
 **/
GdkGLPixmap *
gdk_gl_pixmap_new (GdkGLConfig *glconfig, GdkPixmap *pixmap, const int *attrib_list)
{
  Display *xdisplay;
  XVisualInfo *xvinfo;
  GLXPixmap glxpixmap;

  Window root_return;
  int x_return, y_return;
  unsigned int width_return, height_return;
  unsigned int border_width_return;
  unsigned int depth_return;

  GDK_GL_NOTE_FUNC ();

  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), NULL);
  g_return_val_if_fail (GDK_IS_PIXMAP (pixmap), NULL);

  xdisplay = GDK_GL_CONFIG_XDISPLAY (glconfig);
  xvinfo = GDK_GL_CONFIG_XVINFO (glconfig);

  /*
   * Get X Pixmap.
   */

  Pixmap xpixmap = GDK_DRAWABLE_XID (GDK_DRAWABLE (pixmap));

  /*
   * Check depth of the X pixmap.
   */

  if (!XGetGeometry (xdisplay, xpixmap,
                     &root_return,
                     &x_return, &y_return,
                     &width_return, &height_return,
                     &border_width_return,
                     &depth_return))
    return NULL;

  if (depth_return != (unsigned int) xvinfo->depth)
    return NULL;

  /*
   * Create GLXPixmap.
   */

  GdkGL_GLX_MESA_pixmap_colormap* mesa_ext = gdk_gl_get_GLX_MESA_pixmap_colormap (glconfig);
  if (mesa_ext) {
      /* If GLX_MESA_pixmap_colormap is supported. */

      GDK_GL_NOTE_FUNC_IMPL ("glXCreateGLXPixmapMESA");

      glxpixmap = mesa_ext->glXCreateGLXPixmapMESA (xdisplay, xvinfo, xpixmap, GDK_GL_CONFIG_XCOLORMAP (glconfig));
  } else {
      GDK_GL_NOTE_FUNC_IMPL ("glXCreateGLXPixmap");

      glxpixmap = glXCreateGLXPixmap (xdisplay, xvinfo, xpixmap);
  }

  if (glxpixmap == None)
    return NULL;

  /*
   * Instantiate the GdkGLPixmapImplX11 object.
   */

  GdkGLPixmap* glpixmap = g_object_new (GDK_TYPE_GL_PIXMAP, NULL);

  glpixmap->drawable = GDK_DRAWABLE (pixmap);
  g_object_add_weak_pointer (G_OBJECT (glpixmap->drawable), (gpointer *) &(glpixmap->drawable));

  glpixmap->glxpixmap = glxpixmap;

  glpixmap->glconfig = glconfig;
  g_object_ref (G_OBJECT (glpixmap->glconfig));

  glpixmap->is_destroyed = FALSE;

  return glpixmap;
}

/**
 * gdk_x11_gl_config_get_xdisplay:
 * @glconfig: a #GdkGLConfig.
 *
 * Gets X Display.
 *
 * Return value: (transfer none): pointer to the Display.
 **/
Display *
gdk_x11_gl_config_get_xdisplay (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG(glconfig), NULL);

  return GDK_GL_CONFIG(glconfig)->xdisplay;
}

