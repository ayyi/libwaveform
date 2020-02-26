/* gdktexture.h
 *
 * Copyright 2016  Benjamin Otte
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

#ifndef __gdk_texture_h__
#define __gdk_texture_h__

#include <gdk/gdktypes.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

typedef struct _GdkTexture      GdkTexture;
typedef struct _GdkTextureClass GdkTextureClass;

#define GDK_TYPE_TEXTURE     (gdk_texture_get_type ())
#define GDK_TEXTURE(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDK_TYPE_TEXTURE, GdkTexture))
#define GDK_IS_TEXTURE(obj)  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDK_TYPE_TEXTURE))

G_DEFINE_AUTOPTR_CLEANUP_FUNC(GdkTexture, g_object_unref)


GType                   gdk_texture_get_type                   (void) G_GNUC_CONST;

GdkTexture *            gdk_texture_new_for_pixbuf             (GdkPixbuf       *pixbuf);
GdkTexture *            gdk_texture_new_from_resource          (const char      *resource_path);
GdkTexture *            gdk_texture_new_from_file              (GFile           *file,
                                                                GError         **error);

int                     gdk_texture_get_width                  (GdkTexture      *texture);
int                     gdk_texture_get_height                 (GdkTexture      *texture);

void                    gdk_texture_download                   (GdkTexture      *texture,
                                                                guchar          *data,
                                                                gsize            stride);
gboolean                gdk_texture_save_to_png                (GdkTexture      *texture,
                                                                const char      *filename);

G_END_DECLS

#endif
