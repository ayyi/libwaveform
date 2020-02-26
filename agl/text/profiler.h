/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | Copyright (C) 2020-2020 Tim Orford <tim@orford.org>                  |
* | Copyright (C) GTK+ Team and others                                   |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __gsk_profiler_h__
#define __gsk_profiler_h__

#ifdef HAVE_PROFILER

#include <stdbool.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GSK_TYPE_PROFILER (gsk_profiler_get_type ())
G_DECLARE_FINAL_TYPE (GskProfiler, gsk_profiler, GSK, PROFILER, GObject)

GskProfiler* gsk_profiler_new              (void);

GQuark       gsk_profiler_add_counter      (GskProfiler*, const char* counter_name, const char* description, gboolean can_reset);
GQuark       gsk_profiler_add_timer        (GskProfiler*, const char* timer_name, const char* description, gboolean invert, gboolean can_reset);

void         gsk_profiler_counter_inc      (GskProfiler*, GQuark counter_id);
void         gsk_profiler_counter_add      (GskProfiler*, GQuark counter_id, gint64 increment);
void         gsk_profiler_counter_set      (GskProfiler*, GQuark counter_id, gint64 value);
void         gsk_profiler_timer_begin      (GskProfiler*, GQuark timer_id);
gint64       gsk_profiler_timer_end        (GskProfiler*, GQuark timer_id);
void         gsk_profiler_timer_set        (GskProfiler*, GQuark timer_id, gint64 value);

gint64       gsk_profiler_counter_get      (GskProfiler*, GQuark counter_id);
gint64       gsk_profiler_timer_get        (GskProfiler*, GQuark timer_id);
gint64       gsk_profiler_timer_get_start  (GskProfiler*, GQuark timer_id);

void         gsk_profiler_reset            (GskProfiler*);

void         gsk_profiler_push_samples     (GskProfiler*);
void         gsk_profiler_append_counters  (GskProfiler*, GString* buffer);
void         gsk_profiler_append_timers    (GskProfiler*, GString* buffer);

void         gsk_profiler_begin_gpu_region (GskProfiler*);
guint64      gsk_profiler_end_gpu_region   (GskProfiler*);

bool         profiler_is_running           (GskProfiler*);
void         profiler_add_mark             (gint64 start, guint64 duration, const char* name, const char* message);

G_END_DECLS

#endif
#endif
