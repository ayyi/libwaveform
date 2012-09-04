/*
  copyright (C) 2012 Tim Orford <tim@orford.org>

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

  ---------------------------------------------------------------

  WaveformView is a Gtk widget based on GtkDrawingArea.
  It displays an audio waveform derived from a peak file.

*/
#ifndef __waveform_view_h__
#define __waveform_view_h__

#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include "waveform/typedefs.h"
#include "waveform/utils.h"
#include "waveform/canvas.h"
#include "waveform/peak.h"

G_BEGIN_DECLS

#define TYPE_WAVEFORM_VIEW (waveform_view_get_type ())
#define WAVEFORM_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_WAVEFORM_VIEW, WaveformView))
#define WAVEFORM_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_WAVEFORM_VIEW, WaveformViewClass))
#define IS_WAVEFORM_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_WAVEFORM_VIEW))
#define IS_WAVEFORM_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_WAVEFORM_VIEW))
#define WAVEFORM_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_WAVEFORM_VIEW, WaveformViewClass))

typedef struct _WaveformViewClass WaveformViewClass;
typedef struct _WaveformViewPrivate WaveformViewPrivate;

struct _WaveformView {
	GtkDrawingArea       parent_instance;
	Waveform*            waveform;
	float                zoom;
	uint64_t             start_frame;
	WaveformViewPrivate* priv;
};

struct _WaveformViewClass {
	GtkDrawingAreaClass parent_class;
};


GType           waveform_view_get_type      () G_GNUC_CONST;
void            waveform_view_set_gl        (GdkGLContext*);

WaveformView*   waveform_view_new           (Waveform*);
void            waveform_view_load_file     (WaveformView*, const char*); //be careful, it force loads, even if already loaded.
void            waveform_view_set_waveform  (WaveformView*, Waveform*);
void            waveform_view_set_zoom      (WaveformView*, float);
void            waveform_view_set_start     (WaveformView*, int64_t);
void            waveform_view_set_colour    (WaveformView*, uint32_t fg, uint32_t bg);
void            waveform_view_set_show_rms  (WaveformView*, gboolean);
void            waveform_view_set_show_grid (WaveformView*, gboolean);

WaveformCanvas* waveform_view_get_canvas    (WaveformView*);


G_END_DECLS

#endif //__waveform_view_h__
