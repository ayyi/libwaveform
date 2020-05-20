/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) GTK+ Team and others                                   |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __driver_h__
#define __driver_h__

#include <cairo.h>
#include <gdk/gdk.h>
#include <graphene.h>
#include "agl/typedefs.h"

typedef struct {
   float position[2];
   float uv[2];
} GskQuadVertex;

typedef struct {
   cairo_rectangle_int_t rect;
   guint texture_id;
} TextureSlice;


void            driver_init                      ();
void            driver_free                      ();
void            driver_begin_frame               ();
void            driver_end_frame                 ();
bool            driver_in_frame                  ();
#if 0
int             driver_get_texture_for_texture   (GdkTexture*, int min_filter, int mag_filter);
int             driver_get_texture_for_pointer   (gpointer);
void            driver_set_texture_for_pointer   (gpointer, TextureId);
#endif
int             driver_create_texture            (float width, float height);
#if 0
void            driver_create_render_target      (int width, int height, int* out_texture_id, int* out_render_target_id);
#endif
void            driver_mark_texture_permanent    (AGlTextureId);
void            driver_bind_source_texture       (AGlTextureId);

void            driver_init_texture_empty        (AGlTextureId, int min_filter, int max_filter);

void            driver_destroy_texture           (AGlTextureId);
int             driver_collect_textures          ();
#if 0
void            gsk_gl_driver_slice_texture      (GdkTexture*, TextureSlice** out_slices, guint* out_n_slices);
#endif

#endif
