/*

  Test of the libwaveform WaveformView widget in hires mode.
  ----------------------------------------------------------

  Hires mode uses asynchronous loading.
  When the widget is initially loaded in this mode, different
  code paths are excercised.

  A single waveform is displayed.
  The keys +- and cursor left/right keys can be used to zoom and in and scroll.

  --------------------------------------------------------------

  Copyright (C) 2012-2020 Tim Orford <tim@orford.org>

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
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#include <gdk/gdkkeysyms.h>
#include "waveform/view_plus.h"
#include "test/common.h"

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";

static bool  window_on_delete (GtkWidget*, GdkEvent*, gpointer);

#define WAV "short.wav"

gpointer tests[] = {};


int
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ perr("sizeof(off_t)=%zu\n", sizeof(off_t)); return EXIT_FAILURE; }

	set_log_handlers();

	wf_debug = 0;

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				dbg(0, "non-interative");
				g_timeout_add(3000, (gpointer)window_on_delete, NULL);
				break;
		}
	}

	gtk_init(&argc, &argv);
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	WaveformViewPlus* waveform = waveform_view_plus_new(NULL);
	#if 0
	waveform_view_set_show_grid(waveform, true);
	#endif
	gtk_container_add((GtkContainer*)window, (GtkWidget*)waveform);

	gtk_widget_show_all(window);

	char* filename = find_wav(WAV);
	waveform_view_plus_load_file(waveform, filename, NULL, NULL);
	g_free(filename);

	gboolean key_press (GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		WaveformViewPlus* waveform = user_data;
		int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform_view_plus_get_zoom(waveform);

		switch(event->keyval){
			case 61:
				waveform_view_plus_set_zoom(waveform, waveform_view_plus_get_zoom(waveform) * 1.5);
				break;
			case 45:
				waveform_view_plus_set_zoom(waveform, waveform_view_plus_get_zoom(waveform) / 1.5);
				break;
			case KEY_Left:
			case KEY_KP_Left:
				dbg(1, "left");
				waveform_view_plus_set_start(waveform, waveform->start_frame - n_visible_frames / 10);
				break;
			case KEY_Right:
			case KEY_KP_Right:
				dbg(1, "right");
				waveform_view_plus_set_start(waveform, waveform->start_frame + n_visible_frames / 10);
				break;
			case GDK_KP_Enter:
				break;
			case (char)'<':
				break;
			case '>':
				break;
			case 113:
				exit(EXIT_SUCCESS);
				break;
			case GDK_Delete:
				break;
			default:
				dbg(1, "%i", event->keyval);
				break;
		}
		return TRUE;
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), waveform);

	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


static bool
window_on_delete (GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	gtk_main_quit();
	return false;
}
