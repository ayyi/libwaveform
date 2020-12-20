/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform https://github.com/ayyi/libwaveform |
* | copyright (C) 2013-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __common2_c__
#define __wf_private__
#include "config.h"
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#define XLIB_ILLEGAL_ACCESS // needed to access Display internals
#include <X11/Xlib.h>
#if defined(USE_GTK) || defined(__GTK_H__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif
#include <glib-object.h>
#include "agl/actor.h"
#include "waveform/utils.h"
#ifdef USE_EPOXY
# define __glx_test__
#endif
#include "test/common2.h"
#include "ui/view_plus.h"
#include "wf/private.h"


char*
find_wav (const char* wav)
{
	if(wav[0] == '/'){
		return g_strdup(wav);
	}
	char* filename = g_build_filename(g_get_current_dir(), wav, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	wf_free(filename);

	filename = g_build_filename(g_get_current_dir(), "test", wav, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	wf_free(filename);

	filename = g_build_filename(g_get_current_dir(), "test/data", wav, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	wf_free(filename);

	filename = g_build_filename(g_get_current_dir(), "data", wav, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	wf_free(filename);

	return NULL;
}


const char*
find_data_dir ()
{
	static char* dirs[] = {"test/data", "data"};

	int i; for(i=0;i<G_N_ELEMENTS(dirs);i++){
		if(g_file_test(dirs[i], G_FILE_TEST_EXISTS)){
			return dirs[i];
		}
	}

	return NULL;
}


static GHashTable* key_handlers = NULL;

KeyHandler*
key_lookup (int keycode)
{
	return key_handlers ? g_hash_table_lookup(key_handlers, &keycode) : NULL;
}


/**
 * Display the refresh rate of the display using the GLX_OML_sync_control
 * extension.
 */
void
show_refresh_rate (Display* dpy)
{
#if defined(GLX_OML_sync_control) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
	int32_t  n;
	int32_t  d;

	PFNGLXGETMSCRATEOMLPROC get_msc_rate = (PFNGLXGETMSCRATEOMLPROC)glXGetProcAddressARB((const GLubyte*) "glXGetMscRateOML");
	if (get_msc_rate != NULL) {
		(*get_msc_rate)(dpy, glXGetCurrentDrawable(), &n, &d);
		printf( "refresh rate: %.1fHz\n", (float) n / d);
		return;
	}
#endif
	printf("glXGetMscRateOML not supported.\n");
}


#if defined(USE_GTK) || defined(__GTK_H__)
static bool
on_window_delete (GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	gtk_main_quit();

	return false;
}


int
gtk_window (Key keys[], WindowFn content)
{
	GdkGLConfig* glconfig;
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		perr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
	}

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	content((GtkWindow*)window, glconfig);

	gtk_widget_show_all(window);

	add_key_handlers_gtk((GtkWindow*)window, NULL, keys);

	g_signal_connect(window, "delete-event", G_CALLBACK(on_window_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


	static KeyHold key_hold = {0, NULL};
	static bool key_down = false;

	static gboolean key_hold_on_timeout (gpointer user_data)
	{
		WaveformViewPlus* waveform = user_data;
		if(key_hold.handler) key_hold.handler(waveform);
		return G_SOURCE_CONTINUE;
	}

	static gboolean key_press (GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		if(key_down){
			// key repeat
			return true;
		}

		KeyHandler* handler = g_hash_table_lookup(key_handlers, &event->keyval);
		if(handler){
			key_down = true;
			if(key_hold.timer) pwarn("timer already started");
			key_hold.timer = g_timeout_add(100, key_hold_on_timeout, user_data);
			key_hold.handler = handler;

			handler(user_data);
		}
		else dbg(1, "%i", event->keyval);

		return key_down;
	}

	static gboolean key_release(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		PF;
		if(!key_down) return AGL_NOT_HANDLED; // sometimes happens at startup

		key_down = false;
		g_source_remove0(key_hold.timer);

		return true;
	}

void
add_key_handlers_gtk (GtkWindow* window, gpointer user_data, Key keys[])
{
	//list of keys must be terminated with a key of value zero.

	key_handlers = g_hash_table_new(g_int_hash, g_int_equal);
	int i = 0; while(true){
		Key* key = &keys[i];
		if(i > 100 || !key->key) break;
		g_hash_table_insert(key_handlers, &key->key, key->handler);
		i++;
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), user_data);
	g_signal_connect(window, "key-release-event", G_CALLBACK(key_release), user_data);
}
#endif


void
add_key_handlers (Key keys[])
{
	if(!key_handlers){
		key_handlers = g_hash_table_new(g_int_hash, g_int_equal);

		int i = 0; while(true) {
			Key* key = &keys[i];
			if(i > 100 || !key->key) break;
			g_hash_table_insert(key_handlers, &key->key, key->handler);
			i++;
		}
	}
}


