/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2022 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | WaveformViewPlus is a Gtk widget based on GtkDrawingArea.            |
 | It displays an audio waveform represented by a Waveform object.      |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include <gtk/gtk.h>
#include "agl/gtk-area.h"
#include "actors/background.h"
#include "waveform/actor.h"
#include "waveform/grid.h"
#include "waveform/spp.h"
#include "waveform/text.h"
#include "waveform/view_plus.h"

G_BEGIN_DECLS

#define TYPE_WAVEFORM_VIEW_PLUS            (waveform_view_plus_get_type ())
#define WAVEFORM_VIEW_PLUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_WAVEFORM_VIEW_PLUS, WaveformViewPlus))
#define WAVEFORM_VIEW_PLUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_WAVEFORM_VIEW_PLUS, WaveformViewPlusClass))
#define IS_WAVEFORM_VIEW_PLUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_WAVEFORM_VIEW_PLUS))
#define IS_WAVEFORM_VIEW_PLUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_WAVEFORM_VIEW_PLUS))
#define WAVEFORM_VIEW_PLUS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_WAVEFORM_VIEW_PLUS, WaveformViewPlusClass))

typedef struct _WaveformViewPlusClass WaveformViewPlusClass;
typedef struct _WaveformViewPlusPrivate WaveformViewPlusPrivate;

struct _WaveformViewPlus {
	AGlGtkArea               parent_instance;

	Waveform*                waveform;
#ifndef USE_CANVAS_SCALING
	float                    zoom;        // zoom is 1 when whole file is visible
#endif
	int64_t                  start_frame;

	uint32_t                 bg_colour;

	GtkGesture*              click;

	WaveformViewPlusPrivate* priv;
};

struct _WaveformViewPlusClass {
	AGlGtkAreaClass parent_class;
};


GType             waveform_view_plus_get_type      () G_GNUC_CONST;
void              waveform_view_plus_set_gl        (GdkGLContext*);

WaveformViewPlus* waveform_view_plus_new           (Waveform*);
void              waveform_view_plus_load_file     (WaveformViewPlus*, const char*, WfCallback3, gpointer); // be careful, it force loads, even if already loaded.
void              waveform_view_plus_set_waveform  (WaveformViewPlus*, Waveform*);
float             waveform_view_plus_get_zoom      (WaveformViewPlus*);
void              waveform_view_plus_set_zoom      (WaveformViewPlus*, float);
void              waveform_view_plus_set_start     (WaveformViewPlus*, int64_t);
void              waveform_view_plus_set_region    (WaveformViewPlus*, int64_t, int64_t);
void              waveform_view_plus_set_colour    (WaveformViewPlus*, uint32_t fg, uint32_t bg);
void              waveform_view_plus_set_show_rms  (WaveformViewPlus*, bool);
AGlActor*         waveform_view_plus_add_layer     (WaveformViewPlus*, AGlActor*, int z);
AGlActor*         waveform_view_plus_get_layer     (WaveformViewPlus*, int);
void              waveform_view_plus_remove_layer  (WaveformViewPlus*, AGlActor*);

WaveformContext*  waveform_view_plus_get_context   (WaveformViewPlus*);
WaveformActor*    waveform_view_plus_get_actor     (WaveformViewPlus*);


G_END_DECLS
