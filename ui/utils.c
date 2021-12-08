/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include <math.h>
#include <sys/ioctl.h>
#include <glib.h>
#ifdef USE_GTK
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif
#include "transition/frameclock.h"
#include "agl/actor.h"
#include "wf/debug.h"
#include "wf/waveform.h"
#define __wf_canvas_priv__
#include "ui/context.h"
#include "ui/pixbuf.h"
#include "ui/utils.h"


#undef SHOW_TIME
#ifdef SHOW_TIME
static uint64_t _get_time ()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}
#endif


void
wf_deinterleave (float* src, float** dest, uint64_t n_frames)
{
	int f; for(f=0;f<n_frames;f++){
		int c; for(c=0;c<WF_STEREO;c++){
			dest[c][f] = src[f * WF_STEREO + c];
		}
	}
}


void
wf_deinterleave16 (short* src, short** dest, uint64_t n_frames)
{
	int f; for(f=0;f<n_frames;f++){
		int c; for(c=0;c<WF_STEREO;c++){
			dest[c][f] = src[f * WF_STEREO + c];
		}
	}
}


float
wf_int2db (short x)
{
	//converts a signed 16bit int to a dB value.

	float y;

	if(x != 0){
		y = -20.0 * log10(32768.0/ABS(x));
		//printf("int2db: %f\n", 32768.0/abs(x));
	} else {
		y = -100.0;
	}

	return y;
}


//perhaps make all gtk stuff private to the widgets
#ifdef USE_GTK

uint32_t
wf_get_gtk_fg_color (GtkWidget* widget, GtkStateType state)
{
	GtkStyle* style = gtk_widget_get_style(widget);
	//GdkColor fg = style->fg[state];
	return wf_color_gdk_to_rgba(&style->fg[state]);
}


uint32_t
wf_get_gtk_text_color (GtkWidget* widget, GtkStateType state)
{
	GtkStyle* style = gtk_style_copy(gtk_widget_get_style(widget));
	GdkColor c = style->text[state];
	g_object_unref(style);

	return wf_color_gdk_to_rgba(&c);
}


uint32_t
wf_get_gtk_base_color (GtkWidget* widget, GtkStateType state, char alpha)
{
	GtkStyle* style = gtk_style_copy(gtk_widget_get_style(widget));
	GdkColor c = style->base[state];
	g_object_unref(style);

	return (wf_color_gdk_to_rgba(&c) & 0xffffff00) | alpha;
}
#endif


void
wf_colour_rgba_to_float (AGlColourFloat* colour, uint32_t rgba)
{
	//returned values are in the range 0.0 to 1.0;

	g_return_if_fail(colour);

	colour->r = (float)((rgba & 0xff000000) >> 24) / 0xff;
	colour->g = (float)((rgba & 0x00ff0000) >> 16) / 0xff;
	colour->b = (float)((rgba & 0x0000ff00) >>  8) / 0xff;
}


#ifdef USE_GTK
uint32_t
wf_color_gdk_to_rgba (GdkColor* color)
{
	return ((color->red / 0x100) << 24) + ((color->green / 0x100) << 16) + ((color->blue / 0x100) << 8) + 0xff;
}
#endif


bool
wf_colour_is_dark_rgba (uint32_t colour)
{
	int r = (colour & 0xff000000) >> 24;
	int g = (colour & 0x00ff0000) >> 16;
	int b = (colour & 0x0000ff00) >>  8;

	int average = (r + g + b ) / 3;
	return (average < 0x80);
}


guint64
wf_get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


#ifdef USE_GTK
#define WAVEFORM_START_DRAW(wfc) \
	if(wfc->_draw_depth) pwarn("START_DRAW: already drawing"); \
	wfc->_draw_depth++; \
	if (actor_not_is_gtk(wfc->root->root) || \
		(wfc->_draw_depth > 1) || gdk_gl_drawable_make_current (wfc->root->root->gl.gdk.drawable, wfc->root->root->gl.gdk.context) \
		) {
#else
#define WAVEFORM_START_DRAW(wfc) \
	;
#endif

#ifdef USE_GTK
#define WAVEFORM_END_DRAW(wa) \
	wa->_draw_depth--; \
	if(wa->root->root->type == CONTEXT_TYPE_GTK){ \
		if(!wa->_draw_depth) ; \
	} \
	} else pwarn("!! gl_begin fail")
#else
#define WAVEFORM_END_DRAW(wa) \
	;
#endif

#define WAVEFORM_IS_DRAWING(wa) \
	(wa->_draw_depth > 0)


/*
 *  Load the Alphabuf into the gl texture identified by texture_name.
 *  -the user can usually free the Alphabuf afterwards as it is unlikely to be needed again.
 */
void
wf_load_texture_from_alphabuf (WaveformContext* wfc, int texture_name, AlphaBuf* alphabuf)
{
	g_return_if_fail(alphabuf);
	g_return_if_fail(texture_name);

	AGl* agl = agl_get_instance();

#ifdef USE_MIPMAPPING
	guchar* generate_mipmap (AlphaBuf* a, int level)
	{
		int r = 1 << level;
		int height = MAX(1, a->height / r);
		int width = agl->have & AGL_HAVE_NPOT_TEXTURES ? MAX(1, a->width / r) : height;
		guchar* buf = g_malloc(width * height);

		for (int y=0;y<height;y++) {
			for (int x=0;x<width;x++) {
				//TODO find max of all peaks, dont just use one.
				buf[width * y + x] = a->buf[a->width * y * r + x * r];
			}
		}
		return buf;
	}
#endif

	WAVEFORM_START_DRAW(wfc) {
		//note: gluBuild2DMipmaps is deprecated. instead use GL_GENERATE_MIPMAP (requires GL 1.4)

		glBindTexture(GL_TEXTURE_2D, texture_name);
		int width = agl->have & AGL_HAVE_NPOT_TEXTURES ? alphabuf->width : alphabuf->height;
		dbg (2, "copying texture... width=%i texture_id=%u", width, texture_name);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8, width, alphabuf->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, alphabuf->buf);

#ifdef USE_MIPMAPPING
		{
			for(int l=1;l<16;l++){
				guchar* buf = generate_mipmap(alphabuf, l);
				int width = (agl->have & AGL_HAVE_NPOT_TEXTURES ? alphabuf->width : alphabuf->height) / (1<<l);
				int width = alphabuf->height / (1<<l);
				glTexImage2D(GL_TEXTURE_2D, l, GL_ALPHA8, width, alphabuf->height/(1<<l), 0, GL_ALPHA, GL_UNSIGNED_BYTE, buf);
				wf_free(buf);
				int w = alphabuf->width / (1<<l);
				int h = alphabuf->height / (1<<l);
				if ((w < 2) && (h < 2)) break;
			}
		}
#endif

#ifdef USE_MIPMAPPING
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
#else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
		// if TEX_BORDER is used, clamping will make no difference as we dont reach the edge of the texture.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //note using this stops gaps between blocks.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // prevent wrapping. GL_CLAMP_TO_EDGE uses the nearest texture value, and will not fade to the border colour like GL_CLAMP
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); //prevent wrapping

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		if (!glIsTexture(texture_name)) pwarn("texture not loaded! %i", texture_name);
	} WAVEFORM_END_DRAW(wfc);

	gl_warn("copy to texture");
}


static void
wf_context_init_gl (WaveformContext* wfc)
{
	PF;

	AGl* agl = agl_get_instance();

	if (!agl->pref_use_shaders) {
		wfc->use_1d_textures = false;
		return;
	}

	WAVEFORM_START_DRAW(wfc) {

		if (!wfc->root) {
			agl_gl_init();
		}

		if (!agl->use_shaders) {
			agl_use_program(NULL);
			wfc->use_1d_textures = false;
		}

	} WAVEFORM_END_DRAW(wfc);
}


static gboolean
__wf_canvas_try_drawable (gpointer _wfc)
{
	WaveformContext* wfc = _wfc;
	AGlScene* scene = wfc->root->root;

	AGl* agl = agl_get_instance();

#ifdef USE_GTK
	if ((scene->type == CONTEXT_TYPE_GTK) && !wfc->root->root->gl.gdk.drawable) {
		return G_SOURCE_CONTINUE;
	}
#endif

	wf_context_init_gl(wfc);

	if (scene->draw) wf_context_queue_redraw(wfc);
	wfc->use_1d_textures = agl->use_shaders;

	return (wfc->priv->pending_init = G_SOURCE_REMOVE);
}


void
wf_gl_init (WaveformContext* wfc, AGlActor* root)
{
	if (wfc->root) {
		if (__wf_canvas_try_drawable(wfc)) wfc->priv->pending_init = g_idle_add(__wf_canvas_try_drawable, wfc);
	}
}


#if 0
#include "agl/utils.h"
void
fbo_2_png(AGlFBO* fbo)
{
	int width = agl_power_of_two(fbo->width);
	int height = agl_power_of_two(fbo->height);

	unsigned char buffer[width * height * 4];
	glBindTexture(GL_TEXTURE_2D, fbo->texture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(buffer, GDK_COLORSPACE_RGB, TRUE, 8, width, height, width*4, NULL, NULL);
	gdk_pixbuf_save(pixbuf, "test.png", "png", NULL, NULL);
	g_object_unref(pixbuf);
}
#endif


#ifndef USE_OPENGL
int
agl_power_of_two (guint a)
{
	// return the next power of two up from the given value.

	int i = 0;
	int orig = a;
	a = MAX(1, a - 1);
	while (a) {
		a = a >> 1;
		i++;
	}
	dbg (3, "%i -> %i", orig, 1 << i);
	return 1 << i;
}
#endif
