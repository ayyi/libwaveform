/*
 * pango_gl.h: OpenGl/Freetype2 backend
 *
 * Copyright (C) 1999 Red Hat Software
 * Copyright (C) 2000 Tor Lillqvist
 * Copyright (C) 2006 Marc Lehmann <pcg@goof.com>
 *
 */
#ifndef PANGOCLUTTER_H__
#define PANGOCLUTTER_H__

#define PANGO_ENABLE_BACKEND

/* we always want to disable cast checks */
#ifndef G_DISABLE_CAST_CHECKS
#define G_DISABLE_CAST_CHECKS
#endif

#include <glib-object.h>
#include <pango/pango.h>
#include <fontconfig/fontconfig.h>
//#include <clutter/clutter-color.h>

G_BEGIN_DECLS

#define PANGO_TYPE_GL_FONT_MAP       (pango_gl_font_map_get_type ())
#define PANGO_GL_FONT_MAP(object)    (G_TYPE_CHECK_INSTANCE_CAST ((object), PANGO_TYPE_GL_FONT_MAP, PangoGlFontMap))
#define PANGO_GL_IS_FONT_MAP(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), PANGO_TYPE_GL_FONT_MAP))

typedef struct _PangoGlFontMap      PangoGlFontMap;
typedef struct _PangoGlFontMapClass PangoGlFontMapClass;

typedef void   (*PangoGlSubstituteFunc)                 (FcPattern*, gpointer data);

GType          pango_gl_font_map_get_type               ();

PangoFontMap*  pango_gl_font_map_new                    ();
void           pango_gl_font_map_set_default_substitute (PangoGlFontMap*, PangoGlSubstituteFunc, gpointer data, GDestroyNotify notify);
void           pango_gl_font_map_set_resolution         (PangoGlFontMap*, double dpi);
void           pango_gl_font_map_substitute_changed     (PangoGlFontMap*);
PangoContext*  pango_gl_font_map_create_context         (PangoGlFontMap*);
PangoRenderer* pango_gl_font_map_get_renderer           (PangoGlFontMap*);
FT_Library    _pango_gl_font_map_get_library            (PangoFontMap*);

G_END_DECLS

#endif
