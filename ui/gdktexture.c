/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <gdk/gdk.h>
#include "waveform/pixbuf.h"
#include "ui/gdktexture.h"

GdkTexture*
waveform_peak_to_texture_sync (Waveform* waveform, WfPeakParams* params)
{
	AGlPixbuf pixbuf = {
		.width = params->width,
		.height = params->height,
		.rowstride = 3 * params->width,
		.data = g_malloc0(3 * params->width * params->height),
	};
	waveform_peak_to_pixbuf(waveform, &pixbuf, params);

	GBytes* bytes = g_bytes_new_take(pixbuf.data, pixbuf.rowstride * pixbuf.height);
	GdkTexture* texture = gdk_memory_texture_new(params->width, params->height, GDK_MEMORY_R8G8B8, bytes, pixbuf.rowstride);
	g_bytes_unref(bytes);

	return texture;
}

void
waveform_peak_to_texture (Waveform* waveform, WfPeakParams* spec, WfGdkTextureCallback callback, gpointer user_data)
{
	typedef struct {
		WfGdkTextureCallback* callback;
		gpointer user_data;
	} C;

	void pixbuf_loaded (Waveform* w, GdkPixbuf* pixbuf, gpointer _c)
	{
		C* c = _c;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		GdkTexture* texture = gdk_texture_new_for_pixbuf(pixbuf);
#pragma GCC diagnostic pop
		c->callback(w, texture, c->user_data);

		g_free(c);
	}

	GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, CLAMP(spec->width, 8, 1920), CLAMP(spec->height, 8, 600));
	waveform_peak_to_pixbuf_async(waveform, pixbuf, spec->region, spec->colour, spec->bgcolour, pixbuf_loaded, WF_NEW(C,
		callback = callback,
		user_data = user_data,
	));
}
