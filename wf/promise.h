/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __wf_promise_h__
#define __wf_promise_h__

typedef void (*WfPromiseCallback) (gpointer user_data, gpointer);

typedef union
{
    int    i;
    float  f;
    char*  c;

} PromiseVal;

typedef struct {
    PromiseVal    value;
    GList*        callbacks;  // type WfPromiseCallback
    GList*        children;   // type AMPromise
    gpointer      user_data;
    gboolean      is_resolved;
    GError*       error;
    int           refcount;
} AMPromise;

AMPromise* am_promise_new          (gpointer);
void       am_promise_unref        (AMPromise*);
void       am_promise_add_callback (AMPromise*, WfPromiseCallback, gpointer);
void       am_promise_when         (AMPromise*, AMPromise*, ...);
void       am_promise_resolve      (AMPromise*, PromiseVal*);
void       am_promise_fail         (AMPromise*, GError*);

#define am_promise_unref0(var) ((var == NULL) ? NULL : (var = (am_promise_unref(var), NULL)))

#endif
