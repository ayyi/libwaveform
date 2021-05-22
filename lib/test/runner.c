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

static void next_test ();

bool abort_on_fail = true;

static gboolean fn (gpointer user_data) { next_test(); return G_SOURCE_REMOVE; }


void
test_init (gpointer tests[], int n_tests)
{
	TEST.n_tests = n_tests;
	dbg(2, "n_tests=%i", n_tests);

	set_log_handlers();

	g_idle_add(fn, NULL);
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
		printf("finished all. passed=%s %i %s failed=%s %i %s\n", green, TEST.n_passed, ayyi_white, (TEST.n_failed ? red : ayyi_white), TEST.n_failed, ayyi_white);
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
