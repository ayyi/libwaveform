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


/**
 * SECTION:AGlTransform
 * @Title: AGlTransform
 * @Short_description: A description for transform operations
 *
 * #AGlTransform is an object to describe transform matrices. Unlike
 * #graphene_matrix_t, #AGlTransform retains the steps in how a transform was
 * constructed, and allows inspecting them. It is modeled after the way
 * CSS describes transforms.
 *
 * #AGlTransform objects are immutable and cannot be changed after creation.
 * This means code can safely expose them as properties of objects without
 * having to worry about others changing them.
 */

#include "config.h"
#include "transform.h"

typedef struct _AGlTransformClass AGlTransformClass;

struct _AGlTransform
{
  const AGlTransformClass *transform_class;

  AGlTransformCategory category;
  AGlTransform *next;
};

struct _AGlTransformClass
{
  gsize struct_size;
  const char *type_name;

  void                  (* finalize)            (AGlTransform           *transform);
  void                  (* to_matrix)           (AGlTransform           *transform,
                                                 graphene_matrix_t      *out_matrix);
  void                  (* apply_2d)            (AGlTransform           *transform,
                                                 float                  *out_xx,
                                                 float                  *out_yx,
                                                 float                  *out_xy,
                                                 float                  *out_yy,
                                                 float                  *out_dx,
                                                 float                  *out_dy);
  void                  (* apply_affine)        (AGlTransform           *transform,
                                                 float                  *out_scale_x,
                                                 float                  *out_scale_y,
                                                 float                  *out_dx,
                                                 float                  *out_dy);
  void                  (* apply_translate)     (AGlTransform           *transform,
                                                 float                  *out_dx,
                                                 float                  *out_dy);
  void                  (* print)               (AGlTransform           *transform,
                                                 GString                *string);
  AGlTransform *        (* apply)               (AGlTransform           *transform,
                                                 AGlTransform           *apply_to);
  AGlTransform *        (* invert)              (AGlTransform           *transform,
                                                 AGlTransform           *next);
  /* both matrices have the same type */
  gboolean              (* equal)               (AGlTransform           *first_transform,
                                                 AGlTransform           *second_transform);
};

/**
 * AGlTransform: (ref-func agl_transform_ref) (unref-func agl_transform_unref)
 *
 * The `AGlTransform` structure contains only private data.
 */

G_DEFINE_BOXED_TYPE (AGlTransform, agl_transform, agl_transform_ref, agl_transform_unref)

static bool agl_transform_is_identity (AGlTransform*);

#ifndef HAVE_GLIB_2_58
#define G_APPROX_VALUE(a, b, epsilon) \
  (((a) > (b) ? (a) - (b) : (b) - (a)) < (epsilon))
#endif


static inline gboolean
agl_transform_has_class (AGlTransform* self, const AGlTransformClass* transform_class)
{
	return self != NULL && self->transform_class == transform_class;
}


/*< private >
 * agl_transform_alloc:
 * @transform_class: class structure for this self
 * @category: The category of this transform. Will be used to initialize
 *     the result's category together with &next's category
 * @next: (transfer full): Next matrix to multiply with or %NULL if none
 *
 * Returns: (transfer full): the newly created #AGlTransform
 */
static gpointer
agl_transform_alloc (const AGlTransformClass *transform_class,
                     AGlTransformCategory     category,
                     AGlTransform            *next)
{
	g_return_val_if_fail (transform_class, NULL);

#ifdef HAVE_GLIB_2_58
	AGlTransform* self = g_atomic_rc_box_alloc0 (transform_class->struct_size);
#else
	AGlTransform* self = g_malloc0 (transform_class->struct_size);
#endif

	self->transform_class = transform_class;
	self->category = next ? MIN (category, next->category) : category;
	self->next = agl_transform_is_identity (next) ? NULL : next;

	return self;
}

/*** IDENTITY ***/

static void
agl_identity_transform_finalize (AGlTransform *transform)
{
}

static void
agl_identity_transform_to_matrix (AGlTransform      *transform,
                                  graphene_matrix_t *out_matrix)
{
	graphene_matrix_init_identity (out_matrix);
}

static void
agl_identity_transform_apply_2d (AGlTransform *transform,
                                 float        *out_xx,
                                 float        *out_yx,
                                 float        *out_xy,
                                 float        *out_yy,
                                 float        *out_dx,
                                 float        *out_dy)
{
}

static void
agl_identity_transform_apply_affine (AGlTransform *transform,
                                     float        *out_scale_x,
                                     float        *out_scale_y,
                                     float        *out_dx,
                                     float        *out_dy)
{
}

static void
agl_identity_transform_apply_translate (AGlTransform *transform,
                                        float        *out_dx,
                                        float        *out_dy)
{
}

static void
agl_identity_transform_print (AGlTransform* transform, GString* string)
{
  g_string_append (string, "none");
}

static AGlTransform*
agl_identity_transform_apply (AGlTransform* transform, AGlTransform* apply_to)
{
	/* We do the following to make sure inverting a non-NULL transform
	 * will return a non-NULL transform.
	 */
	if (apply_to)
		return apply_to;
	else
		return agl_transform_new ();
}

static AGlTransform *
agl_identity_transform_invert (AGlTransform* transform, AGlTransform* next)
{
	/* We do the following to make sure inverting a non-NULL transform
	 * will return a non-NULL transform.
	 */
	if (next)
		return next;
	else
		return agl_transform_new ();
}

static gboolean
agl_identity_transform_equal (AGlTransform* first_transform, AGlTransform* second_transform)
{
	return TRUE;
}

static const AGlTransformClass AGL_IDENTITY_TRANSFORM_CLASS = {
	sizeof (AGlTransform),
	"AGlIdentityMatrix",
	agl_identity_transform_finalize,
	agl_identity_transform_to_matrix,
	agl_identity_transform_apply_2d,
	agl_identity_transform_apply_affine,
	agl_identity_transform_apply_translate,
	agl_identity_transform_print,
	agl_identity_transform_apply,
	agl_identity_transform_invert,
	agl_identity_transform_equal,
};

/*<private>
 * agl_transform_is_identity:
 * @transform: (allow-none): A transform or %NULL
 *
 * Checks if the transform is a representation of the identity
 * transform.
 *
 * This is different from a transform like `scale(2) scale(0.5)`
 * which just results in an identity transform when simplified.
 *
 * Returns: %TRUE  if this transform is a representation of
 *     the identity transform
 **/
static bool
agl_transform_is_identity (AGlTransform* self)
{
  return self == NULL ||
         (self->transform_class == &AGL_IDENTITY_TRANSFORM_CLASS && agl_transform_is_identity (self->next));
}

#if 0
/*** MATRIX ***/

typedef struct _AGlMatrixTransform AGlMatrixTransform;

struct _AGlMatrixTransform
{
  AGlTransform parent;

  graphene_matrix_t matrix;
};

static void
agl_matrix_transform_finalize (AGlTransform *self)
{
}

static void
agl_matrix_transform_to_matrix (AGlTransform      *transform,
                                graphene_matrix_t *out_matrix)
{
  AGlMatrixTransform *self = (AGlMatrixTransform *) transform;

  graphene_matrix_init_from_matrix (out_matrix, &self->matrix);
}

static void
agl_matrix_transform_apply_2d (AGlTransform *transform,
                               float        *out_xx,
                               float        *out_yx,
                               float        *out_xy,
                               float        *out_yy,
                               float        *out_dx,
                               float        *out_dy)
{
  AGlMatrixTransform *self = (AGlMatrixTransform *) transform;
  graphene_matrix_t mat;

  graphene_matrix_init_from_2d (&mat,
                                *out_xx, *out_yx,
                                *out_xy, *out_yy,
                                *out_dx, *out_dy);
  graphene_matrix_multiply (&self->matrix, &mat, &mat);

  /* not using graphene_matrix_to_2d() because it may
   * fail the is_2d() check due to improper rounding */
  *out_xx = graphene_matrix_get_value (&mat, 0, 0);
  *out_yx = graphene_matrix_get_value (&mat, 0, 1);
  *out_xy = graphene_matrix_get_value (&mat, 1, 0);
  *out_yy = graphene_matrix_get_value (&mat, 1, 1);
  *out_dx = graphene_matrix_get_value (&mat, 3, 0);
  *out_dy = graphene_matrix_get_value (&mat, 3, 1);
}

static void
agl_matrix_transform_apply_affine (AGlTransform *transform,
                                   float        *out_scale_x,
                                   float        *out_scale_y,
                                   float        *out_dx,
                                   float        *out_dy)
{
  AGlMatrixTransform *self = (AGlMatrixTransform *) transform;

  switch (transform->category)
  {
    case AGL_TRANSFORM_CATEGORY_UNKNOWN:
    case AGL_TRANSFORM_CATEGORY_ANY:
    case AGL_TRANSFORM_CATEGORY_3D:
    case AGL_TRANSFORM_CATEGORY_2D:
    default:
      g_assert_not_reached ();
      break;

    case AGL_TRANSFORM_CATEGORY_2D_AFFINE:
      *out_dx += *out_scale_x * graphene_matrix_get_x_translation (&self->matrix);
      *out_dy += *out_scale_y * graphene_matrix_get_y_translation (&self->matrix);
      *out_scale_x *= graphene_matrix_get_x_scale (&self->matrix);
      *out_scale_y *= graphene_matrix_get_y_scale (&self->matrix);
      break;

    case AGL_TRANSFORM_CATEGORY_2D_TRANSLATE:
      *out_dx += *out_scale_x * graphene_matrix_get_x_translation (&self->matrix);
      *out_dy += *out_scale_y * graphene_matrix_get_y_translation (&self->matrix);
      break;

    case AGL_TRANSFORM_CATEGORY_IDENTITY:
      break;
  }
}

static void
agl_matrix_transform_apply_translate (AGlTransform *transform,
                                      float        *out_dx,
                                      float        *out_dy)
{
  AGlMatrixTransform *self = (AGlMatrixTransform *) transform;

  switch (transform->category)
  {
    case AGL_TRANSFORM_CATEGORY_UNKNOWN:
    case AGL_TRANSFORM_CATEGORY_ANY:
    case AGL_TRANSFORM_CATEGORY_3D:
    case AGL_TRANSFORM_CATEGORY_2D:
    case AGL_TRANSFORM_CATEGORY_2D_AFFINE:
    default:
      g_assert_not_reached ();
      break;

    case AGL_TRANSFORM_CATEGORY_2D_TRANSLATE:
      *out_dx += graphene_matrix_get_x_translation (&self->matrix);
      *out_dy += graphene_matrix_get_y_translation (&self->matrix);
      break;

    case AGL_TRANSFORM_CATEGORY_IDENTITY:
      break;
  }
}
#endif

static void
string_append_double (GString *string,
                      double   d)
{
  char buf[G_ASCII_DTOSTR_BUF_SIZE];

  g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%g", d);
  g_string_append (string, buf);
}


#if 0
static void
agl_matrix_transform_print (AGlTransform *transform,
                            GString      *string)
{
  AGlMatrixTransform *self = (AGlMatrixTransform *) transform;
  guint i;
  float f[16];

  g_string_append (string, "matrix3d(");
  graphene_matrix_to_float (&self->matrix, f);
  for (i = 0; i < 16; i++)
    {
      if (i > 0)
        g_string_append (string, ", ");
      string_append_double (string, f[i]);
    }
  g_string_append (string, ")");
}

static AGlTransform *
agl_matrix_transform_apply (AGlTransform *transform,
                            AGlTransform *apply_to)
{
  AGlMatrixTransform *self = (AGlMatrixTransform *) transform;

  return agl_transform_matrix_with_category (apply_to,
                                             &self->matrix,
                                             transform->category);
}

static AGlTransform *
agl_matrix_transform_invert (AGlTransform *transform,
                             AGlTransform *next)
{
  AGlMatrixTransform *self = (AGlMatrixTransform *) transform;
  graphene_matrix_t inverse;

  if (!graphene_matrix_inverse (&self->matrix, &inverse))
    {
      agl_transform_unref (next);
      return NULL;
    }

  return agl_transform_matrix_with_category (next,
                                             &inverse,
                                             transform->category);
}

static gboolean
agl_matrix_transform_equal (AGlTransform *first_transform,
                            AGlTransform *second_transform)
{
  AGlMatrixTransform *first = (AGlMatrixTransform *) first_transform;
  AGlMatrixTransform *second = (AGlMatrixTransform *) second_transform;

  if (graphene_matrix_equal_fast (&first->matrix, &second->matrix))
    return TRUE;

  return graphene_matrix_equal (&first->matrix, &second->matrix);
}
#endif

#if 0
static const AGlTransformClass AGL_TRANSFORM_TRANSFORM_CLASS =
{
  sizeof (AGlMatrixTransform),
  "AGlMatrixTransform",
  agl_matrix_transform_finalize,
  agl_matrix_transform_to_matrix,
  agl_matrix_transform_apply_2d,
  agl_matrix_transform_apply_affine,
  agl_matrix_transform_apply_translate,
  agl_matrix_transform_print,
  agl_matrix_transform_apply,
  agl_matrix_transform_invert,
  agl_matrix_transform_equal,
};

AGlTransform *
agl_transform_matrix_with_category (AGlTransform* next, const graphene_matrix_t* matrix, AGlTransformCategory category)
{
	AGlMatrixTransform *result = agl_transform_alloc (&AGL_TRANSFORM_TRANSFORM_CLASS, category, next);

	graphene_matrix_init_from_matrix (&result->matrix, matrix);

	return &result->parent;
}

/**
 * agl_transform_matrix:
 * @next: (allow-none) (transfer full): the next transform
 * @matrix: the matrix to multiply @next with
 *
 * Multiplies @next with the given @matrix.
 *
 * Returns: The new matrix
 **/
AGlTransform *
agl_transform_matrix (AGlTransform            *next,
                      const graphene_matrix_t *matrix)
{
	return agl_transform_matrix_with_category (next, matrix, AGL_TRANSFORM_CATEGORY_UNKNOWN);
}
#endif

/*** TRANSLATE ***/

typedef struct
{
   AGlTransform parent;
   graphene_point3d_t point;

} AGlTranslateTransform;


static void
agl_translate_transform_finalize (AGlTransform *self)
{
}

static void
agl_translate_transform_to_matrix (AGlTransform* transform, graphene_matrix_t* out_matrix)
{
	AGlTranslateTransform* self = (AGlTranslateTransform*)transform;

	graphene_matrix_init_translate (out_matrix, &self->point);
}

static void
agl_translate_transform_apply_2d (AGlTransform *transform,
                                  float        *out_xx,
                                  float        *out_yx,
                                  float        *out_xy,
                                  float        *out_yy,
                                  float        *out_dx,
                                  float        *out_dy)
{
  AGlTranslateTransform *self = (AGlTranslateTransform *) transform;

  g_assert (self->point.z == 0.0);

  *out_dx += *out_xx * self->point.x + *out_xy * self->point.y;
  *out_dy += *out_yx * self->point.x + *out_yy * self->point.y;
}

static void
agl_translate_transform_apply_affine (AGlTransform *transform,
                                      float        *out_scale_x,
                                      float        *out_scale_y,
                                      float        *out_dx,
                                      float        *out_dy)
{
  AGlTranslateTransform *self = (AGlTranslateTransform *) transform;

  g_assert (self->point.z == 0.0);

  *out_dx += *out_scale_x * self->point.x;
  *out_dy += *out_scale_y * self->point.y;
}

static void
agl_translate_transform_apply_translate (AGlTransform *transform,
                                         float        *out_dx,
                                         float        *out_dy)
{
  AGlTranslateTransform *self = (AGlTranslateTransform *) transform;

  g_assert (self->point.z == 0.0);

  *out_dx += self->point.x;
  *out_dy += self->point.y;
}

static AGlTransform *
agl_translate_transform_apply (AGlTransform *transform,
                               AGlTransform *apply_to)
{
  AGlTranslateTransform *self = (AGlTranslateTransform *) transform;

  return agl_transform_translate_3d (apply_to, &self->point);
}

static AGlTransform *
agl_translate_transform_invert (AGlTransform *transform,
                                AGlTransform *next)
{
  AGlTranslateTransform *self = (AGlTranslateTransform *) transform;

  return agl_transform_translate_3d (next, &GRAPHENE_POINT3D_INIT (-self->point.x, -self->point.y, -self->point.z));
}


static gboolean
agl_translate_transform_equal (AGlTransform* first_transform, AGlTransform* second_transform)
{
	AGlTranslateTransform *first = (AGlTranslateTransform *) first_transform;
	AGlTranslateTransform *second = (AGlTranslateTransform *) second_transform;

	return graphene_point3d_equal (&first->point, &second->point);
}


static void
agl_translate_transform_print (AGlTransform* transform, GString* string)
{
	AGlTranslateTransform *self = (AGlTranslateTransform *) transform;

	if (self->point.z == 0)
		g_string_append (string, "translate(");
	else
		g_string_append (string, "translate3d(");

	string_append_double (string, self->point.x);
	g_string_append (string, ", ");
	string_append_double (string, self->point.y);
	if (self->point.z != 0) {
		g_string_append (string, ", ");
		string_append_double (string, self->point.z);
	}
	g_string_append (string, ")");
}


static const AGlTransformClass AGL_TRANSLATE_TRANSFORM_CLASS = {
	sizeof (AGlTranslateTransform),
	"AGlTranslateTransform",
	agl_translate_transform_finalize,
	agl_translate_transform_to_matrix,
	agl_translate_transform_apply_2d,
	agl_translate_transform_apply_affine,
	agl_translate_transform_apply_translate,
	agl_translate_transform_print,
	agl_translate_transform_apply,
	agl_translate_transform_invert,
	agl_translate_transform_equal,
};


/**
 * agl_transform_translate:
 * @next: (allow-none) (transfer full): the next transform
 * @point: the point to translate the matrix by
 *
 * Translates @next in 2dimensional space by @point.
 *
 * Returns: The new matrix
 **/
AGlTransform*
agl_transform_translate (AGlTransform* next, const graphene_point_t* point)
{
	graphene_point3d_t point3d;

	graphene_point3d_init (&point3d, point->x, point->y, 0);

	return agl_transform_translate_3d (next, &point3d);
}


/**
 * agl_transform_translate_3d:
 * @next: (allow-none) (transfer full): the next transform
 * @point: the point to translate the matrix by
 *
 * Translates @next by @point.
 *
 * Returns: The new matrix
 **/
AGlTransform*
agl_transform_translate_3d (AGlTransform* next, const graphene_point3d_t* point)
{
	AGlTranslateTransform* result;

	if (graphene_point3d_equal (point, graphene_point3d_zero ()))
		return next;

	if (agl_transform_has_class (next, &AGL_TRANSLATE_TRANSFORM_CLASS)) {
		AGlTranslateTransform* t = (AGlTranslateTransform *) next;
		AGlTransform* r = agl_transform_translate_3d (
			agl_transform_ref (next->next),
			&GRAPHENE_POINT3D_INIT(t->point.x + point->x, t->point.y + point->y, t->point.z + point->z)
		);
		agl_transform_unref (next);
		return r;
	}

	result = agl_transform_alloc (
		&AGL_TRANSLATE_TRANSFORM_CLASS,
		point->z == 0.0
			? AGL_TRANSFORM_CATEGORY_2D_TRANSLATE
			: AGL_TRANSFORM_CATEGORY_3D,
		next
	);

	graphene_point3d_init_from_point (&result->point, point);

	return &result->parent;
}

/*** ROTATE ***/

#if 0
typedef struct _AGlRotateTransform AGlRotateTransform;

struct _AGlRotateTransform
{
  AGlTransform parent;

  float angle;
};

static void
agl_rotate_transform_finalize (AGlTransform *self)
{
}

static inline void
_sincos (float deg, float* out_s, float* out_c)
{
  if (deg == 90.0)
    {
      *out_c = 0.0;
      *out_s = 1.0;
    }
  else if (deg == 180.0)
    {
      *out_c = -1.0;
      *out_s = 0.0;
    }
  else if (deg == 270.0)
    {
      *out_c = 0.0;
      *out_s = -1.0;
    }
  else if (deg == 0.0)
    {
      *out_c = 1.0;
      *out_s = 0.0;
    }
  else
    {
      float angle = deg * M_PI / 180.0;

#ifdef HAVE_SINCOSF
      sincosf (angle, out_s, out_c);
#else
      *out_s = sinf (angle);
      *out_c = cosf (angle);
#endif

    }
}


static void
agl_rotate_transform_to_matrix (AGlTransform* transform, graphene_matrix_t* out_matrix)
{
	AGlRotateTransform *self = (AGlRotateTransform *) transform;
	float c, s;

	_sincos (self->angle, &s, &c);

	graphene_matrix_init_from_2d (out_matrix, c, s, -s, c, 0, 0);
}


static void
agl_rotate_transform_apply_2d (AGlTransform* transform, float* out_xx, float* out_yx, float* out_xy, float* out_yy, float* out_dx, float* out_dy)
{
	AGlRotateTransform *self = (AGlRotateTransform *) transform;
	float s, c, xx, xy, yx, yy;

	if (fmodf (self->angle, 360.0f) == 0.0)
		return;

	_sincos (self->angle, &s, &c);

	xx =  c * *out_xx + s * *out_xy;
	yx =  c * *out_yx + s * *out_yy;
	xy = -s * *out_xx + c * *out_xy;
	yy = -s * *out_yx + c * *out_yy;

	*out_xx = xx;
	*out_yx = yx;
	*out_xy = xy;
	*out_yy = yy;
}


static AGlTransform *
agl_rotate_transform_apply (AGlTransform* transform, AGlTransform* apply_to)
{
	AGlRotateTransform *self = (AGlRotateTransform *) transform;

	return agl_transform_rotate (apply_to, self->angle);
}

static AGlTransform *
agl_rotate_transform_invert (AGlTransform* transform, AGlTransform* next)
{
	AGlRotateTransform *self = (AGlRotateTransform *) transform;

	return agl_transform_rotate (next, - self->angle);
}

static gboolean
agl_rotate_transform_equal (AGlTransform* first_transform, AGlTransform* second_transform)
{
	AGlRotateTransform *first = (AGlRotateTransform *) first_transform;
	AGlRotateTransform *second = (AGlRotateTransform *) second_transform;

	return G_APPROX_VALUE (first->angle, second->angle, 0.01f);
}

static void
agl_rotate_transform_print (AGlTransform *transform,
                            GString      *string)
{
  AGlRotateTransform *self = (AGlRotateTransform *) transform;

  g_string_append (string, "rotate(");
  string_append_double (string, self->angle);
  g_string_append (string, ")");
}

static const AGlTransformClass AGL_ROTATE_TRANSFORM_CLASS =
{
  sizeof (AGlRotateTransform),
  "AGlRotateTransform",
  agl_rotate_transform_finalize,
  agl_rotate_transform_to_matrix,
  agl_rotate_transform_apply_2d,
  NULL,
  NULL,
  agl_rotate_transform_print,
  agl_rotate_transform_apply,
  agl_rotate_transform_invert,
  agl_rotate_transform_equal,
};

static inline float
normalize_angle (float angle)
{
  float f;

  if (angle >= 0 && angle < 360)
    return angle;

  f = angle - (360 * ((int)(angle / 360.0)));

  if (f < 0)
    f = 360 + f;

  return f;
}

/**
 * agl_transform_rotate:
 * @next: (allow-none) (transfer full): the next transform
 * @angle: the rotation angle, in degrees (clockwise)
 *
 * Rotates @next @angle degrees in 2D - or in 3Dspeak, around the z axis.
 *
 * Returns: The new matrix
 **/
AGlTransform *
agl_transform_rotate (AGlTransform *next,
                      float         angle)
{
  AGlRotateTransform *result;

  if (angle == 0.0f)
    return next;

  if (agl_transform_has_class (next, &AGL_ROTATE_TRANSFORM_CLASS))
    {
      AGlTransform *r  = agl_transform_rotate (agl_transform_ref (next->next),
                                               ((AGlRotateTransform *) next)->angle + angle);
      agl_transform_unref (next);
      return r;
    }

  result = agl_transform_alloc (&AGL_ROTATE_TRANSFORM_CLASS,
                                AGL_TRANSFORM_CATEGORY_2D,
                                next);

  result->angle = normalize_angle (angle);

  return &result->parent;
}

/*** ROTATE 3D ***/

typedef struct _AGlRotate3dTransform AGlRotate3dTransform;

struct _AGlRotate3dTransform
{
  AGlTransform parent;

  float angle;
  graphene_vec3_t axis;
};

static void
agl_rotate3d_transform_finalize (AGlTransform *self)
{
}

static void
agl_rotate3d_transform_to_matrix (AGlTransform      *transform,
                                  graphene_matrix_t *out_matrix)
{
  AGlRotate3dTransform *self = (AGlRotate3dTransform *) transform;

  graphene_matrix_init_rotate (out_matrix, self->angle, &self->axis);
}

static AGlTransform *
agl_rotate3d_transform_apply (AGlTransform *transform,
                              AGlTransform *apply_to)
{
  AGlRotate3dTransform *self = (AGlRotate3dTransform *) transform;

  return agl_transform_rotate_3d (apply_to, self->angle, &self->axis);
}

static AGlTransform *
agl_rotate3d_transform_invert (AGlTransform *transform,
                               AGlTransform *next)
{
  AGlRotate3dTransform *self = (AGlRotate3dTransform *) transform;

  return agl_transform_rotate_3d (next, - self->angle, &self->axis);
}

static gboolean
agl_rotate3d_transform_equal (AGlTransform *first_transform,
                              AGlTransform *second_transform)
{
  AGlRotate3dTransform *first = (AGlRotate3dTransform *) first_transform;
  AGlRotate3dTransform *second = (AGlRotate3dTransform *) second_transform;

  return G_APPROX_VALUE (first->angle, second->angle, 0.01f) &&
         graphene_vec3_equal (&first->axis, &second->axis);
}

static void
agl_rotate3d_transform_print (AGlTransform *transform,
                            GString      *string)
{
  AGlRotate3dTransform *self = (AGlRotate3dTransform *) transform;
  float f[3];
  guint i;

  g_string_append (string, "rotate3d(");
  graphene_vec3_to_float (&self->axis, f);
  for (i = 0; i < 3; i++)
    {
      string_append_double (string, f[i]);
      g_string_append (string, ", ");
    }
  string_append_double (string, self->angle);
  g_string_append (string, ")");
}

static const AGlTransformClass AGL_ROTATE3D_TRANSFORM_CLASS =
{
  sizeof (AGlRotate3dTransform),
  "AGlRotate3dTransform",
  agl_rotate3d_transform_finalize,
  agl_rotate3d_transform_to_matrix,
  NULL,
  NULL,
  NULL,
  agl_rotate3d_transform_print,
  agl_rotate3d_transform_apply,
  agl_rotate3d_transform_invert,
  agl_rotate3d_transform_equal,
};

/**
 * agl_transform_rotate_3d:
 * @next: (allow-none) (transfer full): the next transform
 * @angle: the rotation angle, in degrees (clockwise)
 * @axis: The rotation axis
 *
 * Rotates @next @angle degrees around @axis.
 *
 * For a rotation in 2D space, use agl_transform_rotate().
 *
 * Returns: The new matrix
 **/
AGlTransform *
agl_transform_rotate_3d (AGlTransform          *next,
                         float                  angle,
                         const graphene_vec3_t *axis)
{
  AGlRotate3dTransform *result;

  if (graphene_vec3_get_x (axis) == 0.0 && graphene_vec3_get_y (axis) == 0.0)
    return agl_transform_rotate (next, angle);

  if (angle == 0.0f)
    return next;

  result = agl_transform_alloc (&AGL_ROTATE3D_TRANSFORM_CLASS,
                                AGL_TRANSFORM_CATEGORY_3D,
                                next);

  result->angle = normalize_angle (angle);
  graphene_vec3_init_from_vec3 (&result->axis, axis);

  return &result->parent;
}
#endif


/*** SCALE ***/

typedef struct _AGlScaleTransform AGlScaleTransform;

struct _AGlScaleTransform
{
  AGlTransform parent;

  float factor_x;
  float factor_y;
  float factor_z;
};


static void
agl_scale_transform_finalize (AGlTransform *self)
{
}


static void
agl_scale_transform_to_matrix (AGlTransform* transform, graphene_matrix_t* out_matrix)
{
  AGlScaleTransform *self = (AGlScaleTransform *) transform;

  graphene_matrix_init_scale (out_matrix, self->factor_x, self->factor_y, self->factor_z);
}

static void
agl_scale_transform_apply_2d (AGlTransform *transform,
                              float        *out_xx,
                              float        *out_yx,
                              float        *out_xy,
                              float        *out_yy,
                              float        *out_dx,
                              float        *out_dy)
{
  AGlScaleTransform *self = (AGlScaleTransform *) transform;

  g_assert (self->factor_z == 1.0);

  *out_xx *= self->factor_x;
  *out_yx *= self->factor_x;
  *out_xy *= self->factor_y;
  *out_yy *= self->factor_y;
}

static void
agl_scale_transform_apply_affine (AGlTransform *transform,
                                  float        *out_scale_x,
                                  float        *out_scale_y,
                                  float        *out_dx,
                                  float        *out_dy)
{
  AGlScaleTransform *self = (AGlScaleTransform *) transform;

  g_assert (self->factor_z == 1.0);

  *out_scale_x *= self->factor_x;
  *out_scale_y *= self->factor_y;
}

static AGlTransform *
agl_scale_transform_apply (AGlTransform *transform,
                           AGlTransform *apply_to)
{
  AGlScaleTransform *self = (AGlScaleTransform *) transform;

  return agl_transform_scale_3d (apply_to, self->factor_x, self->factor_y, self->factor_z);
}

static AGlTransform *
agl_scale_transform_invert (AGlTransform *transform,
                            AGlTransform *next)
{
  AGlScaleTransform *self = (AGlScaleTransform *) transform;

  return agl_transform_scale_3d (next,
                                 1.f / self->factor_x,
                                 1.f / self->factor_y,
                                 1.f / self->factor_z);
}

static gboolean
agl_scale_transform_equal (AGlTransform *first_transform,
                           AGlTransform *second_transform)
{
  AGlScaleTransform *first = (AGlScaleTransform *) first_transform;
  AGlScaleTransform *second = (AGlScaleTransform *) second_transform;

  return G_APPROX_VALUE (first->factor_x, second->factor_x, FLT_EPSILON) &&
         G_APPROX_VALUE (first->factor_y, second->factor_y, FLT_EPSILON) &&
         G_APPROX_VALUE (first->factor_z, second->factor_z, FLT_EPSILON);
}

static void
agl_scale_transform_print (AGlTransform *transform,
                           GString      *string)
{
  AGlScaleTransform *self = (AGlScaleTransform *) transform;

  if (self->factor_z == 1.0)
    {
      g_string_append (string, "scale(");
      string_append_double (string, self->factor_x);
      if (self->factor_x != self->factor_y)
        {
          g_string_append (string, ", ");
          string_append_double (string, self->factor_y);
        }
      g_string_append (string, ")");
    }
  else
    {
      g_string_append (string, "scale3d(");
      string_append_double (string, self->factor_x);
      g_string_append (string, ", ");
      string_append_double (string, self->factor_y);
      g_string_append (string, ", ");
      string_append_double (string, self->factor_z);
      g_string_append (string, ")");
    }
}


static const AGlTransformClass AGL_SCALE_TRANSFORM_CLASS = {
	sizeof (AGlScaleTransform),
	"AGlScaleTransform",
	agl_scale_transform_finalize,
	agl_scale_transform_to_matrix,
	agl_scale_transform_apply_2d,
	agl_scale_transform_apply_affine,
	NULL,
	agl_scale_transform_print,
	agl_scale_transform_apply,
	agl_scale_transform_invert,
	agl_scale_transform_equal,
};

/**
 * agl_transform_scale:
 * @next: (allow-none) (transfer full): the next transform
 * @factor_x: scaling factor on the X axis
 * @factor_y: scaling factor on the Y axis
 *
 * Scales @next in 2-dimensional space by the given factors.
 * Use agl_transform_scale_3d() to scale in all 3 dimensions.
 *
 * Returns: The new matrix
 **/
AGlTransform *
agl_transform_scale (AGlTransform* next, float factor_x, float factor_y)
{
	return agl_transform_scale_3d (next, factor_x, factor_y, 1.0);
}


/**
 * agl_transform_scale_3d:
 * @next: (allow-none) (transfer full): the next transform
 * @factor_x: scaling factor on the X axis
 * @factor_y: scaling factor on the Y axis
 * @factor_z: scaling factor on the Z axis
 *
 * Scales @next by the given factors.
 *
 * Returns: The new matrix
 **/
AGlTransform *
agl_transform_scale_3d (AGlTransform* next, float factor_x, float factor_y, float factor_z)
{
	if (factor_x == 1 && factor_y == 1 && factor_z == 1)
		return next;

	if (agl_transform_has_class (next, &AGL_SCALE_TRANSFORM_CLASS)) {
		AGlScaleTransform *scale = (AGlScaleTransform *) next;
		AGlTransform *r = agl_transform_scale_3d (agl_transform_ref (next->next),
			scale->factor_x * factor_x,
			scale->factor_y * factor_y,
			scale->factor_z * factor_z
		);
		agl_transform_unref (next);
		return r;
	}

	AGlScaleTransform* result = agl_transform_alloc (
		&AGL_SCALE_TRANSFORM_CLASS,
		factor_z != 1.0
			? AGL_TRANSFORM_CATEGORY_3D
			: AGL_TRANSFORM_CATEGORY_2D_AFFINE,
		next
	);

	result->factor_x = factor_x;
	result->factor_y = factor_y;
	result->factor_z = factor_z;

	return &result->parent;
}


#if 0
/*** PERSPECTIVE ***/

typedef struct _AGlPerspectiveTransform AGlPerspectiveTransform;

struct _AGlPerspectiveTransform
{
  AGlTransform parent;

  float depth;
};

static void
agl_perspective_transform_finalize (AGlTransform *self)
{
}

static void
agl_perspective_transform_to_matrix (AGlTransform      *transform,
                                     graphene_matrix_t *out_matrix)
{
  AGlPerspectiveTransform *self = (AGlPerspectiveTransform *) transform;
  float f[16] = { 1.f, 0.f, 0.f,  0.f,
                  0.f, 1.f, 0.f,  0.f,
                  0.f, 0.f, 1.f, self->depth ? -1.f / self->depth : 0.f,
                  0.f, 0.f, 0.f,  1.f };

  graphene_matrix_init_from_float (out_matrix, f);
}


static AGlTransform *
agl_perspective_transform_apply (AGlTransform *transform,
                                 AGlTransform *apply_to)
{
  AGlPerspectiveTransform *self = (AGlPerspectiveTransform *) transform;

  return agl_transform_perspective (apply_to, self->depth);
}

static AGlTransform *
agl_perspective_transform_invert (AGlTransform *transform,
                                  AGlTransform *next)
{
  AGlPerspectiveTransform *self = (AGlPerspectiveTransform *) transform;

  return agl_transform_perspective (next, - self->depth);
}

static gboolean
agl_perspective_transform_equal (AGlTransform *first_transform,
                                 AGlTransform *second_transform)
{
  AGlPerspectiveTransform *first = (AGlPerspectiveTransform *) first_transform;
  AGlPerspectiveTransform *second = (AGlPerspectiveTransform *) second_transform;

  return G_APPROX_VALUE (first->depth, second->depth, 0.001f);
}

static void
agl_perspective_transform_print (AGlTransform *transform,
                                 GString      *string)
{
  AGlPerspectiveTransform *self = (AGlPerspectiveTransform *) transform;

  g_string_append (string, "perspective(");
  string_append_double (string, self->depth);
  g_string_append (string, ")");
}

static const AGlTransformClass AGL_PERSPECTIVE_TRANSFORM_CLASS = {
	sizeof (AGlPerspectiveTransform),
	"AGlPerspectiveTransform",
	agl_perspective_transform_finalize,
	agl_perspective_transform_to_matrix,
	NULL,
	NULL,
	NULL,
	agl_perspective_transform_print,
	agl_perspective_transform_apply,
	agl_perspective_transform_invert,
	agl_perspective_transform_equal,
};

/**
 * agl_transform_perspective:
 * @next: (allow-none) (transfer full): the next transform
 * @depth: distance of the z=0 plane. Lower values give a more
 *     flattened pyramid and therefore a more pronounced
 *     perspective effect.
 *
 * Applies a perspective projection transform. This transform
 * scales points in X and Y based on their Z value, scaling
 * points with positive Z values away from the origin, and
 * those with negative Z values towards the origin. Points
 * on the z=0 plane are unchanged.
 *
 * Returns: The new matrix
 **/
AGlTransform *
agl_transform_perspective (AGlTransform* next, float depth)
{
	AGlPerspectiveTransform *result;

	if (agl_transform_has_class (next, &AGL_PERSPECTIVE_TRANSFORM_CLASS)) {
		AGlTransform *r = agl_transform_perspective(
			agl_transform_ref (next->next),
			((AGlPerspectiveTransform *) next)->depth + depth
		);
		agl_transform_unref (next);
		return r;
	}

	result = agl_transform_alloc (&AGL_PERSPECTIVE_TRANSFORM_CLASS, AGL_TRANSFORM_CATEGORY_ANY, next);

	result->depth = depth;

	return &result->parent;
}
#endif


/*** PUBLIC API ***/

static void
agl_transform_finalize (AGlTransform* self)
{
	self->transform_class->finalize (self);

	agl_transform_unref (self->next);
}


/**
 * agl_transform_ref:
 * @self: (allow-none): a #AGlTransform
 *
 * Acquires a reference on the given #AGlTransform.
 *
 * Returns: (transfer none): the #AGlTransform with an additional reference
 */
AGlTransform*
agl_transform_ref (AGlTransform *self)
{
	if (self == NULL)
		return NULL;

#ifdef HAVE_GLIB_2_58
	return g_atomic_rc_box_acquire (self);
#else
	return self;
#endif
}


/**
 * agl_transform_unref:
 * @self: (allow-none): a #AGlTransform
 *
 * Releases a reference on the given #AGlTransform.
 *
 * If the reference was the last, the resources associated to the @self are
 * freed.
 */
void
agl_transform_unref (AGlTransform *self)
{
	if (self == NULL)
		return;

#ifdef HAVE_GLIB_2_58
	g_atomic_rc_box_release_full (self, (GDestroyNotify) agl_transform_finalize);
#else
	agl_transform_finalize(self);
	g_free(self);
#endif
}


/**
 * agl_transform_print:
 * @self: (allow-none): a #AGlTransform
 * @string:  The string to print into
 *
 * Converts @self into a human-readable string representation suitable
 * for printing that can later be parsed with agl_transform_parse().
 **/
void
agl_transform_print (AGlTransform* self, GString* string)
{
	g_return_if_fail (string != NULL);

	if (self == NULL) {
		g_string_append (string, "none");
		return;
	}

	if (self->next != NULL) {
		agl_transform_print (self->next, string);
		g_string_append (string, " ");
	}

	self->transform_class->print (self, string);
}

/**
 * agl_transform_to_string:
 * @self: (allow-none): a #AGlTransform
 *
 * Converts a matrix into a string that is suitable for
 * printing and can later be parsed with agl_transform_parse().
 *
 * This is a wrapper around agl_transform_print(), see that function
 * for details.
 *
 * Returns: A new string for @self
 **/
char *
agl_transform_to_string (AGlTransform *self)
{
  GString *string;

  string = g_string_new ("");

  agl_transform_print (self, string);

  return g_string_free (string, FALSE);
}


/**
 * agl_transform_to_matrix:
 * @self: (allow-none): a #AGlTransform
 * @out_matrix: (out caller-allocates): The matrix to set
 *
 * Computes the actual value of @self and stores it in @out_matrix.
 * The previous value of @out_matrix will be ignored.
 **/
void
agl_transform_to_matrix (AGlTransform      *self, graphene_matrix_t *out_matrix)
{
  graphene_matrix_t m;

  if (self == NULL)
    {
      graphene_matrix_init_identity (out_matrix);
      return;
    }

  agl_transform_to_matrix (self->next, out_matrix);
  self->transform_class->to_matrix (self, &m);
  graphene_matrix_multiply (&m, out_matrix, out_matrix);
}


#if 0
/**
 * agl_transform_to_2d:
 * @self: a 2D #AGlTransform
 * @out_xx: (out): return location for the xx member
 * @out_yx: (out): return location for the yx member
 * @out_xy: (out): return location for the xy member
 * @out_yy: (out): return location for the yy member
 * @out_dx: (out): return location for the x0 member
 * @out_dy: (out): return location for the y0 member
 *
 * Converts a #AGlTransform to a 2D transformation
 * matrix.
 * @self must be a 2D transformation. If you are not
 * sure, use agl_transform_get_category() >=
 * %AGL_TRANSFORM_CATEGORY_2D to check.
 *
 * The returned values have the following layout:
 *
 * |[<!-- language="plain" -->
 *   | xx yx |   |  a  b  0 |
 *   | xy yy | = |  c  d  0 |
 *   | dx dy |   | tx ty  1 |
 * ]|
 *
 * This function can be used to convert between a #AGlTransform
 * and a matrix type from other 2D drawing libraries, in particular
 * Cairo.
 */
void
agl_transform_to_2d (AGlTransform *self,
                     float        *out_xx,
                     float        *out_yx,
                     float        *out_xy,
                     float        *out_yy,
                     float        *out_dx,
                     float        *out_dy)
{
  if (self == NULL ||
      self->category < AGL_TRANSFORM_CATEGORY_2D)
    {
      if (self != NULL)
        {
          char *s = agl_transform_to_string (self);
          g_warning ("Given transform \"%s\" is not a 2D transform.", s);
          g_free (s);
        }
      *out_xx = 1.0f;
      *out_yx = 0.0f;
      *out_xy = 0.0f;
      *out_yy = 1.0f;
      *out_dx = 0.0f;
      *out_dy = 0.0f;
      return;
    }

  agl_transform_to_2d (self->next,
                       out_xx, out_yx,
                       out_xy, out_yy,
                       out_dx, out_dy);

  self->transform_class->apply_2d (self,
                                   out_xx, out_yx,
                                   out_xy, out_yy,
                                   out_dx, out_dy);
}
#endif


/**
 * agl_transform_to_affine:
 * @self: a #AGlTransform
 * @out_scale_x: (out): return location for the scale
 *     factor in the x direction
 * @out_scale_y: (out): return location for the scale
 *     factor in the y direction
 * @out_dx: (out): return location for the translation
 *     in the x direction
 * @out_dy: (out): return location for the translation
 *     in the y direction
 *
 * Converts a #AGlTransform to 2D affine transformation
 * factors.
 * @self must be a 2D transformation. If you are not
 * sure, use agl_transform_get_category() >= 
 * %AGL_TRANSFORM_CATEGORY_2D_AFFINE to check.
 */
void
agl_transform_to_affine (AGlTransform *self,
                         float        *out_scale_x,
                         float        *out_scale_y,
                         float        *out_dx,
                         float        *out_dy)
{
  if (self == NULL ||
      self->category < AGL_TRANSFORM_CATEGORY_2D_AFFINE)
    {
      if (self != NULL)
        {
          char *s = agl_transform_to_string (self);
          g_warning ("Given transform \"%s\" is not an affine 2D transform.", s);
          g_free (s);
        }
      *out_scale_x = 1.0f;
      *out_scale_y = 1.0f;
      *out_dx = 0.0f;
      *out_dy = 0.0f;
      return;
    }

  agl_transform_to_affine (self->next, out_scale_x, out_scale_y, out_dx, out_dy);

  self->transform_class->apply_affine (self, out_scale_x, out_scale_y, out_dx, out_dy);
}


/**
 * agl_transform_to_translate:
 * @self: a #AGlTransform
 * @out_dx: (out): return location for the translation
 *     in the x direction
 * @out_dy: (out): return location for the translation
 *     in the y direction
 *
 * Converts a #AGlTransform to a translation operation.
 * @self must be a 2D transformation. If you are not
 * sure, use agl_transform_get_category() >= 
 * %AGL_TRANSFORM_CATEGORY_2D_TRANSLATE to check.
 */
void
agl_transform_to_translate (AGlTransform* self, float* out_dx, float* out_dy)
{
	if (self == NULL || self->category < AGL_TRANSFORM_CATEGORY_2D_TRANSLATE) {
		if (self != NULL) {
			char* s = agl_transform_to_string (self);
			g_warning ("Given transform \"%s\" is not a 2D translation.", s);
			g_free (s);
		}
		*out_dx = 0.0f;
		*out_dy = 0.0f;
		return;
	}

	agl_transform_to_translate (self->next, out_dx, out_dy);

	self->transform_class->apply_translate (self, out_dx, out_dy);
}


/**
 * agl_transform_transform:
 * @next: (allow-none) (transfer full): Transform to apply @other to
 * @other: (allow-none):  Transform to apply
 *
 * Applies all the operations from @other to @next. 
 *
 * Returns: The new matrix
 **/
AGlTransform *
agl_transform_transform (AGlTransform* next, AGlTransform* other)
{
	if (other == NULL)
		return next;

	next = agl_transform_transform (next, other->next);
	return other->transform_class->apply (other, next);
}


#if 0
/**
 * agl_transform_invert:
 * @self: (allow-none) (transfer full): Transform to invert
 *
 * Inverts the given transform.
 *
 * If @self is not invertible, %NULL is returned.
 * Note that inverting %NULL also returns %NULL, which is
 * the correct inverse of %NULL. If you need to differentiate
 * between those cases, you should check @self is not %NULL
 * before calling this function.
 *
 * Returns: The inverted transform or %NULL if the transform
 *     cannot be inverted.
 **/
AGlTransform *
agl_transform_invert (AGlTransform *self)
{
  AGlTransform *result = NULL;
  AGlTransform *cur;

  for (cur = self; cur; cur = cur->next)
    {
      result = cur->transform_class->invert (cur, result);
      if (result == NULL)
        break;
    }

  agl_transform_unref (self);

  return result;
}
#endif


/**
 * agl_transform_equal:
 * @first: the first matrix
 * @second: the second matrix
 *
 * Checks two matrices for equality. Note that matrices need to be literally
 * identical in their operations, it is not enough that they return the
 * same result in agl_transform_to_matrix().
 *
 * Returns: %TRUE if the two matrices can be proven to be equal
 **/
gboolean
agl_transform_equal (AGlTransform* first, AGlTransform* second)
{
	if (first == second)
		return TRUE;

	if (first == NULL || second == NULL)
		return FALSE;

	if (first->transform_class != second->transform_class)
		return FALSE;

	if (!agl_transform_equal (first->next, second->next))
		return FALSE;

	return first->transform_class->equal (first, second);
}


/**
 * agl_transform_get_category:
 * @self: (allow-none): A #AGlTransform
 *
 * Returns the category this transform belongs to.
 *
 * Returns: The category of the transform
 **/
AGlTransformCategory
agl_transform_get_category (AGlTransform *self)
{
	if (self == NULL)
		return AGL_TRANSFORM_CATEGORY_IDENTITY;

	return self->category;
}


/*
 * agl_transform_new: (constructor):
 *
 * Creates a new identity matrix. This function is meant to be used by language
 * bindings. For C code, this equivalent to using %NULL.
 *
 * Returns: A new identity matrix
 **/
AGlTransform *
agl_transform_new (void)
{
  return agl_transform_alloc (&AGL_IDENTITY_TRANSFORM_CLASS, AGL_TRANSFORM_CATEGORY_IDENTITY, NULL);
}


/**
 * agl_transform_transform_bounds:
 * @self: a #AGlTransform
 * @rect: a #graphene_rect_t
 * @out_rect: (out caller-allocates): return location for the bounds
 *   of the transformed rectangle
 *
 * Transforms a #graphene_rect_t using the given transform @self.
 * The result is the bounding box containing the coplanar quad.
 **/
void
agl_transform_transform_bounds (AGlTransform* self, const graphene_rect_t* rect, graphene_rect_t* out_rect)
{
	switch (agl_transform_get_category (self)) {
		case AGL_TRANSFORM_CATEGORY_IDENTITY:
			graphene_rect_init_from_rect (out_rect, rect);
			break;

		case AGL_TRANSFORM_CATEGORY_2D_TRANSLATE: {
			float dx, dy;

			agl_transform_to_translate (self, &dx, &dy);
			out_rect->origin.x = rect->origin.x + dx;
			out_rect->origin.y = rect->origin.y + dy;
			out_rect->size.width = rect->size.width;
			out_rect->size.height = rect->size.height;
			}
			break;

		case AGL_TRANSFORM_CATEGORY_2D_AFFINE: {
			float dx, dy, scale_x, scale_y;

			agl_transform_to_affine (self, &scale_x, &scale_y, &dx, &dy);

			out_rect->origin.x = (rect->origin.x * scale_x) + dx;
			out_rect->origin.y = (rect->origin.y * scale_y) + dy;
			out_rect->size.width = rect->size.width * scale_x;
			out_rect->size.height = rect->size.height * scale_y;
			}
			break;

		case AGL_TRANSFORM_CATEGORY_UNKNOWN:
		case AGL_TRANSFORM_CATEGORY_ANY:
		case AGL_TRANSFORM_CATEGORY_3D:
		case AGL_TRANSFORM_CATEGORY_2D:
		default: {
			graphene_matrix_t mat;

			agl_transform_to_matrix (self, &mat);
			graphene_matrix_transform_bounds (&mat, rect, out_rect);
			}
			break;
	}
}


/**
 * agl_transform_transform_point:
 * @self: a #AGlTransform
 * @point: a #graphene_point_t
 * @out_point: (out caller-allocates): return location for
 *   the transformed point
 *
 * Transforms a #graphene_point_t using the given transform @self.
 */
#if 0
void
agl_transform_transform_point (AGlTransform* self, const graphene_point_t* point, graphene_point_t* out_point)
{
  switch (agl_transform_get_category (self))
    {
    case AGL_TRANSFORM_CATEGORY_IDENTITY:
      *out_point = *point;
      break;

    case AGL_TRANSFORM_CATEGORY_2D_TRANSLATE:
      {
        float dx, dy;

        agl_transform_to_translate (self, &dx, &dy);
        out_point->x = point->x + dx;
        out_point->y = point->y + dy;
      }
    break;

    case AGL_TRANSFORM_CATEGORY_2D_AFFINE:
      {
        float dx, dy, scale_x, scale_y;

        agl_transform_to_affine (self, &scale_x, &scale_y, &dx, &dy);

        out_point->x = (point->x * scale_x) + dx;
        out_point->y = (point->y * scale_y) + dy;
      }
    break;

    case AGL_TRANSFORM_CATEGORY_UNKNOWN:
    case AGL_TRANSFORM_CATEGORY_ANY:
    case AGL_TRANSFORM_CATEGORY_3D:
    case AGL_TRANSFORM_CATEGORY_2D:
    default:
      {
        graphene_matrix_t mat;

        agl_transform_to_matrix (self, &mat);
        graphene_matrix_transform_point (&mat, point, out_point);
      }
      break;
    }
}


static guint
agl_transform_parse_float (GtkCssParser* parser, guint n, gpointer data)
{
  float *f = data;
  double d;

  if (!gtk_css_parser_consume_number (parser, &d))
    return 0;

  f[n] = d;
  return 1;
}


static guint
agl_transform_parse_scale (GtkCssParser* parser, guint n, gpointer data)
{
  float *f = data;
  double d;

  if (!gtk_css_parser_consume_number (parser, &d))
    return 0;

  f[n] = d;
  f[1] = d;
  return 1;
}


gboolean
agl_transform_parser_parse (GtkCssParser* parser, AGlTransform** out_transform)
{
  const GtkCssToken *token;
  AGlTransform *transform = NULL;
  float f[16] = { 0, };
  gboolean parsed_something = FALSE;

  token = gtk_css_parser_get_token (parser);
  if (gtk_css_token_is_ident (token, "none"))
    {
      gtk_css_parser_consume_token (parser);
      *out_transform = NULL;
      return TRUE;
    }

  while (TRUE)
    {
      if (gtk_css_token_is_function (token, "matrix"))
        {
          graphene_matrix_t matrix;
          if (!gtk_css_parser_consume_function (parser, 6, 6, agl_transform_parse_float, f))
            goto fail;

          graphene_matrix_init_from_2d (&matrix, f[0], f[1], f[2], f[3], f[4], f[5]);
          transform = agl_transform_matrix_with_category (transform, &matrix, AGL_TRANSFORM_CATEGORY_2D);
        }
      else if (gtk_css_token_is_function (token, "matrix3d"))
        {
          graphene_matrix_t matrix;
          if (!gtk_css_parser_consume_function (parser, 16, 16, agl_transform_parse_float, f))
            goto fail;

          graphene_matrix_init_from_float (&matrix, f);
          transform = agl_transform_matrix (transform, &matrix);
        }
      else if (gtk_css_token_is_function (token, "perspective"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_perspective (transform, f[0]);
        }
      else if (gtk_css_token_is_function (token, "rotate") ||
               gtk_css_token_is_function (token, "rotateZ"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_rotate (transform, f[0]);
        }
      else if (gtk_css_token_is_function (token, "rotate3d"))
        {
          graphene_vec3_t axis;

          if (!gtk_css_parser_consume_function (parser, 4, 4, agl_transform_parse_float, f))
            goto fail;

          graphene_vec3_init (&axis, f[0], f[1], f[2]);
          transform = agl_transform_rotate_3d (transform, f[3], &axis);
        }
      else if (gtk_css_token_is_function (token, "rotateX"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_rotate_3d (transform, f[0], graphene_vec3_x_axis ());
        }
      else if (gtk_css_token_is_function (token, "rotateY"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_rotate_3d (transform, f[0], graphene_vec3_y_axis ());
        }
      else if (gtk_css_token_is_function (token, "scale"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 2, agl_transform_parse_scale, f))
            goto fail;

          transform = agl_transform_scale (transform, f[0], f[1]);
        }
      else if (gtk_css_token_is_function (token, "scale3d"))
        {
          if (!gtk_css_parser_consume_function (parser, 3, 3, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_scale_3d (transform, f[0], f[1], f[2]);
        }
      else if (gtk_css_token_is_function (token, "scaleX"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_scale (transform, f[0], 1.f);
        }
      else if (gtk_css_token_is_function (token, "scaleY"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_scale (transform, 1.f, f[0]);
        }
      else if (gtk_css_token_is_function (token, "scaleZ"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_scale_3d (transform, 1.f, 1.f, f[0]);
        }
      else if (gtk_css_token_is_function (token, "translate"))
        {
          f[1] = 0.f;
          if (!gtk_css_parser_consume_function (parser, 1, 2, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_translate (transform, &GRAPHENE_POINT_INIT (f[0], f[1]));
        }
      else if (gtk_css_token_is_function (token, "translate3d"))
        {
          if (!gtk_css_parser_consume_function (parser, 3, 3, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_translate_3d (transform, &GRAPHENE_POINT3D_INIT (f[0], f[1], f[2]));
        }
      else if (gtk_css_token_is_function (token, "translateX"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_translate (transform, &GRAPHENE_POINT_INIT (f[0], 0.f));
        }
      else if (gtk_css_token_is_function (token, "translateY"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_translate (transform, &GRAPHENE_POINT_INIT (0.f, f[0]));
        }
      else if (gtk_css_token_is_function (token, "translateZ"))
        {
          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          transform = agl_transform_translate_3d (transform, &GRAPHENE_POINT3D_INIT (0.f, 0.f, f[0]));
        }
      else if (gtk_css_token_is_function (token, "skew"))
        {
          graphene_matrix_t matrix;

          if (!gtk_css_parser_consume_function (parser, 2, 2, agl_transform_parse_float, f))
            goto fail;

          f[0] = f[0] / 180.0 * G_PI;
          f[1] = f[1] / 180.0 * G_PI;

          graphene_matrix_init_skew (&matrix, f[0], f[1]);
          transform = agl_transform_matrix (transform, &matrix);
        }
      else if (gtk_css_token_is_function (token, "skewX"))
        {
          graphene_matrix_t matrix;

          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          f[0] = f[0] / 180.0 * G_PI;

          graphene_matrix_init_skew (&matrix, f[0], 0);
          transform = agl_transform_matrix (transform, &matrix);
        }
      else if (gtk_css_token_is_function (token, "skewY"))
        {
          graphene_matrix_t matrix;

          if (!gtk_css_parser_consume_function (parser, 1, 1, agl_transform_parse_float, f))
            goto fail;

          f[0] = f[0] / 180.0 * G_PI;

          graphene_matrix_init_skew (&matrix, 0, f[0]);
          transform = agl_transform_matrix (transform, &matrix);
        }
      else
        {
          break;
        }

      parsed_something = TRUE;
      token = gtk_css_parser_get_token (parser);
    }

  if (!parsed_something)
    {
      gtk_css_parser_error_syntax (parser, "Expected a transform");
      goto fail;
    }

  *out_transform = transform;
  return TRUE;

fail:
  agl_transform_unref (transform);
  *out_transform = NULL;
  return FALSE;
}

/**
 * agl_transform_parse:
 * @string: the string to parse
 * @out_transform: (out): The location to put the transform in
 *
 * Parses the given @string into a transform and puts it in
 * @out_transform. Strings printed via agl_transform_to_string()
 * can be read in again successfully using this function.
 *
 * If @string does not describe a valid transform, %FALSE is
 * returned and %NULL is put in @out_transform.
 *
 * Returns: %TRUE if @string described a valid transform.
 **/
gboolean
agl_transform_parse (const char* string, AGlTransform** out_transform)
{
  GtkCssParser *parser;
  GBytes *bytes;
  gboolean result;

  g_return_val_if_fail (string != NULL, FALSE);
  g_return_val_if_fail (out_transform != NULL, FALSE);

  bytes = g_bytes_new_static (string, strlen (string));
  parser = gtk_css_parser_new_for_bytes (bytes, NULL, NULL, NULL, NULL, NULL);

  result = agl_transform_parser_parse (parser, out_transform);

  if (result && !gtk_css_parser_has_token (parser, GTK_CSS_TOKEN_EOF))
    {
      g_clear_pointer (out_transform, agl_transform_unref);
      result = FALSE;
    }
  gtk_css_parser_unref (parser);
  g_bytes_unref (bytes);

  return result; 
}
#endif
