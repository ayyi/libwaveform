/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2024 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */
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

*/

#include "config.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "agl/actor.h"
#include "actors/spinner.h"
#include "waveform/view_plus.h"
#include "common.h"

extern char* basename (const char*);

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
	"data/v_short.wav",
	"data/mono_0:00.wav",
	//"data/u8.wav",
};

KeyHandler
	next_wav,
	prev_wav,
	toggle_layers,
	toggle_grid,
	unrealise,
	delete,
	play,
	stop;

extern bool key_down;
extern KeyHold key_hold;

AGlKey keys[] = {
	{GDK_KEY_KP_Enter, NULL},
	{'<',              NULL},
	{'>',              NULL},
	{(char)'n',        next_wav},
	{(char)'p',        prev_wav},
	{(char)'l',        toggle_layers},
	{(char)'g',        toggle_grid},
	{(char)'u',        unrealise},
	{GDK_KEY_Delete,   delete},
	{65438,            stop},
	{65421,            play},
	{0},
};

gpointer tests[] = {};
uint32_t _time = 1000 + 321;
GtkWidget* box = NULL;
WaveformViewPlus* view = NULL;

struct Layers {
    AGlActor* grid;
    AGlActor* spp;
    AGlActor* spinner;
} layers;

void  on_quit         (GSimpleAction*, GVariant*, gpointer);
void  show_wav        (WaveformViewPlus*, const char* filename);
char* format_channels (int n_channels);
void  format_time     (char* str, int64_t t_ms);


static void
activate (GtkApplication* app, gpointer user_data)
{
	wf_debug = 0;

	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), 640, 160);

	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_window_set_child (GTK_WINDOW (window), box);

	WaveformViewPlus* waveform = view = waveform_view_plus_new(NULL);
	gtk_widget_set_hexpand ((GtkWidget*)view, TRUE);
	gtk_widget_set_vexpand ((GtkWidget*)view, TRUE);
	gtk_box_append (GTK_BOX(box), GTK_WIDGET(view));
	waveform_view_plus_set_show_rms(waveform, false);
	waveform_view_plus_set_colour(waveform, 0xccccddaa, 0x000000ff);
#if 0
	waveform_view_plus_set_region(waveform, 0, 32383); // start in hi-res mode
#endif

	waveform_view_plus_add_layer(waveform, background_actor(NULL), 0);

	layers.grid = waveform_view_plus_add_layer(waveform, grid_actor(waveform_view_plus_get_actor(waveform)), 0);

	AGlActor* text_layer = waveform_view_plus_add_layer(waveform, text_actor(NULL), 3);
	((TextActor*)text_layer)->title = g_strdup("Waveform Title");
	((TextActor*)text_layer)->text = g_strdup("Waveform text waveform text");
	text_actor_set_colour((TextActor*)text_layer, 0x33aaffff, 0xffff00ff);

	layers.spp = waveform_view_plus_add_layer(waveform, wf_spp_actor(waveform_view_plus_get_actor(waveform)), 0);
	wf_spp_actor_set_time((SppActor*)layers.spp, (_time += 50, _time));

	layers.spinner = waveform_view_plus_add_layer(waveform, agl_spinner(waveform_view_plus_get_actor(waveform)), 0);

	char* filename = find_wav(wavs[0]);
	show_wav(waveform, filename);
	g_free(filename);

	GActionEntry app_entries[] =
	{
		{ "quit", on_quit, NULL, NULL, NULL }
	};
	g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);
	gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.quit", (const char* const*)(char*[]){"Q", "<Ctrl>Q", NULL});

	add_key_handlers_gtk((GtkWindow*)window, waveform, (AGlKey*)&keys);
}




void on_quit (GSimpleAction* a, GVariant* v, gpointer app)
{

#ifdef WITH_VALGRIND
	gboolean on_idle (void* app)
	{
		agl_gl_uninit ();
		g_application_quit (G_APPLICATION (app));

		return G_SOURCE_REMOVE;
	}

	GtkWindow* window = (GtkWindow*)gtk_widget_get_parent(box);
	gtk_window_set_child (window, NULL);
	view = NULL;
	box = NULL;
	gtk_window_destroy (window);

	g_idle_add (on_idle, app);
#else
	g_application_quit (G_APPLICATION (app));
#endif
}


void
show_wav (WaveformViewPlus* view, const char* filename)
{
	g_assert(filename);

	agl_spinner_start((AGlSpinner*)layers.spinner);

	typedef struct {
		WaveformViewPlus* view;
		float             zoom;
	} C;

	void on_loaded_ (Waveform* w, GError* error, gpointer _view)
	{
		WaveformViewPlus* view = _view;

		agl_spinner_stop((AGlSpinner*)layers.spinner);

		if (error) {
			AGlActor* text_layer = agl_actor__find_by_class((AGlActor*)waveform_view_plus_get_actor(view), text_actor_get_class());
			if (text_layer) {
				text_actor_set_text(((TextActor*)text_layer), NULL, g_strdup(error->message));
			}
		}
	}

	waveform_view_plus_load_file(view, filename, on_loaded_, view);

	gboolean on_loaded (gpointer _c)
	{
		C* c = _c;
#ifndef USE_CANVAS_SCALING
		waveform_view_plus_set_zoom(c->view, c->zoom);
#endif
		g_free(c);
		return G_SOURCE_REMOVE;
	}

	g_idle_add(on_loaded, AGL_NEW(C,
		.view = view,
#ifndef USE_CANVAS_SCALING
		.zoom = view->zoom
#endif
	));

	g_assert(view->waveform);

	AGlActor* text_layer = agl_actor__find_by_class((AGlActor*)waveform_view_plus_get_actor(view), text_actor_get_class());
	if (text_layer) {
		char* text = NULL;
		Waveform* w = view->waveform;
		if (w->n_channels) {
			char* ch_str = format_channels(w->n_channels);
			char length[32]; format_time(length, (w->n_frames * 1000) / w->samplerate);
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
toggle_layers (gpointer view)
{
	static bool visible = true;
	visible = !visible;
	if (visible) {
		layers.spp = waveform_view_plus_add_layer((WaveformViewPlus*)view, wf_spp_actor(waveform_view_plus_get_actor((WaveformViewPlus*)view)), 0);
	} else {
		waveform_view_plus_remove_layer((WaveformViewPlus*)view, layers.spp);
		layers.spp = NULL;
	}
}


void
toggle_grid (gpointer view)
{
	static bool visible = true;
	visible = !visible;
	if (visible) {
		layers.grid = waveform_view_plus_add_layer((WaveformViewPlus*)view, grid_actor(waveform_view_plus_get_actor((WaveformViewPlus*)view)), 0);
	} else {
		waveform_view_plus_remove_layer((WaveformViewPlus*)view, layers.grid);
	}
}


void
unrealise (gpointer view)
{
	gboolean reattach (gpointer _view)
	{
		gtk_box_append (GTK_BOX(box), GTK_WIDGET(_view));
		g_object_unref(_view);
		return G_SOURCE_REMOVE;
	}

	dbg(0, "-----------------------------");
	g_object_ref((GObject*)view);
	gtk_box_remove ((GtkBox*)box, (GtkWidget*)view);
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


/*
 *  Note that after the widget is destroyed the screen will not be updated, so the waveform will remain on the screen
 */
static bool
test_delete ()
{
	if (!view) return false;

	g_object_weak_ref((GObject*)view->waveform, finalize_notify, NULL);

	if(dt.finalize_done){
		pwarn("waveform should not be free'd");
		return false;
	}

	gtk_box_remove((GtkBox*)box, (GtkWidget*)view);
	view = NULL;

	if (!dt.finalize_done) {
		pwarn("waveform was not free'd");
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
stop (gpointer view)
{
	if (play_timer) {
		g_source_remove (play_timer);
		play_timer = 0;
	} else {
		if (layers.spp) wf_spp_actor_set_time((SppActor*)layers.spp, (_time = 0));
	}
}


void
play (gpointer view)
{
	gboolean tick (gpointer view)
	{
		if (layers.spp) wf_spp_actor_set_time((SppActor*)layers.spp, (_time += 50, _time));
		return true;
	}

	if(!play_timer) play_timer = g_timeout_add(50, tick, view);
}


char*
format_channels (int n_channels)
{
	return g_strdup("mono");
}


void
format_time (char* str, int64_t t_ms)
{
	int64_t secs = t_ms / 1000;
	int64_t mins = secs / 60;
	secs = secs % 60;
	int64_t ms = t_ms % 1000;
	snprintf(str, 31, "%"PRIi64":%02"PRIi64":%02"PRIi64, mins, secs, ms);
}

#include "test/_gtk.c"
