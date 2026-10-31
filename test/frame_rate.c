/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | Test program to measure framerates using the libwaveform             |
 | WaveformView widget.                                                 |
 |                                                                      |
 | Framerate should be easily maintained at 60fps at 1920x1080          |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */
#define __wf_private__
#include "config.h"
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#include <gdk/gdkkeysyms-compat.h>
#include "waveform/view_plus.h"
#include "test/common2.h"

//#define WAV "mono_0:10.wav"
#define WAV "stereo_0:10.wav"

static uint64_t get_time ();

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
	{}
};

static const char* const short_options = "n";


int
main (int argc, char* argv[])
{
	g_return_val_if_fail(sizeof(off_t) == 8, EXIT_FAILURE);

	set_log_handlers();

	int opt;
	while ((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch (opt) {
			case 'n':
				g_timeout_add(3000, (gpointer)exit, NULL);
				break;
		}
	}

	gtk_init(&argc, &argv);
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	static GtkWidget* box;
#if GTK_MAJOR_VERSION < 3
	box = gtk_vbox_new(FALSE, 4);
#else
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#endif
	gtk_container_add((GtkContainer*)window, box);

	WaveformViewPlus* waveform[2] = {waveform_view_plus_new(NULL),};
	gtk_box_pack_start((GtkBox*)box, (GtkWidget*)waveform[0], TRUE, TRUE, 0);

	gtk_widget_show_all(window);

	char* filename = find_wav(WAV);
	waveform_view_plus_load_file(waveform[0], filename, NULL, NULL);
	g_free(filename);

	gboolean key_press (GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		WaveformViewPlus** waveform = user_data;

		switch (event->keyval) {
			case 61:
				waveform_view_plus_set_zoom(waveform[0], waveform_view_plus_get_zoom(waveform[0]) * 1.5);
				break;
			case 45:
				waveform_view_plus_set_zoom(waveform[0], waveform_view_plus_get_zoom(waveform[0]) / 1.5);
				break;
			case GDK_Left:
			case GDK_KP_Left:
				dbg(0, "left");
				waveform_view_plus_set_start(waveform[0], waveform[0]->start_frame - 8192 / waveform_view_plus_get_zoom(waveform[0]));
				break;
			case GDK_Right:
			case GDK_KP_Right:
				dbg(0, "right");
				waveform_view_plus_set_start(waveform[0], waveform[0]->start_frame + 8192 / waveform_view_plus_get_zoom(waveform[0]));
				break;
			case GDK_2:
				if (!waveform[1]) {
					gtk_box_pack_start((GtkBox*)box, (GtkWidget*)(waveform[1] = waveform_view_plus_new(NULL)), TRUE, TRUE, 0);
					char* filename = g_build_filename(g_get_current_dir(), WAV, NULL);
					waveform_view_plus_load_file(waveform[1], filename, NULL, NULL);
					g_free(filename);
					gtk_widget_show((GtkWidget*)waveform[1]);
				}
				dbg(0, "2");
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

	gboolean on_timeout (gpointer _waveform)
	{
		WaveformViewPlus* waveform = _waveform;

		static uint64_t frame = 0;
		static uint64_t t0    = 0;
		if (!frame)
			t0 = get_time();
		else {
			uint64_t time = get_time();
			if (!(frame % 200))
				printf("rate=%.1f fps\n", ((float)frame / ((float)(time - t0))) * 1000.0);

			if (!(frame % 8)) {
				float v = (frame % 16) ? 1.5 : 2.0 / 3.0;
				if (v > 16.0) v = 1.0;
				waveform_view_plus_set_zoom(waveform, waveform_view_plus_get_zoom(waveform) * v);
			}
			gtk_widget_queue_draw((GtkWidget*)waveform);
		}
		frame++;
		return G_SOURCE_CONTINUE;
	}
	g_timeout_add(15, on_timeout, waveform[0]);

	gtk_main();

	return EXIT_SUCCESS;
}


uint64_t
get_time ()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}
