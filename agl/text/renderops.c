/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) GTK+ Team and others                                   |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "agl/debug.h"
#include "text/renderops.h"
#include "text/gdkrgba.h"

typedef struct
{
   float scale_x;
   float scale_y;

   float dx_before;
   float dy_before;

} OpsMatrixMetadata;

typedef struct
{
   AGlTransform*     transform;
   OpsMatrixMetadata metadata;

} MatrixStackEntry;

static gpointer ops_begin (RenderOpBuilder*, OpKind);


static inline gboolean
rect_equal (const graphene_rect_t* a, const graphene_rect_t* b)
{
	return memcmp (a, b, sizeof (graphene_rect_t)) == 0;
}


static inline bool
rounded_rect_equal (const AGlRoundedRect* r1, const AGlRoundedRect* r2)
{
	if (!r1)
		return FALSE;

	if (r1->bounds.origin.x != r2->bounds.origin.x ||
		r1->bounds.origin.y != r2->bounds.origin.y ||
		r1->bounds.size.width != r2->bounds.size.width ||
		r1->bounds.size.height != r2->bounds.size.height)
			return FALSE;

	for (int i = 0; i < 4; i ++)
		if (r1->corner[i].width != r2->corner[i].width ||
			r1->corner[i].height != r2->corner[i].height)
				return FALSE;

	return TRUE;
}


static inline bool
rounded_rect_corners_equal (const AGlRoundedRect* r1, const AGlRoundedRect* r2)
{
	if (!r1)
		return FALSE;

	for (int i = 0; i < 4; i ++)
		if (r1->corner[i].width != r2->corner[i].width ||
			r1->corner[i].height != r2->corner[i].height)
				return FALSE;

	return TRUE;
}


#define pstate() (builder->programs.state)
#define pcurrent() (builder->programs.current)

static inline ProgramState*
get_current_program_state (RenderOpBuilder* builder)
{
	if (!pcurrent())
		return NULL;

	return &pstate()[pcurrent()->index];
}


/*
 *   Reset everything in the builder except programs.state
 *   (not sure this is safe)
 */
void
ops_finish (RenderOpBuilder* builder)
{
	dbg(2, "resetting builder...");

	if (builder->modelview.stack)
		g_array_free (builder->modelview.stack, TRUE);
	builder->modelview.stack = NULL;
	builder->modelview.current = NULL;

	if (builder->clip.stack)
		g_array_free (builder->clip.stack, TRUE);
	builder->clip.stack = NULL;
	builder->clip.current = NULL;

	builder->dx = 0;
	builder->dy = 0;
	builder->scale_x = 1;
	builder->scale_y = 1;
	builder->current_texture = 0;
	builder->programs.current = NULL;
	graphene_matrix_init_identity (&builder->current_projection);
	builder->current_viewport = GRAPHENE_RECT_INIT (0, 0, 0, 0);
}


#ifdef DEBUG
void
ops_dump_framebuffer (RenderOpBuilder* builder, const char* filename, int width, int height)
{
	OpDumpFrameBuffer *op;

	op = ops_begin (builder, OP_DUMP_FRAMEBUFFER);
	op->filename = g_strdup (filename);
	op->width = width;
	op->height = height;
}


void
ops_push_debug_group (RenderOpBuilder* builder, const char* text)
{
	OpDebugGroup* op = ops_begin (builder, OP_PUSH_DEBUG_GROUP);
	strncpy (op->text, text, sizeof(op->text) - 1);
	op->text[sizeof(op->text) - 1] = 0; /* Ensure zero terminated */
}


void
ops_pop_debug_group (RenderOpBuilder* builder)
{
	ops_begin (builder, OP_POP_DEBUG_GROUP);
}
#endif


float
ops_get_scale (const RenderOpBuilder *builder)
{
#if 1
	g_assert (builder->modelview.stack);
	g_assert (builder->modelview.stack->len);
#else
	if(!builder->modelview.stack) dbg(0, "warning modelview.stack not set");
#endif

	return MAX (builder->scale_x, builder->scale_y);
}


static void
extract_matrix_metadata (AGlTransform* transform, OpsMatrixMetadata* md)
{
	float dummy;

	switch (agl_transform_get_category (transform)) {
		case AGL_TRANSFORM_CATEGORY_IDENTITY:
		case AGL_TRANSFORM_CATEGORY_2D_TRANSLATE:
			md->scale_x = 1;
			md->scale_y = 1;
			break;

		case AGL_TRANSFORM_CATEGORY_2D_AFFINE:
			agl_transform_to_affine (transform, &md->scale_x, &md->scale_y, &dummy, &dummy);
			break;

		case AGL_TRANSFORM_CATEGORY_UNKNOWN:
		case AGL_TRANSFORM_CATEGORY_ANY:
		case AGL_TRANSFORM_CATEGORY_3D:
		case AGL_TRANSFORM_CATEGORY_2D:
			{
				graphene_vec3_t col1;
				graphene_vec3_t col2;
				graphene_matrix_t m;

				agl_transform_to_matrix (transform, &m);

				/* TODO: 90% sure this is incorrect. But we should never hit this code
				 * path anyway. */
				graphene_vec3_init (&col1,
					graphene_matrix_get_value (&m, 0, 0),
					graphene_matrix_get_value (&m, 1, 0),
					graphene_matrix_get_value (&m, 2, 0)
				);

				graphene_vec3_init (&col2,
					graphene_matrix_get_value (&m, 0, 1),
					graphene_matrix_get_value (&m, 1, 1),
					graphene_matrix_get_value (&m, 2, 1)
				);

				md->scale_x = graphene_vec3_length (&col1);
				md->scale_y = graphene_vec3_length (&col2);
			}
			break;
		default:
			{}
	}
}


void
ops_transform_bounds_modelview (const RenderOpBuilder* builder, const graphene_rect_t* src, graphene_rect_t* dst)
{
	graphene_rect_t r = *src;

	g_assert (builder->modelview.stack);
	g_assert (builder->modelview.stack->len);

	r.origin.x += builder->dx;
	r.origin.y += builder->dy;

	agl_transform_transform_bounds (builder->modelview.current, &r, dst);
}


void
ops_init (RenderOpBuilder* builder)
{
	PF2;

	if(builder->vertices){
		g_array_unref (builder->vertices);
		op_buffer_destroy (&builder->render_ops);
	}

	*builder = (RenderOpBuilder){
		.current_opacity = 1.0f,
		.vertices = g_array_new (FALSE, TRUE, sizeof(GskQuadVertex))
	};

	op_buffer_init (&builder->render_ops);

	for (int i = 0; i < GL_N_PROGRAMS; i ++) {
		builder->programs.state[i].opacity = 1.0f;
	}
}


/*
 *  ops_free is only called when a builder is destroyed
 */
void
ops_free (RenderOpBuilder* builder)
{
	PF;

	for (int i = 0; i < GL_N_PROGRAMS; i ++) {
		agl_transform_unref (builder->programs.state[i].modelview);
	}

	g_clear_pointer(&builder->vertices, g_array_unref);

	if(builder->modelview.stack){
		dbg(1, "matrixstack: TODO why not freed in ops_finish? len=%i", builder->modelview.stack->len);
		for(int i=0;i<builder->modelview.stack->len;i++){
			MatrixStackEntry* entry = &g_array_index (builder->modelview.stack, MatrixStackEntry, i);
			if(entry->transform){
				agl_transform_unref(entry->transform);
			}
		}
		g_array_free (builder->modelview.stack, TRUE);
		builder->modelview.stack = NULL;
	}

	op_buffer_destroy (&builder->render_ops);
}


void
ops_set_program (RenderOpBuilder* builder, const Program* program)
{
	if (builder->programs.current == program)
		return;

	OpProgram* op = ops_begin (builder, OP_CHANGE_PROGRAM);
	op->program = program;

	builder->programs.current = program;

	ProgramState* program_state = &builder->programs.state[program->index];

	if (memcmp (&builder->current_projection, &program_state->projection, sizeof (graphene_matrix_t)) != 0) {
		OpMatrix* opm = ops_begin (builder, OP_CHANGE_PROJECTION);
		opm->matrix = builder->current_projection;
		program_state->projection = builder->current_projection;
	}

	if (program_state->modelview == NULL || !agl_transform_equal (builder->modelview.current, program_state->modelview)) {
		OpMatrix* opm = ops_begin (builder, OP_CHANGE_MODELVIEW);
		agl_transform_to_matrix (builder->modelview.current, &opm->matrix);
		agl_transform_unref (program_state->modelview);
		program_state->modelview = agl_transform_ref (builder->modelview.current);
	}

	if (!rect_equal (&builder->current_viewport, &program_state->viewport)) {
		OpViewport* opv = ops_begin (builder, OP_CHANGE_VIEWPORT);
		opv->viewport = builder->current_viewport;
		program_state->viewport = builder->current_viewport;
	}

	if (!rounded_rect_equal (builder->clip.current, &program_state->clip)) {
		OpClip* opc = ops_begin (builder, OP_CHANGE_CLIP);
		opc->clip = *builder->clip.current;
		opc->send_corners = !rounded_rect_corners_equal (builder->clip.current, &program_state->clip);
		program_state->clip = *builder->clip.current;
	}

	if (program_state->opacity != builder->current_opacity) {
		OpOpacity* opo = ops_begin (builder, OP_CHANGE_OPACITY);
		opo->opacity = builder->current_opacity;
		program_state->opacity = builder->current_opacity;
	}
}


static void
ops_set_clip (RenderOpBuilder* builder, const AGlRoundedRect* clip)
{
	ProgramState* current_program_state = get_current_program_state (builder);

	if (current_program_state && rounded_rect_equal (&current_program_state->clip, clip))
		return;

	OpClip* op;
	if (!(op = op_buffer_peek_tail_checked (&builder->render_ops, OP_CHANGE_CLIP))) {
		op = op_buffer_add (&builder->render_ops, OP_CHANGE_CLIP);
		op->send_corners = !current_program_state || !rounded_rect_corners_equal (&current_program_state->clip, clip);
	} else {
		/* If the op before sent the corners, this one needs, too */
		op->send_corners |= !current_program_state || !rounded_rect_corners_equal (&current_program_state->clip, clip);
	}

	op->clip = *clip;

	if (current_program_state)
		current_program_state->clip = *clip;
}


void
ops_push_clip (RenderOpBuilder* self, const AGlRoundedRect* clip)
{
	if (G_UNLIKELY (!self->clip.stack))
		self->clip.stack = g_array_new (FALSE, TRUE, sizeof(AGlRoundedRect));

	g_assert (self->clip.stack);
#ifdef DEBUG
	int len = self->clip.stack->len;
#endif

	g_array_append_val (self->clip.stack, *clip);
	self->clip.current = &g_array_index (self->clip.stack, AGlRoundedRect, self->clip.stack->len - 1);
	ops_set_clip (self, clip);

	dbg(2, "%i-->%i %.0f,%0.f w=%.0f h=%.0f", len, self->clip.stack->len, self->clip.current->bounds.origin.x, self->clip.current->bounds.origin.y, self->clip.current->bounds.size.width, self->clip.current->bounds.size.height);
}


void
ops_pop_clip (RenderOpBuilder* self)
{
	g_assert (self->clip.stack);
	g_assert (self->clip.stack->len);
#ifdef DEBUG
	int len = self->clip.stack->len;
#endif

	self->clip.stack->len --;
	const AGlRoundedRect* head = &g_array_index (self->clip.stack, AGlRoundedRect, self->clip.stack->len - 1);

	if (self->clip.stack->len) {
		self->clip.current = head;
		ops_set_clip (self, head);
	} else {
		self->clip.current = NULL;
	}

	dbg(2, "%i-->%i %p", len, self->clip.stack->len, self->clip.stack);
}


bool
ops_has_clip (RenderOpBuilder *self)
{
	return self->clip.stack != NULL && self->clip.stack->len > 1;
}


static void
ops_set_modelview_internal (RenderOpBuilder* builder, AGlTransform* transform)
{
	ProgramState* current_program_state = get_current_program_state (builder);

#if 0
  XXX This is not possible if we want pop() to work.
  if (builder->current_program && agl_transform_equal (builder->current_program_state->modelview, transform))
    return;
#endif

	OpMatrix* op;
	if (!(op = op_buffer_peek_tail_checked (&builder->render_ops, OP_CHANGE_MODELVIEW)))
		op = op_buffer_add (&builder->render_ops, OP_CHANGE_MODELVIEW);

	agl_transform_to_matrix (transform, &op->matrix);

	g_return_if_fail(!isnan(graphene_matrix_get_value (&op->matrix, 3, 0)));

	if (builder->programs.current) {
		agl_transform_unref (current_program_state->modelview);
		current_program_state->modelview = agl_transform_ref (transform);
	}
}


/**
 * ops_set_modelview:
 * @builder
 * @transform: (transfer full): The new modelview transform
 *
 * This sets the modelview to the given one without looking at the
 * one that's currently set
 */
void
ops_set_modelview (RenderOpBuilder* builder, AGlTransform* transform)
{
	if (G_UNLIKELY (!builder->modelview.stack))
		builder->modelview.stack = g_array_new (FALSE, TRUE, sizeof(MatrixStackEntry));

	g_assert (builder->modelview.stack);

	g_array_set_size (builder->modelview.stack, builder->modelview.stack->len + 1);
	MatrixStackEntry* entry = &g_array_index (builder->modelview.stack, MatrixStackEntry, builder->modelview.stack->len - 1);

	entry->transform = transform;

	entry->metadata.dx_before = builder->dx;
	entry->metadata.dy_before = builder->dy;
	extract_matrix_metadata (entry->transform, &entry->metadata);

	builder->dx = 0;
	builder->dy = 0;
	builder->modelview.current = entry->transform;
	builder->scale_x = entry->metadata.scale_x;
	builder->scale_y = entry->metadata.scale_y;

	ops_set_modelview_internal (builder, entry->transform);
}


/*  Set the given modelview to the one we get when multiplying
 *  the given modelview with the current one.
 */
void
ops_push_modelview (RenderOpBuilder* builder, AGlTransform* transform)
{
	if (G_UNLIKELY (builder->modelview.stack == NULL))
		builder->modelview.stack = g_array_new (FALSE, TRUE, sizeof (MatrixStackEntry));

	g_assert (builder->modelview.stack != NULL);

	g_array_set_size (builder->modelview.stack, builder->modelview.stack->len + 1);
	MatrixStackEntry* entry = &g_array_index (builder->modelview.stack, MatrixStackEntry, builder->modelview.stack->len - 1);

	if (G_LIKELY (builder->modelview.stack->len >= 2)) {
		const MatrixStackEntry* cur = &g_array_index (builder->modelview.stack, MatrixStackEntry, builder->modelview.stack->len - 2);

		/* Multiply given matrix with current modelview */
		AGlTransform* t = NULL;
		t = agl_transform_translate (agl_transform_ref (cur->transform), &(graphene_point_t) { builder->dx, builder->dy});
		t = agl_transform_transform (t, transform);
		entry->transform = t;
	} else {
		entry->transform = agl_transform_ref (transform);
	}

	entry->metadata.dx_before = builder->dx;
	entry->metadata.dy_before = builder->dy;
	extract_matrix_metadata (entry->transform, &entry->metadata);

	builder->dx = 0;
	builder->dy = 0;
	builder->scale_x = entry->metadata.scale_x;
	builder->scale_y = entry->metadata.scale_y;
	builder->modelview.current = entry->transform;

	ops_set_modelview_internal (builder, entry->transform);
}


void
ops_pop_modelview (RenderOpBuilder* builder)
{
	g_assert (builder->modelview.stack);
	g_assert (builder->modelview.stack->len);

	const MatrixStackEntry* head = &g_array_index (builder->modelview.stack, MatrixStackEntry, builder->modelview.stack->len - 1);
	builder->dx = head->metadata.dx_before;
	builder->dy = head->metadata.dy_before;
	agl_transform_unref (head->transform);

	builder->modelview.stack->len --;
	head = &g_array_index (builder->modelview.stack, MatrixStackEntry, builder->modelview.stack->len - 1);

	if (builder->modelview.stack->len >= 1) {
		builder->scale_x = head->metadata.scale_x;
		builder->scale_y = head->metadata.scale_y;
		builder->modelview.current = head->transform;
		ops_set_modelview_internal (builder, head->transform);
	} else {
		builder->modelview.current = NULL;
	}
}


graphene_matrix_t
ops_set_projection (RenderOpBuilder* builder, const graphene_matrix_t* projection)
{
	ProgramState* current_program_state = get_current_program_state (builder);

	OpMatrix* op;
	if (!(op = op_buffer_peek_tail_checked (&builder->render_ops, OP_CHANGE_PROJECTION)))
		op = op_buffer_add (&builder->render_ops, OP_CHANGE_PROJECTION);

	op->matrix = *projection;

	if (builder->programs.current)
		current_program_state->projection = *projection;

	graphene_matrix_t prev_mv = builder->current_projection;
	builder->current_projection = *projection;

	return prev_mv;
}


graphene_rect_t
ops_set_viewport (RenderOpBuilder* builder, const graphene_rect_t* viewport)
{
	ProgramState* current_program_state = get_current_program_state (builder);

	if (current_program_state != NULL && rect_equal (&current_program_state->viewport, viewport))
		return current_program_state->viewport;

	OpViewport* op = ops_begin (builder, OP_CHANGE_VIEWPORT);
	op->viewport = *viewport;

	if (builder->programs.current != NULL)
		current_program_state->viewport = *viewport;

	graphene_rect_t prev_viewport = builder->current_viewport;
	builder->current_viewport = *viewport;

	return prev_viewport;
}


void
ops_set_texture (RenderOpBuilder* builder, int texture_id)
{
	if (builder->current_texture == texture_id)
		return;

	OpTexture* op = ops_begin (builder, OP_CHANGE_SOURCE_TEXTURE);
	op->texture_id = texture_id;
	builder->current_texture = texture_id;
}


float
ops_set_opacity (RenderOpBuilder* builder, float opacity)
{
	ProgramState* current_program_state = get_current_program_state (builder);

	if (builder->current_opacity == opacity)
		return opacity;

	OpOpacity* op;
	if (!(op = op_buffer_peek_tail_checked (&builder->render_ops, OP_CHANGE_OPACITY)))
		op = op_buffer_add (&builder->render_ops, OP_CHANGE_OPACITY);

	op->opacity = opacity;

	float prev_opacity = builder->current_opacity;
	builder->current_opacity = opacity;

	if (builder->programs.current != NULL)
		current_program_state->opacity = opacity;

	return prev_opacity;
}


/*
 *  The color is now copied so it does not have to remain valid after calling this fn
 */
void
ops_set_color (RenderOpBuilder* builder, const GdkRGBA* color)
{
	ProgramState* current_program_state = get_current_program_state (builder);

	if (gdk_rgba_equal (color, &current_program_state->color))
		return;

	current_program_state->color = *color;

	OpColor* op = ops_begin (builder, OP_CHANGE_COLOR);
	op->rgba = *color;
}


#if 0
void
ops_set_color_matrix (RenderOpBuilder* builder, const graphene_matrix_t* matrix, const graphene_vec4_t* offset)
{
  ProgramState *current_program_state = get_current_program_state (builder);
  OpColorMatrix *op;

  if (memcmp (matrix,
              &current_program_state->color_matrix.matrix,
              sizeof (graphene_matrix_t)) == 0 &&
      memcmp (offset,
              &current_program_state->color_matrix.offset,
              sizeof (graphene_vec4_t)) == 0)
    return;

  current_program_state->color_matrix.matrix = *matrix;
  current_program_state->color_matrix.offset = *offset;

  op = ops_begin (builder, OP_CHANGE_COLOR_MATRIX);
  op->matrix = matrix;
  op->offset = offset;
}


void
ops_set_border (RenderOpBuilder* builder, const AGlRoundedRect* outline)
{
	ProgramState *current_program_state = get_current_program_state (builder);
	OpBorder *op;

	if (memcmp (&current_program_state->border.outline, outline, sizeof (AGlRoundedRect)) == 0)
		return;

	current_program_state->border.outline = *outline;

	op = ops_begin (builder, OP_CHANGE_BORDER);
	op->outline = *outline;
}


void
ops_set_border_width (RenderOpBuilder* builder, const float* widths)
{
	ProgramState *current_program_state = get_current_program_state (builder);
	OpBorder *op;

	if (memcmp (current_program_state->border.widths, widths, sizeof (float) * 4) == 0)
		return;

	memcpy (&current_program_state->border.widths, widths, sizeof (float) * 4);

	op = ops_begin (builder, OP_CHANGE_BORDER_WIDTH);
	op->widths[0] = widths[0];
	op->widths[1] = widths[1];
	op->widths[2] = widths[2];
	op->widths[3] = widths[3];
}


void
ops_set_border_color (RenderOpBuilder* builder, const GdkRGBA* color)
{
  ProgramState *current_program_state = get_current_program_state (builder);
  OpBorder *op;

  if (gdk_rgba_equal (color, &current_program_state->border.color))
    return;

  op = op_buffer_add (&builder->render_ops, OP_CHANGE_BORDER_COLOR);
  op->color = color;

  current_program_state->border.color = *color;
}
#endif


GskQuadVertex *
ops_draw (RenderOpBuilder* builder, const GskQuadVertex vertex_data[GL_N_VERTICES])
{
	OpDraw* op;

	if ((op = op_buffer_peek_tail_checked (&builder->render_ops, AGL_OP_DRAW))) {
		op->vao_size += GL_N_VERTICES;
	} else {
		op = op_buffer_add (&builder->render_ops, AGL_OP_DRAW);
		op->vao_offset = builder->vertices->len;
		op->vao_size = GL_N_VERTICES;
	}

	if (vertex_data) {
		g_array_append_vals (builder->vertices, vertex_data, GL_N_VERTICES);
		return NULL; /* Better not use this on the caller side */
	}

	g_array_set_size (builder->vertices, builder->vertices->len + GL_N_VERTICES);
	return &g_array_index (builder->vertices, GskQuadVertex, builder->vertices->len - GL_N_VERTICES);
}


#if 0
/* The offset is only valid for the current modelview.
 * Setting a new modelview will add the offset to that matrix
 * and reset the internal offset to 0. */
void
ops_offset (RenderOpBuilder* builder, float x, float y)
{
	builder->dx += x;
	builder->dy += y;
}
#endif


static gpointer
ops_begin (RenderOpBuilder* builder, OpKind kind)
{
	return op_buffer_add (&builder->render_ops, kind);
}


void
ops_reset (RenderOpBuilder* builder)
{
	PF2;

	op_buffer_clear (&builder->render_ops);
	g_array_set_size (builder->vertices, 0);
}


OpBuffer*
ops_get_buffer (RenderOpBuilder* builder)
{
	return &builder->render_ops;
}
