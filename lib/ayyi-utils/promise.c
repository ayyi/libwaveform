/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://ayyi.org              |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <stdbool.h>
#include "glib.h"
#include "ayyi-utils/utils.h"
#include "promise.h"

typedef struct {
    AyyiPromiseCallback callback;
    gpointer          user_data;
} Item;


AyyiPromise*
ayyi_promise_new (gpointer user_data)
{
	return AYYI_NEW(AyyiPromise,
		.user_data = user_data,
		.refcount = 1
	);
}


void
ayyi_promise_unref (AyyiPromise* p)
{
	if (!--p->refcount) {
		g_list_free_full(p->children, (GDestroyNotify)ayyi_promise_unref);
		g_list_free_full(p->callbacks, g_free);
		g_clear_pointer(&p->error, g_error_free);
		g_free(p);
	}
}


void
_ayyi_promise_callback (AyyiPromise* p)
{
	if (p->callbacks) {
		p->refcount++; // allows promise to be unreffed in a user callback.

		GList* l = p->callbacks;
		for(;l;l=l->next){
			Item* item = l->data;
			item->callback(p->user_data, item->user_data);
		}

		// each callback is only ever called once
		g_list_free_full(p->callbacks, g_free);
		p->callbacks = NULL;

		ayyi_promise_unref(p);
	}
}


static void
_add_callback (AyyiPromise* p, AyyiPromiseCallback callback, gpointer user_data)
{
	Item* item = AYYI_NEW(Item,
		.callback = callback,
		.user_data = user_data
	);
	p->callbacks = g_list_append(p->callbacks, item);
}


void
ayyi_promise_add_callback (AyyiPromise* p, AyyiPromiseCallback callback, gpointer user_data)
{
	_add_callback(p, callback, user_data);
	if(p->is_resolved) _ayyi_promise_callback(p);
}


void
ayyi_promise_resolve (AyyiPromise* p, AyyiPromiseVal* value)
{
	if (!p->is_resolved) {
		if (value) p->value = *value;
		p->is_resolved = true;
		_ayyi_promise_callback(p);
	}
}


/*
 *  When the promise fails, the main callbacks are called.
 *  The client needs to check the error property to see if the promise has failed.
 */
void
ayyi_promise_fail (AyyiPromise* p, GError* error)
{
	p->error = error;
	ayyi_promise_resolve(p, NULL);
}


	static void then (gpointer _, gpointer _parent)
	{
		AyyiPromise* parent = _parent;
		g_return_if_fail(parent);

		bool complete = true;
		GList* l = parent->children;
		for(;l;l=l->next){
			AyyiPromise* p = l->data;
			if(!p->is_resolved){
				complete = false;
				break;
			}
		}
		if(complete) ayyi_promise_resolve(parent, &(AyyiPromiseVal){.i=-1});
	}

	static void add_child (AyyiPromise* promise, AyyiPromise* child)
	{
		g_return_if_fail(child);
		promise->children = g_list_append(promise->children, child);
		ayyi_promise_add_callback(child, then, promise);
	}
/*
 *  The promise will be resolved when all the child promises are resolved.
 *
 *  The last parameter must be NULL
 */
void
ayyi_promise_when (AyyiPromise* promise, AyyiPromise* p, ...)
{
	if (!p) return;

	add_child(promise, p);

	va_list args;
	va_start(args, p);
	AyyiPromise* q;
	while ((q = va_arg (args, AyyiPromise*))) {
		add_child(promise, q);
	}
	va_end(args);
}


