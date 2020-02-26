/* GSK - The GTK Scene Kit
 *
 * Copyright 2017 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __gsk_pango_h__
#define __gsk_pango_h__

#include <stdint.h>
#include <pango/pango.h>
#include "agl/text/gdkrgba.h"

G_BEGIN_DECLS

#define GSK_TYPE_PANGO_RENDERER    (gsk_pango_renderer_get_type ())
#define GSK_PANGO_RENDERER(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSK_TYPE_PANGO_RENDERER, GskPangoRenderer))
#define GSK_IS_PANGO_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSK_TYPE_PANGO_RENDERER))

typedef struct _GskPangoRenderer      GskPangoRenderer;
typedef struct _GskPangoRendererClass GskPangoRendererClass;

GType         gsk_pango_renderer_get_type (void) G_GNUC_CONST;

PangoContext* agl_pango_get_context       ();
void          agl_pango_show_layout       (PangoLayout*, int, int, float, uint32_t fg_color);

G_END_DECLS

#endif
