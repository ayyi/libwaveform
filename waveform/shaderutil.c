/**
 * Utilities for OpenGL shading language
 *
 * Brian Paul
 * 9 April 2008
 */


#define __wf_private__
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <glib.h>
#include "waveform/gl_ext.h"
#include "waveform/utils.h"
#include "waveform/shaderutil.h"


static void
init()
{
   static GLboolean firstCall = GL_TRUE;
   if (firstCall) {
      firstCall = GL_FALSE;
   }
}


GLboolean
shaders_supported()
{
   const char* version = (const char*)glGetString(GL_VERSION);
   if (version[0] == '2' && version[1] == '.') {
      return GL_TRUE;
   }
#if 0
   else if (glutExtensionSupported("GL_ARB_vertex_shader")
            && glutExtensionSupported("GL_ARB_fragment_shader")
            && glutExtensionSupported("GL_ARB_shader_objects")) {
      fprintf(stderr, "Warning: Trying ARB GLSL instead of OpenGL 2.x.  This may not work.\n");
      return GL_TRUE;
   }
   return GL_TRUE;
#endif
	return GL_FALSE;
}


GLuint
compile_shader_text(GLenum shaderType, const char* text)
{
   GLint stat;

   init();

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
compile_shader_file(GLenum shaderType, const char* filename)
{
   const int max = 100*1000;
   GLuint shader = 0;

   init();

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
      shader = compile_shader_text(shaderType, buffer);
   }

   fclose(f);
   free(buffer);

   return shader;
}


GLuint
link_shaders(GLuint vertShader, GLuint fragShader)
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
link_shaders2(GLuint vertShader, GLuint fragShader1, GLuint vertShader2, GLuint fragShader2)
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


void
uniforms_init(GLuint program, struct uniform_info uniforms[])
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

