/*
  copyright (C) 2012-2013 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifndef __waveform_gl_utils_h__
#define __waveform_gl_utils_h__
#include "waveform/typedefs.h"

#ifdef __waveform_gl_utils_c__
gboolean __wf_drawing = FALSE;
int __draw_depth = 0;
#else
extern gboolean __wf_drawing;
extern int __draw_depth;
#endif
#define WF_START_DRAW \
	if(__wf_drawing){ gwarn("START_DRAW: already drawing"); } \
	__draw_depth++; \
	__wf_drawing = TRUE; \
	if ((__draw_depth > 1) || gdk_gl_drawable_gl_begin (view->priv->canvas->gl_drawable, view->priv->canvas->gl_context)) {
#define WF_END_DRAW \
	__draw_depth--; \
	if(!__draw_depth) gdk_gl_drawable_gl_end(view->priv->canvas->gl_drawable); \
	} else { gwarn("!! gl_begin fail"); } \
	(__wf_drawing = FALSE);
#define ASSERT_DRAWING g_return_if_fail(__wf_drawing);

#endif //__waveform_gl_utils_h__
