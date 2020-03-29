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
#ifndef __op_buffer_h__
#define __op_buffer_h__

#include <gdk/gdk.h>
#include <graphene.h>
#include "agl/text/types.h"
#include "agl/text/enums.h"
#include "agl/text/gdkrgba.h"
#include "agl/text/driver.h"

typedef struct _Program Program;

typedef enum
{
  AGL_OP_NONE                          =  0,
  OP_CHANGE_OPACITY                    =  1,
  OP_CHANGE_COLOR                      =  2,
  OP_CHANGE_PROJECTION                 =  3,
  OP_CHANGE_MODELVIEW                  =  4,
  OP_CHANGE_PROGRAM                    =  5,
  OP_CHANGE_RENDER_TARGET              =  6,
  OP_CHANGE_CLIP                       =  7,
  OP_CHANGE_VIEWPORT                   =  8,
  OP_CHANGE_SOURCE_TEXTURE             =  9,
#if 0
  OP_CHANGE_REPEAT                     = 10,
#endif
  OP_CHANGE_LINEAR_GRADIENT            = 11,
  OP_CHANGE_COLOR_MATRIX               = 12,
  OP_CHANGE_BLUR                       = 13,
  OP_CHANGE_INSET_SHADOW               = 14,
  OP_CHANGE_OUTSET_SHADOW              = 15,
  OP_CHANGE_BORDER                     = 16,
  OP_CHANGE_BORDER_COLOR               = 17,
  OP_CHANGE_BORDER_WIDTH               = 18,
  OP_CHANGE_CROSS_FADE                 = 19,
  OP_CHANGE_UNBLURRED_OUTSET_SHADOW    = 20,
  OP_CLEAR                             = 21,
  AGL_OP_DRAW                          = 22,
  OP_DUMP_FRAMEBUFFER                  = 23,
  OP_PUSH_DEBUG_GROUP                  = 24,
  OP_POP_DEBUG_GROUP                   = 25,
  OP_CHANGE_BLEND                      = 26,
  OP_LAST
} OpKind;

/* OpNode are allocated within OpBuffer.pos, but we keep
 * a secondary index into the locations of that buffer
 * from OpBuffer.index. This allows peeking at the kind
 * and quickly replacing existing entries when necessary.
 */
typedef struct
{
  guint  pos;
  OpKind kind;
} OpBufferEntry;

/*
 *  All ops are stored in a single buffer Opbuffer.buf
 *  'pos' is the stack pointer
 *  'index' is an array of pointers into the buffer that allows for iteration
 */
typedef struct
{
  guint8* buf;
  gsize   len;
  gsize   pos;
  GArray* index;
} OpBuffer;

typedef struct
{
  float opacity;
} OpOpacity;

typedef struct
{
  graphene_matrix_t matrix;
} OpMatrix;

typedef struct
{
  const Program* program;
} OpProgram;

typedef struct
{
  GdkRGBA rgba;
} OpColor;

typedef struct
{
  int render_target_id;
} OpRenderTarget;

typedef struct
{
  AGlRoundedRect clip;
  guint send_corners: 1;
} OpClip;

typedef struct
{
  graphene_rect_t viewport;
} OpViewport;

typedef struct
{
  int texture_id;
} OpTexture;

typedef struct
{
  gsize vao_offset;
  gsize vao_size;
} OpDraw;

typedef struct
{
  const GskColorStop *color_stops;
  graphene_point_t start_point;
  graphene_point_t end_point;
  int n_color_stops;
} OpLinearGradient;

typedef struct
{
  const graphene_matrix_t* matrix;
  const graphene_vec4_t* offset;
} OpColorMatrix;

typedef struct
{
  float radius;
  graphene_size_t size;
  float dir[2];
} OpBlur;

typedef struct
{
  AGlRoundedRect outline;
  float spread;
  float offset[2];
  const GdkRGBA *color;
} OpShadow;

typedef struct
{
  float widths[4];
  const GdkRGBA* color;
  AGlRoundedRect outline;
} OpBorder;

typedef struct
{
  float progress;
  int source2;
} OpCrossFade;

typedef struct
{
  char* filename;
  int width;
  int height;
} OpDumpFrameBuffer;

typedef struct
{
  char text[64];
} OpDebugGroup;

typedef struct
{
  int source2;
  int mode;
} OpBlend;

typedef struct
{
  float child_bounds[4];
  float texture_rect[4];
} OpRepeat;

void     op_buffer_init    (OpBuffer*);
void     op_buffer_destroy (OpBuffer*);
void     op_buffer_clear   (OpBuffer*);
gpointer op_buffer_add     (OpBuffer*, OpKind);

typedef struct
{
   GArray*   index;
   OpBuffer* buffer;
   guint     pos;

} OpBufferIter;


static inline void
op_buffer_iter_init (OpBufferIter* iter, OpBuffer* buffer)
{
	iter->index = buffer->index;
	iter->buffer = buffer;
	iter->pos = 1; /* Skip first AGL_OP_NONE */
}


static inline gpointer
op_buffer_iter_next (OpBufferIter* iter, OpKind* kind)
{
	if (iter->pos == iter->index->len)
		return NULL;

	const OpBufferEntry* entry = &g_array_index (iter->index, OpBufferEntry, iter->pos);

	iter->pos++;

	*kind = entry->kind;
	return &iter->buffer->buf[entry->pos];
}


static inline void
op_buffer_pop_tail (OpBuffer* buffer)
{
	/* Never truncate the first AGL_OP_NONE */
	if G_LIKELY (buffer->index->len > 0)
		buffer->index->len--;
}


static inline gpointer
op_buffer_peek_tail (OpBuffer* buffer, OpKind* kind)
{
	const OpBufferEntry* entry = &g_array_index (buffer->index, OpBufferEntry, buffer->index->len - 1);
	*kind = entry->kind;
	return &buffer->buf[entry->pos];
}


static inline gpointer
op_buffer_peek_tail_checked (OpBuffer* buffer, OpKind kind)
{
	const OpBufferEntry* entry = &g_array_index (buffer->index, OpBufferEntry, buffer->index->len - 1);

	if (entry->kind == kind)
		return &buffer->buf[entry->pos];

	return NULL;
}


static inline guint
op_buffer_n_ops (OpBuffer* buffer)
{
	return buffer->index->len - 1;
}

#endif
