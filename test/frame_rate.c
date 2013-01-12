/*

  Test program to measure framerates using the libwaveform WaveformView
  widget.

  Redraws are requested at 100Hz and the resultant number of draws
  is counted.

  On budget hardware, full screen drawing while continuously animating
  doesnt report more than 1% cpu usage, even when frame-rate drops.
  With all effects enabled, framerate drops to 18fps at fullscreen,
  though this is due entirely to the shader fx.
  (2012.10.05).

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
#include "test/ayyi_utils.h"

#define bool gboolean

static void     set_log_handlers ();
static uint64_t get_time         ();


int
main (int argc, char* argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%i\n", sizeof(off_t)); return EXIT_FAILURE; }

	set_log_handlers();

	gtk_init(&argc, &argv);
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	WaveformView* waveform = waveform_view_new(NULL);
	waveform_view_set_show_rms(waveform, false);
	#if 0
	WaveformCanvas* wfc = waveform_view_get_canvas(waveform);
	wf_canvas_set_use_shaders(wfc, false);
	#endif
	gtk_container_add((GtkContainer*)window, (GtkWidget*)waveform);

	gtk_widget_show_all(window);

	//#define WAV "test/data/mono_1.wav"
	#define WAV "test/data/stereo_1.wav"
	char* filename = g_build_filename(g_get_current_dir(), WAV, NULL);
	waveform_view_load_file(waveform, filename);
	g_free(filename);

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
			case GDK_KEY_Left:
			case GDK_KEY_KP_Left:
				dbg(0, "left");
				waveform_view_set_start(waveform, waveform->start_frame - 8192 / waveform->zoom);
				break;
			case GDK_KEY_Right:
			case GDK_KEY_KP_Right:
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
	g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

	gboolean on_timeout(gpointer _waveform)
	{
		WaveformView* waveform = _waveform;

		static uint64_t frame = 0;
		static uint64_t t0    = 0;
		if(!frame) t0 = get_time();
		else{
			uint64_t time = get_time();
			if(!(frame % 200))
				printf("rate=%.1f fps\n", ((float)frame / ((float)time - t0)) * 1000.0);

			if(!(frame % 8)){
				float v = (frame % 16) ? 1.5 : 2.0/3.0;
				if(v > 16.0) v = 1.0;
				waveform_view_set_zoom(waveform, waveform->zoom * v);
			}
			gtk_widget_queue_draw((GtkWidget*)waveform);
		}
		frame++;
		return IDLE_CONTINUE;
	}
	g_timeout_add(15, on_timeout, waveform);

	gtk_main();

	return EXIT_SUCCESS;
}


uint64_t
get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


void
set_log_handlers()
{
	void log_handler(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data)
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

	g_log_set_handler (NULL, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("Gtk", G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("Gdk", G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("GLib", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("GLib-GObject", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	char* domain[] = {"Waveform"};
	int i; for(i=0;i<G_N_ELEMENTS(domain);i++){
		g_log_set_handler (domain[i], G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	}
}


