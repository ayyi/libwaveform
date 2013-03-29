/*
  copyright (C) 2012 Tim Orford <tim@orford.org>

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
#define __waveform_gl_utils_c__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <GL/gl.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include "waveform/typedefs.h"
#include "waveform/gl_utils.h"

static TextureUnit* active_texture_unit = NULL;


TextureUnit*
texture_unit_new(GLenum unit)
{
	TextureUnit* texture_unit = g_new0(TextureUnit, 1);
	texture_unit->unit = unit;
	return texture_unit;
}


void
texture_unit_use_texture(TextureUnit* unit, int texture)
{
	g_return_if_fail(unit);

	if(TRUE || active_texture_unit != unit){
		active_texture_unit = unit;
		glActiveTexture(unit->unit);
	}
	glBindTexture(GL_TEXTURE_1D, texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


