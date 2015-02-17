/*

  Demonstration of the libwaveform WaveformViewPlus widget.
  ---------------------------------------------------------

  It displays a single waveform.
  The keys +- and cursor left/right keys can be used to zoom and in and scroll.

  The two important relevant lines are:
      WaveformView* waveform = waveform_view_plus_new(NULL);
      waveform_view_plus_load_file(waveform, "test/data/mono_1.wav");
  This will create a new Gtk widget that you pack and show as normal.

  In addition to the functions above that are also available in
  the simpler WaveformView widget, further options are available which
  support the display of title text and info text:
      void waveform_view_plus_set_title  (WaveformViewPlus*, const char*);
      void waveform_view_plus_set_text   (WaveformViewPlus*, const char*);
      void waveform_view_plus_set_colour (WaveformViewPlus*, uint32_t fg, uint32_t bg, uint32_t title1, uint32_t title2);

  It also offers a 'play counter'. To display a cursor and readout of the
  current time, set the time to a non-default value and redraw.
      void waveform_view_plus_set_time  (WaveformViewPlus*, uint32_t time_in_milliseconds);

  The WaveformView interface is designed to be easy to use.
  For a more powerful but more complicated interface, see WaveformActor

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
#include "waveform/view_plus.h"
#include "test/ayyi_utils.h"
#include "common.h"

#define bool gboolean

const char* wavs[] = {
	"test/data/stereo_1.wav",
	"test/data/mono_1.wav",
//	"test/data/1_block.wav",
//	"test/data/3_blocks.wav",
//	"test/data/2_blocks.wav",
};

KeyHandler
	zoom_in,
	zoom_out,
	scroll_left,
	scroll_right,
	next_wav,
	toggle_shaders,
	toggle_grid,
	play,
	stop,
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
	{(char)'s',     toggle_shaders},
	{(char)'g',     toggle_grid},
	{GDK_Delete,    NULL},
	{65438,         stop},
	{65421,         play},
	{113,           quit},
	{0},
};

gpointer tests[] = {};
uint32_t _time = 1000 + 321;


int
main (int argc, char *argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%i\n", sizeof(off_t)); return EXIT_FAILURE; }

	set_log_handlers();

	wf_debug = 0;

	gtk_init(&argc, &argv);
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	#if 0
	agl_get_instance()->pref_use_shaders = false;
	#endif

	WaveformViewPlus* waveform = waveform_view_plus_new(NULL);
	waveform_view_plus_set_title(waveform, "Waveform Title");
	waveform_view_plus_set_text(waveform, "Waveform text waveform text");
	waveform_view_plus_set_show_rms(waveform, false);
	waveform_view_plus_set_colour(waveform, 0xccccddaa, 0x000000ff, 0x33aaffff, 0xffff00ff);
	waveform_view_plus_set_time(waveform, _time);
	waveform_view_plus_set_show_grid(waveform, true);
	#if 0
	wf_canvas_set_use_shaders(wfc, false);
	#endif
	gtk_widget_set_size_request((GtkWidget*)waveform, 480, 160);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)waveform);

	gtk_widget_show_all(window);

	char* filename = find_wav(wavs[0]);
	waveform_view_plus_load_file(waveform, filename);
	g_free(filename);

	add_key_handlers((GtkWindow*)window, (WaveformView*)waveform, (Key*)&keys);

	bool window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
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
	waveform_view_plus_set_zoom((WaveformViewPlus*)waveform, waveform->zoom * 1.5);
}


void
zoom_out(WaveformView* waveform)
{
	waveform_view_plus_set_zoom((WaveformViewPlus*)waveform, waveform->zoom / 1.5);
}


void
scroll_left(WaveformView* waveform)
{
	int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	waveform_view_plus_set_start((WaveformViewPlus*)waveform, waveform->start_frame - n_visible_frames / 10);
}


void scroll_right(WaveformView* waveform)
{
	int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	waveform_view_plus_set_start((WaveformViewPlus*)waveform, waveform->start_frame + n_visible_frames / 10);
}


void next_wav(WaveformView* waveform)
{
	WaveformViewPlus* view = (WaveformViewPlus*)waveform;

	static int i = 0; i = (i + 1) % 2;

	printf("next...\n");

	typedef struct {
		WaveformViewPlus* view;
		float             zoom;
	} C;
	C* c = g_new0(C, 1);
	*c = (C){
		.view = view,
		.zoom = view->zoom
	};

	char* filename = find_wav(wavs[i]);
	waveform_view_plus_load_file(view, filename);
	g_free(filename);

	// TODO fix widget so that zoom can be set imediately

	bool on_loaded(gpointer _c)
	{
		C* c = _c;
		waveform_view_plus_set_zoom(c->view, c->zoom);
		g_free(c);
		return G_SOURCE_REMOVE;
	}
	g_idle_add(on_loaded, c);
}


void
toggle_shaders(WaveformView* view)
{
	printf("...\n");
	WaveformCanvas* wfc = waveform_view_get_canvas(view);
	wf_canvas_set_use_shaders(wfc, !agl_get_instance()->use_shaders);

	char* filename = find_wav(wavs[0]);
	waveform_view_plus_load_file((WaveformViewPlus*)view, filename);
	g_free(filename);
}



void
toggle_grid(WaveformView* view)
{
	static bool visible = true;
	waveform_view_plus_set_show_grid((WaveformViewPlus*)view, visible = !visible);
}


static guint play_timer = 0;

void
stop(WaveformView* waveform)
{
	if(play_timer){
		g_source_remove (play_timer);
		play_timer = 0;
	}else{
		waveform_view_plus_set_time((WaveformViewPlus*)waveform, (_time = 0));
	}
}


void
play(WaveformView* waveform)
{
	bool tick(gpointer waveform)
	{
		waveform_view_plus_set_time((WaveformViewPlus*)waveform, (_time += 50, _time));
		return true;
	}

	if(!play_timer) play_timer = g_timeout_add(50, tick, waveform);
}


