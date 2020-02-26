/*
 * Copyright © 2016  Endless
 *             2018  Timm Bäder <mail@baedert.org>
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
 * Authors: Timm Bäder <mail@baedert.org>
 */

#ifndef __renderer_h__
#define __renderer_h__

#include "agl/text/renderops.h"
#ifdef DEBUG
#include "agl/text/profiler.h"
#endif

typedef struct
{
   GArray*          builders;
   RenderOpBuilder* current_builder;

   union {
      Program programs[GL_N_PROGRAMS];
      struct {
         Program blend_program;
#if 0
         Program blit_program;
         Program blur_program;
         Program border_program;
         Program color_matrix_program;
#endif
         Program color_program;
         Program coloring_program;
#if 0
         Program cross_fade_program;
         Program inset_shadow_program;
         Program linear_gradient_program;
         Program outset_shadow_program;
         Program repeat_program;
         Program unblurred_outset_shadow_program;
#endif
      };
   };

#ifdef HAVE_PROFILER
   GskProfiler* profiler;

   struct {
      GQuark frames;
   } profile_counters;

   struct {
      GQuark cpu_time;
      GQuark gpu_time;
   } profile_timers;
#endif

} Renderer;

#ifndef __gsk_pango_c__
extern Renderer renderer;
#endif

void renderer_init            ();
void renderer_push_builder    ();
void renderer_pop_builder     ();
void renderer_render          (RenderOpBuilder*);
bool renderer_create_programs (GError**);

#define builders() (renderer.builders)
#define builder()  (renderer.current_builder)

#endif
