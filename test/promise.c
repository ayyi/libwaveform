/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2024 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 | Tests for the libwaveform Promise object
 |
 */

#define __wf_private__

#include "config.h"
#include <glib.h>
#include "decoder/ad.h"
#include "test/utils.h"
#include "test/wf_runner.h"
#include "test/promise.h"


void
test_1 ()
{
	START_TEST;

	static bool done = false;
	static int done_val1 = 0;
	static int done_val2 = 0;

	void on_ready (gpointer user_data, gpointer callback_data)
	{
		done = true;
		done_val1 = GPOINTER_TO_INT(user_data);
		done_val2 = GPOINTER_TO_INT(callback_data);
	}

	AMPromise* promise = am_promise_new(GINT_TO_POINTER(5));
	am_promise_add_callback (promise, on_ready, GINT_TO_POINTER(6));

	assert(!done, "callback called too early");

	PromiseVal val = {.i = 7};
	am_promise_resolve(promise, &val);

	assert(done, "callback not called");
	assert(promise->is_resolved, "not resolved");
	assert(!promise->error, "promise error");
	assert(done_val1 == 5, "val wrong %i", done_val1);
	assert(done_val2 == 6, "val wrong %i", done_val2);
	assert(promise->value.i == val.i, "val wrong %i", promise->value.i);

	am_promise_unref(promise);

	FINISH_TEST;
}


void
test_2_when ()
{
	START_TEST;

	AMPromise* promise = am_promise_new(GINT_TO_POINTER(4));
	AMPromise* promise1 = am_promise_new(GINT_TO_POINTER(5));
	AMPromise* promise2 = am_promise_new(GINT_TO_POINTER(6));

	am_promise_when(promise, promise1, promise2, NULL);

	assert(!promise->is_resolved, "too early");

	am_promise_resolve(promise1, NULL);
	assert(!promise->is_resolved, "too early");
	assert(promise1->is_resolved, "too early");
	assert(!promise2->is_resolved, "too early");

	am_promise_resolve(promise2, NULL);
	assert(promise->is_resolved, "not resolved");
	assert(promise1->is_resolved, "promise1 not resolved");
	assert(promise2->is_resolved, "promise2 not resolved");

	am_promise_unref(promise);

	FINISH_TEST;
}


	typedef struct {
		WfTest     test;
		AMPromise* promise;
	} C3;

	static gboolean test_3_1 (gpointer _c)
	{
		C3* c3 = _c;

		assert_and_stop(!c3->promise->is_resolved, "too early");

		am_promise_resolve(c3->promise, NULL);

		return G_SOURCE_REMOVE;
	}

	static gboolean test_3_finish (gpointer _c)
	{
		WfTest* c = _c;
		C3* c3 = _c;

		am_promise_unref(c3->promise);

		WF_TEST_FINISH_TIMER_STOP;
	}

	void test_3_on_resolve (gpointer _c, gpointer callback_data)
	{
		WfTest* c = _c;
		C3* c3 = _c;

		assert(c3->promise->is_resolved, "not resolved");

		g_idle_add(test_3_finish, c);
	}

void
test_3 ()
{
	START_TEST;

	C3* c = WF_NEW(C3,
		.test = {
			.test_idx = __test_idx,
		}
	);

	c->promise = am_promise_new(c);
	am_promise_add_callback(c->promise, test_3_on_resolve, GINT_TO_POINTER(6));

	g_timeout_add(10, test_3_1, c);
}


	typedef struct {
		WfTest test;
		GList* promises;
	} C4;

	static gboolean test_4_resolve (gpointer promise)
	{
		am_promise_resolve((AMPromise*)promise, NULL);

		return G_SOURCE_REMOVE;
	}

	static gboolean test_4_1 (gpointer _c)
	{
		C4* c4 = _c;

		GList* l = c4->promises;
		for(;l;l=l->next){
			AMPromise* promise = l->data;
			assert_and_stop(!promise->is_resolved, "too early");

			g_timeout_add(get_random_int(2000), test_4_resolve, promise);
		}

		return G_SOURCE_REMOVE;
	}

	static gboolean test_4_finish (gpointer _c)
	{
		WfTest* c = _c;

		WF_TEST_FINISH_TIMER_STOP;
	}

	static gboolean test_4_after_resolve (gpointer _promise)
	{
		AMPromise* promise = _promise;

		am_promise_unref(promise);

		return G_SOURCE_REMOVE;
	}

void
test_4_many ()
{
	void test_4_on_resolve(gpointer _c, gpointer callback_data)
	{
		WfTest* c = _c;
		C4* c4 = _c;
		AMPromise* promise = callback_data;

		assert(promise->is_resolved, "not resolved");

		c4->promises = g_list_remove(c4->promises, promise);

		if(!g_list_length(c4->promises))
			g_idle_add(test_4_finish, c);

		g_idle_add(test_4_after_resolve, promise);
	}

	START_TEST;

	C4* c = WF_NEW(C4,
		.test = {
			.test_idx = __test_idx,
		}
	);

	int i; for(i=0;i<256;i++){
		AMPromise* promise = am_promise_new(c);
		am_promise_add_callback(promise, test_4_on_resolve, promise);

		c->promises = g_list_append(c->promises, promise);
	}

	g_timeout_add(10, test_4_1, c);
}
