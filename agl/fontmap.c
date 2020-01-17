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

#include "config.h"
#include <glib.h>
#include <string.h>
#include <errno.h>

#define PANGO_ENABLE_BACKEND
#include <pango/pangofc-font.h>
#include <pango/pangofc-fontmap.h>
#include <fontconfig/fontconfig.h>

#ifdef HAVE_PANGO_1_44
#include "pango/pangofc-fontmap-private.h"
#endif

#include "agl/fontmap.h"
#include "agl/pango_font.h"
#include "agl/pango_render.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static GQuark pango_gl_font_map_get_priv_key ();

struct _PangoGlFontMap
{
  PangoFcFontMap parent_instance;
};

struct _PangoGlFontMapClass
{
   PangoFcFontMapClass parent_class;
};

G_DEFINE_TYPE (PangoGlFontMap, pango_gl_font_map, PANGO_TYPE_FC_FONT_MAP)

/*
 *  This will be attached to a standard PangoFontMap as qdata
 */
typedef struct
{
   FT_Library library;

   double dpi;

   PangoGlSubstituteFunc substitute_func; // function to call on prepared patterns to do final config tweaking.
   gpointer              substitute_data;
   GDestroyNotify        substitute_destroy;

   PangoRenderer* renderer;

} PangoGlFontMapPriv;


PangoFontMap*
pango_gl_font_map_new ()
{
	PangoGlFontMap* fontmap = g_object_new (PANGO_TYPE_GL_FONT_MAP, NULL);
  
	PangoGlFontMapPriv* priv = g_new0(PangoGlFontMapPriv, 1);
	priv->dpi = 96.0;

	FT_Error error = FT_Init_FreeType(&priv->library);
	if (error != FT_Err_Ok) g_critical ("pango_gl_font_map_new: Could not initialize freetype");

	g_object_set_qdata_full(G_OBJECT(fontmap), pango_gl_font_map_get_priv_key (), priv, g_free);

	return (PangoFontMap *)fontmap;
}


static PangoGlFontMapPriv*
_pango_gl_font_map_get_priv (PangoGlFontMap* fontmap)
{
  return g_object_get_qdata(G_OBJECT(fontmap), pango_gl_font_map_get_priv_key());
}


static void
pango_gl_font_map_finalize (GObject* object)
{
	PangoGlFontMap* fontmap = PANGO_GL_FONT_MAP(object);
  
	PangoGlFontMapPriv* priv = _pango_gl_font_map_get_priv(fontmap);

	if (priv->renderer) g_object_unref (priv->renderer);

	if (priv->substitute_destroy) priv->substitute_destroy (priv->substitute_data);

	FT_Done_FreeType (priv->library);

	G_OBJECT_CLASS (pango_gl_font_map_parent_class)->finalize (object);
}


void
pango_gl_font_map_set_default_substitute (PangoGlFontMap* fontmap, PangoGlSubstituteFunc func, gpointer data, GDestroyNotify notify)
{
	PangoGlFontMapPriv* priv = _pango_gl_font_map_get_priv(fontmap);

	if (priv->substitute_destroy) priv->substitute_destroy (priv->substitute_data);

	priv->substitute_func = func;
	priv->substitute_data = data;
	priv->substitute_destroy = notify;
  
	pango_fc_font_map_cache_clear(PANGO_FC_FONT_MAP(fontmap));
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
 **/
void
pango_gl_font_map_substitute_changed (PangoGlFontMap* fontmap)
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
 **/
PangoContext*
pango_gl_font_map_create_context (PangoGlFontMap *fontmap)
{
	g_return_val_if_fail (PANGO_GL_IS_FONT_MAP (fontmap), NULL);
  
	return pango_fc_font_map_create_context (PANGO_FC_FONT_MAP (fontmap));
}


FT_Library
_pango_gl_font_map_get_library (PangoFontMap* fontmap_)
{
	PangoGlFontMap* fontmap = (PangoGlFontMap*)fontmap_;
  
	PangoGlFontMapPriv* priv = _pango_gl_font_map_get_priv(fontmap);

	return priv->library;
}


void
pango_gl_font_map_set_resolution (PangoGlFontMap* fontmap, double dpi)
{
	g_return_if_fail (PANGO_GL_IS_FONT_MAP (fontmap));

	PangoGlFontMapPriv* priv = _pango_gl_font_map_get_priv(fontmap);

	priv->dpi = dpi;

	pango_gl_font_map_substitute_changed (fontmap);
}


/**
 * pango_gl_font_map_get_renderer:
 * @fontmap: a #PangoGlFontmap
 * 
 * Gets the singleton PangoGlRenderer for this fontmap.
 * 
 * Return value: 
 **/
PangoRenderer*
pango_gl_font_map_get_renderer (PangoGlFontMap* fontmap)
{
	PangoGlFontMapPriv* priv = _pango_gl_font_map_get_priv(fontmap);

	if (!priv->renderer)
		priv->renderer = g_object_new (PANGO_TYPE_GL_RENDERER, NULL);

	return priv->renderer;
}


static void
pango_gl_font_map_default_substitute (PangoFcFontMap* fcfontmap, FcPattern* pattern)
{
	PangoGlFontMap* fontmap = PANGO_GL_FONT_MAP (fcfontmap);
	PangoGlFontMapPriv* priv = _pango_gl_font_map_get_priv(fontmap);

	FcConfigSubstitute (NULL, pattern, FcMatchPattern);

	if (priv->substitute_func)
		priv->substitute_func (pattern, priv->substitute_data);

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
pango_gl_font_map_get_resolution (PangoFcFontMap* fcfontmap, PangoContext* context)
{
	PangoGlFontMapPriv* priv = _pango_gl_font_map_get_priv((PangoGlFontMap*)fcfontmap);

	return priv->dpi;
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
}


static GQuark
pango_gl_font_map_get_priv_key ()
{
	static GQuark priv_key = 0;

	if (G_UNLIKELY (priv_key == 0))
		priv_key = g_quark_from_static_string ("PangoGlFontMap");

	return priv_key;
}
