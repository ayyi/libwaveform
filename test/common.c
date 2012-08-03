/*
  This file is part of the Ayyi Project. http://ayyi.org
  copyright (C) 2004-2012 Tim Orford <tim@orford.org>

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
#define __common_c__
#include "config.h"
#include <stdio.h>
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
#include <glib-object.h>
#include "waveform/utils.h"
#include "test/ayyi_utils.h"
#include "test/common.h"

static void log_handler(const gchar*, GLogLevelFlags, const gchar*, gpointer);

int      n_failed = 0;
int      n_passed = 0;
gboolean abort_on_fail  = true;
gboolean passed         = false;
int      test_finished  = false;  //current test has finished. Go onto the next test.
int      current_test = -1;
extern char     current_test_name[];
extern gpointer tests[];

static int __n_tests = 0;

//TODO is dupe
#define FAIL_TEST_TIMER(msg) \
	{test_finished = true; \
	passed = false; \
	printf("%s%s%s\n", red, msg, white); \
	test_finished_(); \
	return TIMER_STOP;}
//------------------



void
test_init(gpointer tests[], int n_tests)
{
	__n_tests = n_tests;
	dbg(2, "n_tests=%i", __n_tests);

	memset(&app, 0, sizeof(struct _app));

	g_log_set_handler (NULL, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("Gtk", G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("Gdk", G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("GLib", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("GLib-GObject", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("Ayyi", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	g_log_set_handler ("Waveform", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);

	g_thread_init(NULL);
	g_type_init();
}


void
next_test()
{
	bool
	run_test(gpointer test)
	{
		((Test)test)();
		return TIMER_STOP;
	}

	bool _exit()
	{
		exit(EXIT_SUCCESS);
		return TIMER_STOP;
	}

	printf("\n");
	current_test++;
	if(current_test < __n_tests){
		if(app.timeout) g_source_remove (app.timeout);
		test_finished = false;
		gboolean (*test)() = tests[current_test];
		dbg(2, "test %i of %i.", current_test + 1, __n_tests);
		g_timeout_add(300, run_test, test);

		bool on_test_timeout(gpointer _user_data)
		{
			FAIL_TEST_TIMER("TEST TIMEOUT\n");
			return TIMER_STOP;
		}
		app.timeout = g_timeout_add(20000, on_test_timeout, NULL);
	}
	else{ printf("finished all. passed=%s%i%s failed=%s%i%s\n", green, app.n_passed, white, (n_failed ? red : white), n_failed, white); g_timeout_add(1000, _exit, NULL); }
}


void
test_finished_()
{
	dbg(2, "... passed=%i", passed);
	if(passed) app.n_passed++; else n_failed++;
	//log_print(passed ? LOG_OK : LOG_FAIL, "%s", current_test_name);
	if(!passed && abort_on_fail) current_test = 1000;
	next_test();
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
      printf("log_handler(): level=%i %s\n", log_level, message);
      break;
  }
}


void
add_key_handler(GtkWindow* window, WaveformView* waveform, Key keys[])
{
	//list of keys must be terminated with a key of value zero.

	static KeyHold key_hold = {0, NULL};
	static bool key_down = false;
	static GHashTable* key_handlers = NULL;

	key_handlers = g_hash_table_new(g_int_hash, g_int_equal);
	int i = 0; while(true){
		Key* key = &keys[i];
		if(i > 100 || !key->key) break;
		g_hash_table_insert(key_handlers, &key->key, key->handler);
		i++;
	}

	gboolean key_hold_on_timeout(gpointer user_data)
	{
		WaveformView* waveform = user_data;
		if(key_hold.handler) key_hold.handler(waveform);
		return TIMER_CONTINUE;
	}

	gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		WaveformView* waveform = user_data;

		if(key_down){
			// key repeat
			return true;
		}
		key_down = true;

		KeyHandler* handler = g_hash_table_lookup(key_handlers, &event->keyval);
		if(handler){
			if(key_hold.timer) gwarn("timer already started");
			key_hold.timer = g_timeout_add(100, key_hold_on_timeout, waveform);
			key_hold.handler = handler;
	
			handler(waveform);
		}
		else dbg(1, "%i", event->keyval);

		return true;
	}

	gboolean key_release(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		PF0;
		if(!key_down) gwarn("key_down not set");
		key_down = false;
		g_source_remove(key_hold.timer);
		key_hold.timer = 0;

		return true;
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), waveform);
	g_signal_connect(window, "key-release-event", G_CALLBACK(key_release), waveform);

}


gboolean
get_random_boolean()
{
	int r = rand();
	int s = RAND_MAX / 2;
	int t = r / s;
	return t;
}


int
get_random_int(int max)
{
	//printf("R=%i %i\n", RAND_MAX, 1 << sizeof(int));
	//int R = RAND_MAX / 2 + 100000;
	int t;
	//int i; for(i=0;i<10;i++){
	int r = rand();
		int s = RAND_MAX / max;
		t = r / s;
	//	printf("r=%i s=%i t=%i\n", r, s, t);
	//}
   return t;
}


void
errprintf4(char* format, ...)
{
	char str[256];

	va_list argp;           //points to each unnamed arg in turn
	va_start(argp, format); //make ap (arg pointer) point to 1st unnamed arg
	vsprintf(str, format, argp);
	va_end(argp);           //clean up

	printf("%s%s%s\n", red, str, white);
}


