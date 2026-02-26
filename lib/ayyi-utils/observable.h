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

#ifndef __ayyi_observable_h__
#define __ayyi_observable_h__

#include <stdbool.h>
#include <glib-object.h>

typedef union
{
    int         i;
    unsigned    u;
    float       f;
    double      d;
    char*       c;
    int64_t     b;
    void*       p;
    struct { int32_t val, prev; } s;
} AGlVal;

typedef struct {
   AGlVal value;
   AGlVal min;
   AGlVal max;
   GList* subscriptions;
} AGlObservable;

#define AyyiVal AGlVal
#define AyyiObservable AGlObservable
#define ayyi_observable_new agl_observable_new
#define ayyi_observable_set_int agl_observable_set_int

typedef void   (*AGlObservableFn)    (AGlObservable*, AGlVal, gpointer);
typedef AGlVal (*AGlObservableMapFn) (AGlObservable*, AGlVal, gpointer);

AGlObservable* agl_observable_new         ();
void           agl_observable_free        (AGlObservable*);
void           agl_observable_set         (AGlObservable*, AGlVal);
bool           agl_observable_set_int     (AGlObservable*, int);
void           agl_observable_set_float   (AGlObservable*, float);
void           agl_observable_subscribe   (AGlObservable*, AGlObservableFn, gpointer);
void           agl_observable_subscribe_with_state
                                          (AGlObservable*, AGlObservableFn, gpointer);
void           agl_observable_add_closure (AGlObservable*, GObject*, AGlObservableFn, gpointer);
void           agl_observable_unsubscribe (AGlObservable*, AGlObservableFn, gpointer);
AGlObservable* agl_observable_map         (AGlObservable*, AGlObservableMapFn, gpointer);

/*
 *  ArrayObservable is a GPtrArray that emits when items are added or removed.
 *  The value property contains the last item that was changed.
 */
typedef struct {
    AGlObservable observable;
    GPtrArray*    array;
    enum {
       AYYI_OBSERVABLE_ADD,
       AYYI_OBSERVABLE_REMOVE,
    }             change;
} AyyiArrayObservable;

AGlObservable* ayyi_array_observable_new    ();
void           ayyi_array_observable_add    (AGlObservable*, gpointer);
void           ayyi_array_observable_remove (AGlObservable*, gpointer);

#endif
