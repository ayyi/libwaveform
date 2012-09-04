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
#include "agl/ext.h"
#include "agl/utils.h"

#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define dbg(A, B, ...) wf_debug_printf(__func__, A, B, ##__VA_ARGS__)

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


GLuint
agl_create_program(AGlShader* sh, AGlUniformInfo* uniforms)
{
	GLuint vert_shader = sh->vertex_file
		? agl_compile_shader_file(GL_VERTEX_SHADER, sh->vertex_file)
		: agl_compile_shader_text(GL_VERTEX_SHADER, sh->text->vert);
	GLuint frag_shader = sh->fragment_file
		? agl_compile_shader_file(GL_FRAGMENT_SHADER, sh->fragment_file)
		: agl_compile_shader_text(GL_FRAGMENT_SHADER, sh->text->frag);

	GLint status;
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE){
		printf("shader compile error! %i\n", status);
	}

	GLuint program = sh->program = agl_link_shaders(vert_shader, frag_shader);
	dbg(2, "%u %u program=%u", vert_shader, frag_shader, program);

	glUseProgram(program);

	agl_uniforms_init(program, uniforms);

	sh->uniforms = uniforms;

	return program;
}


GLuint
agl_compile_shader_text(GLenum shaderType, const char* text)
{
   GLint stat;

   GLuint shader = glCreateShader(shaderType);
   glShaderSource(shader, 1, (const GLchar**) &text, NULL);
   glCompileShader(shader);
   glGetShaderiv(shader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      GLchar log[1000];
      GLsizei len;
      glGetShaderInfoLog(shader, 1000, &len, log);
      fprintf(stderr, "Error: problem compiling shader: %s\n", log);
      exit(1);
   }
   return shader;
}


/**
 * Read a shader from a file.
 */
GLuint
agl_compile_shader_file(GLenum shaderType, const char* filename)
{
   const int max = 100*1000;
   GLuint shader = 0;

   gchar* local_path = g_build_filename("shaders", filename, NULL);
   FILE* f = fopen(local_path, "r");
   g_free(local_path);
   if (!f) {
      //try installed file...
      gchar* path = g_build_filename(PACKAGE_DATA_DIR, PACKAGE, "shaders", filename, NULL);
      f = fopen(path, "r");
      g_free(path);

      if (!f) {
         gwarn("unable to open shader file %s", filename);
         return 0;
      }
   }

   char* buffer = (char*) malloc(max);
   int n = fread(buffer, 1, max, f);
   /*printf("read %d bytes from shader file %s\n", n, filename);*/
   if (n > 0) {
      buffer[n] = 0;
      shader = agl_compile_shader_text(shaderType, buffer);
   }

   fclose(f);
   free(buffer);

   return shader;
}


void
agl_uniforms_init(GLuint program, struct _uniform_info uniforms[])
{
	GLuint i;
	dbg(1, "program=%u", program);

	for (i = 0; uniforms[i].name; i++) {
		uniforms[i].location = glGetUniformLocation(program, uniforms[i].name);
		//note location zero is ok.
		if(uniforms[i].location < 0) gwarn("location=%i", uniforms[i].location);

		//printf("uniform: '%s' location=%d\n", uniforms[i].name, uniforms[i].location);

		switch (uniforms[i].size) {
			case 1:
				if (uniforms[i].type == GL_INT)
					glUniform1i(uniforms[i].location, (GLint) uniforms[i].value[0]);
				else
					glUniform1fv(uniforms[i].location, 1, uniforms[i].value);
				break;
			case 2:
				glUniform2fv(uniforms[i].location, 1, uniforms[i].value);
				break;
			case 3:
				glUniform3fv(uniforms[i].location, 1, uniforms[i].value);
				break;
			case 4:
				glUniform4fv(uniforms[i].location, 1, uniforms[i].value);
				break;
			default:
				abort();
		}
	}
}


GLuint
agl_link_shaders(GLuint vertShader, GLuint fragShader)
{
   GLuint program = glCreateProgram();

   assert(vertShader || fragShader);

   if (fragShader)
      glAttachShader(program, fragShader);
   if (vertShader)
      glAttachShader(program, vertShader);
   glLinkProgram(program);

   /* check link */
   {
      GLint stat;
      glGetProgramiv(program, GL_LINK_STATUS, &stat);
      if (!stat) {
         GLchar log[1000];
         GLsizei len;
         glGetProgramInfoLog(program, 1000, &len, log);
         fprintf(stderr, "Shader link error:\n%s\n", log);
         return 0;
      }
   }

   return program;
}


GLuint
agl_link_shaders2(GLuint vertShader, GLuint fragShader1, GLuint vertShader2, GLuint fragShader2)
{
   GLuint program = glCreateProgram();

   assert(vertShader || fragShader1);

   if (vertShader)
      glAttachShader(program, vertShader);
   if (fragShader1)
      glAttachShader(program, fragShader1);
   if (vertShader2)
      glAttachShader(program, vertShader2);
   if (fragShader2)
      glAttachShader(program, fragShader2);
   glLinkProgram(program);

   /* check link */
   {
      GLint stat;
      glGetProgramiv(program, GL_LINK_STATUS, &stat);
      if (!stat) {
         GLchar log[1000];
         GLsizei len;
         glGetProgramInfoLog(program, 1000, &len, log);
         fprintf(stderr, "Shader link error:\n%s\n", log);
         return 0;
      }
   }

   return program;
}


