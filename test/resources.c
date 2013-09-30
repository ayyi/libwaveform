/*

  Test for memory leaks and texture usage.

  --------------------------------------------------------------

  Copyright (C) 2012 Tim Orford <tim@orford.org>

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
#define __ayyi_private__
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
#include "waveform/view.h"
#include "waveform/peak.h"
#include "test/ayyi_utils.h"

#define bool gboolean

static void log_handler (const gchar*, GLogLevelFlags, const gchar*, gpointer);
static void set_log_handlers();

#define WAV1 "test/data/mono_1.wav"
#define WAV2 "test/data/stereo_1.wav"

int
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%i\n", sizeof(off_t)); return EXIT_FAILURE; }

	g_log_set_handler (NULL, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("Gtk", G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("Gdk", G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("GLib", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	set_log_handlers();

	wf_debug = 1;

	gtk_init(&argc, &argv);
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	static WaveformView* waveform; waveform = waveform_view_new(NULL);
	waveform_view_set_show_rms(waveform, false);
	#if 0
	WaveformCanvas* wfc = waveform_view_get_canvas(waveform);
	wf_canvas_set_use_shaders(wfc, false);
	#endif
	gtk_container_add((GtkContainer*)window, (GtkWidget*)waveform);

	gtk_widget_show_all(window);

	void load_wave(const char* wav)
	{
		PF;
		char* filename = g_build_filename(g_get_current_dir(), wav, NULL);
		waveform_view_load_file(waveform, filename);
		g_free(filename);
		//dbg(0, "use_count=%i", waveform->waveform->use_count);
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
		return TIMER_STOP;
	}
	g_timeout_add(3000, swap_wav, NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


static void
log_handler(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data)
{
  switch(log_level){
    case G_LOG_LEVEL_CRITICAL:
      printf("%s %s\n", ayyi_err, message);
      break;
    case G_LOG_LEVEL_WARNING:
      printf("%s %s\n", ayyi_warn, message);
      break;
    default:
      dbg(0, "level=%i %s", log_level, message);
      break;
  }
}


void
set_log_handlers()
{
	char* domain[] = {"Waveform", "GLib-GObject"};
	int i; for(i=0;i<G_N_ELEMENTS(domain);i++){
		g_log_set_handler (domain[i], G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	}
}


