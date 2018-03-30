/*

  Test for memory leaks and texture usage.

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
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "waveform/waveform.h"
#include "waveform/view.h"
#include "test/common2.h"

#define WAV1 "mono_0:10.wav"
#define WAV2 "stereo_0:10.wav"

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";


int
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%zu\n", sizeof(off_t)); return EXIT_FAILURE; }

	set_log_handlers();

	wf_debug = 0;

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(4000, (gpointer)exit, NULL);
				break;
		}
	}

	gtk_init(&argc, &argv);
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	static WaveformView* waveform; waveform = waveform_view_new(NULL);
	waveform_view_set_show_rms(waveform, false);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)waveform);

	gtk_widget_show_all(window);

	void load_wave(const char* wav)
	{
		PF;
		char* filename = find_wav(wav);
		waveform_view_load_file(waveform, filename);
		g_free(filename);
	}
	load_wave(WAV1);

	gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		WaveformView* waveform = user_data;

		switch(event->keyval){
			case 61:
				waveform_view_set_zoom(waveform, waveform->zoom * 1.5);
				break;
			case 45:
				waveform_view_set_zoom(waveform, waveform->zoom / 1.5);
				break;
			case KEY_Left:
			case KEY_KP_Left:
				dbg(0, "left");
				waveform_view_set_start(waveform, waveform->start_frame - 8192 / waveform->zoom);
				break;
			case KEY_Right:
			case KEY_KP_Right:
				dbg(0, "right");
				waveform_view_set_start(waveform, waveform->start_frame + 8192 / waveform->zoom);
				break;
			case GDK_KP_Enter:
				break;
			case 113:
				exit(EXIT_SUCCESS);
				break;
			case GDK_Delete:
				break;
			default:
				dbg(0, "%i", event->keyval);
				break;
		}
		return TRUE;
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), waveform);

	gboolean window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gboolean swap_wav(gpointer data)
	{
		load_wave(WAV2);
		return G_SOURCE_REMOVE;
	}
	g_timeout_add(3000, swap_wav, NULL);

	gtk_main();

	return EXIT_SUCCESS;
}



