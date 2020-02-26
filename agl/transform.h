/*
 * Copyright © 2019 Benjamin Otte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Benjamin Otte <otte@gnome.org>
 */


#ifndef __agl_transform_h__
#define __agl_transform_h__

#include "text/enums.h"
#include "text/types.h"

G_BEGIN_DECLS

/**
 * AGlTransformCategory:
 * @AGL_TRANSFORM_CATEGORY_UNKNOWN: The category of the matrix has not been
 *     determined.
 * @AGL_TRANSFORM_CATEGORY_ANY: Analyzing the matrix concluded that it does
 *     not fit in any other category.
 * @AGL_TRANSFORM_CATEGORY_3D: The matrix is a 3D matrix. This means that
 *     the w column (the last column) has the values (0, 0, 0, 1).
 * @AGL_TRANSFORM_CATEGORY_2D: The matrix is a 2D matrix. This is equivalent
 *     to graphene_matrix_is_2d() returning %TRUE. In particular, this
 *     means that Cairo can deal with the matrix.
 * @AGL_TRANSFORM_CATEGORY_2D_AFFINE: The matrix is a combination of 2D scale
 *     and 2D translation operations. In particular, this means that any
 *     rectangle can be transformed exactly using this matrix.
 * @AGL_TRANSFORM_CATEGORY_2D_TRANSLATE: The matrix is a 2D translation.
 * @AGL_TRANSFORM_CATEGORY_IDENTITY: The matrix is the identity matrix.
 *
 * The categories of matrices relevant for AGL and GTK. Note that any
 * category includes matrices of all later categories. So if you want
 * to for example check if a matrix is a 2D matrix,
 * `category >= AGL_TRANSFORM_CATEGORY_2D` is the way to do this.
 *
 * Also keep in mind that rounding errors may cause matrices to not
 * conform to their categories. Otherwise, matrix operations done via
 * mutliplication will not worsen categories. So for the matrix
 * multiplication `C = A * B`, `category(C) = MIN (category(A), category(B))`.
 */
typedef enum
{
  AGL_TRANSFORM_CATEGORY_UNKNOWN,
  AGL_TRANSFORM_CATEGORY_ANY,
  AGL_TRANSFORM_CATEGORY_3D,
  AGL_TRANSFORM_CATEGORY_2D,
  AGL_TRANSFORM_CATEGORY_2D_AFFINE,
  AGL_TRANSFORM_CATEGORY_2D_TRANSLATE,
  AGL_TRANSFORM_CATEGORY_IDENTITY
} AGlTransformCategory;

typedef struct _AGlTransform AGlTransform;

#define AGL_TYPE_TRANSFORM (agl_transform_get_type ())

GType          agl_transform_get_type         (void) G_GNUC_CONST;

AGlTransform*  agl_transform_ref              (AGlTransform*);
void           agl_transform_unref            (AGlTransform*);

void           agl_transform_print            (AGlTransform*, GString*);
char*          agl_transform_to_string        (AGlTransform*);
gboolean       agl_transform_parse            (const char* string, AGlTransform** out_transform);
void           agl_transform_to_matrix        (AGlTransform*, graphene_matrix_t* out_matrix);
void           agl_transform_to_2d            (AGlTransform*, float* out_xx, float* out_yx, float* out_xy, float* out_yy, float* out_dx, float* out_dy);
void           agl_transform_to_affine        (AGlTransform*, float* out_scale_x, float* out_scale_y, float* out_dx, float* out_dy);
void           agl_transform_to_translate     (AGlTransform*, float* out_dx, float* out_dy);

AGlTransformCategory
               agl_transform_get_category     (AGlTransform*) G_GNUC_PURE;
gboolean       agl_transform_equal            (AGlTransform*, AGlTransform*) G_GNUC_PURE;

AGlTransform*  agl_transform_new              (void);
AGlTransform*  agl_transform_transform        (AGlTransform*, AGlTransform*) G_GNUC_WARN_UNUSED_RESULT;
AGlTransform*  agl_transform_invert           (AGlTransform*) G_GNUC_WARN_UNUSED_RESULT;
AGlTransform*  agl_transform_matrix           (AGlTransform*, const graphene_matrix_t*) G_GNUC_WARN_UNUSED_RESULT;
AGlTransform*  agl_transform_translate        (AGlTransform*, const graphene_point_t*) G_GNUC_WARN_UNUSED_RESULT;
AGlTransform*  agl_transform_translate_3d     (AGlTransform*, const graphene_point3d_t*) G_GNUC_WARN_UNUSED_RESULT;
AGlTransform*  agl_transform_rotate           (AGlTransform*, float angle) G_GNUC_WARN_UNUSED_RESULT;
AGlTransform*  agl_transform_rotate_3d        (AGlTransform*, float angle, const graphene_vec3_t* axis) G_GNUC_WARN_UNUSED_RESULT;
AGlTransform*  agl_transform_scale            (AGlTransform*, float factor_x, float factor_y) G_GNUC_WARN_UNUSED_RESULT;
AGlTransform*  agl_transform_scale_3d         (AGlTransform*, float factor_x, float factor_y, float factor_z) G_GNUC_WARN_UNUSED_RESULT;
AGlTransform*  agl_transform_perspective      (AGlTransform*, float depth) G_GNUC_WARN_UNUSED_RESULT;

void           agl_transform_transform_bounds (AGlTransform*, const graphene_rect_t*, graphene_rect_t* out);
void           agl_transform_transform_point  (AGlTransform*, const graphene_point_t*, graphene_point_t* out);

G_END_DECLS

#endif
