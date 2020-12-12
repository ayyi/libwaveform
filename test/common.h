/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
/**
 *  common code for automated tests - move stuff for non-automated tests to common2.h
 */
#include "wf/debug.h"
#include "wf/waveform.h"
#include "test/common2.h"

#define TIMER_CONTINUE TRUE
#define TIMER_STOP FALSE

extern int current_test;

#ifndef __common_c__
extern
#endif
struct _app
{
	int            timeout;
	int            n_passed;
#ifdef __common_c__
} app = {0,};
#else
} app;
#endif

typedef void (*Test)    ();

void test_init          (gpointer tests[], int);
void next_test          ();
void test_finished_     ();
void reset_timeout      (int ms);

bool get_random_boolean ();
int  get_random_int     (int max);
void create_large_file  (char*);

void errprintf4         (char* format, ...);

KeyHandler* key_lookup  (int keycode);

#define START_TEST \
	static int step = 0;\
	static int __test_idx; \
	__test_idx = current_test; \
	if(!step){ \
		g_strlcpy(current_test_name, __func__, 64); \
		printf("%srunning %i of %zu: %s%s ...\n", ayyi_bold, current_test + 1, G_N_ELEMENTS(tests), __func__, ayyi_white); \
	} \
	if(test_finished) return;

#define START_LONG_TEST \
	START_TEST; \
	g_source_remove (app.timeout); \
	app.timeout = 0;

#define NEXT_CALLBACK(A, B, C) \
	step++; \
	void (*callback)() = callbacks[step]; \
	callback(A, B, C);

#define FINISH_TEST \
	if(__test_idx != current_test) return; \
	printf("%s: finish\n", current_test_name); \
	test_finished = true; \
	passed = true; \
	test_finished_(); \
	return;

#define FINISH_TEST_TIMER_STOP \
	if(__test_idx != current_test) return G_SOURCE_REMOVE; \
	test_finished = true; \
	passed = true; \
	test_finished_(); \
	return G_SOURCE_REMOVE;

#define FAIL_TEST(msg, ...) \
	{test_finished = true; \
	passed = false; \
	errprintf4(msg, ##__VA_ARGS__); \
	test_finished_(); \
	return; }

#define FAIL_TEST_TIMER(msg) \
	{test_finished = true; \
	passed = false; \
	printf("%s%s%s\n", red, msg, ayyi_white); \
	test_finished_(); \
	return G_SOURCE_REMOVE;}

typedef struct {
	int test_idx;
} WfTest;

WfTest* wf_test_new();

#define NEW_TEST() \
	({ \
	g_strlcpy(current_test_name, __func__, 64); \
	printf("%srunning %i of %zu: %s%s ...\n", ayyi_bold, current_test + 1, G_N_ELEMENTS(tests), __func__, ayyi_white); \
	if(test_finished) return; \
	wf_test_new(); \
	})

#define WF_TEST_FINISH \
	if(c->test_idx != current_test) return; \
	printf("%s: finish\n", current_test_name); \
	test_finished = true; \
	passed = true; \
	test_finished_(); \
	wf_free(c); \
	return;

#define WF_TEST_FINISH_TIMER_STOP \
	if(c->test_idx != current_test) return G_SOURCE_REMOVE; \
	test_finished = true; \
	passed = true; \
	test_finished_(); \
	wf_free(c); \
	return G_SOURCE_REMOVE;

#define assert(A, B, ...) \
	{bool __ok_ = ((A) != 0); \
	{if(!__ok_) perr(B, ##__VA_ARGS__); } \
	{if(!__ok_) FAIL_TEST("assertion failed") }}

#define assert_and_stop(A, B, ...) \
	{bool __ok_ = ((A) != 0); \
	{if(!__ok_) perr(B, ##__VA_ARGS__); } \
	{if(!__ok_) FAIL_TEST_TIMER("assertion failed") }}

#define FAIL_IF_ERROR \
	if(error && *error) FAIL_TEST((*error)->message);

typedef void (TestFn)();

#ifdef __common_c__
char       current_test_name[64];
#else
extern gboolean passed;
extern int      test_finished;
extern char     current_test_name[];
#endif
