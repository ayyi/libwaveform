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

#include <stdlib.h>
#include <string.h>

#include "gdkglx.h"
#include "gdkglprivate.h"
#include "gdkglquery.h"

/*
 * This code is based on glutExtensionSupported().
 */

/**
 * gdk_gl_query_gl_extension:
 * @extension: name of OpenGL extension.
 *
 * Determines whether a given OpenGL extension is supported.
 *
 * There must be a valid current rendering context to call
 * gdk_gl_query_gl_extension().
 *
 * gdk_gl_query_gl_extension() returns information about OpenGL extensions
 * only. This means that window system dependent extensions (for example,
 * GLX extensions) are not reported by gdk_gl_query_gl_extension().
 *
 * Return value: TRUE if the OpenGL extension is supported, FALSE if not 
 *               supported.
 **/
gboolean
gdk_gl_query_gl_extension (const char *extension)
{
  static const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;

  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr (extension, ' ');
  if (where || *extension == '\0')
    return FALSE;

  if (extensions == NULL)
    extensions = glGetString (GL_EXTENSIONS);

  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string.  Don't be fooled by sub-strings,
     etc. */
  start = extensions;
  for (;;)
    {
      /* If your application crashes in the strstr routine below,
         you are probably calling gdk_gl_query_gl_extension without
         having a current window.  Calling glGetString without
         a current OpenGL context has unpredictable results.
         Please fix your program. */
      where = (GLubyte *) strstr ((const char *) start, extension);
      if (where == NULL)
        break;

      terminator = where + strlen (extension);

      if (where == start || *(where - 1) == ' ')
        if (*terminator == ' ' || *terminator == '\0')
          {
            GDK_GL_NOTE (MISC, g_message (" - %s - supported", extension));
            return TRUE;
          }

      start = terminator;
    }

  GDK_GL_NOTE (MISC, g_message (" - %s - not supported", extension));

  return FALSE;
}

/*< private >*/
void
_gdk_gl_print_gl_info (void)
{
  static gboolean done = FALSE;

  if (!done)
    {
      g_message (" -- GL_VENDOR     : %s", glGetString (GL_VENDOR));
      g_message (" -- GL_RENDERER   : %s", glGetString (GL_RENDERER));
      g_message (" -- GL_VERSION    : %s", glGetString (GL_VERSION));
      g_message (" -- GL_EXTENSIONS : %s", glGetString (GL_EXTENSIONS));

      done = TRUE;
    }
}

/**
 * gdk_gl_query_extension:
 *
 * Indicates whether the window system supports the OpenGL extension
 * (GLX, WGL, etc.).
 *
 * Return value: TRUE if OpenGL is supported, FALSE otherwise.
 **/
gboolean
gdk_gl_query_extension (void)
{
	return glXQueryExtension (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), NULL, NULL);
}

/*
 * This code is based on __glutIsSupportedByGLX().
 */

/**
 * gdk_x11_gl_query_glx_extension:
 * @glconfig: a #GdkGLConfig.
 * @extension: name of GLX extension.
 *
 * Determines whether a given GLX extension is supported.
 *
 * Return value: TRUE if the GLX extension is supported, FALSE if not
 *               supported.
 **/
gboolean
gdk_x11_gl_query_glx_extension (GdkGLConfig *glconfig, const char  *extension)
{
  static const char *extensions = NULL;
  const char *start;
  char *where, *terminator;
  int major, minor;

  g_return_val_if_fail (GDK_IS_GL_CONFIG(glconfig), FALSE);

  /* Extension names should not have spaces. */
  where = strchr (extension, ' ');
  if (where || *extension == '\0')
    return FALSE;

  if (extensions == NULL)
    {
      /* Be careful not to call glXQueryExtensionsString if it
         looks like the server doesn't support GLX 1.1.
         Unfortunately, the original GLX 1.0 didn't have the notion
         of GLX extensions. */

      glXQueryVersion (GDK_GL_CONFIG_XDISPLAY (glconfig), &major, &minor);

      if ((major == 1 && minor < 1) || (major < 1))
        return FALSE;

      int screen_num = GDK_GL_CONFIG(glconfig)->screen_num;
      extensions = glXQueryExtensionsString (GDK_GL_CONFIG_XDISPLAY (glconfig), screen_num);
    }

  /* It takes a bit of care to be fool-proof about parsing
     the GLX extensions string.  Don't be fooled by
     sub-strings,  etc. */
  start = extensions;
  for (;;)
    {
      where = strstr (start, extension);
      if (where == NULL)
        break;

      terminator = where + strlen (extension);

      if (where == start || *(where - 1) == ' ')
        if (*terminator == ' ' || *terminator == '\0')
          {
            GDK_GL_NOTE (MISC, g_message (" - %s - supported", extension));
            return TRUE;
          }

      start = terminator;
    }

  GDK_GL_NOTE (MISC, g_message (" - %s - not supported", extension));

  return FALSE;
}

/**
 * gdk_gl_get_proc_address:
 * @proc_name: function name.
 *
 * Returns the address of the OpenGL, GLU, or GLX function.
 *
 * Return value: (type gpointer) (transfer none): the address of the function named by @proc_name.
 **/

GdkGLProc
gdk_gl_get_proc_address (const char *proc_name)
{
  typedef GdkGLProc (*__glXGetProcAddressProc) (const GLubyte *);
  static __glXGetProcAddressProc glx_get_proc_address = (__glXGetProcAddressProc) -1;
  gchar *file_name;
  GModule *module;
  GdkGLProc proc_address = NULL;

  GDK_GL_NOTE_FUNC ();

  if (glx_get_proc_address == (__glXGetProcAddressProc) -1) {
      /*
       * Look up glXGetProcAddress () function.
       */

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
      file_name = g_module_build_path (NULL, "GL");
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
      GDK_GL_NOTE (MISC, g_message (" - Open %s", file_name));
      module = g_module_open (file_name, G_MODULE_BIND_LAZY);
      g_free (file_name);

      if (module) {
          g_module_symbol (module, "glXGetProcAddress", (gpointer) &glx_get_proc_address);
          if (glx_get_proc_address == NULL) {
            g_module_symbol (module, "glXGetProcAddressARB", (gpointer) &glx_get_proc_address);
            if (glx_get_proc_address == NULL) {
              g_module_symbol (module, "glXGetProcAddressEXT", (gpointer) &glx_get_proc_address);
            }
          }
          GDK_GL_NOTE (MISC, g_message (" - glXGetProcAddress () - %s", glx_get_proc_address ? "supported" : "not supported"));
          g_module_close (module);
      } else {
        g_warning ("Cannot open %s", file_name);
        glx_get_proc_address = NULL;
        return NULL;
      }
  }

  /* Try glXGetProcAddress () */

  if (glx_get_proc_address) {
    proc_address = glx_get_proc_address ((unsigned char *) proc_name);
    GDK_GL_NOTE (IMPL, g_message (" ** glXGetProcAddress () - %s", proc_address ? "succeeded" : "failed"));
    if (proc_address)
      return proc_address;
  }

  /* Try g_module_symbol () */

  /* libGL */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  file_name = g_module_build_path (NULL, "GL");
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  GDK_GL_NOTE (MISC, g_message (" - Open %s", file_name));
  module = g_module_open (file_name, G_MODULE_BIND_LAZY);
  g_free (file_name);

  if (module) {
    g_module_symbol (module, proc_name, (gpointer) &proc_address);
    GDK_GL_NOTE (MISC, g_message (" - g_module_symbol () - %s", proc_address ? "succeeded" : "failed"));
    g_module_close (module);
  } else {
    g_warning ("Cannot open %s", file_name);
  }

  if (!proc_address) {
      /* libGLcore */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
      file_name = g_module_build_path (NULL, "GLcore");
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
      GDK_GL_NOTE (MISC, g_message (" - Open %s", file_name));
      module = g_module_open (file_name, G_MODULE_BIND_LAZY);
      g_free (file_name);

      if (module) {
        g_module_symbol (module, proc_name, (gpointer) &proc_address);
        GDK_GL_NOTE (MISC, g_message (" - g_module_symbol () - %s", proc_address ? "succeeded" : "failed"));
        g_module_close (module);
      }
  }

  return proc_address;
}


void
_gdk_gl_print_glx_info (Display *xdisplay, int screen_num)
{
	static gboolean done = FALSE;

	if (!done) {
		g_message (" -- Server GLX_VENDOR     : %s", glXQueryServerString (xdisplay, screen_num, GLX_VENDOR));
		g_message (" -- Server GLX_VERSION    : %s", glXQueryServerString (xdisplay, screen_num, GLX_VERSION));
		g_message (" -- Server GLX_EXTENSIONS : %s", glXQueryServerString (xdisplay, screen_num, GLX_EXTENSIONS));

		g_message (" -- Client GLX_VENDOR     : %s", glXGetClientString (xdisplay, GLX_VENDOR));
		g_message (" -- Client GLX_VERSION    : %s", glXGetClientString (xdisplay, GLX_VERSION));
		g_message (" -- Client GLX_EXTENSIONS : %s", glXGetClientString (xdisplay, GLX_EXTENSIONS));

		done = TRUE;
	}
}
