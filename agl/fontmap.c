/* Pango
 * Clutter fonts handling
 *
 * Copyright (C) 2000 Red Hat Software
 * Copyright (C) 2000 Tor Lillqvist
 * Copyright (C) 2006 Marc Lehmann <pcg@goof.com>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define PANGO_ENABLE_BACKEND
#include <pango/pangofc-font.h>
#include <pango/pangofc-fontmap.h>
#include <fontconfig/fontconfig.h>

#include "agl/fontmap.h"
#include "agl/pango_font.h"
#include "agl/pango_render.h"


struct _PangoGlFontMap
{
  PangoFcFontMap parent_instance;

  FT_Library library;

  double dpi;

  /* Function to call on prepared patterns to do final
   * config tweaking.
   */
  PangoGlSubstituteFunc substitute_func;
  gpointer substitute_data;
  GDestroyNotify substitute_destroy;

  PangoRenderer *renderer;
};

struct _PangoGlFontMapClass
{
  PangoFcFontMapClass parent_class;
};

G_DEFINE_TYPE (PangoGlFontMap, pango_gl_font_map, PANGO_TYPE_FC_FONT_MAP)

static void
pango_gl_font_map_finalize (GObject *object)
{
  PangoGlFontMap *fontmap = PANGO_GL_FONT_MAP (object);
  
  if (fontmap->renderer) g_object_unref (fontmap->renderer);

  if (fontmap->substitute_destroy) fontmap->substitute_destroy (fontmap->substitute_data);

  FT_Done_FreeType (fontmap->library);

  G_OBJECT_CLASS (pango_gl_font_map_parent_class)->finalize (object);
}

PangoFontMap *
pango_gl_font_map_new (void)
{
  FT_Error error;
  
  /* Make sure that the type system is initialized */
  g_type_init ();

  PangoGlFontMap *fontmap = g_object_new (PANGO_TYPE_GL_FONT_MAP, NULL);
  
  error = FT_Init_FreeType (&fontmap->library);
  if (error != FT_Err_Ok) g_critical ("pango_gl_font_map_new: Could not initialize freetype");

  return (PangoFontMap *)fontmap;
}

void
pango_gl_font_map_set_default_substitute (PangoGlFontMap *fontmap, PangoGlSubstituteFunc func, gpointer data, GDestroyNotify notify)
{
  if (fontmap->substitute_destroy) fontmap->substitute_destroy (fontmap->substitute_data);

  fontmap->substitute_func = func;
  fontmap->substitute_data = data;
  fontmap->substitute_destroy = notify;
  
  pango_fc_font_map_cache_clear (PANGO_FC_FONT_MAP (fontmap));
}

/**
 * pango_gl_font_map_substitute_changed:
 * @fontmap: a #PangoGlFontmap
 * 
 * Call this function any time the results of the
 * default substitution function set with
 * pango_gl_font_map_set_default_substitute() change.
 * That is, if your subsitution function will return different
 * results for the same input pattern, you must call this function.
 *
 * Since: 1.2
 **/
void
pango_gl_font_map_substitute_changed (PangoGlFontMap *fontmap)
{
  pango_fc_font_map_cache_clear (PANGO_FC_FONT_MAP (fontmap));
}

/**
 * pango_gl_font_map_create_context:
 * @fontmap: a #PangoGlFontmap
 * 
 * Create a #PangoContext for the given fontmap.
 * 
 * Return value: the newly created context; free with g_object_unref().
 *
 * Since: 1.2
 **/
PangoContext *
pango_gl_font_map_create_context (PangoGlFontMap *fontmap)
{
  g_return_val_if_fail (PANGO_GL_IS_FONT_MAP (fontmap), NULL);
  
  return pango_fc_font_map_create_context (PANGO_FC_FONT_MAP (fontmap));
}

FT_Library
_pango_gl_font_map_get_library (PangoFontMap *fontmap_)
{
  PangoGlFontMap *fontmap = (PangoGlFontMap *)fontmap_;
  
  return fontmap->library;
}

void
pango_gl_font_map_set_resolution (PangoGlFontMap *fontmap, double dpi)
{
  g_return_if_fail (PANGO_GL_IS_FONT_MAP (fontmap));

  fontmap->dpi = dpi;

  pango_gl_font_map_substitute_changed (fontmap);
}

/**
 * _pango_gl_font_map_get_renderer:
 * @fontmap: a #PangoGlFontmap
 * 
 * Gets the singleton PangoGlRenderer for this fontmap.
 * 
 * Return value: 
 **/
PangoRenderer *
pango_gl_font_map_get_renderer (PangoGlFontMap *fontmap)
{
  if (!fontmap->renderer)
    fontmap->renderer = g_object_new (PANGO_TYPE_GL_RENDERER, NULL);

  return fontmap->renderer;
}

static void
pango_gl_font_map_default_substitute (PangoFcFontMap *fcfontmap, FcPattern *pattern)
{
  PangoGlFontMap *fontmap = PANGO_GL_FONT_MAP (fcfontmap);

  FcConfigSubstitute (NULL, pattern, FcMatchPattern);

  if (fontmap->substitute_func)
    fontmap->substitute_func (pattern, fontmap->substitute_data);

#if 0
  FcValue v;
  if (FcPatternGet (pattern, FC_DPI, 0, &v) == FcResultNoMatch)
    FcPatternAddDouble (pattern, FC_DPI, fontmap->dpi_y);
#endif

   /* Turn off hinting, since we most of the time are not using the glyphs
    * from our cache at their nativly rendered resolution
    */
  FcPatternDel (pattern, FC_HINTING);
  FcPatternAddBool (pattern, FC_HINTING, FALSE);

  FcDefaultSubstitute (pattern);
}

static PangoFcFont*
pango_gl_font_map_new_font (PangoFcFontMap *fcfontmap, FcPattern *pattern)
{
  return (PangoFcFont *)_pango_gl_font_new (PANGO_GL_FONT_MAP (fcfontmap), pattern);
}

static double
pango_gl_font_map_get_resolution (PangoFcFontMap *fcfontmap, PangoContext *context)
{
  return ((PangoGlFontMap *)fcfontmap)->dpi;
}

static void
pango_gl_font_map_class_init (PangoGlFontMapClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  PangoFcFontMapClass *fcfontmap_class = PANGO_FC_FONT_MAP_CLASS (class);
  
  gobject_class->finalize = pango_gl_font_map_finalize;
  fcfontmap_class->default_substitute = pango_gl_font_map_default_substitute;
  fcfontmap_class->new_font = pango_gl_font_map_new_font;
  fcfontmap_class->get_resolution = pango_gl_font_map_get_resolution;
}

static void
pango_gl_font_map_init (PangoGlFontMap *fontmap)
{
  fontmap->library = NULL;
  fontmap->dpi = 96.0;
}

