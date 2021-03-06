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

#pragma once

typedef void (*Test) ();

typedef struct {
	int n_tests;
	int n_passed;
	int n_failed;
	int timeout;
	bool passed;
	struct {
		int    test;
		char   name[64];
		bool   finished;  // current test has finished. Go onto the next test.
	}   current;
} Test_t;

#ifdef __runner_c__
Test_t TEST = {.current = {-1}};
#else
extern Test_t TEST;
#endif

void test_init          (gpointer tests[], int);
void test_finish        ();
void test_reset_timeout (int ms);


#define START_TEST \
	static int step = 0;\
	static int __test_idx; \
	__test_idx = TEST.current.test; \
	if(!step){ \
		g_strlcpy(TEST.current.name, __func__, 64); \
		printf("%srunning %i of %i: %s%s ...\n", ayyi_bold, TEST.current.test + 1, TEST.n_tests, __func__, ayyi_white); \
	} \
	if(TEST.current.finished) return;

#define FINISH_TEST \
	if(__test_idx != TEST.current.test) return; \
	printf("%s: finish\n", TEST.current.name); \
	TEST.current.finished = true; \
	TEST.passed = true; \
	test_finish(); \
	return;

#define FINISH_TEST_TIMER_STOP \
	if(__test_idx != TEST.current.test) return G_SOURCE_REMOVE; \
	TEST.current.finished = true; \
	TEST.passed = true; \
	test_finish(); \
	return G_SOURCE_REMOVE;

#define FAIL_TEST(msg, ...) \
	{TEST.current.finished = true; \
	TEST.passed = false; \
	errprintf4(msg, ##__VA_ARGS__); \
	test_finish(); \
	return; }

#define FAIL_TEST_TIMER(msg) \
	{TEST.current.finished = true; \
	TEST.passed = false; \
	printf("%s%s%s\n", red, msg, ayyi_white); \
	test_finish(); \
	return G_SOURCE_REMOVE;}

extern char red      [10];
extern char green    [10];
