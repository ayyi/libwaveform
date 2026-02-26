/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

typedef void (*AyyiPromiseCallback) (gpointer user_data, gpointer);

typedef union
{
    int    i;
    float  f;
    char*  c;

} AyyiPromiseVal;

typedef struct {
    AyyiPromiseVal value;
    GList*         callbacks;  // type AyyiPromiseCallback
    GList*         children;   // type AyyiPromise
    gpointer       user_data;
    gboolean       is_resolved;
    GError*        error;
    int            refcount;
} AyyiPromise;

AyyiPromise* ayyi_promise_new          (gpointer);
void         ayyi_promise_unref        (AyyiPromise*);
void         ayyi_promise_add_callback (AyyiPromise*, AyyiPromiseCallback, gpointer);
void         ayyi_promise_when         (AyyiPromise*, AyyiPromise*, ...);
void         ayyi_promise_resolve      (AyyiPromise*, AyyiPromiseVal*);
void         ayyi_promise_fail         (AyyiPromise*, GError*);
