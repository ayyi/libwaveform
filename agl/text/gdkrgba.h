/* GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __GDK_RGBA_H__
#define __GDK_RGBA_H__

#include <gdk/gdktypes.h>

G_BEGIN_DECLS

typedef struct
{
  float red;
  float green;
  float blue;
  float alpha;
} GdkRGBA;


#define GDK_TYPE_RGBA (gdk_rgba_get_type ())

GType     gdk_rgba_get_type  (void) G_GNUC_CONST;

GdkRGBA * gdk_rgba_copy      (const GdkRGBA *rgba);
void      gdk_rgba_free      (GdkRGBA       *rgba);

gboolean  gdk_rgba_is_clear  (const GdkRGBA *rgba);
gboolean  gdk_rgba_is_opaque (const GdkRGBA *rgba);

guint     gdk_rgba_hash      (gconstpointer  p);
gboolean  gdk_rgba_equal     (gconstpointer  p1,
                              gconstpointer  p2);


G_END_DECLS

#endif
