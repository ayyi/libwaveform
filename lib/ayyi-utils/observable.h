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
} AyyiVal;

typedef struct {
   AyyiVal value;
   AyyiVal min;
   AyyiVal max;
   GList* subscriptions;
} AyyiObservable;

typedef void    (*AyyiObservableFn)    (AyyiObservable*, AyyiVal, gpointer);
typedef AyyiVal (*AyyiObservableMapFn) (AyyiObservable*, AyyiVal, gpointer);

AyyiObservable* ayyi_observable_new         ();
void            ayyi_observable_free        (AyyiObservable*);
void            ayyi_observable_set         (AyyiObservable*, AyyiVal);
bool            ayyi_observable_set_int     (AyyiObservable*, int);
void            ayyi_observable_set_float   (AyyiObservable*, float);
void            ayyi_observable_set_string  (AyyiObservable*, const char*);
void            ayyi_observable_subscribe   (AyyiObservable*, AyyiObservableFn, gpointer);
void            ayyi_observable_subscribe_with_state
                                            (AyyiObservable*, AyyiObservableFn, gpointer);
void            ayyi_observable_add_closure (AyyiObservable*, GObject*, AyyiObservableFn, gpointer);
void            ayyi_observable_unsubscribe (AyyiObservable*, AyyiObservableFn, gpointer);
AyyiObservable* ayyi_observable_map         (AyyiObservable*, AyyiObservableMapFn, gpointer);

/*
 *  ArrayObservable is a GPtrArray that emits when items are added or removed.
 *  The value property contains the last item that was changed.
 */
typedef struct {
    AyyiObservable observable;
    GPtrArray*     array;
    enum {
       AYYI_OBSERVABLE_ADD,
       AYYI_OBSERVABLE_REMOVE,
    }              change;
} AyyiArrayObservable;

AyyiObservable* ayyi_array_observable_new    ();
void            ayyi_array_observable_add    (AyyiObservable*, gpointer);
void            ayyi_array_observable_remove (AyyiObservable*, gpointer);

#endif
