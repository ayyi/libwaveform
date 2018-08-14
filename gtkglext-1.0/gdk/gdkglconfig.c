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

#include <string.h>

#include "gdkglprivate.h"
#include "gdkglconfig.h"

#include <gdk/gdk.h>

gboolean _gdk_gl_config_no_standard_colormap = FALSE;

G_DEFINE_TYPE (GdkGLConfig, gdk_gl_config, G_TYPE_OBJECT)


static void
gdk_gl_config_init (GdkGLConfig *self)
{
}

static void
gdk_gl_config_impl_x11_finalize (GObject *object)
{
  GdkGLConfig* glconfig = GDK_GL_CONFIG (object);

  GDK_GL_NOTE_FUNC_PRIVATE ();

  XFree (glconfig->xvinfo);

  g_object_unref (G_OBJECT (glconfig->colormap));

  G_OBJECT_CLASS (gdk_gl_config_parent_class)->finalize (object);
}

static void
gdk_gl_config_class_init (GdkGLConfigClass *klass)
{
  GDK_GL_NOTE_FUNC_PRIVATE ();

  ((GObjectClass*)klass)->finalize = gdk_gl_config_impl_x11_finalize;
}

static GdkGLConfig*
gdk_gl_config_new_ci (GdkScreen *screen, GdkGLConfigMode mode)
{
  GdkGLConfig *glconfig = NULL;
  static const int buf_size_list[] = { 16, 12, 8, 4, 2, 1, 0 };
  int list[32];
  int n = 0;
  int i;

  list[n++] = GDK_GL_BUFFER_SIZE;
  list[n++] = 1;
  if (mode & GDK_GL_MODE_DOUBLE)
    {
      list[n++] = GDK_GL_DOUBLEBUFFER;
    }
  if (mode & GDK_GL_MODE_DEPTH)
    {
      list[n++] = GDK_GL_DEPTH_SIZE;
      list[n++] = 1;
    }
  if (mode & GDK_GL_MODE_STENCIL)
    {
      list[n++] = GDK_GL_STENCIL_SIZE;
      list[n++] = 1;
    }
  list[n] = GDK_GL_ATTRIB_LIST_NONE;

  /* from GLUT */
  /* glXChooseVisual specify GLX_BUFFER_SIZE prefers the
     "smallest index buffer of at least the specified size".
     This would be reasonable if GLUT allowed the user to
     specify the required buffe size, but GLUT's display mode
     is too simplistic (easy to use?). GLUT should try to find
     the "largest".  So start with a large buffer size and
     shrink until we find a matching one that exists. */

  for (i = 0; buf_size_list[i]; i++)
    {
      /* XXX Assumes list[1] is where GDK_GL_BUFFER_SIZE parameter is. */
      list[1] = buf_size_list[i];

      glconfig = gdk_gl_config_new_for_screen (screen, list);

      if (glconfig != NULL)
        return glconfig;
    }

  return NULL;
}

static GdkGLConfig*
gdk_gl_config_new_rgb (GdkScreen *screen, GdkGLConfigMode mode)
{
  int list[32];
  int n = 0;

  list[n++] = GDK_GL_RGBA;
  list[n++] = GDK_GL_RED_SIZE;
  list[n++] = 1;
  list[n++] = GDK_GL_GREEN_SIZE;
  list[n++] = 1;
  list[n++] = GDK_GL_BLUE_SIZE;
  list[n++] = 1;
  if (mode & GDK_GL_MODE_ALPHA)
    {
      list[n++] = GDK_GL_ALPHA_SIZE;
      list[n++] = 1;
    }
  if (mode & GDK_GL_MODE_DOUBLE)
    {
      list[n++] = GDK_GL_DOUBLEBUFFER;
    }
  if (mode & GDK_GL_MODE_DEPTH)
    {
      list[n++] = GDK_GL_DEPTH_SIZE;
      list[n++] = 1;
    }
  if (mode & GDK_GL_MODE_STENCIL)
    {
      list[n++] = GDK_GL_STENCIL_SIZE;
      list[n++] = 1;
    }
  if (mode & GDK_GL_MODE_ACCUM)
    {
      list[n++] = GDK_GL_ACCUM_RED_SIZE;
      list[n++] = 1;
      list[n++] = GDK_GL_ACCUM_GREEN_SIZE;
      list[n++] = 1;
      list[n++] = GDK_GL_ACCUM_BLUE_SIZE;
      list[n++] = 1;
      if (mode & GDK_GL_MODE_ALPHA)
        {
          list[n++] = GDK_GL_ACCUM_ALPHA_SIZE;
          list[n++] = 1;
        }
    }
  list[n] = GDK_GL_ATTRIB_LIST_NONE;

  return gdk_gl_config_new_for_screen (screen, list);
}

static GdkGLConfig *
gdk_gl_config_new_by_mode_common (GdkScreen *screen, GdkGLConfigMode mode)
{
#define _GL_CONFIG_NEW_BY_MODE(__screen, __mode)        \
  ( ((__mode) & GDK_GL_MODE_INDEX) ?                    \
    gdk_gl_config_new_ci (__screen, __mode) :           \
    gdk_gl_config_new_rgb (__screen, __mode) )

  GdkGLConfig* glconfig = _GL_CONFIG_NEW_BY_MODE (screen, mode);
  if (!glconfig) {
    /* Fallback cases when can't get exactly what was asked for... */
    if (!(mode & GDK_GL_MODE_DOUBLE)) {
        /* If we can't find a single buffered visual, try looking
           for a double buffered visual.  We can treat a double
           buffered visual as a single buffered visual by changing
           the draw buffer to GL_FRONT and treating any swap
           buffers as no-ops. */
        mode |= GDK_GL_MODE_DOUBLE;
        glconfig = _GL_CONFIG_NEW_BY_MODE (screen, mode);
        if (glconfig)
          glconfig->as_single_mode = TRUE;
    }
  }

#undef _GL_CONFIG_NEW_BY_MODE

  return glconfig;
}

/**
 * gdk_gl_config_new_by_mode:
 * @mode: display mode bit mask.
 *
 * Returns an OpenGL frame buffer configuration that match the specified
 * display mode.
 *
 * Return value: the new #GdkGLConfig.
 **/
GdkGLConfig *
gdk_gl_config_new_by_mode (GdkGLConfigMode mode)
{
  GdkScreen *screen;

  screen = gdk_screen_get_default ();

  return gdk_gl_config_new_by_mode_common (screen, mode);
}

/**
 * gdk_gl_config_new_by_mode_for_screen:
 * @screen: target screen.
 * @mode: display mode bit mask.
 *
 * Returns an OpenGL frame buffer configuration that match the specified
 * display mode.
 *
 * Return value: the new #GdkGLConfig.
 **/
GdkGLConfig *
gdk_gl_config_new_by_mode_for_screen (GdkScreen *screen, GdkGLConfigMode mode)
{
  return gdk_gl_config_new_by_mode_common (screen, mode);
}

/**
 * gdk_gl_config_get_layer_plane:
 * @glconfig: a #GdkGLConfig.
 *
 * Gets the layer plane (level) of the frame buffer.
 * Zero is the default frame buffer.
 * Positive layer planes correspond to frame buffers that overlay the default
 * buffer, and negative layer planes correspond to frame buffers that underlie
 * the default frame buffer.
 *
 * Return value: layer plane.
 **/
gint
gdk_gl_config_get_layer_plane (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), 0);

  return glconfig->layer_plane;
}

/**
 * gdk_gl_config_get_n_aux_buffers:
 * @glconfig: a #GdkGLConfig.
 *
 * Gets the number of auxiliary color buffers.
 *
 * Return value: number of auxiliary color buffers.
 **/
gint
gdk_gl_config_get_n_aux_buffers (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), 0);

  return glconfig->n_aux_buffers;
}

/**
 * gdk_gl_config_get_n_sample_buffers:
 * @glconfig: a #GdkGLConfig.
 *
 * Gets the number of multisample buffers.
 *
 * Return value: number of multisample buffers.
 **/
gint
gdk_gl_config_get_n_sample_buffers (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), 0);

  return glconfig->n_sample_buffers;
}

/**
 * gdk_gl_config_is_rgba:
 * @glconfig: a #GdkGLConfig.
 *
 * Returns whether the configured frame buffer is RGBA mode.
 *
 * Return value: TRUE if the configured frame buffer is RGBA mode, FALSE
 *               otherwise.
 **/
gboolean
gdk_gl_config_is_rgba (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), FALSE);

  return glconfig->is_rgba;
}

/**
 * gdk_gl_config_is_double_buffered:
 * @glconfig: a #GdkGLConfig.
 *
 * Returns whether the configuration supports the double-buffered visual.
 *
 * Return value: TRUE if the double-buffered visual is supported, FALSE
 *               otherwise.
 **/
gboolean
gdk_gl_config_is_double_buffered (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), FALSE);

  return (glconfig->is_double_buffered && (!glconfig->as_single_mode));
}

/**
 * gdk_gl_config_has_alpha:
 * @glconfig: a #GdkGLConfig.
 *
 * Returns whether the configured color buffer has alpha bits.
 *
 * Return value: TRUE if the color buffer has alpha bits, FALSE otherwise.
 **/
gboolean
gdk_gl_config_has_alpha (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), FALSE);

  return glconfig->has_alpha;
}

/**
 * gdk_gl_config_has_depth_buffer:
 * @glconfig: a #GdkGLConfig.
 *
 * Returns whether the configured frame buffer has depth buffer.
 *
 * Return value: TRUE if the frame buffer has depth buffer, FALSE otherwise.
 **/
gboolean
gdk_gl_config_has_depth_buffer (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), FALSE);

  return glconfig->has_depth_buffer;
}

/**
 * gdk_gl_config_has_stencil_buffer:
 * @glconfig: a #GdkGLConfig.
 *
 * Returns whether the configured frame buffer has stencil buffer.
 *
 * Return value: TRUE if the frame buffer has stencil buffer, FALSE otherwise.
 **/
gboolean
gdk_gl_config_has_stencil_buffer (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), FALSE);

  return glconfig->has_stencil_buffer;
}

/**
 * gdk_gl_config_has_accum_buffer:
 * @glconfig: a #GdkGLConfig.
 *
 * Returns whether the configured frame buffer has accumulation buffer.
 *
 * Return value: TRUE if the frame buffer has accumulation buffer, FALSE
 *               otherwise.
 **/
gboolean
gdk_gl_config_has_accum_buffer (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), FALSE);

  return glconfig->has_accum_buffer;
}

#ifdef HAVE_LIBXMU

#include <X11/Xatom.h>  /* for XA_RGB_DEFAULT_MAP atom */

#ifdef HAVE_XMU_STDCMAP_H
#include <Xmu/StdCmap.h>  /* for XmuLookupStandardColormap */
#else
#include <X11/Xmu/StdCmap.h>  /* for XmuLookupStandardColormap */
#endif

#endif /* HAVE_LIBXMU */

/* 
 * Get standard RGB colormap
 */

#ifdef HAVE_GDK_X11_COLORMAP_FOREIGN_NEW
static GdkColormap *
gdk_gl_config_get_std_rgb_colormap (GdkScreen *screen, XVisualInfo *xvinfo, gboolean is_mesa_glx)
{
  int screen_num;
  Window xroot_window;
  Status status;
  Colormap xcolormap = None;
  XStandardColormap *standard_cmaps;
  int i, num_cmaps;
  GdkVisual *visual;

  GDK_GL_NOTE_FUNC_PRIVATE ();

  GdkDisplay* display = gdk_screen_get_display (screen);
  Display* xdisplay = GDK_DISPLAY_XDISPLAY (display);
  screen_num = xvinfo->screen;
  xroot_window = RootWindow (xdisplay, screen_num);

  /*
   * (ripped from GLUT)
   * Hewlett-Packard supports a feature called "HP Color Recovery".
   * Mesa has code to use HP Color Recovery.  For Mesa to use this feature,
   * the atom _HP_RGB_SMOOTH_MAP_LIST must be defined on the root window AND
   * the colormap obtainable by XGetRGBColormaps for that atom must be set on
   * the window.  If that colormap is not set, the output will look stripy.
   */

  if (is_mesa_glx && xvinfo->visual->class == TrueColor && xvinfo->depth == 8) {
      Atom xa_hp_cr_maps;

      GDK_GL_NOTE (MISC, g_message (" -- Try to find a standard RGB colormap with HP Color Recovery"));

      xa_hp_cr_maps = gdk_x11_get_xatom_by_name_for_display (display, "_HP_RGB_SMOOTH_MAP_LIST");

      status = XGetRGBColormaps (xdisplay, xroot_window, &standard_cmaps, &num_cmaps, xa_hp_cr_maps);
      if (status)
        {
          for (i = 0; i < num_cmaps; i++)
            {
              if (standard_cmaps[i].visualid == xvinfo->visualid)
                {
                  xcolormap = standard_cmaps[i].colormap;
                  break;
                }
            }

          XFree (standard_cmaps);

          if (xcolormap != None)
            {
              GDK_GL_NOTE (MISC,
                g_message (" -- Colormap: standard RGB with HP Color Recovery"));

              visual = gdk_x11_screen_lookup_visual (screen, xvinfo->visualid);
              return gdk_x11_colormap_foreign_new (visual, xcolormap);
            }
        }
    }

#if defined(HAVE_LIBXMU) && !defined(_DISABLE_STANDARD_RGB_CMAP)

  /*
   * (ripped from GLUT)
   * Solaris 2.4 and 2.5 have a bug in their XmuLookupStandardColormap
   * implementations.  Please compile your Solaris 2.4 or 2.5 version of
   * GtkGLExt with -D_DISABLE_STANDARD_RGB_CMAP to work around this bug.
   * The symptom of the bug is that programs will get a BadMatch error
   * from XCreateWindow when creating a window because Solaris 2.4 and 2.5
   * create a corrupted RGB_DEFAULT_MAP property.  Note that this workaround
   * prevents colormap sharing between applications, perhaps leading
   * unnecessary colormap installations or colormap flashing.  Sun fixed
   * this bug in Solaris 2.6.
   */

  if (!_gdk_gl_config_no_standard_colormap)
    {
      GDK_GL_NOTE (MISC, g_message (" -- Try to find a standard RGB colormap"));

      status = XmuLookupStandardColormap (xdisplay, screen_num, xvinfo->visualid, xvinfo->depth, XA_RGB_DEFAULT_MAP, False, True);
      if (status)
        {
          status = XGetRGBColormaps (xdisplay, xroot_window, &standard_cmaps, &num_cmaps, XA_RGB_DEFAULT_MAP);
          if (status)
            {
              for (i = 0; i < num_cmaps; i++)
                {
                  if (standard_cmaps[i].visualid == xvinfo->visualid)
                    {
                      xcolormap = standard_cmaps[i].colormap;
                      break;
                    }
                }

              XFree (standard_cmaps);

              if (xcolormap != None)
                {
                  GDK_GL_NOTE (MISC, g_message (" -- Colormap: standard RGB"));

                  visual = gdk_x11_screen_lookup_visual (screen, xvinfo->visualid);
                  return gdk_x11_colormap_foreign_new (visual, xcolormap);
                }
            }
        }
    }

#endif /* defined(HAVE_LIBXMU) && !defined(_DISABLE_STANDARD_RGB_CMAP) */

  return NULL;
}
#endif

/* 
 * Setup colormap.
 */

static GdkColormap *
gdk_gl_config_setup_colormap (GdkScreen *screen, XVisualInfo *xvinfo, gboolean is_rgba, gboolean is_mesa_glx)
{
	GdkColormap *colormap;
	GdkVisual *visual;
	gboolean overlay_supported;

	GDK_GL_NOTE_FUNC_PRIVATE ();

	g_return_val_if_fail(is_rgba, NULL);

	/* Try default colormap. */

	colormap = gdk_screen_get_default_colormap (screen);
	visual = gdk_colormap_get_visual (colormap);
	if (GDK_VISUAL_XVISUAL (visual)->visualid == xvinfo->visualid) {
		GDK_GL_NOTE (MISC, g_message (" -- Colormap: screen default"));
		g_object_ref (G_OBJECT (colormap));
		return colormap;
	}

	/* Try standard RGB colormap. */

#ifdef HAVE_GDK_X11_COLORMAP_FOREIGN_NEW
	colormap = gdk_gl_config_get_std_rgb_colormap (screen, xvinfo, is_mesa_glx);
	if (colormap)
		return colormap;
#endif

	/* New colormap. */

	GDK_GL_NOTE (MISC, g_message (" -- Colormap: new"));
	visual = gdk_x11_screen_lookup_visual (screen, xvinfo->visualid);
	return gdk_colormap_new (visual, FALSE);
}

static void
gdk_gl_config_init_attrib (GdkGLConfig *glconfig)
{
  int value;

#define _GET_CONFIG(__attrib) \
  glXGetConfig (glconfig->xdisplay, glconfig->xvinfo, __attrib, &value)

  /* RGBA mode? */
  _GET_CONFIG (GLX_RGBA);
  glconfig->is_rgba = value ? TRUE : FALSE;

  /* Layer plane. */
  _GET_CONFIG (GLX_LEVEL);
  glconfig->layer_plane = value;

  /* Double buffering is supported? */
  _GET_CONFIG (GLX_DOUBLEBUFFER);
  glconfig->is_double_buffered = value ? TRUE : FALSE;

  /* Number of aux buffers */
  _GET_CONFIG (GLX_AUX_BUFFERS);
  glconfig->n_aux_buffers = value;

  /* Has alpha bits? */
  _GET_CONFIG (GLX_ALPHA_SIZE);
  glconfig->has_alpha = value ? TRUE : FALSE;

  /* Has depth buffer? */
  _GET_CONFIG (GLX_DEPTH_SIZE);
  glconfig->has_depth_buffer = value ? TRUE : FALSE;

  /* Has stencil buffer? */
  _GET_CONFIG (GLX_STENCIL_SIZE);
  glconfig->has_stencil_buffer = value ? TRUE : FALSE;

  /* Has accumulation buffer? */
  _GET_CONFIG (GLX_ACCUM_RED_SIZE);
  glconfig->has_accum_buffer = value ? TRUE : FALSE;

  /* Number of multisample buffers (not supported yet) */
  glconfig->n_sample_buffers = 0;

#undef _GET_CONFIG
}

static GdkGLConfig *
gdk_gl_config_new_common (GdkScreen *screen, const int *attrib_list)
{
  int is_rgba;

  GDK_GL_NOTE_FUNC_PRIVATE ();

  Display* xdisplay = GDK_SCREEN_XDISPLAY (screen);
  int screen_num = GDK_SCREEN_XNUMBER (screen);

  GDK_GL_NOTE (MISC, _gdk_gl_print_glx_info (xdisplay, screen_num));

  /*
   * Find an OpenGL-capable visual.
   */

  GDK_GL_NOTE_FUNC_IMPL ("glXChooseVisual");

  XVisualInfo* xvinfo = glXChooseVisual (xdisplay, screen_num, (int *) attrib_list);
  if (!xvinfo)
    return NULL;

  GDK_GL_NOTE (MISC, g_message (" -- glXChooseVisual: screen number = %d", xvinfo->screen));
  GDK_GL_NOTE (MISC, g_message (" -- glXChooseVisual: visual id = 0x%lx", xvinfo->visualid));

  /*
   * Instantiate the GdkGLConfigImplX11 object.
   */

  GdkGLConfig *glconfig = g_object_new (GDK_TYPE_GL_CONFIG, NULL);

  glconfig->xdisplay = xdisplay;
  glconfig->screen_num = screen_num;
  glconfig->xvinfo = xvinfo;
  glconfig->screen = screen;

  /* Using Mesa? */
  if (strstr (glXQueryServerString (xdisplay, screen_num, GLX_VERSION), "Mesa"))
    glconfig->is_mesa_glx = TRUE;
  else
    glconfig->is_mesa_glx = FALSE;

  /*
   * Get an appropriate colormap.
   */

  /* RGBA mode? */
  glXGetConfig (xdisplay, xvinfo, GLX_RGBA, &is_rgba);

  glconfig->colormap = gdk_gl_config_setup_colormap (glconfig->screen, glconfig->xvinfo, is_rgba, glconfig->is_mesa_glx);

  GDK_GL_NOTE (MISC, g_message (" -- Colormap: visual id = 0x%lx", GDK_VISUAL_XVISUAL (glconfig->colormap->visual)->visualid));

  /*
   * Init configuration attributes.
   */

  gdk_gl_config_init_attrib (glconfig);

  return glconfig;
}

/**
 * gdk_gl_config_new:
 * @attrib_list: (array): a list of attribute/value pairs. The last attribute must
 *               be GDK_GL_ATTRIB_LIST_NONE.
 *
 * Returns an OpenGL frame buffer configuration that match the specified
 * attributes.
 *
 * attrib_list is a int array that contains the attribute/value pairs.
 * Available attributes are: 
 * GDK_GL_USE_GL, GDK_GL_BUFFER_SIZE, GDK_GL_LEVEL, GDK_GL_RGBA,
 * GDK_GL_DOUBLEBUFFER, GDK_GL_AUX_BUFFERS,
 * GDK_GL_RED_SIZE, GDK_GL_GREEN_SIZE, GDK_GL_BLUE_SIZE, GDK_GL_ALPHA_SIZE,
 * GDK_GL_DEPTH_SIZE, GDK_GL_STENCIL_SIZE, GDK_GL_ACCUM_RED_SIZE,
 * GDK_GL_ACCUM_GREEN_SIZE, GDK_GL_ACCUM_BLUE_SIZE, GDK_GL_ACCUM_ALPHA_SIZE.
 *
 * Return value: the new #GdkGLConfig.
 **/
GdkGLConfig *
gdk_gl_config_new (const int *attrib_list)
{
  GDK_GL_NOTE_FUNC ();

  g_return_val_if_fail (attrib_list != NULL, NULL);

  GdkScreen* screen = gdk_screen_get_default ();

  return gdk_gl_config_new_common (screen, attrib_list);
}

/**
 * gdk_gl_config_new_for_screen:
 * @screen: target screen.
 * @attrib_list: (array): a list of attribute/value pairs. The last attribute must
 *               be GDK_GL_ATTRIB_LIST_NONE.
 *
 * Returns an OpenGL frame buffer configuration that match the specified
 * attributes.
 *
 * Return value: the new #GdkGLConfig.
 **/
GdkGLConfig *
gdk_gl_config_new_for_screen (GdkScreen *screen, const int *attrib_list)
{
  GDK_GL_NOTE_FUNC ();

  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);
  g_return_val_if_fail (attrib_list, NULL);

  return gdk_gl_config_new_common (screen, attrib_list);
}

/*
 * XVisualInfo returned by this function should be freed by XFree ().
 */
static XVisualInfo *
gdk_x11_gl_get_xvinfo (Display  *xdisplay, int       screen_num, VisualID  xvisualid)
{
  XVisualInfo xvinfo_template;
  XVisualInfo *xvinfo_list;
  int nitems_return;

  GDK_GL_NOTE_FUNC_PRIVATE ();

  xvinfo_template.visualid = xvisualid;
  xvinfo_template.screen = screen_num;

  xvinfo_list = XGetVisualInfo (xdisplay, VisualIDMask | VisualScreenMask, &xvinfo_template, &nitems_return);

  /* Returned XVisualInfo needs to be unique */
  g_assert (xvinfo_list != NULL && nitems_return == 1);

  return xvinfo_list;
}

static GdkGLConfig *
gdk_x11_gl_config_new_from_visualid_common (GdkScreen *screen, VisualID   xvisualid)
{
  Display *xdisplay;
  int screen_num;
  XVisualInfo *xvinfo;
  int is_rgba;

  GDK_GL_NOTE_FUNC_PRIVATE ();

  xdisplay = GDK_SCREEN_XDISPLAY (screen);
  screen_num = GDK_SCREEN_XNUMBER (screen);

  GDK_GL_NOTE (MISC, g_message (" -- GLX_VENDOR     : %s", glXGetClientString (xdisplay, GLX_VENDOR)));
  GDK_GL_NOTE (MISC, g_message (" -- GLX_VERSION    : %s", glXGetClientString (xdisplay, GLX_VERSION)));
  GDK_GL_NOTE (MISC, g_message (" -- GLX_EXTENSIONS : %s", glXGetClientString (xdisplay, GLX_EXTENSIONS)));

  /*
   * Get XVisualInfo.
   */

  xvinfo = gdk_x11_gl_get_xvinfo (xdisplay, screen_num, xvisualid);
  if (xvinfo == NULL)
    return NULL;

  GDK_GL_NOTE (MISC, g_message (" -- gdk_x11_gl_get_xvinfo: screen number = %d", xvinfo->screen));
  GDK_GL_NOTE (MISC, g_message (" -- gdk_x11_gl_get_xvinfo: visual id = 0x%lx", xvinfo->visualid));

  /*
   * Instantiate the GdkGLConfigImplX11 object.
   */

  GdkGLConfig* glconfig = g_object_new (GDK_TYPE_GL_CONFIG, NULL);

  glconfig->xdisplay = xdisplay;
  glconfig->screen_num = screen_num;
  glconfig->xvinfo = xvinfo;
  glconfig->screen = screen;

  /* Using Mesa? */
  if (strstr (glXQueryServerString (xdisplay, screen_num, GLX_VERSION), "Mesa"))
    glconfig->is_mesa_glx = TRUE;
  else
    glconfig->is_mesa_glx = FALSE;

  /*
   * Get an appropriate colormap.
   */

  /* RGBA mode? */
  glXGetConfig (xdisplay, xvinfo, GLX_RGBA, &is_rgba);

  glconfig->colormap = gdk_gl_config_setup_colormap (glconfig->screen, glconfig->xvinfo, is_rgba, glconfig->is_mesa_glx);

  GDK_GL_NOTE (MISC, g_message (" -- Colormap: visual id = 0x%lx", GDK_VISUAL_XVISUAL (glconfig->colormap->visual)->visualid));

  /*
   * Init configuration attributes.
   */

  gdk_gl_config_init_attrib (glconfig);

  return glconfig;
}

/**
 * gdk_x11_gl_config_new_from_visualid:
 * @xvisualid: visual ID.
 *
 * Creates #GdkGLConfig from given visual ID that specifies the OpenGL-capable
 * visual.
 *
 * Return value: the new #GdkGLConfig.
 **/
GdkGLConfig *
gdk_x11_gl_config_new_from_visualid (VisualID xvisualid)
{
  GdkScreen *screen;

  GDK_GL_NOTE_FUNC ();

  screen = gdk_screen_get_default ();

  return gdk_x11_gl_config_new_from_visualid_common (screen, xvisualid);
}

/**
 * gdk_x11_gl_config_new_from_visualid_for_screen:
 * @screen: target screen.
 * @xvisualid: visual ID.
 *
 * Creates #GdkGLConfig from given visual ID that specifies the OpenGL-capable
 * visual.
 *
 * Return value: the new #GdkGLConfig.
 **/
GdkGLConfig *
gdk_x11_gl_config_new_from_visualid_for_screen (GdkScreen *screen, VisualID xvisualid)
{
  GDK_GL_NOTE_FUNC ();

  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);

  return gdk_x11_gl_config_new_from_visualid_common (screen, xvisualid);
}

/**
 * gdk_gl_config_get_screen:
 * @glconfig: a #GdkGLConfig.
 *
 * Gets #GdkScreen.
 *
 * Return value: (transfer none): the #GdkScreen.
 **/
GdkScreen *
gdk_gl_config_get_screen (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG(glconfig), NULL);

  return glconfig->screen;
}

/**
 * gdk_gl_config_get_attrib:
 * @glconfig: a #GdkGLConfig.
 * @attribute: the attribute to be returned.
 * @value: (out): returns the requested value.
 *
 * Gets information about a OpenGL frame buffer configuration.
 *
 * Return value: TRUE if it succeeded, FALSE otherwise.
 **/
gboolean
gdk_gl_config_get_attrib (GdkGLConfig *glconfig, int attribute, int *value)
{

  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), FALSE);

  int ret = glXGetConfig (glconfig->xdisplay, glconfig->xvinfo, attribute, value);

  return (ret == Success);
}

/**
 * gdk_gl_config_get_colormap:
 * @glconfig: a #GdkGLConfig.
 *
 * Gets the #GdkColormap that is appropriate for the OpenGL frame buffer
 * configuration.
 *
 * Return value: (transfer none): the appropriate #GdkColormap.
 **/
GdkColormap *
gdk_gl_config_get_colormap (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), NULL);

  return glconfig->colormap;
}

/**
 * gdk_gl_config_get_visual:
 * @glconfig: a #GdkGLConfig.
 *
 * Gets the #GdkVisual that is appropriate for the OpenGL frame buffer
 * configuration.
 *
 * Return value: (transfer none): the appropriate #GdkVisual.
 **/
GdkVisual *
gdk_gl_config_get_visual (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), NULL);

  return gdk_colormap_get_visual(glconfig->colormap);
}

/**
 * gdk_gl_config_get_depth:
 * @glconfig: a #GdkGLConfig.
 *
 * Gets the color depth of the OpenGL-capable visual.
 *
 * Return value: number of bits per pixel
 **/
gint
gdk_gl_config_get_depth (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), 0);

  return glconfig->xvinfo->depth;
}

/**
 * gdk_x11_gl_config_get_xvinfo:
 * @glconfig: a #GdkGLConfig.
 *
 * Gets XVisualInfo data.
 *
 * Return value: (transfer none): pointer to the XVisualInfo data.
 **/
XVisualInfo *
gdk_x11_gl_config_get_xvinfo (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), NULL);

  return glconfig->xvinfo;
}

/**
 * gdk_x11_gl_config_is_mesa_glx:
 * @glconfig: a #GdkGLConfig.
 *
 * Returns whether the server's GLX entension is Mesa.
 *
 * Return value: TRUE if Mesa GLX, FALSE otherwise.
 **/
gboolean
gdk_x11_gl_config_is_mesa_glx (GdkGLConfig *glconfig)
{
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), FALSE);

  return glconfig->is_mesa_glx;
}
