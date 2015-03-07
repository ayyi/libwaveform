/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifndef __wf_promise_h__
#define __wf_promise_h__

typedef void (*AMPromiseCallback) (gpointer user_data, gpointer);

typedef union
{
    int    i;
    float  f;
    char*  c;

} PromiseVal;

typedef struct {
    PromiseVal    value;
    GList*        callbacks;  // type AMPromiseCallback
    GList*        children;   // type AMPromise
    gpointer      user_data;
    gboolean      is_resolved;
    gboolean      is_failed;
    int           refcount;
} AMPromise;

AMPromise* am_promise_new          (gpointer);
void       am_promise_unref        (AMPromise*);
void       am_promise_add_callback (AMPromise*, AMPromiseCallback, gpointer);
void       am_promise_when         (AMPromise*, AMPromise*, ...);
void       am_promise_resolve      (AMPromise*, PromiseVal*);
void       am_promise_fail         (AMPromise*);

#endif
