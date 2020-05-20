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

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"

#include "gdkglprivate.h"
#include "gdkglconfig.h"
#include "gdkglwindow.h"
#include "gdkglcontext.h"

G_DEFINE_TYPE (GdkGLWindow, gdk_gl_window, GDK_TYPE_GL_DRAWABLE)


static void
gdk_gl_window_init (GdkGLWindow *self)
{
}

static void
gdk_gl_window_finalize (GObject *object)
{
  GdkGLWindow *glwindow = GDK_GL_WINDOW (object);

  GDK_GL_NOTE_FUNC_PRIVATE ();

  if (glwindow->drawable)
    g_object_remove_weak_pointer (G_OBJECT (glwindow->drawable), (gpointer *) &(glwindow->drawable));

  _gdk_gl_window_destroy (glwindow);

  g_object_unref (G_OBJECT (glwindow->glconfig));

  G_OBJECT_CLASS (gdk_gl_window_parent_class)->finalize (object);
}

static void
gdk_gl_window_class_init (GdkGLWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  GDK_GL_NOTE_FUNC_PRIVATE ();

  object_class->finalize = gdk_gl_window_finalize;
}

void
_gdk_gl_window_destroy (GdkGLWindow *glwindow)
{

  GDK_GL_NOTE_FUNC_PRIVATE ();

  if (glwindow->is_destroyed)
    return;

  Display *xdisplay = GDK_GL_CONFIG_XDISPLAY (glwindow->glconfig);

  if (glwindow->glxwindow == glXGetCurrentDrawable ()) {
    glXWaitGL ();

    GDK_GL_NOTE_FUNC_IMPL ("glXMakeCurrent");
    glXMakeCurrent (xdisplay, None, NULL);
  }

  /* If GLX_MESA_release_buffers is supported. */
  GdkGL_GLX_MESA_release_buffers *mesa_ext = gdk_gl_get_GLX_MESA_release_buffers (glwindow->glconfig);
  if (mesa_ext) {
    GDK_GL_NOTE_FUNC_IMPL ("glXReleaseBuffersMESA");
    mesa_ext->glXReleaseBuffersMESA (xdisplay, glwindow->glxwindow);
  }

  glwindow->glxwindow = None;

  glwindow->is_destroyed = TRUE;
}

/*
 * attrib_list is currently unused. This must be set to NULL or empty
 * (first attribute of None). See GLX 1.3 spec.
 */
/**
 * gdk_gl_window_new:
 * @glconfig: a #GdkGLConfig.
 * @window: the #GdkWindow to be used as the rendering area.
 * @attrib_list: (array) (allow-none): this must be set to NULL or empty (first attribute of None).
 *
 * Creates an on-screen rendering area.
 * attrib_list is currently unused. This must be set to NULL or empty
 * (first attribute of None). See GLX 1.3 spec.
 *
 * Return value: the new #GdkGLWindow.
 **/
GdkGLWindow *
gdk_gl_window_new (GdkGLConfig *glconfig, GdkWindow *window, const int *attrib_list)
{
  GDK_GL_NOTE_FUNC ();

  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), NULL);
  g_return_val_if_fail (GDK_IS_WINDOW (window), NULL);

  /*
   * Get X Window.
   */

  Window glxwindow = GDK_DRAWABLE_XID (GDK_DRAWABLE (window));

  /*
   * Instantiate the GdkGLWindowImplX11 object.
   */

  GdkGLWindow *glwindow = g_object_new (GDK_TYPE_GL_WINDOW, NULL);

  glwindow->drawable = GDK_DRAWABLE (window);
  g_object_add_weak_pointer (G_OBJECT (glwindow->drawable), (gpointer *) &(glwindow->drawable));

  glwindow->glxwindow = glxwindow;

  glwindow->glconfig = glconfig;
  g_object_ref (G_OBJECT (glwindow->glconfig));

  glwindow->is_destroyed = FALSE;

  return glwindow;
}

/*< private >*/
void
_gdk_gl_window_get_size (GdkGLDrawable *gldrawable, gint *width, gint *height)
{
  g_return_if_fail (GDK_IS_GL_WINDOW (gldrawable));

  GdkDrawable *real_drawable = ((GdkGLWindow *) gldrawable)->drawable;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->get_size (real_drawable, width, height);
}

/**
 * gdk_gl_window_get_window:
 * @glwindow: a #GdkGLWindow.
 *
 * Returns the #GdkWindow associated with @glwindow.
 *
 * Notice that #GdkGLWindow is not #GdkWindow, but another
 * #GdkDrawable which have an associated #GdkWindow.
 *
 * Return value: (transfer none): the #GdkWindow associated with @glwindow.
 **/
GdkWindow *
gdk_gl_window_get_window (GdkGLWindow *glwindow)
{
  g_return_val_if_fail (GDK_IS_GL_WINDOW (glwindow), NULL);

  return GDK_WINDOW (glwindow->drawable);
}

/*
 * OpenGL extension to GdkWindow
 */

static const gchar quark_gl_window_string[] = "gdk-gl-window-gl-window";
static GQuark quark_gl_window = 0;

/**
 * gdk_window_set_gl_capability:
 * @window: the #GdkWindow to be used as the rendering area.
 * @glconfig: a #GdkGLConfig.
 * @attrib_list: (array) (allow-none): this must be set to NULL or empty (first attribute of None).
 *
 * Set the OpenGL-capability to the @window.
 * This function creates a new #GdkGLWindow held by the @window.
 * attrib_list is currently unused. This must be set to NULL or empty
 * (first attribute of None).
 *
 * Return value: (transfer none): the #GdkGLWindow used by the @window if it is successful,
 *               NULL otherwise.
 **/
GdkGLWindow *
gdk_window_set_gl_capability (GdkWindow *window, GdkGLConfig *glconfig, const int *attrib_list)
{

  GDK_GL_NOTE_FUNC ();

  g_return_val_if_fail (GDK_IS_WINDOW (window), NULL);
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), NULL);

  if (quark_gl_window == 0)
    quark_gl_window = g_quark_from_static_string (quark_gl_window_string);

  /* If already set */
  GdkGLWindow *glwindow = g_object_get_qdata (G_OBJECT (window), quark_gl_window);
  if (glwindow)
    return glwindow;

  /*
   * Create GdkGLWindow
   */

  glwindow = gdk_gl_window_new (glconfig, window, attrib_list);
  if (!glwindow) {
    g_warning ("cannot create GdkGLWindow\n");
    return NULL;
  }

  g_object_set_qdata_full (G_OBJECT(window), quark_gl_window, glwindow, (GDestroyNotify) g_object_unref);

  /* 
   * Set a background of "None" on window to avoid AIX X server crash
   */

  GDK_GL_NOTE (MISC, g_message (" - window->bg_pixmap = %p", ((GdkWindowObject *) window)->bg_pixmap));

  gdk_window_set_back_pixmap (window, NULL, FALSE);

  GDK_GL_NOTE (MISC, g_message (" - window->bg_pixmap = %p", ((GdkWindowObject *) window)->bg_pixmap));

  return glwindow;
}

/**
 * gdk_window_unset_gl_capability:
 * @window: a #GdkWindow.
 *
 * Unset the OpenGL-capability of the @window.
 * This function destroys the #GdkGLWindow held by the @window.
 *
 **/
void
gdk_window_unset_gl_capability (GdkWindow *window)
{
  GdkGLWindow *glwindow;

  GDK_GL_NOTE_FUNC ();

  if (quark_gl_window == 0)
    quark_gl_window = g_quark_from_static_string (quark_gl_window_string);

  /*
   * Destroy OpenGL resources explicitly, then unref.
   */

  glwindow = g_object_get_qdata (G_OBJECT (window), quark_gl_window);
  if (glwindow == NULL)
    return;

  _gdk_gl_window_destroy (glwindow);

  g_object_set_qdata (G_OBJECT (window), quark_gl_window, NULL);
}

/**
 * gdk_window_is_gl_capable:
 * @window: a #GdkWindow.
 *
 * Returns whether the @window is OpenGL-capable.
 *
 * Return value: TRUE if the @window is OpenGL-capable, FALSE otherwise.
 **/
gboolean
gdk_window_is_gl_capable (GdkWindow *window)
{
  g_return_val_if_fail (GDK_IS_WINDOW (window), FALSE);

  return g_object_get_qdata (G_OBJECT (window), quark_gl_window) != NULL ? TRUE : FALSE;
}

/**
 * gdk_window_get_gl_window:
 * @window: a #GdkWindow.
 *
 * Returns the #GdkGLWindow held by the @window.
 *
 * Return value: (transfer none): the #GdkGLWindow.
 **/
GdkGLWindow *
gdk_window_get_gl_window (GdkWindow *window)
{
  g_return_val_if_fail (GDK_IS_WINDOW (window), NULL);

  return g_object_get_qdata (G_OBJECT (window), quark_gl_window);
}

GdkGLConfig*
gdk_gl_window_get_gl_config (GdkGLDrawable *gldrawable)
{
  g_return_val_if_fail (GDK_IS_GL_WINDOW (gldrawable), NULL);

  return GDK_GL_WINDOW (gldrawable)->glconfig;
}

void
gdk_gl_window_swap_buffers (GdkGLDrawable *gldrawable)
{
  g_return_if_fail (GDK_IS_GL_WINDOW(gldrawable));

  Display *xdisplay = GDK_GL_CONFIG_XDISPLAY (GDK_GL_WINDOW (gldrawable)->glconfig);
  Window glxwindow = GDK_GL_WINDOW (gldrawable)->glxwindow;

  if (glxwindow == None)
    return;

  GDK_GL_NOTE_FUNC_IMPL ("glXSwapBuffers");

  glXSwapBuffers (xdisplay, glxwindow);
}

gboolean
gdk_gl_window_make_context_current (GdkGLDrawable *draw, GdkGLContext *glcontext)
{
  GdkGLConfig *glconfig;
  Window glxwindow;
  GLXContext glxcontext;

  g_return_val_if_fail (GDK_IS_GL_WINDOW (draw), FALSE);
  g_return_val_if_fail (GDK_IS_GL_CONTEXT_IMPL_X11 (glcontext), FALSE);

  glconfig = GDK_GL_WINDOW (draw)->glconfig;
  glxwindow = GDK_GL_WINDOW (draw)->glxwindow;
  glxcontext = GDK_GL_CONTEXT_GLXCONTEXT (glcontext);

  if (glxwindow == None || glxcontext == NULL)
    return FALSE;

  GDK_GL_NOTE (MISC, g_message (" -- Window: screen number = %d", GDK_SCREEN_XNUMBER (gdk_drawable_get_screen (GDK_DRAWABLE (draw)))));
  GDK_GL_NOTE (MISC, g_message (" -- Window: visual id = 0x%lx", GDK_VISUAL_XVISUAL (gdk_drawable_get_visual (GDK_DRAWABLE (draw)))->visualid));

  GDK_GL_NOTE_FUNC_IMPL ("glXMakeCurrent");

  if (!glXMakeCurrent (GDK_GL_CONFIG_XDISPLAY (glconfig), glxwindow, glxcontext))
    {
      g_warning ("glXMakeCurrent() failed");
      _gdk_gl_context_set_gl_drawable (glcontext, NULL);
      return FALSE;
    }

  _gdk_gl_context_set_gl_drawable (glcontext, draw);

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

