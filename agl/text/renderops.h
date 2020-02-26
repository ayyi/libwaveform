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
#ifndef __render_ops_h__
#define __render_ops_h__

#include <glib.h>
#include <graphene.h>
#include <gdk/gdk.h>

#include "agl/typedefs.h"
#include "agl/transform.h"
#include "agl/text/driver.h"

#include "agl/text/opbuffer.h"

#define GL_N_VERTICES 6
#define GL_N_PROGRAMS 3

struct _Program
{
  int index;        /* Into the renderer's program array */

  int id;

  /* Common uniform locations */
  int source_location;
  int position_location;
  int uv_location;
  int alpha_location;
  int viewport_location;
  int projection_location;
  int modelview_location;
  int clip_rect_location;

  union {
    struct {
      int color_location;
    } color;
    struct {
      int color_location;
    } coloring;
#if 0
    struct {
      int color_matrix_location;
      int color_offset_location;
    } color_matrix;
    struct {
      int num_color_stops_location;
      int color_stops_location;
      int start_point_location;
      int end_point_location;
    } linear_gradient;
    struct {
      int blur_radius_location;
      int blur_size_location;
      int blur_dir_location;
    } blur;
    struct {
      int color_location;
      int spread_location;
      int offset_location;
      int outline_rect_location;
    } inset_shadow;
    struct {
      int color_location;
      int outline_rect_location;
    } outset_shadow;
    struct {
      int outline_rect_location;
      int color_location;
      int spread_location;
      int offset_location;
    } unblurred_outset_shadow;
    struct {
      int color_location;
      int widths_location;
      int outline_rect_location;
    } border;
    struct {
      int source2_location;
      int progress_location;
    } cross_fade;
#endif
    struct {
      int source2_location;
      int mode_location;
    } blend;
#if 0
    struct {
      int child_bounds_location;
      int texture_rect_location;
    } repeat;
#endif
  };
};

typedef struct
{
   AGlTransform* modelview;
   AGlRoundedRect clip;
   graphene_matrix_t projection;
#if 0
   int source_texture;
#endif
   graphene_rect_t viewport;
   float opacity;
   /* Per-program state */
   union {
      GdkRGBA color;
#if 0
      struct {
         graphene_matrix_t matrix;
         graphene_vec4_t offset;
      } color_matrix;
      struct {
         float widths[4];
         GdkRGBA color;
         AGlRoundedRect outline;
      } border;
#endif
   };
} ProgramState;

typedef struct
{
   AGlFboId          target;

   struct {
      ProgramState   state[GL_N_PROGRAMS]; // the state of the builder, not the graphics card
      const Program* current;
   }                 programs;

   int               current_texture;

   graphene_matrix_t current_projection;
   graphene_rect_t   current_viewport;
   float             current_opacity;
   float             dx, dy;
   float             scale_x, scale_y;

   AGliPt            offset; // absolute position of current node in the scene graph

   OpBuffer          render_ops;
   GArray*           vertices;

   /* Stack of modelview matrices */
   struct {
      GArray*        stack;   // array of MatrixStackEntry
      AGlTransform*  current; // pointer into the stack
   } modelview;

   /* Clip stack */
   struct {
      GArray*               stack;
      const AGlRoundedRect* current; // pointer into the stack
   } clip;
}
RenderOpBuilder;


void            ops_init               (RenderOpBuilder*);
void            ops_free               (RenderOpBuilder*);
void            ops_reset              (RenderOpBuilder*);

void            ops_finish             (RenderOpBuilder*);
void            ops_push_modelview     (RenderOpBuilder*, AGlTransform*);
void            ops_set_modelview      (RenderOpBuilder*, AGlTransform*);
void            ops_pop_modelview      (RenderOpBuilder*);
float           ops_get_scale          (const RenderOpBuilder*);

void            ops_set_program        (RenderOpBuilder*, const Program*);

void            ops_push_clip          (RenderOpBuilder*, const AGlRoundedRect* clip);
void            ops_pop_clip           (RenderOpBuilder*);
bool            ops_has_clip           (RenderOpBuilder*);

void            ops_transform_bounds_modelview
                                       (const RenderOpBuilder*, const graphene_rect_t* src, graphene_rect_t* dst);

graphene_matrix_t
                ops_set_projection     (RenderOpBuilder*, const graphene_matrix_t* projection);

graphene_rect_t ops_set_viewport       (RenderOpBuilder*, const graphene_rect_t* viewport);

void            ops_set_texture        (RenderOpBuilder*, AGlTextureId);

#if 0
float           ops_set_opacity        (RenderOpBuilder*, float opacity);
#endif
void            ops_set_color          (RenderOpBuilder*, const GdkRGBA* color);
#if 0
void            ops_set_color_matrix   (RenderOpBuilder*, const graphene_matrix_t*, const graphene_vec4_t* offset);

void            ops_set_border         (RenderOpBuilder*, const AGlRoundedRect* outline);
void            ops_set_border_width   (RenderOpBuilder*, const float* widths);
void            ops_set_border_color   (RenderOpBuilder*, const GdkRGBA* color);
#endif

GskQuadVertex*  ops_draw               (RenderOpBuilder*, const GskQuadVertex vertex_data[GL_N_VERTICES]);

#if 0
void            ops_offset             (RenderOpBuilder*, float x, float y);
#endif

OpBuffer*       ops_get_buffer         (RenderOpBuilder*);

#ifdef DEBUG
void            ops_dump_framebuffer   (RenderOpBuilder*, const char* filename, int width, int height);
void            ops_push_debug_group   (RenderOpBuilder*, const char*);
void            ops_pop_debug_group    (RenderOpBuilder*);
#endif

#endif
