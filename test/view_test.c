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
#include "common.h"

#define bool gboolean

static void  set_log_handlers();
static char* find_wav();

#define WAV \
	"test/data/mono_1.wav"
//	"test/data/stereo_1.wav"
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
	{GDK_KEY_Left,  scroll_left},
	{GDK_KEY_Right, scroll_right},
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
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%i\n", sizeof(off_t)); return EXIT_FAILURE; }

	set_log_handlers();

	wf_debug = 0;

	gtk_init(&argc, &argv);
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	WaveformView* waveform = waveform_view_new(NULL);
	waveform_view_set_show_rms(waveform, false);
	#if 0
	waveform_view_set_show_grid(waveform, true);
	#endif
	#if 0
	gboolean on_idle(gpointer data)
	{
		WaveformView* v = data;
		WaveformCanvas* wfc = waveform_view_get_canvas(v);
		g_return_val_if_fail(wfc, true);
		wfc->blend = false;
		return IDLE_STOP;
	}
	g_idle_add(on_idle, waveform);
	#endif
	#if 0
	wf_canvas_set_use_shaders(wfc, false);
	#endif
	gtk_widget_set_size_request((GtkWidget*)waveform, 256, 128);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)waveform);

	gtk_widget_show_all(window);

	char* filename = find_wav();
	waveform_view_load_file(waveform, filename);
	g_free(filename);

	add_key_handler((GtkWindow*)window, waveform, (Key*)&keys);

	gboolean window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


void
quit(WaveformView* waveform)
{
	exit(EXIT_SUCCESS);
}


void
zoom_in(WaveformView* waveform)
{
	waveform_view_set_zoom(waveform, waveform->zoom * 1.5);
}


void
zoom_out(WaveformView* waveform)
{
	waveform_view_set_zoom(waveform, waveform->zoom / 1.5);
}


void
scroll_left(WaveformView* waveform)
{
	int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	waveform_view_set_start(waveform, waveform->start_frame - n_visible_frames / 10);
}


void scroll_right(WaveformView* waveform)
{
	int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	waveform_view_set_start(waveform, waveform->start_frame + n_visible_frames / 10);
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

	char* domain[] = {"Waveform", "GLib", "GLib-GObject"};
	int i; for(i=0;i<G_N_ELEMENTS(domain);i++){
		g_log_set_handler (domain[i], G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	}
}


static char*
find_wav()
{
	char* filename = g_build_filename(g_get_current_dir(), WAV, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	g_free(filename);
	filename = g_build_filename(g_get_current_dir(), "../", WAV, NULL);
	return filename;
}


