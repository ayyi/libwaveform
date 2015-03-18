/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

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

#endif //__waveform_gl_utils_h__
