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
#include <stdbool.h>
#include <glib.h>
#include "ayyi-utils/utils.h"
#include "observable.h"

typedef struct {
   AyyiObservableFn fn;
   gpointer         user;
} Subscription;


AyyiObservable*
ayyi_observable_new ()
{
	return AYYI_NEW(AyyiObservable,
		.max.i = INT_MAX
	);
}


/*
 *  This will not free string values. Freeing of string values needs to be done by the user.
 */
void
ayyi_observable_free (AyyiObservable* observable)
{
	g_list_free_full(observable->subscriptions, g_free);
	g_free(observable);
}


/*
 *	Because of the possibility of uninitialized padding
 *	there is no way to check equality of 2 unions so
 *	it is not possible to check here if the value has changed.
 *  Use `ayyi_observable_set_int` or `ayyi_observable_set_float` where possible.
 */
void
ayyi_observable_set (AyyiObservable* observable, AyyiVal value)
{
	observable->value = value;

	for (GList* l = observable->subscriptions; l; l=l->next) {
		Subscription* subscription = l->data;
		subscription->fn(observable, value, subscription->user);
	}
}


bool
ayyi_observable_set_int (AyyiObservable* observable, int value)
{
	value = CLAMP(value, observable->min.i, observable->max.i);

	if (value != observable->value.i) {
		observable->value.i = value;

		GList* next;
		for (GList* l=observable->subscriptions;l;l=next) {
			next = l->next;
			Subscription* subscription = l->data;
			subscription->fn(observable, observable->value, subscription->user);
		}
		return true;
	}
	return false;
}


void
ayyi_observable_set_float (AyyiObservable* observable, float value)
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


/*
 *  Takes ownership of arg `str`
 */
void
ayyi_observable_set_string (AyyiObservable* observable, const char* str)
{
	bool changed = true;

	if (observable->value.c) {
		changed = (!str) || strcmp(str, observable->value.c);
		g_free(changed ? observable->value.c : (char*)str);
	}

	if (changed)
		ayyi_observable_set(observable, (AyyiVal){.c = (char*)str});
}


void
ayyi_observable_subscribe (AyyiObservable* observable, AyyiObservableFn fn, gpointer user)
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
ayyi_observable_subscribe_with_state (AyyiObservable* observable, AyyiObservableFn fn, gpointer user)
{
	ayyi_observable_subscribe(observable, fn, user);
	fn(observable, observable->value, user);
}


/*
 *  This can be used where you need `user_data` to be automatically freed when `object` is destroyed.
 */
void
ayyi_observable_add_closure (AyyiObservable* observable, GObject* object, AyyiObservableFn fn, gpointer user_data)
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

	ayyi_observable_subscribe_with_state(observable, fn, user_data);
}


/*
 *  Disconnect by either fn or user_data if they are set.
 *  If both are set, both must match
 */
void
ayyi_observable_unsubscribe (AyyiObservable* observable, AyyiObservableFn fn, gpointer user)
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


AyyiObservable*
ayyi_observable_map (AyyiObservable* source, AyyiObservableMapFn fn, gpointer user_data)
{
	typedef struct {
		AyyiObservable      observable;
		AyyiObservable*     source;
		AyyiObservableMapFn mapping;
		gpointer            user_data;
	} AyyiObservableMap;

	AyyiObservable* mapped = (AyyiObservable*)AYYI_NEW(AyyiObservableMap,
		.observable.max.i = INT_MAX,
		.source = source,
		.mapping = fn,
		.user_data = user_data
	);

	void map_handler (AyyiObservable* o, AyyiVal value, gpointer user_data)
	{
		AyyiObservableMap* mapped = user_data;

		ayyi_observable_set((AyyiObservable*)mapped, mapped->mapping((AyyiObservable*)mapped, value, mapped->user_data));
	}
	ayyi_observable_subscribe (source, map_handler, mapped);

	return mapped;
}


AyyiObservable*
ayyi_array_observable_new ()
{
	AyyiArrayObservable* observable = AYYI_NEW(AyyiArrayObservable,
		.array = g_ptr_array_new()
	);

	return (AyyiObservable*)observable;
}


void
ayyi_array_observable_add (AyyiObservable* observable, gpointer item)
{
	AyyiArrayObservable* array = (AyyiArrayObservable*)observable;

	observable->value.p = item;
	array->change = AYYI_OBSERVABLE_ADD;
	g_ptr_array_add(array->array, item);

	ayyi_observable_set(observable, observable->value);
}


void
ayyi_array_observable_remove (AyyiObservable* observable, gpointer item)
{
	AyyiArrayObservable* array = (AyyiArrayObservable*)observable;

	observable->value.p = item;
	array->change = AYYI_OBSERVABLE_REMOVE;
	g_ptr_array_remove(array->array, item);

	ayyi_observable_set(observable, observable->value);
}
