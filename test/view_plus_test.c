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
  the simpler WaveformView widget, additional 'layers' can be added,
  supporting, for example, the display of title text and info text:

    AGlActor* text_layer = waveform_view_plus_add_layer(waveform, text_actor(NULL), 0);
    ((TextActor*)text_layer)->title = g_strdup("Waveform Title");
    text_actor_set_colour((TextActor*)text_layer, 0x33aaffff, 0xffff00ff);

  The SPP layer provides a 'play counter'. To display a cursor and readout
  of the current time, set the time to a non-default value and redraw.

    AGlActor* spp = waveform_view_plus_get_layer(waveform, 5);
    wf_spp_actor_set_time((SppActor*)spp, time_in_milliseconds);

  The WaveformView interface is designed to be easy to use.
  For a more powerful but more complicated interface, see WaveformActor

  --------------------------------------------------------------

  Copyright (C) 2012-2019 Tim Orford <tim@orford.org>

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "agl/actor.h"
#include "waveform/view_plus.h"
#include "common.h"

extern char* basename(const char*);

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";

const char* wavs[] = {
	"data/mono_0:10.flac",
	"data/stereo_0:10.flac",
	"data/stereo_0:10.wav",
	"data/1_block.wav",
	"data/2_blocks.wav",
	"data/mono_0:10.mp3",
	"data/mono_10:00.wav",
	"data/mono_0:10.opus",
	"data/mono_0:10.m4a",
	"data/stereo_0:10.opus",
	"data/mono_0:10.wav",
	"data/piano.wav",
	//"data/u8.wav",
};

KeyHandler
	next_wav,
	prev_wav,
	toggle_shaders,
	toggle_grid,
	unrealise,
	delete,
	play,
	stop,
	quit;

extern bool key_down;
extern KeyHold key_hold;

Key keys[] = {
	{GDK_KP_Enter,  NULL},
	{'<',           NULL},
	{'>',           NULL},
	{(char)'n',     next_wav},
	{(char)'p',     prev_wav},
	{(char)'s',     toggle_shaders},
	{(char)'g',     toggle_grid},
	{(char)'u',     unrealise},
	{GDK_Delete,    delete},
	{65438,         stop},
	{65421,         play},
	{113,           quit},
	{0},
};

gpointer tests[] = {};
uint32_t _time = 1000 + 321;
GtkWidget* table = NULL;
WaveformViewPlus* view = NULL;
struct Layers {
    AGlActor* grid;
    AGlActor* spp;
    AGlActor* spinner;
} layers;

void  show_wav        (WaveformViewPlus*, const char* filename);
char* format_channels (int n_channels);
void  format_time     (char* str, int64_t t_ms);
void  on_allocate     (GtkWidget*, GtkAllocation*, gpointer);


	static bool window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}

int
main (int argc, char* argv[])
{
	if(sizeof(off_t) != 8){ gerr("sizeof(off_t)=%zu\n", sizeof(off_t)); return EXIT_FAILURE; }

	set_log_handlers();

	wf_debug = 0;

	gtk_init(&argc, &argv);

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(3000, (gpointer)window_on_delete, NULL);
				break;
		}
	}

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	#if 0
	agl_get_instance()->pref_use_shaders = false;
	#endif

	WaveformViewPlus* waveform = view = waveform_view_plus_new(NULL);
	waveform_view_plus_set_show_rms(waveform, false);
	waveform_view_plus_set_colour(waveform, 0xccccddaa, 0x000000ff);

	waveform_view_plus_add_layer(waveform, background_actor(NULL), 0);

	layers.grid = waveform_view_plus_add_layer(waveform, grid_actor(waveform_view_plus_get_actor(waveform)), 0);

	AGlActor* text_layer = waveform_view_plus_add_layer(waveform, text_actor(NULL), 3);
	((TextActor*)text_layer)->title = g_strdup("Waveform Title");
	((TextActor*)text_layer)->text = g_strdup("Waveform text waveform text");
	text_actor_set_colour((TextActor*)text_layer, 0x33aaffff, 0xffff00ff);

	layers.spp = waveform_view_plus_add_layer(waveform, wf_spp_actor(waveform_view_plus_get_actor(waveform)), 0);
	wf_spp_actor_set_time((SppActor*)layers.spp, (_time += 50, _time));

	layers.spinner = waveform_view_plus_add_layer(waveform, wf_spinner(waveform_view_plus_get_actor(waveform)), 0);

	char* filename = find_wav(wavs[0]);
	show_wav(waveform, filename);
	g_free(filename);

#if 0
	waveform_view_plus_set_region(waveform, 0, 32383); // start in hi-res mode
#endif

	gtk_widget_set_size_request((GtkWidget*)waveform, 640, 160);

	GtkWidget* scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	gtk_container_add((GtkContainer*)window, scrolledwindow);

	table = gtk_table_new(1, 2, false);
	gtk_table_attach(GTK_TABLE(table), (GtkWidget*)waveform, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_scrolled_window_add_with_viewport((GtkScrolledWindow*)scrolledwindow, table);

	gtk_widget_show_all(window);

	add_key_handlers_gtk((GtkWindow*)window, (WaveformView*)waveform, (Key*)&keys);

	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	g_signal_connect(window, "size-allocate", G_CALLBACK(on_allocate), waveform);

	gtk_main();

	return EXIT_SUCCESS;
}


void
on_allocate (GtkWidget* widget, GtkAllocation* allocation, gpointer _view)
{
	if(!view) return;

	static int height = 0;

	if(allocation->height != height){
		gtk_widget_set_size_request((GtkWidget*)view, -1, allocation->height);
		height = allocation->height;
	}
}


void
quit (gpointer waveform)
{
	exit(EXIT_SUCCESS);
}


	typedef struct {
		WaveformViewPlus* view;
		float             zoom;
	} C;

	void on_loaded_(Waveform* w, GError* error, gpointer _view)
	{
		WaveformViewPlus* view = _view;

		wf_spinner_stop((WfSpinner*)layers.spinner);

		if(error){
			AGlActor* text_layer = waveform_view_plus_get_layer(view, 3);
			if(text_layer){
				text_actor_set_text(((TextActor*)text_layer), NULL, g_strdup(error->message));
			}
		}
	}

	bool on_loaded(gpointer _c)
	{
		C* c = _c;
#ifndef USE_CANVAS_SCALING
		waveform_view_plus_set_zoom(c->view, c->zoom);
#endif
		g_free(c);
		return G_SOURCE_REMOVE;
	}

void
show_wav (WaveformViewPlus* view, const char* filename)
{
	g_assert(filename);

	C* c = AGL_NEW(C,
		.view = view,
#ifndef USE_CANVAS_SCALING
		.zoom = view->zoom
#endif
	);

	wf_spinner_start((WfSpinner*)layers.spinner);

	waveform_view_plus_load_file(view, filename, on_loaded_, view);

	g_idle_add(on_loaded, c);

	g_assert(view->waveform);

	AGlActor* text_layer = waveform_view_plus_get_layer(view, 3);
	if(text_layer){
		char* text = NULL;
		Waveform* w = view->waveform;
		if(w->n_channels){
			char* ch_str = format_channels(w->n_channels);
			char length[32]; format_time(length, (w->n_frames * 1000)/w->samplerate);
			char fs_str[32] = {'\0'}; //samplerate_format(fs_str, sample->sample_rate); strcpy(fs_str + strlen(fs_str), " kHz");

			text = g_strdup_printf("%s  %s  %s", length, ch_str, fs_str);

			g_free(ch_str);
		}

		text_actor_set_text(((TextActor*)text_layer), g_strdup(basename(filename)), text);
	}
}


static int i = 0;

void
next_wav (gpointer view)
{
	printf("next...\n");

	i = (i + 1) % G_N_ELEMENTS(wavs);

	char* filename = find_wav(wavs[i]);
	show_wav(view, filename);

	g_free(filename);
}


void
prev_wav (gpointer waveform)
{
	printf("prev...\n");

	i = (i - 1) % G_N_ELEMENTS(wavs);
	char* filename = find_wav(wavs[i]);
	show_wav(waveform, filename);

	g_free(filename);
}


void
toggle_shaders (gpointer view)
{
	printf(">> %s ...\n", __func__);

	agl_actor__set_use_shaders(((AGlActor*)waveform_view_plus_get_actor((WaveformViewPlus*)view))->root, !agl_get_instance()->use_shaders);

	char* filename = find_wav(wavs[0]);
	waveform_view_plus_load_file((WaveformViewPlus*)view, filename, NULL, NULL);
	g_free(filename);
}



void
toggle_grid (gpointer view)
{
	static bool visible = true;
	visible = !visible;
	if(visible){
		layers.grid = waveform_view_plus_add_layer((WaveformViewPlus*)view, grid_actor(waveform_view_plus_get_actor((WaveformViewPlus*)view)), 0);
	}else{
		waveform_view_plus_remove_layer((WaveformViewPlus*)view, layers.grid);
	}
}


	static bool reattach (gpointer _view)
	{
		gtk_table_attach(GTK_TABLE(table), (GtkWidget*)_view, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
		g_object_unref(_view);
		return G_SOURCE_REMOVE;
	}

void
unrealise (gpointer view)
{
	dbg(0, "-----------------------------");
	g_object_ref((GObject*)view);
	gtk_container_remove ((GtkContainer*)table, (GtkWidget*)view);
	g_timeout_add(600, reattach, view);
}


typedef struct {
    int finalize_done;
} DeleteTest;

DeleteTest dt = {0,};

static void
finalize_notify (gpointer data, GObject* was)
{
	PF;

	dt.finalize_done = true;
}


static bool
test_delete ()
{
	if(!view) return false;

	g_object_weak_ref((GObject*)view->waveform, finalize_notify, NULL);

	if(dt.finalize_done){
		gwarn("waveform should not be free'd");
		return false;
	}

	gtk_widget_destroy((GtkWidget*)view);
	view = NULL;

	if(!dt.finalize_done){
		gwarn("waveform was not free'd");
		return false;
	}

	return true;
}


void
delete (gpointer view)
{
	test_delete();
}


static guint play_timer = 0;

void
stop(gpointer view)
{
	if(play_timer){
		g_source_remove (play_timer);
		play_timer = 0;
	}else{
		wf_spp_actor_set_time((SppActor*)layers.spp, (_time = 0));
	}
}


	static bool tick(gpointer view)
	{
		wf_spp_actor_set_time((SppActor*)layers.spp, (_time += 50, _time));
		return true;
	}

void
play(gpointer view)
{
	if(!play_timer) play_timer = g_timeout_add(50, tick, view);
}


char*
format_channels(int n_channels)
{
	return g_strdup("mono");
}


void
format_time(char* str, int64_t t_ms)
{
	int64_t secs = t_ms / 1000;
	int64_t mins = secs / 60;
	secs = secs % 60;
	int64_t ms = t_ms % 1000;
	snprintf(str, 31, "%"PRIi64":%02"PRIi64":%02"PRIi64, mins, secs, ms);
}

