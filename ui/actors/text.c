/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform                                     |
 | https://github.com/ayyi/libwaveform                                  |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <gdk/gdkkeysyms.h>
#include "agl/behaviours/cache.h"
#include "agl/text/pango.h"
#include "wf/debug.h"
#include "wf/waveform.h"
#include "wf/peakgen.h"
#include "waveform/ui-utils.h"
#include "waveform/shader.h"
#include "waveform/text.h"

extern AssShader ass;

#define FONT \
	"Droid Sans"
	//"Ubuntu"
	//"Open Sans Rg"
	//"Fontin Sans Rg"

#define FONT_SIZE 18
#define N_CHANNELS 2 // luminance + alpha

static void text_actor_free     (AGlActor*);
static void text_actor_set_size (AGlActor*);
static bool text_actor_paint    (AGlActor*);
static void text_actor_init     (AGlActor*);

static AGlActorClass actor_class = {0, "Text", (AGlActorNew*)text_actor, text_actor_free};

static AGl* agl = NULL;
static int instance_count = 0;

static void text_actor_render_text (TextActor*);
static void measure_text           (const char*, int font_size, PangoRectangle*);

typedef struct
{
    int width, height, stride;
    unsigned char* buf;
} Image;


AGlActorClass*
text_actor_get_class ()
{
	return &actor_class;
}


AGlActor*
text_actor (WaveformActor* _)
{
	instance_count++;

	TextActor* ta = agl_actor__new(TextActor,
		.actor = {
			.class = &actor_class,
			.program = (AGlShader*)&ass,
			.init = text_actor_init,
			.paint = text_actor_paint,
			.set_size = text_actor_set_size,
			.behaviours = {
				cache_behaviour(),
			}
		},
		.title_colour1 = 0xff0000ff,
		.title_colour2 = 0xffffffff
	);

	AGlActor* actor = (AGlActor*)ta;

	return actor;
}


static void
text_actor_free (AGlActor* actor)
{
	TextActor* ta = (TextActor*)actor;

	g_clear_pointer(&ta->title, g_free);
	g_clear_pointer(&ta->text, g_free);

	if (!--instance_count) {
		if (ta->texture.ids[0]) {
			glDeleteTextures(1, ta->texture.ids);
			ta->texture.ids[0] = 0;
		}
	}

	g_free(actor);
}


static void
text_actor_init (AGlActor* a)
{
#ifdef USE_GTK
	TextActor* ta = (TextActor*)a;

	if (!ta->title_colour1) ta->title_colour1 = wf_get_gtk_text_color(a->root->gl.gdk.widget, GTK_STATE_NORMAL);
	if (!ta->text_colour) ta->text_colour = wf_get_gtk_base_color(a->root->gl.gdk.widget, GTK_STATE_NORMAL, 0xaa);
#endif

	agl = agl_get_instance();
	agl_set_font_string("Roboto 10"); // initialise the pango context

	ass.uniform.colour1 = ((TextActor*)a)->title_colour1;
	ass.uniform.colour2 = ((TextActor*)a)->title_colour2;
}


void
text_actor_set_colour (TextActor* ta, uint32_t title1, uint32_t title2)
{
	ass.uniform.colour1 = ta->title_colour1 = title1;
	ass.uniform.colour2 = ta->title_colour2 = title2;

	agl_actor__invalidate((AGlActor*)ta);
}


/*
 *  The strings are owned by the actor and will be freed later.
 */
void
text_actor_set_text (TextActor* ta, char* title, char* text)
{
	if (title) {
		wf_set_str(ta->title, title);
	}

	wf_set_str(ta->text, text);

	ta->title_is_rendered = false;

	agl_actor__invalidate((AGlActor*)ta);
}


static bool
text_actor_paint (AGlActor* actor)
{
	TextActor* ta = (TextActor*)actor;

	if (ta->title) {
		if (!ta->title_is_rendered) text_actor_render_text(ta);

		// title text
		agl_enable(AGL_ENABLE_BLEND);
		glActiveTexture(GL_TEXTURE0);

		float th = ((TextActor*)actor)->texture.height;

#undef ALIGN_TOP
#ifdef ALIGN_TOP
		float y1 = -((int)th - ta->_title.height - ta->_title.y_offset);
		agl_textured_rect(ta->texture.ids[0], (actor->region.x2 - actor->region.x1) - ta->_title.width - 4.0f, y, ta->_title.width, th, &(AGlRect){0.0, 0.0, ((float)ta->_title.width) / ta->texture.width, 1.0});
#else
		agl_textured_rect(ta->texture.ids[0],
			actor->region.x2 - ta->_title.width - 4.0f,
			actor->region.y2 - actor->region.y1 - th + ((TextActor*)actor)->baseline - 4.0f,
			ta->_title.width,
			th,
			&(AGlQuad){0.0, 0.0, ((float)ta->_title.width) / ta->texture.width, 1.0}
		);
#endif
	}

	agl_print(2, agl_actor__height(actor) - 16, 0, ta->text_colour, ta->text);

	return true;
}


static void
text_actor_set_size (AGlActor* actor)
{
	float height = MIN(40.0, agl_actor__height(actor->parent));
	actor->region = (AGlfRegion){0, agl_actor__height(actor->parent) - height, agl_actor__width(actor->parent), agl_actor__height(actor->parent)};
}


static void
render_outlined_text_to_a8 (const char* text, const char* font_family, double font_size, double outline_width, Image* out)
{
	#define padding 1
	PangoRectangle rect;
	measure_text(text, font_size, &rect);
	int width = out->width = agl_power_of_two(rect.width);
	int height = out->height = agl_power_of_two(rect.height);
	out->buf = g_malloc0(width * height * N_CHANNELS);

	cairo_surface_t* alpha_surface = ({
		cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);
		cairo_t* cr = cairo_create(surface);

		// Clear surface
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint(cr);
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

		cairo_select_font_face(cr, font_family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr, font_size);

		cairo_text_extents_t extents;
		cairo_text_extents(cr, text, &extents);

		double x = width - extents.width - extents.x_bearing - padding;
		double y = (height - extents.height) / 1 - extents.y_bearing - padding;

		// Draw text path
		cairo_move_to(cr, x, y);
		cairo_text_path(cr, text);

		// Draw outline
		cairo_set_line_width(cr, outline_width);
		cairo_set_source_rgba(cr, 1, 1, 1, 1); // White outline
		cairo_stroke_preserve(cr);

		// Fill text
		cairo_set_source_rgba(cr, 0, 0, 1, 1); // White fill
		cairo_fill(cr);

		cairo_destroy(cr);
		surface;
	});
	const unsigned char* alpha = cairo_image_surface_get_data(alpha_surface);

	cairo_surface_t* luminance_surface = ({
		cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
		cairo_t* cr = cairo_create(surface);

		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint(cr);
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

		// White background
		cairo_set_source_rgba(cr, 1, 1, 1, 1);
		cairo_paint(cr);

		cairo_select_font_face(cr, font_family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr, font_size);

		cairo_text_extents_t extents;
		cairo_text_extents(cr, text, &extents);

		// Draw text in black
		cairo_move_to(cr,
			width - extents.width - extents.x_bearing - padding,
			height - extents.height - extents.y_bearing - padding
		);
		cairo_text_path(cr, text);
		cairo_set_source_rgba(cr, 0, 0, 0, 1);
		cairo_fill(cr);

		cairo_destroy(cr);
		surface;
	});
	const unsigned char* luminance = cairo_image_surface_get_data(luminance_surface);

	#define LUMINANCE_CHANNEL(P) (P * N_CHANNELS)
	#define ALPHA_CHANNEL(P) (P * N_CHANNELS + 1)
	for (int y=0;y<height;y++) {
		for (int x=0;x<width;x++) {
			int p = y * width + x;
			out->buf[LUMINANCE_CHANNEL(p)] = luminance[p * 4 + 1];
			out->buf[ALPHA_CHANNEL(p)] = alpha[p];
		}
	}

	cairo_surface_destroy(alpha_surface);
	cairo_surface_destroy(luminance_surface);
}


static void
text_actor_render_text (TextActor* ta)
{
	AGlActor* actor = (AGlActor*)ta;

	PF;
	if (ta->title_is_rendered) pwarn("title is already rendered");

	GError* error = NULL;
	GRegexMatchFlags flags = 0;
	static GRegex* regex = NULL;
	if (!regex) regex = g_regex_new("[_]", 0, flags, &error);
	gchar* str = g_regex_replace(regex, ta->title, -1, 0, " ", flags, &error);

	g_autofree char* title = g_strdup_printf("%s", str);
	g_free(str);

	Image image;
	render_outlined_text_to_a8(title, "Sans", FONT_SIZE, 2.5, &image);
	g_autofree unsigned char* buffer = image.buf;
	ta->_title = (Title){
		.width = image.width,
		.height = image.height,
	};

#ifdef DEBUG
	if (false) {
		char* buf = g_new0(char, image.width * image.height * 4);
		int stride = image.width * 4;
		for (int y=0;y<image.height;y++) {
			for (int x=0;x<image.width;x++) {
				*(buf + y * stride + x * 4    ) = *(image.buf + y * image.stride + x * N_CHANNELS);
				*(buf + y * stride + x * 4 + 1) = 0;
				*(buf + y * stride + x * 4 + 2) = 0;
				*(buf + y * stride + x * 4 + 3) = *(image.buf + y * image.stride + x * N_CHANNELS + 1);
			}
		}
		GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data((const guchar*)buf, GDK_COLORSPACE_RGB, /*HAS_ALPHA_*/TRUE, 8, image.width, image.height, stride, NULL, NULL);
		gdk_pixbuf_save(pixbuf, "tmp1.png", "png", NULL, NULL);
		g_object_unref(pixbuf);
		g_free(buf);
	}
#endif

	{
		if (((TextActor*)actor)->texture.ids[0]) {
			glGenTextures(1, ((TextActor*)actor)->texture.ids);
			if (gl_error) {
				perr ("couldnt create ass_texture.");
				return;
			}
		}
		ta->texture.width = image.width;
		ta->texture.height = image.height;
		agl_actor__set_size(actor);

		#define pixel_format GL_LUMINANCE_ALPHA
		glBindTexture  (GL_TEXTURE_2D, ta->texture.ids[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, image.width, image.height, 0, pixel_format, GL_UNSIGNED_BYTE, image.buf);
		gl_warn("gl error using ass texture");
	}

	ta->title_is_rendered = true;
}


static void
measure_text (const char* text, int font_size, PangoRectangle* ink_rect)
{
	PangoFontDescription* font_desc = pango_font_description_new();
	pango_font_description_set_family(font_desc, "Sans");
	pango_font_description_set_size(font_desc, font_size * PANGO_SCALE);

	PangoLayout* layout = pango_layout_new (agl_pango_get_context());
	pango_layout_set_text (layout, text, -1);

	pango_layout_set_font_description(layout, font_desc);

	PangoRectangle logical_rect;
	pango_layout_get_pixel_extents(layout, ink_rect, &logical_rect);

	g_object_unref(layout);
	pango_font_description_free(font_desc);
}
