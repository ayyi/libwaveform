/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2018-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __agl_observable_h__
#define __agl_observable_h__

typedef struct {
   int    value;
   int    min;
   int    max;
   GList* subscriptions;
} AGlObservable;

typedef void (*AGlObservableFn) (AGlObservable*, int value, gpointer);

AGlObservable* agl_observable_new       ();
void           agl_observable_free      (AGlObservable*);
void           agl_observable_set       (AGlObservable*, int);
void           agl_observable_subscribe (AGlObservable*, AGlObservableFn, gpointer);

#endif
