/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. http://ayyi.org               |
 | copyright (C) 2013-2021 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 | Common code for automated tests
 | (move stuff for non-automated tests to common2.h)
 |
 */

#include "wf/debug.h"
#include "wf/waveform.h"
#include "test/runner.h"
#include "test/common2.h"

#define TIMER_CONTINUE TRUE

bool get_random_boolean ();
int  get_random_int     (int max);
void create_large_file  (char*);

void errprintf4         (char* format, ...);

KeyHandler* key_lookup  (int keycode);

#define START_LONG_TEST \
	START_TEST; \
	g_source_remove (TEST.timeout); \
	TEST.timeout = 0;

#define NEXT_CALLBACK(A, B, C) \
	step++; \
	void (*callback)() = callbacks[step]; \
	callback(A, B, C);

typedef struct {
	int test_idx;
} WfTest;

WfTest* wf_test_new();

#define NEW_TEST() \
	({ \
	g_strlcpy(TEST.current.name, __func__, 64); \
	printf("%srunning %i of %zu: %s%s ...\n", ayyi_bold, TEST.current.test + 1, G_N_ELEMENTS(tests), __func__, ayyi_white); \
	if(TEST.current.finished) return; \
	wf_test_new(); \
	})

#define WF_TEST_FINISH \
	if(c->test_idx != TEST.current.test) return; \
	printf("%s: finish\n", TEST.current.name); \
	TEST.current.finished = true; \
	TEST.passed = true; \
	test_finish(); \
	wf_free(c); \
	return;

#define WF_TEST_FINISH_TIMER_STOP \
	if(c->test_idx != TEST.current.test) return G_SOURCE_REMOVE; \
	TEST.current.finished = true; \
	TEST.passed = true; \
	test_finish(); \
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

