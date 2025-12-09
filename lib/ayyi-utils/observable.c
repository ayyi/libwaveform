/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2018-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include <glib.h>
#include "ayyi-utils/utils.h"
#include "observable.h"

typedef struct {
   AGlObservableFn fn;
   gpointer        user;
} Subscription;


AGlObservable*
agl_observable_new ()
{
	return AYYI_NEW(AGlObservable,
		.max.i = INT_MAX
	);
}


/*
 *  This will not free string values. Freeing of string values needs to be done by the user.
 */
void
agl_observable_free (AGlObservable* observable)
{
	g_list_free_full(observable->subscriptions, g_free);
	g_free(observable);
}


/*
 *	Because of the possibility of uninitialized padding
 *	there is no way to check equality of 2 unions so
 *	it is not possible to check here if the value has changed.
 *  Use `agl_observable_set_int` or `agl_observable_set_float` where possible.
 */
void
agl_observable_set (AGlObservable* observable, AGlVal value)
{
	observable->value = value;

	for (GList* l = observable->subscriptions; l; l=l->next) {
		Subscription* subscription = l->data;
		subscription->fn(observable, value, subscription->user);
	}
}


bool
agl_observable_set_int (AGlObservable* observable, int value)
{
	value = CLAMP(value, observable->min.i, observable->max.i);

	if (value != observable->value.i) {
		observable->value.i = value;

		GList* l = observable->subscriptions;
		for(;l;l=l->next){
			Subscription* subscription = l->data;
			subscription->fn(observable, observable->value, subscription->user);
		}
		return true;
	}
	return false;
}


void
agl_observable_set_float (AGlObservable* observable, float value)
{
	if (value >= observable->min.f && value <= observable->max.f) {

		observable->value.f = value;

		GList* l = observable->subscriptions;
		for (;l;l=l->next) {
			Subscription* subscription = l->data;
			subscription->fn(observable, observable->value, subscription->user);
		}
	}
}


void
agl_observable_subscribe (AGlObservable* observable, AGlObservableFn fn, gpointer user)
{
	observable->subscriptions = g_list_append(observable->subscriptions, AYYI_NEW(Subscription,
		.fn = fn,
		.user = user
	));
}


/*
 *  Calls back imediately with the current value
 */
void
agl_observable_subscribe_with_state (AGlObservable* observable, AGlObservableFn fn, gpointer user)
{
	agl_observable_subscribe(observable, fn, user);
	fn(observable, observable->value, user);
}


/*
 *  This can be used where you need `user_data` to be automatically freed when `object` is destroyed.
 */
void
agl_observable_add_closure (AGlObservable* observable, GObject* object, AGlObservableFn fn, gpointer user_data)
{
	g_object_watch_closure (object, ({
		GClosure* closure = g_cclosure_new(G_CALLBACK(fn), user_data, (GClosureNotify)g_free);

		void notify (void* _, GClosure* closure)
		{
			g_closure_unref(closure);
		}
		g_closure_add_invalidate_notifier (closure, NULL, notify);

		closure;
	}));

	agl_observable_subscribe_with_state(observable, fn, user_data);
}


/*
 *  Disconnect by either fn or user_data if they are set.
 *  If both are set, both must match
 */
void
agl_observable_unsubscribe (AGlObservable* observable, AGlObservableFn fn, gpointer user)
{
	for (GList* l=observable->subscriptions;l;) {
		Subscription* subscription = l->data;
		GList* link = l;
		l = l->next;
		if ((!fn || fn == (subscription->fn)) && (!user || (user == subscription->user))) {
			g_free(subscription);
			observable->subscriptions = g_list_delete_link(observable->subscriptions, link);
		}
	}
}


AGlObservable*
agl_observable_map (AGlObservable* source, AGlObservableMapFn fn, gpointer user_data)
{
	typedef struct {
		AGlObservable      observable;
		AGlObservable*     source;
		AGlObservableMapFn mapping;
		gpointer           user_data;
	} AGlObservableMap;

	AGlObservable* mapped = (AGlObservable*)AYYI_NEW(AGlObservableMap,
		.observable.max.i = INT_MAX,
		.source = source,
		.mapping = fn,
		.user_data = user_data
	);

	void map_handler (AGlObservable* o, AGlVal value, gpointer user_data)
	{
		AGlObservableMap* mapped = user_data;

		agl_observable_set((AGlObservable*)mapped, mapped->mapping((AGlObservable*)mapped, value, mapped->user_data));
	}
	agl_observable_subscribe (source, map_handler, mapped);

	return mapped;
}
