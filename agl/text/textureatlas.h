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
#ifndef __texture_atlas_h__
#define __texture_atlas_h__

#include "glib.h"
#include "stb_rect_pack.h"

typedef struct
{
   struct stbrp_context context;
   struct stbrp_node *nodes;

   int width;
   int height;

   guint texture_id;

   int unused_pixels; // Pixels of rects that have been used at some point, but are now unused

   void* user_data;

} GskGLTextureAtlas;

typedef struct
{
   int ref_count;

   GPtrArray* atlases;
}
GskGLTextureAtlases;


GskGLTextureAtlases*
        gsk_gl_texture_atlases_new            (void);
GskGLTextureAtlases*
        gsk_gl_texture_atlases_ref            (GskGLTextureAtlases*);
void    gsk_gl_texture_atlases_unref          (GskGLTextureAtlases*);

#if 0
void    gsk_gl_texture_atlases_begin_frame    (GskGLTextureAtlases*, GPtrArray* removed);
#endif
bool    gsk_gl_texture_atlases_pack           (GskGLTextureAtlases*, int width, int height, GskGLTextureAtlas**, int* out_x, int* out_y);
void    gsk_gl_texture_atlas_init             (GskGLTextureAtlas*, int width, int height);
void    gsk_gl_texture_atlas_free             (GskGLTextureAtlas*);

void    gsk_gl_texture_atlas_realize          (GskGLTextureAtlas*);

void    gsk_gl_texture_atlas_mark_unused      (GskGLTextureAtlas*, int width, int height);

void    gsk_gl_texture_atlas_mark_used        (GskGLTextureAtlas*, int width, int height);

bool    gsk_gl_texture_atlas_pack             (GskGLTextureAtlas*, int width, int height, int* out_x, int* out_y);

#if 0
double  gsk_gl_texture_atlas_get_unused_ratio (const GskGLTextureAtlas*);
#endif

#endif
