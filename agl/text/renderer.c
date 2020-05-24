/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | Copyright (C) 2020-2020 Tim Orford <tim@orford.org>                  |
* | Copyright (C) GTK+ Team and others                                   |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "config.h"
#include "agl/debug.h"
#include "agl/utils.h"
#include "text/enums.h"
#include "text/renderer.h"
#include "text/glyphcache.h"
#include "text/opbuffer.h"
#include "text/shaderbuilder.h"
#include "text/profiler.h"
#include "text/roundedrect.h"

#include <agl/ext.h>

extern GResource* gresource_get_resource (void);

#define SHADER_VERSION_GLES        100
#define SHADER_VERSION_GL2_LEGACY  110
#define SHADER_VERSION_GL3_LEGACY  130
#define SHADER_VERSION_GL3         150

#define ORTHO_NEAR_PLANE        -10000
#define ORTHO_FAR_PLANE          10000

#define DEBUG_OPS          0

#define SHADOW_EXTRA_SIZE  4

#if DEBUG_OPS
#define OP_PRINT(format, ...) g_print(format, ## __VA_ARGS__)
#else
#define OP_PRINT(format, ...)
#endif

Renderer renderer = {0,};


typedef enum
{
  FORCE_OFFSCREEN  = 1 << 0,
  RESET_CLIP       = 1 << 1,
  RESET_OPACITY    = 1 << 2,
  DUMP_FRAMEBUFFER = 1 << 3,
  CENTER_CHILD     = 1 << 4,
  NO_CACHE_PLZ     = 1 << 5,
} OffscreenFlags;

typedef struct
{
  int texture_id;
  float x;
  float y;
  float x2;
  float y2;
} TextureRegion;


static inline void
init_full_texture_region (TextureRegion* r, int texture_id)
{
	r->texture_id = texture_id;
	r->x = 0;
	r->y = 0;
	r->x2 = 1;
	r->y2 = 1;
}


static void G_GNUC_UNUSED
dump_framebuffer (const char* filename, int w, int h)
{
	int stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, w);
	guchar* data = g_malloc (h * stride);

	glReadPixels (0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, data);
	cairo_surface_t* s = cairo_image_surface_create_for_data (data, CAIRO_FORMAT_ARGB32, w, h, stride);
	cairo_surface_write_to_png (s, filename);

	cairo_surface_destroy (s);
	g_free (data);
}


static void
renderer_clear_tree (RenderOpBuilder* builder)
{
	//gdk_gl_context_make_current (self->gl_context);

	ops_reset (builder);
	//int removed_textures =
		driver_collect_textures ();

	//dbg2(1, OPENGL, "collected: %d textures", removed_textures);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


static inline void
apply_viewport_op (const Program* program, const OpViewport* op)
{
	OP_PRINT (" -> New Viewport: %.0f, %.0f, %.0f, %.0f",
		op->viewport.origin.x, op->viewport.origin.y,
		op->viewport.size.width, op->viewport.size.height
	);
	glUniform4f (program->viewport_location,
		op->viewport.origin.x, op->viewport.origin.y,
		op->viewport.size.width, op->viewport.size.height
	);
	glViewport (0, 0, op->viewport.size.width, op->viewport.size.height);
}


static inline void
apply_modelview_op (const Program* program, const OpMatrix* op)
{
	float mat[16];

	OP_PRINT (" -> Modelview scale=%.2f translate=(%.1f,%.1f)", graphene_matrix_get_value(&op->matrix, 0, 0), graphene_matrix_get_value(&op->matrix, 3, 0), graphene_matrix_get_value(&op->matrix, 3, 1));
#if 0
	printf("\n");
	graphene_matrix_print(&op->matrix);
#endif
	graphene_matrix_to_float (&op->matrix, mat);
	glUniformMatrix4fv (program->modelview_location, 1, GL_FALSE, mat);
}


static inline void
apply_projection_op (const Program* program, const OpMatrix* op)
{
	float mat[16];

	OP_PRINT (" -> Projection %.1f %.1f %.1f", graphene_matrix_get_value(&op->matrix, 0, 0), graphene_matrix_get_value(&op->matrix, 1, 1), graphene_matrix_get_value(&op->matrix, 2, 2));
	graphene_matrix_to_float (&op->matrix, mat);
	glUniformMatrix4fv (program->projection_location, 1, GL_FALSE, mat);
}


static inline void
apply_program_op (const Program* program, const OpProgram* op)
{
	OP_PRINT (" -> Program: %d", op->program->index);
	agl_use_program_id(op->program->id);
}


static inline void
apply_render_target_op (const Program* program, const OpRenderTarget* op)
{
#if 0
	OP_PRINT (" -> Render Target: %d", op->render_target_id);

	glBindFramebuffer (GL_FRAMEBUFFER, op->render_target_id);

	if (op->render_target_id != 0)
		glDisable (GL_SCISSOR_TEST);
	else
		renderer_setup_render_mode (); /* Reset glScissor etc. */
#endif
}


static inline void
apply_color_op (const Program* program, const OpColor* op)
{
	OP_PRINT (" -> Color: (%f, %f, %f, %f)", op->rgba.red, op->rgba.green, op->rgba.blue, op->rgba.alpha);
	glUniform4fv (program->color.color_location, 1, (float*)&op->rgba);
}


static inline void
apply_opacity_op (const Program* program, const OpOpacity* op)
{
	OP_PRINT (" -> Opacity %f", op->opacity);
	glUniform1f (program->alpha_location, op->opacity);
}


static inline void
apply_source_texture_op (const Program* program, const OpTexture* op)
{
	g_assert(op->texture_id != 0);
	OP_PRINT (" -> New texture: %d", op->texture_id);
	/* Use texture unit 0 for the source */
	glUniform1i (program->source_location, 0);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, op->texture_id);
}


#if 0
static inline void
apply_color_matrix_op (const Program* program, const OpColorMatrix* op)
{
	float mat[16];
	float vec[4];
	OP_PRINT (" -> Color Matrix");
	graphene_matrix_to_float (op->matrix, mat);
	glUniformMatrix4fv (program->color_matrix.color_matrix_location, 1, GL_FALSE, mat);

	graphene_vec4_to_float (op->offset, vec);
	glUniform4fv (program->color_matrix.color_offset_location, 1, vec);
}
#endif


static inline void
apply_clip_op (const Program* program, const OpClip* op)
{
	int count;

	if (op->send_corners) {
		OP_PRINT (" -> Clip: %s", agl_rounded_rect_to_string (&op->clip));
		count = 3;
	} else {
		OP_PRINT (" -> clip: %f, %f, %f, %f", op->clip.bounds.origin.x, op->clip.bounds.origin.y, op->clip.bounds.size.width, op->clip.bounds.size.height);
		count = 1;
	}

	glUniform4fv (program->clip_rect_location, count, (float*)&op->clip.bounds);
}


static inline void
apply_inset_shadow_op (const Program* program, const OpShadow* op)
{
#if 0
	OP_PRINT (" -> inset shadow. Color: %s, Offset: (%f, %f), Spread: %f, Outline: %s", gdk_rgba_to_string (op->color), op->offset[0], op->offset[1], op->spread, agl_rounded_rect_to_string (&op->outline));
	glUniform4fv (program->inset_shadow.color_location, 1, (float *)op->color);
	glUniform2fv (program->inset_shadow.offset_location, 1, op->offset);
	glUniform1f (program->inset_shadow.spread_location, op->spread);
	glUniform4fv (program->inset_shadow.outline_rect_location, 3, (float *)&op->outline.bounds);
#endif
}


#if 0
static inline void
apply_unblurred_outset_shadow_op (const Program  *program, const OpShadow *op)
{
	OP_PRINT (" -> unblurred outset shadow");
	glUniform4fv (program->unblurred_outset_shadow.color_location, 1, (float *)op->color);
	glUniform2fv (program->unblurred_outset_shadow.offset_location, 1, op->offset);
	glUniform1f (program->unblurred_outset_shadow.spread_location, op->spread);
	glUniform4fv (program->unblurred_outset_shadow.outline_rect_location, 3, (float *)&op->outline.bounds);
}
#endif


#if 0
static inline void
apply_outset_shadow_op (const Program* program, const OpShadow* op)
{
	OP_PRINT (" -> outset shadow");
	glUniform4fv (program->outset_shadow.outline_rect_location, 3, (float *)&op->outline.bounds);
}


static inline void
apply_linear_gradient_op (const Program* program, const OpLinearGradient* op)
{
	OP_PRINT (" -> Linear gradient");
	glUniform1i (program->linear_gradient.num_color_stops_location, op->n_color_stops);
	glUniform1fv (program->linear_gradient.color_stops_location, op->n_color_stops * 5, (float *)op->color_stops);
	glUniform2f (program->linear_gradient.start_point_location, op->start_point.x, op->start_point.y);
	glUniform2f (program->linear_gradient.end_point_location, op->end_point.x, op->end_point.y);
}


static inline void
apply_border_op (const Program* program, const OpBorder* op)
{
	OP_PRINT (" -> Border Outline");

	glUniform4fv (program->border.outline_rect_location, 3, (float *)&op->outline.bounds);
}

static inline void
apply_border_width_op (const Program* program, const OpBorder* op)
{
	OP_PRINT (" -> Border width (%f, %f, %f, %f)", op->widths[0], op->widths[1], op->widths[2], op->widths[3]);

	glUniform4fv (program->border.widths_location, 1, op->widths);
}

static inline void
apply_border_color_op (const Program* program, const OpBorder* op)
{
	OP_PRINT (" -> Border color: %s", gdk_rgba_to_string (op->color));
	glUniform4fv (program->border.color_location, 1, (float *)op->color);
}


static inline void
apply_blur_op (const Program* program, const OpBlur* op)
{
	OP_PRINT (" -> Blur");
	glUniform1f (program->blur.blur_radius_location, op->radius);
	glUniform2f (program->blur.blur_size_location, op->size.width, op->size.height);
	glUniform2f (program->blur.blur_dir_location, op->dir[0], op->dir[1]);
}


static inline void
apply_cross_fade_op (const Program* program, const OpCrossFade* op)
{
	/* End texture id */
	glUniform1i (program->cross_fade.source2_location, 1);
	glActiveTexture (GL_TEXTURE0 + 1);
	glBindTexture (GL_TEXTURE_2D, op->source2);
	/* progress */
	glUniform1f (program->cross_fade.progress_location, op->progress);
}
#endif


static inline void
apply_blend_op (const Program* program, const OpBlend* op)
{
	/* End texture id */
	glUniform1i (program->blend.source2_location, 1);
	glActiveTexture (GL_TEXTURE0 + 1);
	glBindTexture (GL_TEXTURE_2D, op->source2);
	/* progress */
	glUniform1i (program->blend.mode_location, op->mode);
}


#if 0
static inline void
apply_repeat_op (const Program* program, const OpRepeat* op)
{
	glUniform4fv (program->repeat.child_bounds_location, 1, op->child_bounds);
	glUniform4fv (program->repeat.texture_rect_location, 1, op->texture_rect);
}
#endif


static void
renderer_render_ops (RenderOpBuilder* builder)
{
	const Program* program = NULL;
	const gsize vertex_data_size = builder->vertices->len * sizeof(GskQuadVertex);
	const float* vertex_data = (float*)builder->vertices->data;
	GLuint buffer_id, vao_id;

#if DEBUG_OPS
	g_print ("============================================\n");
#endif

	if(!builder->vertices->len){
		dbg(2, "nothing to render");
		return; // nothing to render
	}

#if DEBUG_OPS
	if(wf_debug) printf("  %f %f (position)\n", vertex_data[0], vertex_data[1]);
#endif

	glGenVertexArrays (1, &vao_id);
	glBindVertexArray (vao_id);

	glGenBuffers (1, &buffer_id);
	glBindBuffer (GL_ARRAY_BUFFER, buffer_id);

	glBufferData (GL_ARRAY_BUFFER, vertex_data_size, vertex_data, GL_STATIC_DRAW);

	/* 0 = position location */
	glEnableVertexAttribArray (0);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, sizeof(GskQuadVertex), (void*)G_STRUCT_OFFSET(GskQuadVertex, position));

	/* 1 = texture coord location */
	glEnableVertexAttribArray (1);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof(GskQuadVertex), (void*)G_STRUCT_OFFSET(GskQuadVertex, uv));

	OpKind kind;
	OpBufferIter iter;
	op_buffer_iter_init (&iter, ops_get_buffer (builder));
	gpointer ptr;
	while ((ptr = op_buffer_iter_next (&iter, &kind))) {
		if (kind == AGL_OP_NONE)
			continue;

		if (program == NULL &&
			kind != OP_PUSH_DEBUG_GROUP &&
			kind != OP_POP_DEBUG_GROUP &&
			kind != OP_CHANGE_PROGRAM &&
			kind != OP_CHANGE_RENDER_TARGET &&
			kind != OP_CLEAR)
			continue;

		OP_PRINT ("Op %2u: %2u", iter.pos - 2, kind);

		switch (kind) {
			case OP_CHANGE_PROJECTION:
				apply_projection_op (program, ptr);
				break;

			case OP_CHANGE_MODELVIEW:
				apply_modelview_op (program, ptr);
				break;

			case OP_CHANGE_PROGRAM: {
				const OpProgram* op = ptr;
				apply_program_op (program, op);
				program = op->program;
				break;
			}

			case OP_CHANGE_RENDER_TARGET:
				apply_render_target_op (program, ptr);
				break;

			case OP_CLEAR:
				glClearColor (0, 0, 0, 0);
				glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				break;

			case OP_CHANGE_VIEWPORT:
				apply_viewport_op (program, ptr);
				break;

			case OP_CHANGE_OPACITY:
				apply_opacity_op (program, ptr);
				break;

			case OP_CHANGE_COLOR_MATRIX:
				//apply_color_matrix_op (program, ptr);
				break;

			case OP_CHANGE_COLOR:
				g_assert (program == &renderer.color_program || program == &renderer.coloring_program/* || program == &self->shadow_program*/);
				apply_color_op (program, ptr);
				break;

			case OP_CHANGE_BORDER_COLOR:
				//apply_border_color_op (program, ptr);
				break;

			case OP_CHANGE_CLIP:
				apply_clip_op (program, ptr);
				break;

			case OP_CHANGE_SOURCE_TEXTURE:
				apply_source_texture_op (program, ptr);
				break;

			case OP_CHANGE_CROSS_FADE:
				//g_assert (program == &self->cross_fade_program);
				//apply_cross_fade_op (program, ptr);
				break;

			case OP_CHANGE_BLEND:
				g_assert (program == &renderer.blend_program);
				apply_blend_op (program, ptr);
				break;

			case OP_CHANGE_LINEAR_GRADIENT:
				//apply_linear_gradient_op (program, ptr);
				break;

			case OP_CHANGE_BLUR:
				//apply_blur_op (program, ptr);
				break;

			case OP_CHANGE_INSET_SHADOW:
				//apply_inset_shadow_op (program, ptr);
				break;

			case OP_CHANGE_OUTSET_SHADOW:
				//apply_outset_shadow_op (program, ptr);
				break;

			case OP_CHANGE_BORDER:
				//apply_border_op (program, ptr);
				break;

			case OP_CHANGE_BORDER_WIDTH:
				//apply_border_width_op (program, ptr);
				break;

			case OP_CHANGE_UNBLURRED_OUTSET_SHADOW:
				//apply_unblurred_outset_shadow_op (program, ptr);
				break;

			#if 0
			case OP_CHANGE_REPEAT:
				//apply_repeat_op (program, ptr);
				break;
			#endif

			case AGL_OP_DRAW: {
				const OpDraw* op = ptr;

				OP_PRINT (" -> draw %ld, size %ld and program %d", op->vao_offset, op->vao_size, program->index);
				glDrawArrays (GL_TRIANGLES, op->vao_offset, op->vao_size);
				break;
			}

			case OP_DUMP_FRAMEBUFFER: {
				const OpDumpFrameBuffer *op = ptr;
				dump_framebuffer (op->filename, op->width, op->height);
				break;
			}

			case OP_PUSH_DEBUG_GROUP: {
				/*
				const OpDebugGroup *op = ptr;
				gdk_gl_context_push_debug_group (self->gl_context, op->text);
				OP_PRINT (" Debug: %s", op->text);
				*/
				break;
			}

			case OP_POP_DEBUG_GROUP:
				//gdk_gl_context_pop_debug_group (self->gl_context);
				break;

			case AGL_OP_NONE:
			case OP_LAST:
			default:
				g_warn_if_reached ();
		}

		OP_PRINT ("\n");
	}

	glDeleteVertexArrays (1, &vao_id);
	glDeleteBuffers (1, &buffer_id);
}


void
renderer_render (RenderOpBuilder* builder)
{
#if 0
	GPtrArray* removed = g_ptr_array_new ();
	gsk_gl_texture_atlases_begin_frame (self->atlases, removed);
#endif

#if 0
	// not currently using the glyph_cache TODO what does it do exactly?
	gsk_gl_glyph_cache_begin_frame (self->glyph_cache, removed);
#endif

#if 0
	gsk_gl_icon_cache_begin_frame (self->icon_cache, removed);
	gsk_gl_shadow_cache_begin_frame (&self->shadow_cache, self->gl_driver);
	g_ptr_array_unref (removed);
#endif

#if 0
	gdk_gl_context_push_debug_group (self->gl_context, "Adding render ops");
	gsk_gl_renderer_add_render_ops (self, root, builder);
	gdk_gl_context_pop_debug_group (self->gl_context);

	/* We correctly reset the state everywhere */
	g_assert_cmpint (builder.current_render_target, ==, fbo_id);
#endif

	//ops_pop_modelview (builder);

#if 0
	ops_pop_clip (builder);
#endif
	ops_finish (builder);

#if 0
	g_message ("Ops: %u", self->render_ops->len);
#endif

#ifdef HAVE_PROFILER
	GskProfiler* profiler = renderer.profiler;
	gsk_profiler_begin_gpu_region (profiler);
	gsk_profiler_timer_begin (profiler, renderer.profile_timers.cpu_time);
#endif

	//gsk_gl_renderer_setup_render_mode (self);
	//gsk_gl_renderer_clear (self);

	/* Pre-multiplied alpha! */
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation (GL_FUNC_ADD);

	//gdk_gl_context_push_debug_group (self->gl_context, "Rendering ops");
	renderer_render_ops (builder);
	//gdk_gl_context_pop_debug_group (self->gl_context);

#ifdef HAVE_PROFILER
	gsk_profiler_counter_inc (profiler, renderer.profile_counters.frames);

	int64_t start_time = gsk_profiler_timer_get_start (profiler, renderer.profile_timers.cpu_time);
	int64_t cpu_time = gsk_profiler_timer_end (profiler, renderer.profile_timers.cpu_time);
	gsk_profiler_timer_set (profiler, renderer.profile_timers.cpu_time, cpu_time);

	int64_t gpu_time = gsk_profiler_end_gpu_region (renderer.profiler);
	gsk_profiler_timer_set (profiler, renderer.profile_timers.gpu_time, gpu_time);

	gsk_profiler_push_samples (profiler);

	if (profiler_is_running (profiler))
		profiler_add_mark (start_time, cpu_time, "GL render", "");
#endif

	renderer_clear_tree (builder);

	//gdk_gl_context_pop_debug_group (self->gl_context);

	gl_warn("post-render");
}


void
renderer_init ()
{
#if 0
	gsk_ensure_resources ();
#endif

	renderer.builders = g_array_new(false, true, sizeof(RenderOpBuilder));

	renderer_push_builder();

#ifdef HAVE_PROFILER
	{
		GskProfiler* profiler = renderer.profiler = gsk_profiler_new ();

		renderer.profile_counters.frames = gsk_profiler_add_counter (profiler, "frames", "Frames", FALSE);

		renderer.profile_timers.cpu_time = gsk_profiler_add_timer (profiler, "cpu-time", "CPU time", FALSE, TRUE);
		renderer.profile_timers.gpu_time = gsk_profiler_add_timer (profiler, "gpu-time", "GPU time", FALSE, TRUE);
	}
#endif
}


/*
 *  Note that as an alternative to using a new builder,
 *  it is possible to use ops_set_render_target
 */
void
renderer_push_builder ()
{
	g_array_append_val (builders(), ((RenderOpBuilder){0,}));
	builder() = &g_array_index (builders(), RenderOpBuilder, builders()->len - 1);
	ops_init (builder());
}


void
renderer_pop_builder ()
{
	ops_free(builder());
	g_array_remove_index (builders(), builders()->len - 1);
	builder() = &g_array_index (builders(), RenderOpBuilder, builders()->len - 1);
}


#define INIT_PROGRAM_UNIFORM_LOCATION(program_name, uniform_basename) \
	G_STMT_START {\
		renderer.program_name ## _program.program_name.uniform_basename ## _location = \
			glGetUniformLocation(renderer.program_name ## _program.id, "u_" #uniform_basename);\
		g_assert_cmpint (renderer.program_name ## _program.program_name.uniform_basename ## _location, >, -1); \
	} G_STMT_END

#define INIT_COMMON_UNIFORM_LOCATION(program, uniform_basename) \
	G_STMT_START {\
		program->uniform_basename ## _location = glGetUniformLocation(program->id, "u_" #uniform_basename);\
	} G_STMT_END


bool
renderer_create_programs (GError** error)
{
	static const struct {
		const char* resource_path;
		const char* name;
	} program_definitions[] = {
		{ "/org/ayyi/agl/resources/shaders/blend.glsl",          "blend" },
		/*
		{ "/org/gtk/libgsk/glsl/blit.glsl",                      "blit" },
		{ "/org/gtk/libgsk/glsl/blur.glsl",                      "blur" },
		{ "/org/gtk/libgsk/glsl/border.glsl",                    "border" },
		{ "/org/gtk/libgsk/glsl/color_matrix.glsl",              "color matrix" },
		*/
		{ "/org/ayyi/agl/resources/shaders/color.glsl",          "color" },
		{ "/org/ayyi/agl/resources/shaders/coloring.glsl",       "coloring" },
		/*
		{ "/org/gtk/libgsk/glsl/cross_fade.glsl",                "cross fade" },
		{ "/org/gtk/libgsk/glsl/inset_shadow.glsl",              "inset shadow" },
		{ "/org/gtk/libgsk/glsl/linear_gradient.glsl",           "linear gradient" },
		{ "/org/gtk/libgsk/glsl/outset_shadow.glsl",             "outset shadow" },
		{ "/org/gtk/libgsk/glsl/repeat.glsl",                    "repeat" },
		{ "/org/gtk/libgsk/glsl/unblurred_outset_shadow.glsl",   "unblurred_outset shadow" },
		*/
	};
	gboolean success = TRUE;

	GResource* resource = gresource_get_resource();
	g_resources_register(resource);

	GskGLShaderBuilder shader_builder;
	shader_builder_init (&shader_builder,
		"/org/ayyi/agl/resources/shaders/preamble.glsl",
		"/org/ayyi/agl/resources/shaders/preamble.vs.glsl",
		"/org/ayyi/agl/resources/shaders/preamble.fs.glsl"
	);

	g_assert (G_N_ELEMENTS(program_definitions) == GL_N_PROGRAMS);

#ifdef DEBUG
	if (AGL_DEBUG_CHECK (SHADERS))
		shader_builder.debugging = TRUE;
#endif

	AGl* agl = agl_get_instance();

	if (!(agl->have & AGL_HAVE_3_2)){
		if (agl->have &= AGL_HAVE_3_0)
			shader_builder_set_glsl_version (&shader_builder, SHADER_VERSION_GL3_LEGACY);
		else
			shader_builder_set_glsl_version (&shader_builder, SHADER_VERSION_GL2_LEGACY);

		shader_builder.legacy = true;
	} else {
		shader_builder_set_glsl_version (&shader_builder, SHADER_VERSION_GL3);
		shader_builder.gl3 = true;
	}

	for (int i = 0; i < GL_N_PROGRAMS; i ++) {
		Program* prog = &renderer.programs[i];

		prog->index = i;
		prog->id = shader_builder_create_program (&shader_builder, program_definitions[i].resource_path, error);
		if (prog->id < 0) {
			success = FALSE;
			goto out;
		}

		INIT_COMMON_UNIFORM_LOCATION (prog, alpha);
		INIT_COMMON_UNIFORM_LOCATION (prog, source);
		INIT_COMMON_UNIFORM_LOCATION (prog, clip_rect);
		INIT_COMMON_UNIFORM_LOCATION (prog, viewport);
		INIT_COMMON_UNIFORM_LOCATION (prog, projection);
		INIT_COMMON_UNIFORM_LOCATION (prog, modelview);
	}

	/* color */
	INIT_PROGRAM_UNIFORM_LOCATION (color, color);

	/* coloring */
	INIT_PROGRAM_UNIFORM_LOCATION (coloring, color);

#if 0
	/* color matrix */
	INIT_PROGRAM_UNIFORM_LOCATION (color_matrix, color_matrix);
	INIT_PROGRAM_UNIFORM_LOCATION (color_matrix, color_offset);

	/* linear gradient */
	INIT_PROGRAM_UNIFORM_LOCATION (linear_gradient, color_stops);
	INIT_PROGRAM_UNIFORM_LOCATION (linear_gradient, num_color_stops);
	INIT_PROGRAM_UNIFORM_LOCATION (linear_gradient, start_point);
	INIT_PROGRAM_UNIFORM_LOCATION (linear_gradient, end_point);

	/* blur */
	INIT_PROGRAM_UNIFORM_LOCATION (blur, blur_radius);
	INIT_PROGRAM_UNIFORM_LOCATION (blur, blur_size);
	INIT_PROGRAM_UNIFORM_LOCATION (blur, blur_dir);

	/* inset shadow */
	INIT_PROGRAM_UNIFORM_LOCATION (inset_shadow, color);
	INIT_PROGRAM_UNIFORM_LOCATION (inset_shadow, spread);
	INIT_PROGRAM_UNIFORM_LOCATION (inset_shadow, offset);
	INIT_PROGRAM_UNIFORM_LOCATION (inset_shadow, outline_rect);

	/* outset shadow */
	INIT_PROGRAM_UNIFORM_LOCATION (outset_shadow, color);
	INIT_PROGRAM_UNIFORM_LOCATION (outset_shadow, outline_rect);

	/* unblurred outset shadow */
	INIT_PROGRAM_UNIFORM_LOCATION (unblurred_outset_shadow, color);
	INIT_PROGRAM_UNIFORM_LOCATION (unblurred_outset_shadow, spread);
	INIT_PROGRAM_UNIFORM_LOCATION (unblurred_outset_shadow, offset);
	INIT_PROGRAM_UNIFORM_LOCATION (unblurred_outset_shadow, outline_rect);

	/* border */
	INIT_PROGRAM_UNIFORM_LOCATION (border, color);
	INIT_PROGRAM_UNIFORM_LOCATION (border, widths);
	INIT_PROGRAM_UNIFORM_LOCATION (border, outline_rect);

	/* cross fade */
	INIT_PROGRAM_UNIFORM_LOCATION (cross_fade, progress);
	INIT_PROGRAM_UNIFORM_LOCATION (cross_fade, source2);
#endif

	/* blend */
	INIT_PROGRAM_UNIFORM_LOCATION (blend, source2);
	INIT_PROGRAM_UNIFORM_LOCATION (blend, mode);

#if 0
	/* repeat */
	INIT_PROGRAM_UNIFORM_LOCATION (repeat, child_bounds);
	INIT_PROGRAM_UNIFORM_LOCATION (repeat, texture_rect);
#endif

	/*  We initialize the alpha uniform here, since the default value is important.
	 *  We can't do it in the shader like a resonable person would because that doesn't
	 *  work in gles. */
	for (int i = 0; i < GL_N_PROGRAMS; i++) {
		agl_use_program_id(renderer.programs[i].id);
		glUniform1f (renderer.programs[i].alpha_location, 1.0);
	}

  out:
	shader_builder_finish (&shader_builder);

	return success;
}
