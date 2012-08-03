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
#ifndef __shader_util_h__
#define __shader_util_h__

#ifdef __gl_h_
#include "agl/utils.h"

struct _uniform_info
{
   const char* name;
   GLuint      size;
   GLenum      type;      // GL_FLOAT or GL_INT
   GLfloat     value[4];
   GLint       location;  // filled in by InitUniforms()
};

#define END_OF_UNIFORMS   { NULL, 0, GL_NONE, { 0, 0, 0, 0 }, -1 }

extern GLuint    compile_shader_text (GLenum shaderType, const char* text);
extern GLuint    compile_shader_file (GLenum shaderType, const char* filename);
extern GLuint    link_shaders        (GLuint vertShader, GLuint fragShader);
extern GLuint    link_shaders2       (GLuint vert_shader_1, GLuint frag_shader_1, GLuint vert_shader_2, GLuint frag_shader_2);

extern void      uniforms_init       (GLuint program, struct _uniform_info uniforms[]);


#endif // __gl_h_
#endif // __shader_util_h__
