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
#include <pango/pangofc-font.h>
#include <pango/pangofc-fontmap.h>
#include "agl/ext.h"
#include "agl/pango_render.h"
#include "agl/utils.h"

#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define dbg(A, B, ...) wf_debug_printf(__func__, A, B, ##__VA_ARGS__)

static gboolean font_is_scalable(PangoContext*, const char* font_name);


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
	if(!version){
		gwarn("cannot get gl version. incorrect mode?");
		agl->use_shaders = FALSE;
		return GL_FALSE;
	}
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
	agl->use_shaders = FALSE;
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


void
agl_rect(float x, float y, float w, float h)
{
	glBegin(GL_QUADS);
	glVertex2f(x,     y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x,     y + h);
	glEnd();
}


static PangoFontDescription* font_desc = NULL;

void
agl_set_font(char* font_string)
{
	if(font_desc) pango_font_description_free(font_desc);
	font_desc = pango_font_description_from_string(font_string);

	PangoGlRendererClass* PGRC = g_type_class_peek(PANGO_TYPE_GL_RENDERER);

	if(!PGRC->context){
		PangoFontMap* fontmap = pango_gl_font_map_new();
		//pango_gl_font_map_set_resolution (PANGO_GL_FONT_MAP(fontmap), 96.0);
		PGRC->context = pango_gl_font_map_create_context(PANGO_GL_FONT_MAP(fontmap));
	}

	dbg(0, "%s", font_string);
	// for some reason there seems to be an issue with pixmap fonts
	if(!font_is_scalable(PGRC->context, pango_font_description_get_family(font_desc))){
		strcpy(font_string, "Sans 7");
		pango_font_description_free(font_desc);
		font_desc = pango_font_description_from_string(font_string);
	}
	dbg(2, "%s", font_string);

	pango_font_description_set_weight(font_desc, PANGO_WEIGHT_SEMIBOLD); //TODO
}


void
agl_print(int x, int y, double z, uint32_t colour, const char *fmt, ...)
{
	if(!fmt) return;

	va_list args;
	va_start(args, fmt);
	gchar* text = g_strdup_vprintf(fmt, args);
	va_end(args); //text now contains the string.

	gboolean first_time = FALSE;
	PangoGlRendererClass* PGRC = g_type_class_peek(PANGO_TYPE_GL_RENDERER);
#if 0
	if(!PGRC->context){
		first_time = TRUE;
		PangoFontMap* fontmap = pango_gl_font_map_new();
		//pango_gl_font_map_set_resolution (PANGO_GL_FONT_MAP(fontmap), 96.0);
		PGRC->context = pango_gl_font_map_create_context(PANGO_GL_FONT_MAP(fontmap));
	}
#endif

	PangoLayout* layout = pango_layout_new (PGRC->context);
	pango_layout_set_text (layout, text, -1);

#if 0
	if(!font_desc){
		char font_string[64];
		get_font_string(font_string, -3); //TODO why do we have to set this so small to get a reasonable font size?
		font_desc = pango_font_description_from_string(font_string);

		if(!font_is_scalable(PGRC->context, pango_font_description_get_family(font_desc))){
			strcpy(font_string, "Sans 7");
			pango_font_description_free(font_desc);
			font_desc = pango_font_description_from_string(font_string);
		}
		dbg(2, "%s", font_string);

		pango_font_description_set_weight(font_desc, PANGO_WEIGHT_SEMIBOLD); //TODO
	}
#endif

	pango_layout_set_font_description(layout, font_desc);

#if 0
	PangoFontMap* fontmap = pango_context_get_font_map (_context);
	PangoRenderer* renderer = pango_gl_font_map_get_renderer (PANGO_GL_FONT_MAP (fontmap));
#endif

	//------------------------------

	/*
	WfColourFloat cf;
	colour_rgba_to_float(&cf, colour);
	Colour32 c32 = {MIN(0xffU, cf.r * 256.0), MIN(0xffU, cf.g * 256.0), MIN(0xffU, cf.b * 256.0), colour & 0xffU};
	*/
	//pango_renderer_draw_layout (renderer, layout, 10 * PANGO_SCALE, -20 * PANGO_SCALE);
	pango_gl_render_layout (layout, x, y, z, (Colour32*)&colour, 0);
	//pango_gl_render_layout (layout, x, y, z, &c32, 0);

#ifdef TEST
	//prints the whole texture with all glyphs.
	pango_gl_debug_textures();
#endif

	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0); //reset back to the default value
}


static gboolean
font_is_scalable(PangoContext* context, const char* font_name)
{
	//scalable fonts dont list sizes, so if the font has a size list, we assume it is not scalable.
	//TODO surely there is a better way to find a font than iterating over every system font?

	gboolean scalable = TRUE;

	gchar* family_name = g_ascii_strdown(font_name, -1);
	dbg(2, "looking for: %s", family_name);

	PangoFontMap* fontmap = pango_context_get_font_map(context);
	PangoFontFamily** families = NULL;
	int n_families = 0;
	pango_font_map_list_families(fontmap, &families, &n_families);
	int i; for(i=0;i<n_families;i++){
		PangoFontFamily* family = families[i];
		//dbg(0, "family=%s", pango_font_family_get_name(family));
		if(!strcmp(family_name, pango_font_family_get_name(family))){
			PangoFontFace** faces;
			int n_faces;
			pango_font_family_list_faces(family, &faces, &n_faces);
			int j; for(j=0;j<n_faces;j++){
				PangoFontFace* face = faces[j];
				dbg(3, " %s", pango_font_face_get_face_name(face));
				int* sizes = NULL;
				int n_sizes = 0;
				pango_font_face_list_sizes(face, &sizes, &n_sizes);
				if(n_sizes){
					int i; for(i=0;i<n_sizes;i++){
						scalable = FALSE;
						break;
					}
					g_free(sizes);
				}
			}
			if(faces) g_free(faces);
		}
	}
	g_free(family_name);
	dbg(2, "scalable=%i", scalable);
	return scalable;
}


