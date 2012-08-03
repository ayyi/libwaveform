
#define PANGO_GL_FONT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_GL_FONT, PangoGlFontClass))
#define PANGO_GL_IS_FONT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_GL_FONT))
#define PANGO_GL_FONT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_GL_FONT, PangoGlFontClass))

#define PANGO_TYPE_GL_FONT       (pango_gl_font_get_type ())
#define PANGO_GL_FONT(object)    (G_TYPE_CHECK_INSTANCE_CAST ((object), PANGO_TYPE_GL_FONT, PangoGlFont))
#define PANGO_GL_IS_FONT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), PANGO_TYPE_GL_FONT))

#define PANGO_SCALE_26_6     (PANGO_SCALE / (1<<6))
#define PANGO_UNITS_26_6(d)  (PANGO_SCALE_26_6 * (d))
#define PANGO_PIXELS_26_6(d) (d / PANGO_SCALE_26_6)

typedef struct _PangoGlFont       PangoGlFont;
typedef struct _PangoClutterGlyphInfo  PangoClutterGlyphInfo;

struct _PangoGlFont
{
  PangoFcFont    font;
  FT_Face        face;
  int            load_flags;
  int            size;
  GSList        *metrics_by_lang;
  GHashTable    *glyph_info;
  GDestroyNotify glyph_cache_destroy;
};

struct _PangoClutterGlyphInfo
{
  PangoRectangle logical_rect;
  PangoRectangle ink_rect;
  void          *cached_glyph;
};


GType         pango_gl_font_get_type (void);
PangoGlFont* _pango_gl_font_new                    (PangoGlFontMap*, FcPattern*);
FT_Face       pango_gl_font_get_face               (PangoFont*);
PangoGlyph    pango_gl_get_unknown_glyph           (PangoFont*);
void*        _pango_gl_font_get_cache_glyph_data   (PangoFont*, int glyph_index);
void         _pango_gl_font_set_glyph_cache_destroy(PangoFont*, GDestroyNotify destroy_notify);
void         _pango_gl_font_set_cache_glyph_data   (PangoFont*, int glyph_index, void* cached_glyph);

/* HACK make this public to avoid a mass of re-implementation*/
void          pango_fc_font_get_raw_extents        (PangoFcFont*, FT_Int32 load_flags, PangoGlyph      glyph, PangoRectangle *ink_rect, PangoRectangle *logical_rect);

