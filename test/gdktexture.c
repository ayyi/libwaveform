/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | Show GdkTexture output                                               |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <gtk/gtk.h>
#include "agl/utils.h"
#include "agl/actor.h"
#include "waveform/gdktexture.h"
#include "waveform/text.h"
#include "common2.h"

const char* wavs[] = {
	"stereo_0:10.wav",
	"mono_0:10.wav",
	"mono_10:00.wav",
	"1_block.wav",
	"2_blocks.wav",
	"3_blocks.wav",
};

AGlKeyHandler
	zoom_in,
	zoom_out,
	scroll_left,
	scroll_right,
	next_wav;

extern bool key_down;
extern WfKeyHold key_hold;

AGlKey keys[] = {
	{XK_Left,       scroll_left},
	{XK_Right,      scroll_right},
	{61,            zoom_in},
	{45,            zoom_out},
	{XK_KP_Enter,   NULL},
	{(char)'<',     NULL},
	{(char)'>',     NULL},
	{(char)'n',     next_wav},
	{XK_Delete,     NULL},
	{0},
};

typedef struct {
   GtkWidget* widget;
   Waveform* waveform;
   WfSampleRegion region;
} Item;

static Item items[2];
static float zoom = 1.0;
static int x = 0.0;

static void image_async (Item*, GtkAllocation*);


static void
activate (GtkApplication* app, gpointer user_data)
{
	set_log_handlers();

	wf_debug = 0;

	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "GdkTexture");
	gtk_window_set_default_size (GTK_WINDOW (window), 480, 160);

	GtkWidget* box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_window_set_child (GTK_WINDOW (window), box);

	g_autofree char* filename = find_wav(wavs[0]);
	g_return_if_fail(filename);
	items[0].waveform = waveform_new(filename);
	items[1].waveform = g_object_ref(items[0].waveform);

	items[0].region = (WfSampleRegion){0, waveform_get_n_frames(items[0].waveform) - 1};
	items[1].region = (WfSampleRegion){0, waveform_get_n_frames(items[1].waveform) / 4 - 1};

	// synchronous
	{
		#define item0 items[0]

		if (waveform_load_sync(item0.waveform)) {
			WfPeakParams params = {
				.region = &item0.region,
				.width = 480,
				.height = 80,
				.colour = 0xeeeeeebb,
				.bgcolour = 0x000066ff,
			};
			GdkTexture* texture = waveform_peak_to_texture_sync(item0.waveform, &params);
			item0.widget = gtk_picture_new_for_paintable(GDK_PAINTABLE(texture));
			gtk_widget_set_vexpand (item0.widget, TRUE);
			gtk_box_append (GTK_BOX(box), item0.widget);
		}
	}

	// async
	{
		#define item1 items[1]

		item1.widget = gtk_picture_new();
		gtk_widget_set_vexpand (item1.widget, TRUE);
		gtk_box_append (GTK_BOX(box), item1.widget);

		image_async(&item1, &(GtkAllocation){.width = 480, .height = 80});
	}

	add_key_handlers_gtk((GtkWindow*)window, NULL, (AGlKey*)&keys);

	gtk_widget_set_visible(window, true);
}


static void
image_async (Item*, GtkAllocation* size)
{
	void texture_loaded (Waveform* w, GdkTexture* texture, gpointer area)
	{
		gtk_picture_set_paintable(GTK_PICTURE(item1.widget), GDK_PAINTABLE(texture));
		g_object_unref(texture);
	}

	Item* item = &items[1];

	item->region = (WfSampleRegion){x, waveform_get_n_frames(item->waveform) * MIN(1.0, 1.0 / (zoom * 4)) - 1};

	waveform_peak_to_texture(
		item->waveform,
		&(WfPeakParams){
			.region = &item->region,
			.width = 480,
			.height = 80,
			.colour = 0xeeeeeebb,
			.bgcolour = 0x000066ff,
		},
		texture_loaded,
		NULL
	);
}


void
zoom_in (gpointer _)
{
	zoom *= 1.3;

	item1.region = (WfSampleRegion){x, x + (waveform_get_n_frames(item0.waveform) - 1) / zoom};

	image_async(&item1, &(GtkAllocation){gtk_widget_get_width(item1.widget), gtk_widget_get_height(item1.widget)});
}


void
zoom_out (gpointer _)
{
	zoom = MAX(1., zoom / 1.3);

	item1.region = (WfSampleRegion){x, x + (waveform_get_n_frames(item0.waveform) - 1) / zoom};

	image_async(&item1, &(GtkAllocation){.width=gtk_widget_get_width(item1.widget), .height=gtk_widget_get_height(item1.widget)});
}


void
scroll_left (gpointer _)
{
	x = MAX(0, x - 2000);
	item0.region.start = x;
	gtk_widget_queue_draw(item0.widget);
}


void
scroll_right (gpointer _)
{
	x = MIN(
		waveform_get_n_frames(item0.waveform) - item0.region.len,
		x + 2000
	);
	item0.region.start = x;
	gtk_widget_queue_draw(item0.widget);
}


void
next_wav (gpointer _)
{
	static int w = 0;

	w++;

	g_object_unref(item1.waveform);

	char* filename = find_wav(wavs[w % G_N_ELEMENTS(wavs)]);
	g_return_if_fail(filename);
	item1.waveform = waveform_new(filename);
	g_free(filename);

	image_async(&item1, &(GtkAllocation){.width=gtk_widget_get_width(item1.widget), .height=gtk_widget_get_height(item1.widget)});
}


#include "test/_gtk.c"
