/* Pango
 * Rendering routines to Clutter
 *
 * Copyright (C) 2006 Matthew Allum <mallum@o-hand.com>
 * Copyright (C) 2006 Marc Lehmann <pcg@goof.com>
 * Copyright (C) 2004 Red Hat Software
 * Copyright (C) 2000 Tor Lillqvist
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define PANGO_ENABLE_BACKEND

#include <math.h>
#include <string.h>
#include <pango/pango.h>
#include <pango/pangofc-font.h>
#include <pango/pangofc-fontmap.h>
#include <fontconfig/fontconfig.h>
#include <GL/gl.h>
#include "agl/utils.h"

#define USE_SHADERS
#ifdef USE_SHADERS
								#define __wf_private__      // TODO
#include "waveform/utils.h"
#include "agl/shader.h"
#include "agl/ext.h"
#endif

#include "agl/pango_render.h"
#include "agl/pango_font.h"
static PangoGlRendererClass* PGRC = NULL;

/*
 * Texture cache support code
 */

#define TC_WIDTH  256
#define TC_HEIGHT 256
#define TC_ROUND  4

/* Defines duped - fun,fun.. */
#ifndef PANGO_GLYPH_EMPTY
#define PANGO_GLYPH_EMPTY           ((PangoGlyph)0x0FFFFFFF)
#endif
#ifndef PANGO_GLYPH_UNKNOWN_FLAG
#define PANGO_GLYPH_UNKNOWN_FLAG ((PangoGlyph)0x10000000)
#endif
#ifndef PANGO_UNKNOWN_GLYPH_WIDTH
#define PANGO_UNKNOWN_GLYPH_WIDTH 10
#endif
#ifndef PANGO_UNKNOWN_GLYPH_HEIGHT
#define PANGO_UNKNOWN_GLYPH_HEIGHT 14
#endif

#define PANGO_SCALE_26_6 (PANGO_SCALE / (1<<6))

#define FIXED_TO_DOUBLE(x) ((double) ((int)(x) / 65536.0))

/* Where in the 64 bits of double is the mantisa */
#if (__FLOAT_WORD_ORDER == 1234)
#define _CFX_MAN            0
#elif (__FLOAT_WORD_ORDER == 4321)
#define _CFX_MAN            1
#else
#define CFX_NO_FAST_CONVERSIONS
#endif

const double _magic = 68719476736.0*1.5;

static void prepare_run              (PangoRenderer*, PangoLayoutRun*);
static void gl_pango_draw_begin      (PangoRenderer*);
static void draw_glyph               (PangoRenderer*, PangoFont*, PangoGlyph, double x, double y);
static void gl_texture_set_filters   (GLenum target, GLenum min_filter, GLenum max_filter);
static void gl_texture_set_alignment (GLenum target, guint alignment, guint row_length);
static void gl_texture_quad          (gint x1, gint x2, gint y1, gint y2, Fixed tx1, Fixed ty1, Fixed tx2, Fixed ty2);
extern void agl_enable               (gulong flags);
void        gl_trapezoid             (gint y1, gint x11, gint x21, gint y2, gint x12, gint x22);

typedef struct {
  guint name;
  int x, y, w, h;
} tc_area;

typedef struct tc_texture {
  struct tc_texture *next;
  guint name;
  int avail;
} tc_texture;

typedef struct tc_slice {
  guint name;
  int avail, y;
} tc_slice;

static int tc_generation = 0;
static tc_slice slices[TC_HEIGHT / TC_ROUND];
static tc_texture *first_texture;

typedef struct
{
  guint8 *bitmap;
  int     width, stride, height, top, left;
} Glyph;

#undef TEMP
#ifdef TEMP
static char*
test_image_new(int width, int height)
{
	char* temp_buf = g_new0(char, width * height);
	int y; for(y=0;y<height;y++){
		int j; for(j=0;j<width;j++){
			*(temp_buf + y * width + j) = (j * 0xff) / width;
		}
	}
	return temp_buf;
}
#endif

#if 0
static void
save_bitmap(Glyph* g)
{
	//copy bitmap and save it:
	static int n = 0;
	#include <gdk-pixbuf/gdk-pixbuf.h>
	#include <gtk/gtk.h>
	#include "../ayyi/text_render.h"
	void     copy_bitmap_to_pixbuf(guchar* bitmap, GdkPixbuf*, int pitch);
	char fname[64]; sprintf(fname, "temp_glyph%i.png", n);
	GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, g->width, g->height);
	copy_bitmap_to_pixbuf(g->bitmap, pixbuf, g->width);
	gdk_pixbuf_save(pixbuf, fname, "png", NULL, NULL);
	n++;
}
#endif

static void
tc_clear ()
{
  int i;

  for (i = TC_HEIGHT / TC_ROUND; i--; )
    slices [i].name = 0;

  while (first_texture)
    {
      tc_texture *next = first_texture->next;
      //cogl_textures_destroy (1, &first_texture->name);
      glDeleteTextures (1, &first_texture->name);
      g_slice_free (tc_texture, first_texture);
      first_texture = next;
    }

                                 guint t = 2;
                                 glDeleteTextures (1, &t);

  ++tc_generation;
}

static void
tc_get (tc_area *area, int width, int height)
{
  int       slice_height; 
  tc_slice *slice;

  area->w = width;
  area->h = height;

  /* Provide for blank rows/columns of pixels between adjecant glyphs in the
   * texture cache to avoid bilinear interpolation spillage at edges of glyphs.
   */
  width += 1;
  height += 1;

  slice_height = MIN (height + TC_ROUND - 1, TC_HEIGHT) & ~(TC_ROUND - 1);
  slice = slices + slice_height / TC_ROUND;

  width = MIN (width, TC_WIDTH);

  if (!slice->name || slice->avail < width) {
    /* try to find a texture with enough space */
    tc_texture* tex;
    tc_texture* match = 0;

    for (tex = first_texture; tex; tex = tex->next)
      if (tex->avail >= slice_height && (!match || match->avail > tex->avail))
        match = tex;

    /* create a new texture if necessary */
    if (!match) {
      //dbg(1, "creating new texture %i x %i", TC_WIDTH, TC_HEIGHT);

      match = g_slice_new (tc_texture);
      match->next = first_texture;
      first_texture = match;
      match->avail = TC_HEIGHT;

      //cogl_textures_create (1, &match->name);
      glGenTextures (1, &match->name);

      glBindTexture (GL_TEXTURE_2D, match->name);

      /* We might even want to use mipmapping instead of GL_LINEAR here
       * that should allow rerendering of glyphs to look nice even at scales
       * far below 50%.
       */
      gl_texture_set_filters (GL_TEXTURE_2D, GL_LINEAR, GL_NEAREST);

#ifdef TEMP
      char* temp_buf = test_image_new(TC_WIDTH, TC_HEIGHT);
#endif
      // create an empty texture
      glTexImage2D (GL_TEXTURE_2D, 0,
				 GL_ALPHA,
				 TC_WIDTH,
				 TC_HEIGHT,
                 0,
				 GL_ALPHA,
				 GL_UNSIGNED_BYTE,
#ifdef TEMP
                 //gdk_pixbuf_get_pixels(pixbuf));
				 temp_buf);
#else
				 NULL);
#endif

#ifdef TEMP
      //try subimage:
      glTexSubImage2D (GL_TEXTURE_2D, 0,
				 40,        //xoffset
				 90,        //yoffset
				 40,        //subimage width
				 40,        //subimage height
				 GL_ALPHA,
				 GL_UNSIGNED_BYTE,
				 temp_buf);
#endif
    }

    match->avail -= slice_height;

    slice->name  = match->name;
    slice->avail = TC_WIDTH;
    slice->y     = match->avail;
  }

  slice->avail -= width;

  area->name = slice->name;
  area->x    = slice->avail;
  area->y    = slice->y;
}

static void
tc_put (tc_area *area)
{
  /* our management is too primitive to support this operation yet */
}

/*******************/


G_DEFINE_TYPE (PangoGlRenderer, pango_gl_renderer, PANGO_TYPE_RENDERER)

static void *
temp_buffer (size_t size)
{
  static char *buffer;
  static size_t alloc;

  if (size > alloc)
    {
      size = (size + 4095) & ~4095;
      free (buffer);
      alloc = size;
      buffer = malloc (size);
    }

  return buffer;
}

static void
render_box (Glyph *glyph, int width, int height, int top)
{
  int i;
  int left = 0;

  if (height > 2)
    {
      height -= 2;
      top++;
    }

  if (width > 2)
    {
      width -= 2;
      left++;
    }

  glyph->stride = (width + 3) & ~3;
  glyph->width  = width;
  glyph->height = height;
  glyph->top    = top;
  glyph->left   = left;

  glyph->bitmap = temp_buffer (width * height);
  memset (glyph->bitmap, 0, glyph->stride * height);

  //put a 1px box round the edge

  for (i = width; i--; )
    glyph->bitmap [i]
      = glyph->bitmap [i + (height - 1) * glyph->stride] = 0xff;

  for (i = height; i--; )
    glyph->bitmap [i * glyph->stride]
      = glyph->bitmap [i * glyph->stride + (width - 1)] = 0xff;
}

static void
font_render_glyph (Glyph *glyph, PangoFont *font, int glyph_index)
{
  if (glyph_index & PANGO_GLYPH_UNKNOWN_FLAG)
    {
      gwarn("PANGO_GLYPH_UNKNOWN_FLAG");
      PangoFontMetrics *metrics;

      if (!font) goto generic_box;

      metrics = pango_font_get_metrics (font, NULL);
      if (!metrics) goto generic_box;

      render_box (glyph, PANGO_PIXELS (metrics->approximate_char_width),
		         PANGO_PIXELS (metrics->ascent + metrics->descent),
		         PANGO_PIXELS (metrics->ascent));

      pango_font_metrics_unref (metrics);

      return;
    }

  FT_Face face = pango_gl_font_get_face (font);

  if (face)
    {
      PangoGlFont *glfont = (PangoGlFont *)font;

      FT_Load_Glyph (face, glyph_index, glfont->load_flags);
      //convert glyph outline image into bitmap:
      FT_Render_Glyph (face->glyph, ft_render_mode_normal);

      glyph->width  = face->glyph->bitmap.width;
      glyph->stride = face->glyph->bitmap.pitch;
      glyph->height = face->glyph->bitmap.rows;
      glyph->top    = face->glyph->bitmap_top;
      glyph->left   = face->glyph->bitmap_left;
      glyph->bitmap = face->glyph->bitmap.buffer;

      {
        //debugging...
#if 0
        dbg(0, "glyph=%p bitmap=%p width=%i height=%i", glyph, glyph->bitmap, glyph->width, glyph->height);
        save_bitmap(glyph);
#endif
      }
      return;
    }
  else
    generic_box:
      gwarn("no font face.");
      render_box (glyph, PANGO_UNKNOWN_GLYPH_WIDTH, PANGO_UNKNOWN_GLYPH_HEIGHT, PANGO_UNKNOWN_GLYPH_HEIGHT);
}

typedef struct glyph_info
{
  tc_area tex;
  int     left, top;
  int     generation;
}
glyph_info;

static void
free_glyph_info (glyph_info *g)
{
  tc_put (&g->tex);
  g_slice_free (glyph_info, g);
}

static void
draw_glyph (PangoRenderer *renderer_, PangoFont *font,
	    PangoGlyph     glyph,
	    double         x,
	    double         y)
{
  PangoGlRenderer *renderer = PANGO_GL_RENDERER (renderer_);
  struct { float x1, y1, x2, y2; } box;

  if (glyph & PANGO_GLYPH_UNKNOWN_FLAG) {
    glyph = pango_gl_get_unknown_glyph (font);
    if (glyph == PANGO_GLYPH_EMPTY) glyph = PANGO_GLYPH_UNKNOWN_FLAG;
  }

  glyph_info *g = _pango_gl_font_get_cache_glyph_data (font, glyph);

  if (!g || g->generation != tc_generation)
    {
      /*
      if(g) dbg(2, "%i: g=%p %i=%i", glyph, g, g->generation, tc_generation);
      else dbg(2, "g=%p", g);
      */

      Glyph bm;
      font_render_glyph (&bm, font, glyph);

      if (g)
        g->generation = tc_generation;
      else
        {
          g = g_slice_new (glyph_info);

          _pango_gl_font_set_glyph_cache_destroy (font, (GDestroyNotify)free_glyph_info);
          _pango_gl_font_set_cache_glyph_data (font, glyph, g);
        }

      tc_get (&g->tex, bm.width, bm.height);

      g->left = bm.left;
      g->top  = bm.top;

      //dbg(2, "cache miss; subimage2d char=%i tex_name=%i x=%i y=%i w=%i h=%i bm=%p bitmap=%p", glyph, g->tex.name, g->tex.x, g->tex.y, bm.width, bm.height, &bm, bm.bitmap);

      glBindTexture (GL_TEXTURE_2D, g->tex.name);

      gl_texture_set_alignment (GL_TEXTURE_2D, 1, bm.stride);

#ifdef TEMP
      char* temp_buf = test_image_new(100, 10);
      glTexSubImage2D (GL_TEXTURE_2D, 0,
				 g->tex.x,        //xoffset
				 g->tex.y,        //yoffset
				 bm.width,        //subimage width
				 bm.height,       //subimage height
				 GL_ALPHA,
				 GL_UNSIGNED_BYTE,
				 temp_buf);
#else
      // copy the glyph to an area of the texture:
      glTexSubImage2D (GL_TEXTURE_2D, 0,
				 g->tex.x,        //xoffset
				 g->tex.y,        //yoffset
				 bm.width,        //subimage width
				 bm.height,       //subimage height
				 GL_ALPHA,
				 GL_UNSIGNED_BYTE,
				 bm.bitmap);
#endif

      glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP, FALSE);

      renderer->cur_tex = g->tex.name;

	  if(glGetError() != GL_NO_ERROR) gwarn("gl error!");
    }
  //else dbg (4, "cache success %i", glyph);

  x += g->left;
  y -= g->top;

  box.x1 = g->tex.x * (1. / TC_WIDTH );
  box.y1 = g->tex.y * (1. / TC_HEIGHT);
  box.x2 = g->tex.w * (1. / TC_WIDTH ) + box.x1;
  box.y2 = g->tex.h * (1. / TC_HEIGHT) + box.y1;

  if (g->tex.name != renderer->cur_tex) {
    glBindTexture (GL_TEXTURE_2D, g->tex.name);
    renderer->cur_tex = g->tex.name;
  }
  //dbg(4, "box=%.2f %.2f %.2f %.2f", box.x1, box.y1, box.x2, box.y2);
//dbg(0, "width=%.2f height=%.2f", g->tex.w, g->tex.h);

  gl_texture_quad (x,                   //stage coords
		     x + g->tex.w,
		     y,
		     y + g->tex.h,
		     double_to_fixed (box.x1),  //texture coords
		     double_to_fixed (box.y1),
		     double_to_fixed (box.x2),
		     double_to_fixed (box.y2));
}

static void
draw_trapezoid (PangoRenderer   *renderer_,
		PangoRenderPart  part,
		double           y01,
		double           x11,
		double           x21,
		double           y02,
		double           x12,
		double           x22)
{
  PangoGlRenderer *renderer = (PangoGlRenderer *)renderer_;

  if (renderer->cur_tex)
    {
      /* glEnd (); */
      renderer->cur_tex = 0;
    }

  /* Turn texturing off */
  agl_enable (AGL_ENABLE_BLEND);

  gl_trapezoid ((gint) y01,
		  (gint) x11,
		  (gint) x21,
		  (gint) y02,
		  (gint) x12,
		  (gint) x22);

  /* Turn it back on again */
  agl_enable (AGL_ENABLE_TEXTURE_2D|AGL_ENABLE_BLEND);
}

void
pango_gl_render_layout_subpixel (PangoLayout *layout,
				      int           x,
				      int           y,
				      double        z,
				      Colour32      *color,
				      int           flags)
{
  PangoContext  *context;
  PangoFontMap  *fontmap;
  PangoRenderer *renderer;

  context = pango_layout_get_context (layout);
  fontmap = pango_context_get_font_map (context);
  renderer = pango_gl_font_map_get_renderer (PANGO_GL_FONT_MAP (fontmap));

  memcpy (&(PANGO_GL_RENDERER (renderer)->color), color, sizeof(Colour32));

#if 0
  //checks:
  {
    dbg(2, "text=%s", pango_layout_get_text(layout));
    int unknown = pango_layout_get_unknown_glyphs_count(layout);
    int w = pango_layout_get_width(layout); //is -1, ie not set.
    dbg(2, "unknown=%i width=%i", unknown, w);
  }
#endif

  //the pango renderer interface doesnt support z, so we set it using a matrix transformation:
  //The matrix is not cleared, so care must be taken before calling the print function.
  glPushMatrix();
  glTranslatef(0.0, 0.0, z);

  pango_renderer_draw_layout (renderer, layout, x, y);

  glPopMatrix();
}

void
pango_gl_render_layout (PangoLayout  *layout,
			     int           x,
			     int           y,
			     double        z,
			     Colour32      *color,
			     int           flags)
{
  pango_gl_render_layout_subpixel (layout,
					x * PANGO_SCALE,
					y * PANGO_SCALE,
					z,
					color,
					flags);
}

void
pango_gl_render_layout_line (PangoLayoutLine *line,
				  int              x,
				  int              y,
				  Colour32         *color)
{
  PangoContext  *context;
  PangoFontMap  *fontmap;
  PangoRenderer *renderer;

  context = pango_layout_get_context (line->layout);
  fontmap = pango_context_get_font_map (context);
  renderer = pango_gl_font_map_get_renderer (PANGO_GL_FONT_MAP (fontmap));

  memcpy (&(PANGO_GL_RENDERER (renderer)->color), color, sizeof(Colour32));

  pango_renderer_draw_layout_line (renderer, line, x, y);
}

void
pango_gl_render_clear_caches (void)
{
  tc_clear();
}

static void
pango_gl_renderer_init (PangoGlRenderer *renderer)
{
  memset (&renderer->color, 0xff, sizeof(Colour32));
}

static void
prepare_run (PangoRenderer *renderer, PangoLayoutRun *run)
{
  PangoGlRenderer *glrenderer = (PangoGlRenderer *)renderer;
  GSList               *l;
  PangoColor           *fg = 0;

  renderer->underline = PANGO_UNDERLINE_NONE;
  renderer->strikethrough = FALSE;

  for (l = run->item->analysis.extra_attrs; l; l = l->next)
    {
      PangoAttribute *attr = l->data;

      switch (attr->klass->type) {
        case PANGO_ATTR_UNDERLINE:
          renderer->underline = ((PangoAttrInt *)attr)->value;
          break;

        case PANGO_ATTR_STRIKETHROUGH:
          renderer->strikethrough = ((PangoAttrInt *)attr)->value;
          break;

        case PANGO_ATTR_FOREGROUND:
          fg = &((PangoAttrColor *)attr)->color;
          break;
        default:
          break;
      }
    }

  Colour32 col;
  if (fg)
    {
      col.red   = (fg->red   * 255) / 65535;
      col.green = (fg->green * 255) / 65535;
      col.blue  = (fg->blue  * 255) / 65535;
    }
  else
    {
      col.red   = glrenderer->color.red;
      col.green = glrenderer->color.green;
      col.blue  = glrenderer->color.blue;
    }

  col.alpha = glrenderer->color.alpha;

  if (glrenderer->flags & FLAG_INVERSE)
    {
      col.red   ^= 0xffU;
      col.green ^= 0xffU;
      col.blue  ^= 0xffU;
    }

#ifdef USE_SHADERS
	AGl* agl = agl_get_instance();
	if(agl->use_shaders){
		agl->shaders.text->uniform.fg_colour = (col.red << 24) + (col.green << 16) + (col.blue << 8) + col.alpha;
	}else{
#endif
		//cogl_color(&col);
		glColor4ub (col.red, col.green, col.blue, col.alpha);
		//dbg(0, "color=%x %x %x %x", col.red, col.green, col.blue, col.alpha);
#ifdef USE_SHADERS
	}
#endif
}

static void
gl_pango_draw_begin (PangoRenderer *renderer_)
{
  PangoGlRenderer *renderer = (PangoGlRenderer *)renderer_;

  renderer->cur_tex = 0;

  agl_enable (AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);

#if 0
  (also commented out in clutter)

  gl_BlendFuncSeparate (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                        GL_ONE      , GL_ONE_MINUS_SRC_ALPHA);

  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable (GL_ALPHA_TEST);
  glAlphaFunc (GL_GREATER, 0.01f);
#endif
  gl_warn("");
}

static void
gl_pango_draw_end (PangoRenderer *renderer_)
{
  /* (also commented out in clutter)
  PangoGlRenderer *renderer = (PangoGlRenderer *)renderer_;

  if (renderer->cur_tex) glEnd ();
  */
  gl_warn("");
}

static void
pango_gl_renderer_class_init (PangoGlRendererClass *klass)
{
  PangoRendererClass *renderer_class = PANGO_RENDERER_CLASS (klass);

  renderer_class->draw_glyph     = draw_glyph;
  renderer_class->draw_trapezoid = draw_trapezoid;
  renderer_class->prepare_run    = prepare_run;
  renderer_class->begin          = gl_pango_draw_begin;
  renderer_class->end            = gl_pango_draw_end;

  if(sizeof(Colour32) != 4) gwarn("expected Colour32 size of 4: size=%i", sizeof(Colour32));

  PGRC = klass;
}

static void
gl_texture_set_filters (GLenum target, GLenum min_filter, GLenum max_filter)
{
  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, max_filter);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
}

static void
gl_texture_set_alignment (GLenum target, guint alignment, guint row_length)
{
  glPixelStorei (GL_UNPACK_ROW_LENGTH, row_length);
  glPixelStorei (GL_UNPACK_ALIGNMENT, alignment);
}

static void
gl_texture_quad (gint x1, gint x2, gint y1, gint y2, Fixed tx1, Fixed ty1, Fixed tx2, Fixed ty2)
{
  gdouble txf1 = FIXED_TO_DOUBLE (tx1);
  gdouble tyf1 = FIXED_TO_DOUBLE (ty1);
  gdouble txf2 = FIXED_TO_DOUBLE (tx2);
  gdouble tyf2 = FIXED_TO_DOUBLE (ty2);

#define USE_GL_ENABLE
#ifdef USE_GL_ENABLE
  agl_enable (AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);
#else
  glEnable(GL_TEXTURE_2D);
  /*
  tc_texture* tex = first_texture; //TODO
  glBindTexture(GL_TEXTURE_2D, tex->name);
  */

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      
#endif

#ifdef USE_SHADERS
	if(agl_get_instance()->use_shaders){
		agl_use_program((AGlShader*)agl_get_instance()->shaders.text);
	}
#endif
  glBegin (GL_QUADS);
  glTexCoord2f (txf2, tyf2); glVertex2i (x2, y2);
  glTexCoord2f (txf1, tyf2); glVertex2i (x1, y2);
  glTexCoord2f (txf1, tyf1); glVertex2i (x1, y1);
  glTexCoord2f (txf2, tyf1); glVertex2i (x2, y1);
  glEnd ();
}

void
gl_trapezoid (gint y1, gint x11, gint x21, gint y2, gint x12, gint x22)
{
  glBegin (GL_QUADS);
  glVertex2i (x11, y1);
  glVertex2i (x21, y1);
  glVertex2i (x22, y2);
  glVertex2i (x12, y2);
  glEnd ();
}

/*
 * clutter_double_to_fixed :
 * @value: value to be converted
 *
 * A fast conversion from double precision floating to fixed point
 *
 * Return value: Fixed point representation of the value
 *
 * Since: 0.2
 */
Fixed
double_to_fixed (double val)
{
#ifdef CFX_NO_FAST_CONVERSIONS
  return (ClutterFixed)(val * (double)CFX_ONE);
#else
  union
    {
	double d;
	unsigned int i[2];
  } dbl;

  dbl.d = val;
  dbl.d = dbl.d + _magic;
  return dbl.i[_CFX_MAN];
#endif
}

void
pango_gl_debug_textures()
{
  //draw the whole font texture.

  tc_texture* tex = first_texture;

  glBindTexture(GL_TEXTURE_2D, tex->name);
#ifdef USE_GL_ENABLE
  agl_enable (AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);
#else
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
  glColor3f(1.0, 0.0, 0.0);

  double x = 100, y = -100;
  double top = y, left = x, bot = y -128, right = x + 256;

  glBegin( GL_QUADS );
  glTexCoord2d(0.0, 0.0); glVertex2d(left,  top);
  glTexCoord2d(1.0, 0.0); glVertex2d(right, top);
  glTexCoord2d(1.0, 1.0); glVertex2d(right, bot);
  glTexCoord2d(0.0, 1.0); glVertex2d(left,  bot);
  glEnd();
}


