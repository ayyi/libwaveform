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

#define FAIL_IF_ERROR \
	if(error && *error) FAIL_TEST((*error)->message);
