/*

  Show gdk pixbuf output

  --------------------------------------------------------------

  Copyright (C) 2012-2022 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#define __wf_private__

#include "config.h"
#include <gtk/gtk.h>
#include "agl/utils.h"
#include "agl/actor.h"
#include "waveform/view_plus.h"
#include "waveform/pixbuf.h"
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

KeyHandler
	zoom_in,
	zoom_out,
	scroll_left,
	scroll_right,
	next_wav;

extern bool key_down;
extern KeyHold key_hold;

AGlKey keys[] = {
	{KEY_Left,      scroll_left},
	{KEY_Right,     scroll_right},
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
   GdkPixbuf* pixbuf;
} Item;

static Item items[2];
static float zoom = 1.0;
static int x = 0.0;

static void draw_sync   (GtkDrawingArea*, cairo_t*, int width, int height, gpointer);
static void draw_async  (GtkDrawingArea*, cairo_t*, int width, int height, gpointer);
static void image_async (GtkDrawingArea*, GtkAllocation*);


static void
activate (GtkApplication* app, gpointer user_data)
{
	set_log_handlers();

	wf_debug = 0;

	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Pixbuf");
	gtk_window_set_default_size (GTK_WINDOW (window), 480, 160);

	GtkWidget* box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_window_set_child (GTK_WINDOW (window), box);

	char* filename = find_wav(wavs[0]);
	g_return_if_fail(filename);
	items[0].waveform = waveform_new(filename);
	items[1].waveform = g_object_ref(items[0].waveform);
	g_free(filename);

	items[0].region = (WfSampleRegion){0, waveform_get_n_frames(items[0].waveform) - 1};
	items[1].region = (WfSampleRegion){0, waveform_get_n_frames(items[1].waveform) / 4 - 1};

	// synchronous
	{
		#define item0 items[0]

		item0.widget = gtk_drawing_area_new ();
		gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (item0.widget), draw_sync, NULL, NULL);
		gtk_widget_set_vexpand (item0.widget, TRUE);
		gtk_box_append (GTK_BOX(box), item0.widget);
	}

	// async
	{
		#define item1 items[1]

		item1.widget = gtk_drawing_area_new ();
		gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (item1.widget), draw_async, NULL, NULL);
		gtk_widget_set_vexpand (item1.widget, TRUE);
		gtk_box_append (GTK_BOX(box), item1.widget);
	}

	add_key_handlers_gtk((GtkWindow*)window, NULL, (AGlKey*)&keys);

	gtk_widget_show(window);
}


static void draw_sync (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data)
{
	GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, CLAMP(width, 8, 1920), CLAMP(height, 8, 600));
	if (waveform_load_sync(items[0].waveform)) {
		waveform_peak_to_pixbuf(items[0].waveform, pixbuf, &items[0].region, 0xeeeeeebb, 0x000066ff, false);
		gdk_cairo_set_source_pixbuf (cr, pixbuf, 0., 0.);
		cairo_paint(cr);
	}

	g_object_unref(pixbuf);
}


static void
draw_async (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data)
{

	Item* item = &items[1];

	bool changed = !item->pixbuf || gdk_pixbuf_get_width(item->pixbuf) != width || gdk_pixbuf_get_height(item->pixbuf) != height;

	if (changed) {
		image_async(area, &(GtkAllocation){.width = width, .height = height});
	}

	if (item->pixbuf) {
		gdk_cairo_set_source_pixbuf (cr, item->pixbuf, 0., 0.);
		cairo_paint(cr);
	}
}


static void
image_async (GtkDrawingArea* area, GtkAllocation* size)
{
	void pixbuf_loaded (Waveform* w, GdkPixbuf* pixbuf, gpointer area)
	{
		gtk_widget_queue_draw(item1.widget);
	}

	Item* item = &items[1];

	if (item->pixbuf) g_object_unref(item->pixbuf);
	item->region = (WfSampleRegion){x, waveform_get_n_frames(item->waveform) * MIN(1.0, 1.0 / (zoom * 4)) - 1};
	item->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, CLAMP(size->width, 8, 1920), CLAMP(size->height, 8, 600));

	waveform_peak_to_pixbuf_async(item->waveform, item->pixbuf, &item->region, 0xeeeeeebb, 0x000066ff, pixbuf_loaded, area);
}


void
zoom_in (gpointer _)
{
	zoom *= 1.3;

	item0.region = (WfSampleRegion){x, x + (waveform_get_n_frames(item0.waveform) - 1) / zoom};
	gtk_widget_queue_draw(item0.widget);

	image_async((GtkDrawingArea*)item1.widget, &(GtkAllocation){gtk_widget_get_allocated_width(item1.widget), gtk_widget_get_allocated_height(item1.widget)});
}


void
zoom_out (gpointer _)
{
	zoom = MAX(1., zoom / 1.3);

	item0.region = (WfSampleRegion){x, x + (waveform_get_n_frames(item0.waveform) - 1) / zoom};
	gtk_widget_queue_draw(item0.widget);

	image_async((GtkDrawingArea*)item1.widget, &(GtkAllocation){.width=gtk_widget_get_allocated_width(item1.widget), .height=gtk_widget_get_allocated_height(item1.widget)});
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

	image_async((GtkDrawingArea*)item1.widget, &(GtkAllocation){.width=gtk_widget_get_allocated_width(item1.widget), .height=gtk_widget_get_allocated_height(item1.widget)});
}


#include "test/_gtk.c"
