/*

  Show gdk pixbuf output

  --------------------------------------------------------------

  Copyright (C) 2012-2018 Tim Orford <tim@orford.org>

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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "agl/utils.h"
#include "agl/actor.h"
#include "waveform/view_plus.h"
#include "waveform/actors/text.h"
#include "waveform/actors/spp.h"
#include "common.h"

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
	next_wav,
	quit;

extern bool key_down;
extern KeyHold key_hold;

Key keys[] = {
	{KEY_Left,      scroll_left},
	{KEY_Right,     scroll_right},
	{61,            zoom_in},
	{45,            zoom_out},
	{GDK_KP_Enter,  NULL},
	{(char)'<',     NULL},
	{(char)'>',     NULL},
	{(char)'n',     next_wav},
	{GDK_Delete,    NULL},
	{113,           quit},
	{0},
};

gpointer tests[] = {};
uint32_t _time = 1000 + 321;
Waveform* waveform = NULL;
GtkWidget* images[2] = {0,};
WfSampleRegion regions[2];
float zoom = 1.0;
int x = 0.0;

void image_async(GtkAllocation*);


int
main (int argc, char *argv[])
{
	set_log_handlers();

	wf_debug = 0;

	gtk_init(&argc, &argv);
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget* box = gtk_vbox_new(TRUE, 0);
	gtk_container_add((GtkContainer*)window, box);

	char* filename = find_wav(wavs[0]);
	assert_and_stop(filename, "file not found");
	waveform = waveform_new(filename);
	g_free(filename);

	regions[0] = (WfSampleRegion){0, waveform_get_n_frames(waveform) - 1};
	regions[1] = (WfSampleRegion){0, waveform_get_n_frames(waveform) / 4 - 1};

	GtkAllocation size = {.width = 480, .height = 160};

	// synchronous

	void image_sync(GtkAllocation* size)
	{
		GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, CLAMP(size->width, 8, 1920), CLAMP(size->height / 2, 8, 600));
		if(waveform_load_sync(waveform)){
			waveform_peak_to_pixbuf(waveform, pixbuf, &(WfSampleRegion){0, waveform_get_n_frames(waveform) - 1}, 0xeeeeeebb, 0x000066ff, false);
			gtk_image_set_from_pixbuf((GtkImage*)images[0], pixbuf);
		}
		g_object_unref(pixbuf);
	}

	images[0] = gtk_image_new();
	image_sync(&size);
	gtk_box_pack_start((GtkBox*)box, images[0], false, false, 0);

	// async

	images[1] = gtk_image_new();
	image_async(&size);
	gtk_box_pack_start((GtkBox*)box, images[1], false, false, 0);

	gtk_widget_show_all(window);

	add_key_handlers_gtk((GtkWindow*)window, NULL, (Key*)&keys);

	bool window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	void on_allocate(GtkWidget* widget, GtkAllocation* allocation, gpointer view)
	{
		static GtkAllocation a = {0,};

		if(allocation->height != a.height || allocation->width != a.width){

			image_sync(allocation);
			image_async(allocation);

			a = *allocation;
		}
	}
	g_signal_connect(window, "size-allocate", G_CALLBACK(on_allocate), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


void
image_async(GtkAllocation* size)
{
	void pixbuf_loaded(Waveform* w, GdkPixbuf* pixbuf, gpointer _)
	{
		gtk_image_set_from_pixbuf((GtkImage*)images[1], pixbuf);
		g_object_unref(pixbuf);
	}
	regions[1] = (WfSampleRegion){x, waveform_get_n_frames(waveform) * MIN(1.0, 1.0 / (zoom * 4)) - 1};
	GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, CLAMP(size->width, 8, 1920), CLAMP(size->height / 2, 8, 600));
	waveform_peak_to_pixbuf_async(waveform, pixbuf, &regions[1], 0xeeeeeebb, 0x000066ff, pixbuf_loaded, NULL);
}

void
quit(gpointer _)
{
	exit(EXIT_SUCCESS);
}


void
zoom_in(gpointer _)
{
	zoom *= 1.3;
	GtkAllocation size = {.width = 480, .height = 160};
	//image_sync(&size);
	image_async(&size);
}


void
zoom_out(gpointer _)
{
	zoom /= 1.3;
	GtkAllocation size = {.width = 480, .height = 160};
	//image_sync(&size);
	image_async(&size);
}


void
scroll_left(gpointer _)
{
	x = MAX(0, x - 2000);
	GtkAllocation size = {.width = 480, .height = 160};
	image_async(&size);
}


void scroll_right(gpointer _)
{
	x = MIN(waveform_get_n_frames(waveform) - regions[1].len, x + 2000);
	GtkAllocation size = {.width = 480, .height = 160};
	image_async(&size);
}


void next_wav(gpointer _)
{
}


