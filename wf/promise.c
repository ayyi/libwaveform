/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <stdio.h>
#include "glib.h"
#include "wf/utils.h"
#include "wf/promise.h"

typedef struct {
    WfPromiseCallback callback;
    gpointer          user_data;
} Item;


AMPromise*
am_promise_new (gpointer user_data)
{
	return WF_NEW(AMPromise,
		.user_data = user_data,
		.refcount = 1
	);
}


void
am_promise_unref (AMPromise* p)
{
	if(!--p->refcount){
		g_list_free_full(p->children, (GDestroyNotify)am_promise_unref);
		g_list_free_full(p->callbacks, g_free);
		g_clear_pointer(&p->error, g_error_free);
		g_free(p);
	}
}


void
_am_promise_callback (AMPromise* p)
{
	if(p->callbacks){
		p->refcount++; // allows promise to be unreffed in a user callback.

		GList* l = p->callbacks;
		for(;l;l=l->next){
			Item* item = l->data;
			item->callback(p->user_data, item->user_data);
		}

		// each callback is only ever called once
		g_list_free_full(p->callbacks, g_free);
		p->callbacks = NULL;

		am_promise_unref(p);
	}
}


void
_add_callback (AMPromise* p, WfPromiseCallback callback, gpointer user_data)
{
	Item* item = WF_NEW(Item,
		.callback = callback,
		.user_data = user_data
	);
	p->callbacks = g_list_append(p->callbacks, item);
}


void
am_promise_add_callback (AMPromise* p, WfPromiseCallback callback, gpointer user_data)
{
	_add_callback(p, callback, user_data);
	if(p->is_resolved) _am_promise_callback(p);
}


void
am_promise_resolve (AMPromise* p, PromiseVal* value)
{
	if (!p->is_resolved) {
		if (value) p->value = *value;
		p->is_resolved = true;
		_am_promise_callback(p);
	}
}


/*
 *  When the promise fails, the main callbacks are called.
 *  The client needs to check the error property to see if the promise has failed.
 */
void
am_promise_fail (AMPromise* p, GError* error)
{
	p->error = error;
	am_promise_resolve(p, NULL);
}


	static void then (gpointer _, gpointer _parent)
	{
		AMPromise* parent = _parent;
		g_return_if_fail(parent);

		bool complete = true;
		GList* l = parent->children;
		for(;l;l=l->next){
			AMPromise* p = l->data;
			if(!p->is_resolved){
				complete = false;
				break;
			}
		}
		if(complete) am_promise_resolve(parent, &(PromiseVal){.i=-1});
	}

	static void add_child (AMPromise* promise, AMPromise* child)
	{
		g_return_if_fail(child);
		promise->children = g_list_append(promise->children, child);
		am_promise_add_callback(child, then, promise);
	}
/*
 *  The promise will be resolved when all the child promises are resolved.
 *
 *  The last parameter must be NULL
 */
void
am_promise_when (AMPromise* promise, AMPromise* p, ...)
{
	if(!p) return;

	add_child(promise, p);

	va_list args;
	va_start(args, p);
	AMPromise* q;
	while((q = va_arg (args, AMPromise*))){
		add_child(promise, q);
	}
	va_end(args);
}


