/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2024 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 |  Test of the libwaveform WaveformView widget in hires mode.          |
 |  ----------------------------------------------------------          |
 |                                                                      | 
 |  Hires mode uses asynchronous loading.                               |
 |  When the widget is initially loaded in this mode, different         |
 |  code paths are exercised.                                           |
 |                                                                      |
 |  A single waveform is displayed.                                     |
 |  The keys +- and cursor left/right keys can be used to zoom and in   |
 |  and scroll.                                                         |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <gtk/gtk.h>
#include "waveform/view_plus.h"
#include "test/common2.h"

#define WAV "short.wav"


static void
activate (GtkApplication* app, gpointer user_data)
{
	set_log_handlers();

	wf_debug = 1;
	_debug_ = 0;

	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), 320, 160);

	GtkWidget* vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	//GtkWidget* vbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_window_set_child (GTK_WINDOW (window), vbox);

	GtkWidget* box1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_append (GTK_BOX(vbox), box1);
	GtkWidget* box2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_append (GTK_BOX(vbox), box2);

	/*
	 *  Note that WaveformViewPlus creates a new scene,
	 *  so this test contains 3 separate scenes.
	 */
	WaveformViewPlus* waveform = waveform_view_plus_new(NULL);
	set_str(((AGlActor*)waveform_view_plus_get_actor(waveform))->name, g_strdup("Waveform1"));
	gtk_widget_set_hexpand ((GtkWidget*)waveform, TRUE);
	gtk_widget_set_vexpand ((GtkWidget*)waveform, TRUE);
	gtk_box_append (GTK_BOX(box1), GTK_WIDGET(waveform));
#if 0
	waveform_view_set_show_grid(waveform, true);
#endif

	// The 2nd row splits the same wav in two to test that the join is seamless
	WaveformViewPlus* waveform2 = waveform_view_plus_new(NULL);
	set_str(((AGlActor*)waveform_view_plus_get_actor(waveform2))->name, g_strdup("Waveform2"));
	gtk_widget_set_hexpand ((GtkWidget*)waveform2, TRUE);
	gtk_widget_set_vexpand ((GtkWidget*)waveform2, TRUE);
	gtk_box_append (GTK_BOX(box2), GTK_WIDGET(waveform2));

	WaveformViewPlus* waveform3 = waveform_view_plus_new(NULL);
	set_str(((AGlActor*)waveform_view_plus_get_actor(waveform3))->name, g_strdup("Waveform3"));
	gtk_widget_set_hexpand ((GtkWidget*)waveform3, TRUE);
	gtk_widget_set_vexpand ((GtkWidget*)waveform3, TRUE);
	gtk_box_append (GTK_BOX(box2), GTK_WIDGET(waveform3));

	g_autofree char* filename = find_wav(WAV);
	waveform_view_plus_load_file(waveform, filename, NULL, NULL);

	waveform_view_plus_set_waveform(waveform2, waveform->waveform);
	waveform_view_plus_set_region(waveform2, 0, waveform_get_n_frames(waveform->waveform) / 2 - 1);

	waveform_view_plus_set_waveform(waveform3, waveform->waveform);
	waveform_view_plus_set_region(waveform3, waveform_get_n_frames(waveform->waveform) / 2, waveform_get_n_frames(waveform->waveform) - 1);

	gboolean on_key_press_event (GtkEventController* controller, guint keyval, guint keycode, GdkModifierType state, WaveformViewPlus* waveform)
	{
		int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform_view_plus_get_zoom(waveform);

		switch (keyval) {
			case 61:
				waveform_view_plus_set_zoom(waveform, waveform_view_plus_get_zoom(waveform) * 1.5);
				break;
			case 45:
				waveform_view_plus_set_zoom(waveform, waveform_view_plus_get_zoom(waveform) / 1.5);
				break;
			case XK_Left:
			case XK_KP_Left:
				dbg(1, "left");
				waveform_view_plus_set_start(waveform, waveform->start_frame - n_visible_frames / 10);
				break;
			case XK_Right:
			case XK_KP_Right:
				dbg(1, "right");
				waveform_view_plus_set_start(waveform, waveform->start_frame + n_visible_frames / 10);
				break;
			default:
				dbg(1, "%i", keyval);
				break;
		}
		return AGL_NOT_HANDLED;
	}
	GtkEventController* controller = gtk_event_controller_key_new ();
	g_signal_connect (controller, "key-pressed", G_CALLBACK (on_key_press_event), waveform);
	gtk_widget_add_controller (window, controller);

	gtk_widget_set_visible(window, true);
}

#include "test/_gtk.c"
