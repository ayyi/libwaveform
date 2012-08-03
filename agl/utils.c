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

  ---------------------------------------------------------------

  copyright (C) 2008 Brian Paul

*/
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <glib.h>
#include "agl/utils.h"

AGl* agl = NULL;

AGl*
agl_get_instance()
{
	if(!agl){
		agl = g_new0(AGl, 1);
		agl->pref_use_shaders = TRUE;
		agl->use_shaders = TRUE;
	}
	return agl;
}


GLboolean
agl_shaders_supported()
{
	const char* version = (const char*)glGetString(GL_VERSION);
	if (version[0] == '2' && version[1] == '.') {
		return GL_TRUE;
	}
#if 0
	else if (glutExtensionSupported("GL_ARB_vertex_shader")
			&& glutExtensionSupported("GL_ARB_fragment_shader")
			&& glutExtensionSupported("GL_ARB_shader_objects"))
	{
		fprintf(stderr, "Warning: Trying ARB GLSL instead of OpenGL 2.x.  This may not work.\n");
		return GL_TRUE;
	}
	return GL_TRUE;
#endif
	return GL_FALSE;
}


