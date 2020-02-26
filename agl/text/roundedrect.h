/*
 * Copyright 2016  Endless
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
#ifndef __rounded_rect_h__
#define __rounded_rect_h__

#include <cairo.h>

G_BEGIN_DECLS

/**
 * AGL_ROUNDED_RECT_INIT:
 * @_x: the X coordinate of the origin
 * @_y: the Y coordinate of the origin
 * @_w: the width
 * @_h: the height
 *
 * Initializes a #AGlRoundedRect when declaring it.
 * All corner sizes will be initialized to 0.
 */
#define AGL_ROUNDED_RECT_INIT(_x,_y,_w,_h) \
	(AGlRoundedRect) { .bounds = GRAPHENE_RECT_INIT(_x,_y,_w,_h), \
		.corner = { \
		   GRAPHENE_SIZE_INIT(0, 0),\
		   GRAPHENE_SIZE_INIT(0, 0),\
		   GRAPHENE_SIZE_INIT(0, 0),\
		   GRAPHENE_SIZE_INIT(0, 0),\
		}}

AGlRoundedRect* agl_rounded_rect_init             (AGlRoundedRect*, const graphene_rect_t* bounds, const graphene_size_t* top_left, const graphene_size_t* top_right, const graphene_size_t* bottom_right, const graphene_size_t* bottom_left);
AGlRoundedRect* agl_rounded_rect_init_copy        (AGlRoundedRect*, const AGlRoundedRect* src);
AGlRoundedRect* agl_rounded_rect_init_from_rect   (AGlRoundedRect*, const graphene_rect_t* bounds, float radius);

AGlRoundedRect* agl_rounded_rect_normalize        (AGlRoundedRect*);

AGlRoundedRect* agl_rounded_rect_offset           (AGlRoundedRect*, float dx, float dy);
AGlRoundedRect* agl_rounded_rect_shrink           (AGlRoundedRect*, float top, float right, float bottom, float left);

gboolean        agl_rounded_rect_contains_point   (const AGlRoundedRect*, const graphene_point_t* point) G_GNUC_PURE;
gboolean        agl_rounded_rect_contains_rect    (const AGlRoundedRect*, const graphene_rect_t* rect) G_GNUC_PURE;
gboolean        agl_rounded_rect_intersects_rect  (const AGlRoundedRect*, const graphene_rect_t* rect) G_GNUC_PURE;

/* private */

bool            agl_rounded_rect_is_circular      (const AGlRoundedRect*);

void            agl_rounded_rect_path             (const AGlRoundedRect*, cairo_t* cr);
void            agl_rounded_rect_to_float         (const AGlRoundedRect*, float rect[12]);

bool            agl_rounded_rect_equal            (gconstpointer rect1, gconstpointer rect2) G_GNUC_PURE;
char*           agl_rounded_rect_to_string        (const AGlRoundedRect*);


G_END_DECLS

#endif
