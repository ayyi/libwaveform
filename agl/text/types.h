/* GSK - The GTK Scene Kit
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

#ifndef __gsk_types_h__
#define __gsk_types_h__

#include <graphene.h>
#include <gdk/gdk.h>
#include "agl/text/enums.h"
#include "agl/text/gdkrgba.h"

typedef struct _GskRenderer  GskRenderer;
typedef struct _GskTexture   GskTexture;
typedef struct _GskColorStop GskColorStop;

struct _GskColorStop
{
  double offset;
  GdkRGBA color;
};

typedef struct
{
  guint texture_id;
  int width;
  int height;
} GskGLImage;

typedef struct
{
  guchar* data;
  gsize width;
  gsize height;
  gsize stride;
  gsize x;
  gsize y;
} GskImageRegion;

/**
 * AGlRoundedRect:
 * @bounds: the bounds of the rectangle
 * @corner: the size of the 4 rounded corners
 *
 * A rectanglular region with rounded corners.
 *
 * Application code should normalize rectangles using gsk_rounded_rect_normalize();
 * this function will ensure that the bounds of the rectanlge are normalized
 * and ensure that the corner values are positive and the corners do not overlap.
 * All functions taking a #AGlRoundedRect as an argument will internally operate on
 * a normalized copy; all functions returning a #AGlRoundedRect will always return
 * a normalized one.
 */
typedef struct
{
  graphene_rect_t bounds;
  graphene_size_t corner[4];

} AGlRoundedRect;

/**
 * GdkGLError:
 * @GDK_GL_ERROR_NOT_AVAILABLE: OpenGL support is not available
 * @GDK_GL_ERROR_UNSUPPORTED_FORMAT: The requested visual format is not supported
 * @GDK_GL_ERROR_UNSUPPORTED_PROFILE: The requested profile is not supported
 * @AGL_GL_ERROR_COMPILATION_FAILED: The shader compilation failed
 * @AGL_GL_ERROR_LINK_FAILED: The shader linking failed
 *
 * Error enumeration for #GdkGLContext.
 */
typedef enum {
  GDK_GL_ERROR_NOT_AVAILABLE,
  GDK_GL_ERROR_UNSUPPORTED_FORMAT,
  GDK_GL_ERROR_UNSUPPORTED_PROFILE,
  AGL_GL_ERROR_COMPILATION_FAILED,
  AGL_GL_ERROR_LINK_FAILED
} GdkGLError;

#endif
