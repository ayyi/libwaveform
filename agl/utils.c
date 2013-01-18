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
#include "agl/shader.h"
#include "agl/utils.h"

#include "shaders/shaders.c"

extern void wf_debug_printf (const char* func, int level, const char* format, ...); //TODO, perhaps just remove custom debugging messages...
#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define dbg(A, B, ...) wf_debug_printf(__func__, A, B, ##__VA_ARGS__)

static gulong __enable_flags = 0;
#define GL_ENABLE_BLEND        (1<<1)
#define GL_ENABLE_TEXTURE_2D   (1<<2)
#define GL_ENABLE_ALPHA_TEST   (1<<3)
#define GL_ENABLE_TEXTURE_RECT (1<<4)

int _program = 0;
GLenum _wf_ge = 0;

static gboolean font_is_scalable(PangoContext*, const char* font_name);

void  _alphamap_set_uniforms    ();
AGlUniformInfo uniforms4[] = {
   {"tex2d",     1, GL_INT,   { 0, 0, 0, 0 }, -1},
   END_OF_UNIFORMS
};
AlphaMapShader tex2d = {{NULL, NULL, 0, uniforms4, _alphamap_set_uniforms, &alpha_map_text}};
void
_alphamap_set_uniforms()
{
	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(tex2d.uniform.fg_colour & 0xff)) / 0x100};
	wf_rgba_to_float(tex2d.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(tex2d.shader.program, "fg_colour"), 1, fg_colour);
}

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


void
agl_enable (gulong flags)
{
  /* This function essentially caches glEnable state() in the
   * hope of lessening number GL traffic.
  */
  if (flags & GL_ENABLE_BLEND)
    {
      if (!(__enable_flags & GL_ENABLE_BLEND))
        {
          glEnable (GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
      __enable_flags |= GL_ENABLE_BLEND;
    }
  else if (__enable_flags & GL_ENABLE_BLEND)
    {
      glDisable (GL_BLEND);
      __enable_flags &= ~GL_ENABLE_BLEND;
    }

  if (flags & GL_ENABLE_TEXTURE_2D)
    {
      if (!(__enable_flags & GL_ENABLE_TEXTURE_2D))
        glEnable (GL_TEXTURE_2D);
      __enable_flags |= GL_ENABLE_TEXTURE_2D;
    }
  else if (__enable_flags & GL_ENABLE_TEXTURE_2D)
    {
      glDisable (GL_TEXTURE_2D);
      __enable_flags &= ~GL_ENABLE_TEXTURE_2D;
    }

#ifdef GL_TEXTURE_RECTANGLE_ARB
  if (flags & GL_ENABLE_TEXTURE_RECT)
    {
      if (!(__enable_flags & GL_ENABLE_TEXTURE_RECT))
	  glEnable (GL_TEXTURE_RECTANGLE_ARB);
      __enable_flags |= GL_ENABLE_TEXTURE_RECT;
    }
  else if (__enable_flags & GL_ENABLE_TEXTURE_RECT)
    {
      glDisable (GL_TEXTURE_RECTANGLE_ARB);
      __enable_flags &= ~GL_ENABLE_TEXTURE_RECT;
    }
#endif

  if (flags & GL_ENABLE_ALPHA_TEST)
    {
      if (!(__enable_flags & GL_ENABLE_ALPHA_TEST))
        glEnable (GL_ALPHA_TEST);

      __enable_flags |= GL_ENABLE_ALPHA_TEST;
    }
  else if (__enable_flags & GL_ENABLE_ALPHA_TEST)
    {
      glDisable (GL_ALPHA_TEST);
      __enable_flags &= ~GL_ENABLE_ALPHA_TEST;
    }

#if 0
  if (__enable_flags & GL_ENABLE_BLEND)      dbg(0, "blend is enabled.");
  if (__enable_flags & GL_ENABLE_TEXTURE_2D) dbg(0, "texture is enabled.");
  if (__enable_flags & GL_ENABLE_ALPHA_TEST) dbg(0, "alpha_test is enabled."); else dbg(0, "alpha_test is NOT enabled.");
#endif
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


void
agl_shaders_init()
{
	// initialise internal shader programs
	// must not be called until we have and are inside DRAW

	agl_create_program(&tex2d.shader);
	agl->text_shader = &tex2d;
	dbg(2, "text_shader=%i", tex2d.shader.program);
}


GLuint
agl_create_program(AGlShader* sh)
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
//		return 0;
	}

	GLuint program = sh->program = agl_link_shaders(vert_shader, frag_shader);
	dbg(2, "%u %u program=%u", vert_shader, frag_shader, program);

	glUseProgram(program);

	AGlUniformInfo* uniforms = sh->uniforms;
	agl_uniforms_init(program, uniforms);

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
      g_error("problem compiling shader: %s\n", log);
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
	if(!uniforms) return;

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
	static gboolean renderer_inited = FALSE;
	void renderer_init()
	{
		g_type_class_unref (g_type_class_ref (PANGO_TYPE_GL_RENDERER));
		renderer_inited = TRUE;
	}
	if(!renderer_inited) renderer_init();

	if(font_desc) pango_font_description_free(font_desc);
	font_desc = pango_font_description_from_string(font_string);

	PangoGlRendererClass* PGRC = g_type_class_peek(PANGO_TYPE_GL_RENDERER);
	g_return_if_fail(PGRC);

	if(!PGRC->context){
		PangoFontMap* fontmap = pango_gl_font_map_new();
		//pango_gl_font_map_set_resolution (PANGO_GL_FONT_MAP(fontmap), 96.0);
		PGRC->context = pango_gl_font_map_create_context(PANGO_GL_FONT_MAP(fontmap));
	}

	dbg(1, "requested: %s", font_string);
	// for some reason there seems to be an issue with pixmap fonts
	if(!font_is_scalable(PGRC->context, pango_font_description_get_family(font_desc))){
		strcpy(font_string, "Sans 7");
		pango_font_description_free(font_desc);
		font_desc = pango_font_description_from_string(font_string);
	}
	dbg(1, "using: %s", font_string);

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

#if 0
	gboolean first_time = FALSE;
#endif
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


void
wf_canvas_use_program(int program)
{
	//deprecated. use fn below.

	if(agl_get_instance()->use_shaders && (program != _program)){
		dbg(3, "%i", program);
		glUseProgram(_program = program);
	}
}


void
agl_use_program(AGlShader* shader)
{
	int program = shader ? shader->program : 0;

	if(!agl_get_instance()->use_shaders) return;

	if(program != _program){
		dbg(3, "%i", program);
		glUseProgram(_program = program);
	}

	//it remains to be seen whether automatic setting of uniforms gives us enough control.
	if(shader && shader->set_uniforms_) shader->set_uniforms_();
}


void agl_enable_stencil(float x, float y, float w, float h)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glStencilFunc(GL_NEVER, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);  // draw 1s on test fail (always)

	// draw stencil pattern
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);  // needs mask=0xFF

	glPushMatrix();
	glTranslatef(x, y, 0.0f);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(0, h);
	glVertex2f(w, h);
	glVertex2f(w, 0);
	glEnd();
	glPopMatrix();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0x00);

	// draw where stencil's value is 0
	//glStencilFunc(GL_EQUAL, 0, 0xFF);
	/* (nothing to draw) */

	// draw only where stencil's value is 1
	glStencilFunc(GL_EQUAL, 1, 0xFF);
}


void agl_disable_stencil()
{
	glDisable(GL_STENCIL_TEST);
}


int
agl_power_of_two(guint a)
{
	// return the next power of two up from the given value.

	int i = 0;
	int orig = a;
	a = MAX(1, a - 1);
	while(a){
		a = a >> 1;
		i++;
	}
	dbg (2, "%i -> %i", orig, 1 << i);
	return 1 << i;
}


void
agl_print_error(const char* func, int __ge, const char* format, ...)
{
	char str[256];
	char* e_str = NULL;

	switch(__ge) {
		case GL_INVALID_OPERATION:
			e_str = "GL_INVALID_OPERATION";
			break;
		case GL_INVALID_VALUE:
			e_str = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_ENUM:
			e_str = "GL_INVALID_ENUM";
			break;
		case GL_STACK_OVERFLOW:
			e_str = "GL_STACK_OVERFLOW ";
			break;
		case GL_OUT_OF_MEMORY:
			e_str = "GL_OUT_OF_MEMORY";
			break;
		case GL_STACK_UNDERFLOW:
			e_str = "GL_STACK_UNDERFLOW";
			break;
		case GL_NO_ERROR:
			e_str = "GL_NO_ERROR";
			break;
		default:
			fprintf(stderr, "%i ", __ge); //TODO
			break;
	}

    va_list args;
    va_start(args, format);
	vsprintf(str, format, args);
    va_end(args);

	g_warning("%s(): %s %s", func, e_str, str);
}

