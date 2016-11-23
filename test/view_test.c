/*

  Demonstration of the libwaveform WaveformView widget.
  -----------------------------------------------------

  It displays a single waveform.
  The keys +- and cursor left/right keys can be used to zoom and in and scroll.

  The two important relevant lines are:
      WaveformView* waveform = waveform_view_new(NULL);
      waveform_view_load_file(waveform, "test/data/mono_1.wav");
  This will create a new Gtk widget that you pack and show as normal.

  The WaveformView interface is designed to be easy to use.
  For a more powerful but complicated interface, see WaveformActor

  --------------------------------------------------------------

  Copyright (C) 2012-2015 Tim Orford <tim@orford.org>

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
#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "agl/utils.h"
#include "waveform/view.h"
#include "test/ayyi_utils.h"
#include "common.h"

#define WAV \
	"test/data/mono_0:10.wav"
//	"test/data/mono_1.mp3"
//	"test/data/large1.wav"
//	"test/data/stereo_1.wav"
//	"test/data/stereo_1.mp3"
//	"test/data/1_block.wav"
//	"test/data/3_blocks.wav"
//	"test/data/2_blocks.wav"

KeyHandler
	zoom_in,
	zoom_out,
	scroll_left,
	scroll_right,
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
	{GDK_Delete,    NULL},
	{113,           quit},
	{0},
};

gpointer tests[] = {};


int
main (int argc, char* argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%zu\n", sizeof(off_t)); return EXIT_FAILURE; }

	set_log_handlers();

	wf_debug = 0;

	gtk_init(&argc, &argv);
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	#if 0
	agl_get_instance()->pref_use_shaders = false;
	#endif

	WaveformView* waveform = waveform_view_new(NULL);

	#if 0
	waveform_view_set_show_rms(waveform, false);
	waveform_view_add_layer(waveform, grid_actor(waveform_view_get_actor(waveform)), 0);
	#endif

	gtk_widget_set_size_request((GtkWidget*)waveform, 512, 256);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)waveform);

	gtk_widget_show_all(window);

	char* filename = find_wav(WAV);
	waveform_view_load_file(waveform, filename);
	g_free(filename);

	add_key_handlers((GtkWindow*)window, waveform, (Key*)&keys);

	gboolean window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


void
quit(gpointer waveform)
{
	exit(EXIT_SUCCESS);
}


void
zoom_in(gpointer _view)
{
	WaveformView* view = (WaveformView*)_view;

	waveform_view_set_zoom(view, view->zoom * 1.5);
}


void
zoom_out(gpointer _view)
{
	WaveformView* view = (WaveformView*)_view;

	waveform_view_set_zoom(view, view->zoom / 1.5);
}


void
scroll_left(gpointer _view)
{
	WaveformView* view = (WaveformView*)_view;

	int n_visible_frames = ((float)view->waveform->n_frames) / view->zoom;
	waveform_view_set_start(view, view->start_frame - n_visible_frames / 10);
}


void
scroll_right(gpointer _view)
{
	WaveformView* view = (WaveformView*)_view;

	int n_visible_frames = ((float)view->waveform->n_frames) / view->zoom;
	waveform_view_set_start(view, view->start_frame + n_visible_frames / 10);
}


