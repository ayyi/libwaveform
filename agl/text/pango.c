/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | Copyright (C) 2020-2020 Tim Orford <tim@orford.org>                  |
* | Copyright (C) 2017 Red Hat, Inc.                                     |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "config.h"
#include <math.h>
#include <pango/pango.h>
#include "agl/debug.h"
#include "agl/ext.h"
#include "text/enums.h"
#include "text/text_node.h"
#include "text/glyphcache.h"
#include "text/renderops.h"
#include "text/shaderbuilder.h"
#include "text/renderer.h"
#include "text/pango.h"

#define GSK_PANGO_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GSK_TYPE_PANGO_RENDERER, GskPangoRendererClass))
#define GSK_IS_PANGO_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSK_TYPE_PANGO_RENDERER))
#define GSK_PANGO_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GSK_TYPE_PANGO_RENDERER, GskPangoRendererClass))

#define SHADER_VERSION_GLES        100
#define SHADER_VERSION_GL2_LEGACY  110
#define SHADER_VERSION_GL3_LEGACY  130
#define SHADER_VERSION_GL3         150

/*
 * This is a PangoRenderer implementation that translates all the draw calls to
 * gsk render nodes, using the GtkSnapshot helper class. Glyphs are translated
 * to text nodes, all other draw calls fall back to cairo nodes.
 */

struct _GskPangoRenderer
{
   PangoRenderer parent_instance;

   GdkRGBA fg_color;
   graphene_rect_t bounds;

   /* house-keeping options */
   gboolean is_cached_renderer;
};

struct _GskPangoRendererClass
{
   PangoRendererClass parent_class;

   PangoContext* context;
   GskGLTextureAtlases* atlases;
   GskGLGlyphCache* glyph_cache;
};

G_DEFINE_TYPE (GskPangoRenderer, gsk_pango_renderer, PANGO_TYPE_RENDERER)


static GskGLTextureAtlases*
get_texture_atlases_for_display ()
{
	if (g_getenv ("GSK_NO_SHARED_CACHES"))
		return gsk_gl_texture_atlases_new ();

	GskGLTextureAtlases* atlases = gsk_gl_texture_atlases_new ();

	return gsk_gl_texture_atlases_ref (atlases);
}


#if 0
static void
get_color (GskPangoRenderer* crenderer, PangoRenderPart part, GdkRGBA* rgba)
{
	PangoColor* color = pango_renderer_get_color ((PangoRenderer*)crenderer, part);
	guint16 a = pango_renderer_get_alpha ((PangoRenderer*)crenderer, part);

	gdouble red = crenderer->fg_color.red;
	gdouble green = crenderer->fg_color.green;
	gdouble blue = crenderer->fg_color.blue;
	gdouble alpha = crenderer->fg_color.alpha;

	if (color) {
		red = color->red / 65535.;
		green = color->green / 65535.;
		blue = color->blue / 65535.;
		alpha = 1.;
	}

	if (a)
		alpha = a / 65535.;

	*rgba = (GdkRGBA){
		.red = red,
		.green = green,
		.blue = blue,
		.alpha = alpha
	};
}


static void
set_color (GskPangoRenderer* crenderer, PangoRenderPart part, cairo_t* cr)
{
	GdkRGBA rgba = { 0, 0, 0, 1 };

	get_color (crenderer, part, &rgba);
	gdk_cairo_set_source_rgba (cr, &rgba);
}
#endif


static void
agl_pango_renderer_show_text_glyphs (PangoRenderer        *prenderer,
                                     const char           *text,
                                     int                   text_len,
                                     PangoGlyphString     *glyphs,
                                     cairo_text_cluster_t *clusters,
                                     int                   num_clusters,
                                     gboolean              backward,
                                     PangoFont            *font,
                                     int                   x,
                                     int                   y)
{
	GskPangoRendererClass* GPRC = g_type_class_peek(GSK_TYPE_PANGO_RENDERER);

	y /= PANGO_SCALE;

	ops_set_program (renderer.current_builder, &renderer.coloring_program);
#if 0
	GdkRGBA color;
	get_color ((GskPangoRenderer*)prenderer, PANGO_RENDER_PART_FOREGROUND, &color);

	ops_set_color (renderer.current_builder, &color);
#else
	ops_set_color (renderer.current_builder, &((GskPangoRenderer*)prenderer)->fg_color);
#endif

	const float text_scale = ops_get_scale(builder());

	GlyphCacheKey lookup = {
		.data = {
			.font = (PangoFont*)font,
			.scale = (guint)(text_scale * 1024)
		}
	};

	// render code from TextNode
	int x_position = 0;

	for (int i = 0; i < glyphs->num_glyphs; i++){
		const PangoGlyphInfo* gi = &glyphs->glyphs[i];

		if (gi->glyph == PANGO_GLYPH_EMPTY)
			continue;

		float cx = (float)(x_position + gi->geometry.x_offset) / PANGO_SCALE;
		float cy = (float)(gi->geometry.y_offset) / PANGO_SCALE;

		glyph_cache_key_set_glyph_and_shift (&lookup, gi->glyph, x + cx, y + cy);

		const GskGLCachedGlyph* glyph;
		gsk_gl_glyph_cache_lookup_or_add(GPRC->glyph_cache, &lookup, &glyph);

		if (glyph->texture_id == 0)
			goto next;

		ops_set_texture (renderer.current_builder, glyph->texture_id);

		float tx  = glyph->tx;
		float ty  = glyph->ty;
		float tx2 = tx + glyph->tw;
		float ty2 = ty + glyph->th;

		float glyph_x = floor(x + cx + 0.125) + glyph->draw_x;
		float glyph_y = floor(y + cy + 0.125) + glyph->draw_y;
		float glyph_x2 = glyph_x + glyph->draw_width;
		float glyph_y2 = glyph_y + glyph->draw_height;

		ops_draw (renderer.current_builder, (GskQuadVertex[GL_N_VERTICES]) {
			{ { glyph_x,  glyph_y  }, { tx,  ty  }, },
			{ { glyph_x,  glyph_y2 }, { tx,  ty2 }, },
			{ { glyph_x2, glyph_y  }, { tx2, ty  }, },

			{ { glyph_x2, glyph_y2 }, { tx2, ty2 }, },
			{ { glyph_x,  glyph_y2 }, { tx,  ty2 }, },
			{ { glyph_x2, glyph_y  }, { tx2, ty  }, },
		});
	  next:
		x_position += gi->geometry.width;
	}
}


static void
agl_pango_renderer_draw_glyphs (PangoRenderer* renderer, PangoFont* font, PangoGlyphString* glyphs, int x, int y)
{
	agl_pango_renderer_show_text_glyphs (renderer, NULL, 0, glyphs, NULL, 0, FALSE, font, x, y);
}


static void
agl_pango_renderer_draw_glyph_item (PangoRenderer  *renderer,
                                    const char     *text,
                                    PangoGlyphItem *glyph_item,
                                    int             x,
                                    int             y)
{
	PangoFont* font = glyph_item->item->analysis.font;
	PangoGlyphString* glyphs = glyph_item->glyphs;

	agl_pango_renderer_show_text_glyphs (renderer, NULL, 0, glyphs, NULL, 0, FALSE, font, x, y);
}


static void
gsk_pango_renderer_draw_rectangle (PangoRenderer     *renderer,
                                   PangoRenderPart    part,
                                   int                x,
                                   int                y,
                                   int                width,
                                   int                height)
{
#if 0
	GskPangoRenderer *crenderer = (GskPangoRenderer *) (renderer);
	GdkRGBA rgba;
	graphene_rect_t bounds;

	get_color (crenderer, part, &rgba);

	graphene_rect_init (
		&bounds,
		(double)x / PANGO_SCALE, (double)y / PANGO_SCALE,
		(double)width / PANGO_SCALE, (double)height / PANGO_SCALE
	);

	gtk_snapshot_append_color (crenderer->snapshot, &rgba, &bounds, "DrawRectangle");
#endif
}

static void
gsk_pango_renderer_draw_trapezoid (PangoRenderer   *renderer,
                                   PangoRenderPart  part,
                                   double           y1_,
                                   double           x11,
                                   double           x21,
                                   double           y2,
                                   double           x12,
                                   double           x22)
{
#if 0
  GskPangoRenderer *crenderer = (GskPangoRenderer *) (renderer);
  gdouble x, y;

  cairo_t* cr = gtk_snapshot_append_cairo (crenderer->snapshot, &crenderer->bounds, "DrawTrapezoid");

  set_color (crenderer, part, cr);

  x = y = 0;
  cairo_user_to_device_distance (cr, &x, &y);
  cairo_identity_matrix (cr);
  cairo_translate (cr, x, y);

  cairo_move_to (cr, x11, y1_);
  cairo_line_to (cr, x21, y1_);
  cairo_line_to (cr, x22, y2);
  cairo_line_to (cr, x12, y2);
  cairo_close_path (cr);

  cairo_fill (cr);

  cairo_destroy (cr);
#endif
}

#if 0
/* Draws an error underline that looks like one of:
 *              H       E                H
 *     /\      /\      /\        /\      /\               -
 *   A/  \    /  \    /  \     A/  \    /  \              |
 *    \   \  /    \  /   /D     \   \  /    \             |
 *     \   \/  C   \/   /        \   \/   C  \            | height = HEIGHT_SQUARES * square
 *      \      /\  F   /          \  F   /\   \           |
 *       \    /  \    /            \    /  \   \G         |
 *        \  /    \  /              \  /    \  /          |
 *         \/      \/                \/      \/           -
 *         B                         B
 *         |---|
 *       unit_width = (HEIGHT_SQUARES - 1) * square
 *
 * The x, y, width, height passed in give the desired bounding box;
 * x/width are adjusted to make the underline a integer number of units
 * wide.
 */
#define HEIGHT_SQUARES 2.5

static void
draw_error_underline (cairo_t *cr, double x, double y, double width, double height)
{
  double square = height / HEIGHT_SQUARES;
  double unit_width = (HEIGHT_SQUARES - 1) * square;
  double double_width = 2 * unit_width;
  int width_units = (width + unit_width / 2) / unit_width;
  double y_top, y_bottom;
  double x_left, x_middle, x_right;
  int i;

  x += (width - width_units * unit_width) / 2;

  y_top = y;
  y_bottom = y + height;

  /* Bottom of squiggle */
  x_middle = x + unit_width;
  x_right  = x + double_width;
  cairo_move_to (cr, x - square / 2, y_top + square / 2); /* A */
  for (i = 0; i < width_units-2; i += 2)
    {
      cairo_line_to (cr, x_middle, y_bottom); /* B */
      cairo_line_to (cr, x_right, y_top + square); /* C */

      x_middle += double_width;
      x_right  += double_width;
    }
  cairo_line_to (cr, x_middle, y_bottom); /* B */

  if (i + 1 == width_units)
    cairo_line_to (cr, x_middle + square / 2, y_bottom - square / 2); /* G */
  else if (i + 2 == width_units) {
    cairo_line_to (cr, x_right + square / 2, y_top + square / 2); /* D */
    cairo_line_to (cr, x_right, y_top); /* E */
  }

  // Top of squiggle
  x_left = x_middle - unit_width;
  for (; i >= 0; i -= 2)
    {
      cairo_line_to (cr, x_middle, y_bottom - square); /* F */
      cairo_line_to (cr, x_left, y_top);   /* H */

      x_left   -= double_width;
      x_middle -= double_width;
    }
}
#endif


static void
gsk_pango_renderer_draw_error_underline (PangoRenderer* renderer, int x, int y, int width, int height)
{
#if 0
	GskPangoRenderer* crenderer = (GskPangoRenderer*)renderer;

	cairo_t* cr = gtk_snapshot_append_cairo (crenderer->snapshot, &crenderer->bounds, "DrawTrapezoid");

	set_color (crenderer, PANGO_RENDER_PART_UNDERLINE, cr);

	cairo_new_path (cr);

	draw_error_underline(
		cr,
		(double)x / PANGO_SCALE, (double)y / PANGO_SCALE,
		(double)width / PANGO_SCALE, (double)height / PANGO_SCALE
	);

	cairo_fill (cr);

	cairo_destroy (cr);
#endif
}


static void
gsk_pango_renderer_draw_shape (PangoRenderer* renderer, PangoAttrShape* attr, int x, int y)
{
#if 0
	GskPangoRenderer *crenderer = (GskPangoRenderer *) (renderer);
	PangoLayout *layout;
	PangoCairoShapeRendererFunc shape_renderer;
	gpointer shape_renderer_data;
	double base_x = (double)x / PANGO_SCALE;
	double base_y = (double)y / PANGO_SCALE;

	cairo_t* cr = gtk_snapshot_append_cairo (crenderer->snapshot, &crenderer->bounds, "DrawShape");

	layout = pango_renderer_get_layout (renderer);
	if (!layout)
		return;

	shape_renderer = pango_cairo_context_get_shape_renderer (pango_layout_get_context (layout), &shape_renderer_data);

	if (!shape_renderer)
		return;

	set_color (crenderer, PANGO_RENDER_PART_FOREGROUND, cr);

	cairo_move_to (cr, base_x, base_y);

	shape_renderer (cr, attr, FALSE, shape_renderer_data);

	cairo_destroy (cr);
#endif
}


static void
gsk_pango_renderer_init (GskPangoRenderer* crenderer)
{
}


static void
gsk_pango_renderer_class_init (GskPangoRendererClass* klass)
{
	PF;

	PangoRendererClass* renderer_class = PANGO_RENDERER_CLASS (klass);

	renderer_class->draw_glyphs = agl_pango_renderer_draw_glyphs;
	renderer_class->draw_glyph_item = agl_pango_renderer_draw_glyph_item;
	renderer_class->draw_rectangle = gsk_pango_renderer_draw_rectangle;
	renderer_class->draw_trapezoid = gsk_pango_renderer_draw_trapezoid;
	renderer_class->draw_error_underline = gsk_pango_renderer_draw_error_underline;
	renderer_class->draw_shape = gsk_pango_renderer_draw_shape;

	PangoFontMap* font_map = pango_cairo_font_map_get_default();
	klass->context = pango_font_map_create_context(font_map);

	klass->atlases = get_texture_atlases_for_display();
	klass->glyph_cache = gsk_gl_glyph_cache_new(klass->atlases);

	GError* error = NULL;
	if(!renderer_create_programs(&error)){
		printf("%s\n", error->message);
		g_error_free(error);
	}
}


static GskPangoRenderer* cached_renderer = NULL; /* MT-safe */
G_LOCK_DEFINE_STATIC (cached_renderer);

/*
 *  This can probably be removed
 */
static GskPangoRenderer*
acquire_renderer (void)
{
	GskPangoRenderer* renderer;

	if (G_LIKELY (G_TRYLOCK (cached_renderer))) {
		if (G_UNLIKELY (!cached_renderer)) {
			cached_renderer = g_object_new (GSK_TYPE_PANGO_RENDERER, NULL);
			cached_renderer->is_cached_renderer = TRUE;
		}

		renderer = cached_renderer;
	} else {
		renderer = g_object_new (GSK_TYPE_PANGO_RENDERER, NULL);
	}

	return renderer;
}


static void
release_renderer (GskPangoRenderer* renderer)
{
	if (G_LIKELY (renderer->is_cached_renderer)) {
		G_UNLOCK (cached_renderer);
	} else {
		g_object_unref (renderer);
	}
}


/* convenience wrappers using the default renderer */

void
agl_pango_show_layout (PangoLayout* layout, int x, int y, float z, uint32_t _fg_color)
{
	g_return_if_fail(PANGO_IS_LAYOUT(layout));

	GskPangoRenderer* crenderer = acquire_renderer();

	crenderer->fg_color = (GdkRGBA){
		((_fg_color & 0xff000000) >> 24) / 255.0001,
		((_fg_color & 0x00ff0000) >> 16) / 255.0001,
		((_fg_color & 0x0000ff00) >>  8) / 255.0001,
		((_fg_color & 0x000000ff)      ) / 255.0001
	};

	AGlTransform* transform = agl_transform_new();
	AGlTransform* next = transform;
	transform = agl_transform_translate_3d(transform, &(const graphene_point3d_t){
		(x + builder()->offset.x) / (builder()->current_viewport.size.width / 2.0),
		-(y + builder()->offset.y) / (builder()->current_viewport.size.height / 2.0),
		z,
	});
	ops_push_modelview (builder(), transform);

	PangoRectangle ink_rect;
	pango_layout_get_pixel_extents(layout, &ink_rect, NULL);
	graphene_rect_init(&crenderer->bounds, ink_rect.x, ink_rect.y, ink_rect.width, ink_rect.height);

	pango_renderer_draw_layout(PANGO_RENDERER(crenderer), layout, 0, 0);

	ops_pop_modelview (renderer.current_builder);

	release_renderer (crenderer);

	// FIX for errors reported by valgrind TODO revisit
	agl_transform_unref(transform);
	agl_transform_unref(next);

	// TODO because of delayed rendering, this is unlikely to have the intended effect.
	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0); //reset back to the default value
}


PangoContext*
agl_pango_get_context ()
{
	g_type_class_unref (g_type_class_ref (GSK_TYPE_PANGO_RENDERER));
	GskPangoRendererClass* GPRC = g_type_class_peek(GSK_TYPE_PANGO_RENDERER);

	return GPRC->context;
}
