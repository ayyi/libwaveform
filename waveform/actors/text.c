/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#ifdef USE_LIBASS
#include <ass/ass.h>
#endif
#include "agl/ext.h"
#include "agl/utils.h"
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "waveform/peakgen.h"
#include "waveform/shader.h"
#include "waveform/actors/text.h"

#define FONT \
	"Droid Sans"
	//"Ubuntu"
	//"Open Sans Rg"
	//"Fontin Sans Rg"

#define FONT_SIZE 18 //TODO

static void text_actor_free (AGlActor*);

static AGl* agl = NULL;
static AGlActorClass actor_class = {0, "Overview", (AGlActorNew*)text_actor, text_actor_free};
static int instance_count = 0;

#ifdef USE_LIBASS
static void text_actor_render_text (TextActor*);

extern AssShader ass;

char* script = 
	"[Script Info]\n"
	"ScriptType: v4.00+\n"
	"PlayResX: %i\n"
	"PlayResY: %i\n"
	"ScaledBorderAndShadow: yes\n"
	"[V4+ Styles]\n"
	"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n"
	/*
		PrimaryColour:   filling color
		SecondaryColour: for animations
		OutlineColour:   border color
		BackColour:      shadow color

		hex format appears to be AALLXXXXXX where AA=alpha (00=opaque, FF=transparent) and LL=luminance
	*/
	//"Style: Default,Fontin Sans Rg,%i,&H000000FF,&HFF0000FF,&H00FF0000,&H00000000,-1,0,0,0,100,100,0,0,1,2.5,0,1,2,2,2,1\n"
	"Style: Default," FONT ",%i,&H3FFF00FF,&HFF0000FF,&H000000FF,&H00000000,-1,0,0,0,100,100,0,0,1,2.5,0,1,2,2,2,1\n"
	"[Events]\n"
	"Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
	"Dialogue: 0,0:00:00.00,0:00:15.00,Default,,0000,0000,0000,,%s \n";

typedef struct
{
    int width, height, stride;
    unsigned char* buf;      // 8 bit alphamap
} image_t;

static ASS_Library*  ass_library = NULL;
static ASS_Renderer* ass_renderer = NULL;
#endif


#ifdef USE_LIBASS
#ifdef DEBUG
		static void msg_callback(int level, const char* fmt, va_list va, void* data)
		{
			if (wf_debug < 2 || level > 6) return;
			printf("libass: ");
			vprintf(fmt, va);
			printf("\n");
		}
#endif
#endif

static void
_init()
{
	static bool init_done = false;

	void ass_init()
	{
#ifdef USE_LIBASS
		ass_library = ass_library_init();
		if (!ass_library) {
			printf("ass_library_init failed!\n");
			exit(EXIT_FAILURE);
		}

#ifdef DEBUG
		ass_set_message_cb(ass_library, msg_callback, NULL);
#endif

		ass_renderer = ass_renderer_init(ass_library);
		if (!ass_renderer) {
			printf("ass_renderer_init failed!\n");
			exit(EXIT_FAILURE);
		}

		ass_set_fonts(ass_renderer, NULL, "Sans", 1, NULL, 1);
#endif
	}

	if(!init_done){
		agl = agl_get_instance();
		ass_init();
		agl_set_font_string("Roboto 10"); // initialise the pango context

		init_done = true;
	}
}


	static bool text_actor_paint(AGlActor* actor)
	{
		TextActor* ta = (TextActor*)actor;

		if(!agl->use_shaders) agl_enable(0); // TODO find out why this is needed when AGlActor caching is enabled.

		agl_print(2, agl_actor__height(actor) - 16, 0, ta->text_colour, ta->text);

#ifdef USE_LIBASS
		if(ta->title){
			if(!ta->title_is_rendered) text_actor_render_text(ta);

			// title text:
			if(agl->use_shaders){
				agl_enable(AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);
				glActiveTexture(GL_TEXTURE0);

				agl_use_program((AGlShader*)&ass);

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
		}
#endif

		return true;
	}

	static void text_actor_init(AGlActor* a)
	{
#ifdef USE_GTK
		TextActor* ta = (TextActor*)a;

		if(!ta->title_colour1) ta->title_colour1 = wf_get_gtk_text_color(a->root->gl.gdk.widget, GTK_STATE_NORMAL);
		if(!ta->text_colour) ta->text_colour = wf_get_gtk_base_color(a->root->gl.gdk.widget, GTK_STATE_NORMAL, 0xaa);
#endif

#ifdef USE_LIBASS
		if(agl_get_instance()->use_shaders){
			agl_create_program((AGlShader*)&ass);
			ass.uniform.colour1 = ((TextActor*)a)->title_colour1;
			ass.uniform.colour2 = ((TextActor*)a)->title_colour2;
		}
#endif
#ifdef AGL_ACTOR_RENDER_CACHE
		a->fbo = agl_fbo_new(a->region.x2 - a->region.x1, a->region.y2 - a->region.y1, 0, 0);
		a->cache.enabled = true;
#endif
	}

	static void text_actor_set_size(AGlActor* actor)
	{
		// the texture height will not be available first time
		actor->region = (AGlfRegion){0, 0, actor->parent->region.x2 - actor->parent->region.x1, actor->parent->region.y2 - actor->parent->region.y1};
	}

AGlActor*
text_actor(WaveformActor* _)
{
	instance_count++;

	_init();

	TextActor* ta = AGL_NEW(TextActor,
		.actor = {
			.class = &actor_class,
			.name = "Text",
			.init = text_actor_init,
			.paint = text_actor_paint,
			.set_size = text_actor_set_size
		},
		.title_colour1 = 0xff0000ff,
		//ta->title_colour2 = 0xffffffaa,
		.title_colour2 = 0x0000ffff //FIXME
	);

	AGlActor* actor = (AGlActor*)ta;

	return actor;
}


static void
text_actor_free(AGlActor* actor)
{
	TextActor* ta = (TextActor*)actor;

	g_free0(ta->title);
	g_free0(ta->text);

#ifdef USE_LIBASS
	if(!--instance_count){
		ass_renderer_done(ass_renderer);
		ass_library_done(ass_library);
		ass_renderer = NULL;
		ass_library = NULL;
	}
#endif
}

void
text_actor_set_colour (TextActor* ta, uint32_t title1, uint32_t title2)
{
#ifdef USE_LIBASS
	ass.uniform.colour1 = ta->title_colour1 = title1;
	ass.uniform.colour2 = ta->title_colour2 = title2;
#endif

	agl_actor__invalidate((AGlActor*)ta);
}


/*
 *  The strings are owned by the actor and will be freed later.
 */
void
text_actor_set_text (TextActor* ta, char* title, char* text)
{
	if(title){
		g_free0(ta->title);
		ta->title = title;
	}

	g_free0(ta->text);
	ta->text = text;

	ta->title_is_rendered = false;

	agl_actor__invalidate((AGlActor*)ta);
}


#define _r(c)  ( (c)>>24)
#define _g(c)  (((c)>>16)&0xFF)
#define _b(c)  (((c)>> 8)&0xFF)
#define _a(c)  (((c)    )&0xFF)

#define N_CHANNELS 2 // luminance + alpha

#ifdef USE_LIBASS
static void
blend_single(image_t* frame, ASS_Image* img)
{
	// composite img onto frame

	int x, y;
	unsigned char opacity = 255 - _a(img->color);
	unsigned char b = _b(img->color);
	dbg(2, "  %ix%i stride=%i x=%i", img->w, img->h, img->stride, img->dst_x);

	#define LUMINANCE_CHANNEL (x * N_CHANNELS)
	#define ALPHA_CHANNEL (x * N_CHANNELS + 1)
	unsigned char* src = img->bitmap;
	unsigned char* dst = frame->buf + img->dst_y * frame->stride + img->dst_x * N_CHANNELS;
	for (y = 0; y < img->h; ++y) {
		for (x = 0; x < img->w; ++x) {
			unsigned k = ((unsigned) src[x]) * opacity / 255;
			dst[LUMINANCE_CHANNEL] = (k * b + (255 - k) * dst[LUMINANCE_CHANNEL]) / 255;
			dst[ALPHA_CHANNEL] = (k * 255 + (255 - k) * dst[ALPHA_CHANNEL]) / 255;
		}
		src += img->stride;
		dst += frame->stride;
	}
}


static void
text_actor_render_text(TextActor* ta)
{
	AGlActor* actor = (AGlActor*)ta;

	PF;
	if(ta->title_is_rendered) gwarn("title is already rendered");

	GError* error = NULL;
	GRegexMatchFlags flags = 0;
	static GRegex* regex = NULL;
	if(!regex) regex = g_regex_new("[_]", 0, flags, &error);
	gchar* str = g_regex_replace(regex, ta->title, -1, 0, " ", flags, &error);

	char* title = g_strdup_printf("%s", str);
	g_free(str);

	int fh = agl_power_of_two(FONT_SIZE + 4);
	int fw = ta->texture.width = agl_power_of_two(strlen(title) * 20);
	ass_set_frame_size(ass_renderer, fw, fh);

	void title_render(const char* text, image_t* out, Title* t)
	{
		char* script2 = g_strdup_printf(script, fw, fh, FONT_SIZE, text);

		ASS_Track* track = ass_read_memory(ass_library, script2, strlen(script2), NULL);
		g_free(script2);
		if (!track) {
			printf("track init failed!\n");
			return;
		}

		ASS_Image* img = ass_render_frame(ass_renderer, track, 100, NULL);

		*t = (Title){
			.y_offset = fh,
		};

		ASS_Image* i = img;
		for(;i;i=i->next){
			t->height   = MAX(t->height, i->h);
			t->width    = MAX(t->width,  i->dst_x + i->w);
			t->y_offset = MIN(t->y_offset, fh - i->dst_y - i->h); // dst_y is distance from bottom.
		}

		*out = (image_t){fw, fh, fw * N_CHANNELS, g_new0(guchar, fw * fh * N_CHANNELS)};
		for(i=img;i;i=i->next){
			blend_single(out, i); // blend each glyph onto the output buffer.
		}

		ass_free_track(track);
		if(false){
			char* buf = g_new0(char, out->width * out->height * 4);
			int stride = out->width * 4;
			int y; for(y=0;y<fh;y++){
				int x; for(x=0;x<out->width;x++){
					*(buf + y * stride + x * 4    ) = *(out->buf + y * out->stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 1) = 0;
					*(buf + y * stride + x * 4 + 2) = 0;
					*(buf + y * stride + x * 4 + 3) = *(out->buf + y * out->stride + x * N_CHANNELS + 1);
				}
			}
			GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data((const guchar*)buf, GDK_COLORSPACE_RGB, /*HAS_ALPHA_*/TRUE, 8, out->width, out->height, stride, NULL, NULL);
			gdk_pixbuf_save(pixbuf, "tmp1.png", "png", NULL, NULL);
			g_object_unref(pixbuf);
			g_free(buf);
		}

	}

	// do some test renders to find where the baseline is
	{
		image_t out;
		Title t;

		title_render("iey", &out, &t);
		((TextActor*)actor)->baseline = t.height;
		fh = agl_power_of_two(t.height);
		g_free(out.buf);

		title_render("ie", &out, &t);
		((TextActor*)actor)->baseline -= t.height -1; // 1 because of spill below baseline
		g_free(out.buf);
	}

	image_t out;
	title_render(title, &out, &ta->_title);

	{
		if(!((TextActor*)actor)->texture.ids[0]){
			glGenTextures(1, ((TextActor*)actor)->texture.ids);
			if(gl_error){ gerr ("couldnt create ass_texture."); goto out; }
		}
		((TextActor*)actor)->texture.height = out.height;
		agl_actor__set_size(actor);

		int pixel_format = GL_LUMINANCE_ALPHA;
		glBindTexture  (GL_TEXTURE_2D, ((TextActor*)actor)->texture.ids[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, out.width, out.height, 0, pixel_format, GL_UNSIGNED_BYTE, out.buf);
		gl_warn("gl error using ass texture");

#if 0
		{
			char* buf = g_new0(char, out.width * out.height * 4);
			int stride = out.width * 4;
			int y; for(y=0;y<fh;y++){
				int x; for(x=0;x<out.width;x++){
					*(buf + y * stride + x * 4    ) = *(out.buf + y * out.stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 1) = 0;//*(out.buf + y * out.stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 2) = 0;//*(out.buf + y * out.stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 3) = *(out.buf + y * out.stride + x * N_CHANNELS + 1);
				}
			}
			GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data((const guchar*)buf, GDK_COLORSPACE_RGB, /*HAS_ALPHA_*/TRUE, 8, out.width, out.height, stride, NULL, NULL);
			gdk_pixbuf_save(pixbuf, "tmp.png", "png", NULL, NULL);
			g_object_unref(pixbuf);
			g_free(buf);
		}
#endif
	}

	ta->title_is_rendered = true;

  out:
	g_free(out.buf);
	g_free(title);
}
#endif


