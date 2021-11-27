/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2021 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#define __runner_c__

#include "config.h"
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include "glib.h"
#include "debug/debug.h"
#include "runner.h"

extern gpointer tests[];

extern int  setup    (int, char* argv[]);
extern void teardown ();

static void next_test ();

bool abort_on_fail = true;


int
main (int argc, char* argv[])
{
	set_log_handlers();

	gboolean run (gpointer user_data) { next_test(); return G_SOURCE_REMOVE; }

	g_idle_add(run, NULL);

	int r = setup (argc, argv);
	if (r) return r;

	dbg(2, "n_tests=%i", TEST.n_tests);

	if (!TEST.is_gtk)
		g_main_loop_run (g_main_loop_new (NULL, 0));

	exit(1);
}


static gboolean
run_test (gpointer test)
{
	((Test)test)();
	return G_SOURCE_REMOVE;
}


static gboolean
on_test_timeout (gpointer _user_data)
{
	FAIL_TEST_TIMER("TEST TIMEOUT\n");
	return G_SOURCE_REMOVE;
}


static gboolean
__exit ()
{
	exit(TEST.n_failed ? EXIT_FAILURE : EXIT_SUCCESS);
	return G_SOURCE_REMOVE;
}


static void
next_test ()
{
	printf("\n");
	TEST.current.test++;
	if (TEST.timeout)
		g_source_remove (TEST.timeout);
	if (TEST.current.test < TEST.n_tests) {
		TEST.current.finished = false;
		gboolean (*test)() = tests[TEST.current.test];
		dbg(2, "test %i of %i.", TEST.current.test + 1, TEST.n_tests);
		g_timeout_add(200, run_test, test);

		TEST.timeout = g_timeout_add(30000, on_test_timeout, NULL);
	} else {
		printf("finished all. passed=%s %i %s failed=%s %i %s\n", GREEN, TEST.n_passed, ayyi_white, (TEST.n_failed ? RED : ayyi_white), TEST.n_failed, ayyi_white);
		g_timeout_add(1000, __exit, NULL);
	}
}


void
test_finish ()
{
	dbg(2, "... passed=%i", TEST.passed);
	if(TEST.passed) TEST.n_passed++; else TEST.n_failed++;
	if(!TEST.passed && abort_on_fail) TEST.current.test = 1000;
	next_test();
}


static gboolean
on_test_timeout_ (gpointer _user_data)
{
	FAIL_TEST_TIMER("TEST TIMEOUT\n");
	return G_SOURCE_REMOVE;
}


void
test_reset_timeout (int ms)
{
	if(TEST.timeout) g_source_remove (TEST.timeout);

	TEST.timeout = g_timeout_add(ms, on_test_timeout_, NULL);
}

void
test_errprintf (char* format, ...)
{
	char str[256];

	va_list argp;
	va_start(argp, format);
	vsprintf(str, format, argp);
	va_end(argp);

	printf("%s%s%s\n", RED, str, ayyi_white);
}
