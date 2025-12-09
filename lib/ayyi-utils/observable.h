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

#pragma once

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
