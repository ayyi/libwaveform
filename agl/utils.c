/*
  copyright (C) 2013-2019 Tim Orford <tim@orford.org>

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
#define __agl_utils_c__
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
#define g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

static gulong __enable_flags = 0;
#define GL_ENABLE_ALPHA_TEST   (1<<3)
#define GL_ENABLE_TEXTURE_RECT (1<<4)

#define TEXTURE_UNIT 0 // GL_TEXTURE0

int _program = 0;
GLenum _wf_ge = 0;

static gboolean font_is_scalable (PangoContext*, const char* font_name);

static void  _alphamap_set_uniforms();
static AGlUniformInfo uniforms[] = {
   {"tex2d", 1, GL_INT, {TEXTURE_UNIT, 0, 0, 0}, -1},
   END_OF_UNIFORMS
};
AlphaMapShader alphamap = {{NULL, NULL, 0, uniforms, _alphamap_set_uniforms, &alpha_map_text}};
static void
_alphamap_set_uniforms()
{
	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(alphamap.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(alphamap.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(alphamap.shader.program, "fg_colour"), 1, fg_colour);
}

//plain 2d texture
static void _tex_set_uniforms();
static AlphaMapShader tex2d = {{NULL, NULL, 0, uniforms, _tex_set_uniforms, &texture_2d_text}};
//TODO if we pass the shader as arg we can reuse
static void
_tex_set_uniforms()
{
	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(tex2d.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(tex2d.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(tex2d.shader.program, "fg_colour"), 1, fg_colour);
}

//plain colour shader
static void _plain_set_uniforms();
PlainShader plain = {{NULL, NULL, 0, NULL, _plain_set_uniforms, &plain_colour_text}, {0xff000077}};
static void
_plain_set_uniforms()
{
	float colour[4] = {0.0, 0.0, 0.0, ((float)(plain.uniform.colour & 0xff)) / 0x100};
	agl_rgba_to_float(plain.uniform.colour, &colour[0], &colour[1], &colour[2]);
	glUniform4fv(glGetUniformLocation(plain.shader.program, "colour"), 1, colour);
}


static AGl* agl = NULL;

AGl*
agl_get_instance()
{
	if(!agl){
		agl = g_new0(AGl, 1);
		agl->pref_use_shaders = TRUE;
		agl->use_shaders = FALSE;        // not set until we an have active gl context based on the value of pref_use_shaders.
		agl->shaders.alphamap = &alphamap;
		agl->shaders.texture = &tex2d;
		agl->shaders.plain = &plain;
	}
	return agl;
}


#ifdef USE_GTK
static GdkGLContext* share_list = 0;
#endif

void
agl_free ()
{
#ifdef USE_GTK
	// remove the reference that was added by gdk_gl_context_new
	g_object_unref0(share_list);
#endif
}


/*   Returns a global GdkGLContext that can be used to share
 *   OpenGL display lists between multiple drawables with
 *   dynamic lifetimes.
 */
#ifdef USE_GTK
GdkGLContext*
agl_get_gl_context()
{
	if(!share_list){
		GdkGLConfig* const config = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_DEPTH);
		GdkPixmap* const pixmap = gdk_pixmap_new(0, 8, 8, gdk_gl_config_get_depth(config));
		gdk_pixmap_set_gl_capability(pixmap, config, 0);
		share_list = gdk_gl_context_new(gdk_pixmap_get_gl_drawable(pixmap), 0, TRUE, GDK_GL_RGBA_TYPE);
	}

	return share_list;
}
#endif


void
agl_enable (gulong flags)
{
  /* This function essentially caches glEnable state() in the
   * hope of lessening number GL traffic.
  */
  if (flags & AGL_ENABLE_BLEND)
    {
      if (!(__enable_flags & AGL_ENABLE_BLEND))
        {
          glEnable (GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
      __enable_flags |= AGL_ENABLE_BLEND;
    }
  else if (__enable_flags & AGL_ENABLE_BLEND)
    {
      glDisable (GL_BLEND);
      __enable_flags &= ~AGL_ENABLE_BLEND;
    }

  if (flags & AGL_ENABLE_TEXTURE_2D)
    {
      if (!(__enable_flags & AGL_ENABLE_TEXTURE_2D))
        glEnable (GL_TEXTURE_2D);
      __enable_flags |= AGL_ENABLE_TEXTURE_2D;
    }
  else if (__enable_flags & AGL_ENABLE_TEXTURE_2D)
    {
      glDisable (GL_TEXTURE_2D);
      __enable_flags &= ~AGL_ENABLE_TEXTURE_2D;
    }

																			#if 0
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
																			#endif

#if 0
  if (__enable_flags & AGL_ENABLE_BLEND)      dbg(0, "blend is enabled.");
  if (__enable_flags & AGL_ENABLE_TEXTURE_2D) dbg(0, "texture is enabled.");
  if (__enable_flags & AGL_ENABLE_ALPHA_TEST) dbg(0, "alpha_test is enabled."); else dbg(0, "alpha_test is NOT enabled.");
#endif
}


GLboolean
agl_shaders_supported()
{
	agl_get_instance();

	const char* version = (const char*)glGetString(GL_VERSION);

	if(!version){
		gwarn("cannot get gl version. incorrect mode?");
		goto no_shaders;
	}

	if ((version[0] >= '2' || version[0] <= '4') && version[1] == '.') {

		// some hardware cannot support shaders and software fallbacks are too slow
		if(g_strrstr((char*)glGetString(GL_RENDERER), "Intel") && g_strrstr((char*)glGetString(GL_RENDERER), "945")){
			goto no_shaders;
		}

		agl->use_shaders = TRUE;
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
#endif

  no_shaders:
	agl->use_shaders = FALSE;
	return GL_FALSE;
}


void
agl_gl_init()
{
	static gboolean done = FALSE;
	if(done++) return;

	agl_get_instance();
	agl_get_extensions();

	if(agl->pref_use_shaders && !agl_shaders_supported()){
		printf("gl shaders not supported. expect reduced functionality.\n");
		//agl_use_program(NULL);
		//wfc->use_1d_textures = false;
	}
	AGL_DEBUG printf("GL_RENDERER = %s\n", (const char*)glGetString(GL_RENDERER));

	int version = 0;
	const char* _version = (const char*)glGetString(GL_VERSION);
	if(_version){
		gchar** split = g_strsplit(_version, ".", 2);
		if(split){
			version = atoi(split[0]);
			dbg(1, "gl_version=%i", version);
			g_strfreev(split);
		}
	}

	// npot textures are mandatory for opengl 2.0
	// npot capability also means non-square textures are supported.
	// some older hardware (eg radeon x1600) may not have full support, and may drop back to software rendering if certain features are used.
	if(GL_ARB_texture_non_power_of_two || version > 1){
		AGL_DEBUG printf("non_power_of_two textures are available.\n");
		agl->have |= AGL_HAVE_NPOT_TEXTURES;
	}else{
		AGL_DEBUG {
			fprintf(stderr, "GL_ARB_texture_non_power_of_two extension is not available!\n");
			fprintf(stderr, "Framebuffer effects will be lower resolution (lower quality).\n\n");
		}
	}

	if(agl->xvinfo){
		int value;
#define _GET_CONFIG(__attrib) glXGetConfig (agl->xdisplay, agl->xvinfo, __attrib, &value)

		// Has stencil buffer?
		_GET_CONFIG (GLX_STENCIL_SIZE);
		if(value) agl->have |= AGL_HAVE_STENCIL;
	}

#if 0
	// just testing. there is probably a better test.
	if(glBindVertexArrayAPPLE){
		if(wf_debug) printf("vertex arrays available.\n");
	}else{
		fprintf(stderr, "vertex arrays not available!\n");
	}
#endif

	if(agl->use_shaders){
		agl_create_program(&alphamap.shader);
		agl_create_program(&tex2d.shader);
		agl_create_program(&plain.shader);

		agl->shaders.text = &alphamap;
	}
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
		return 0;
	}

	GLuint program = sh->program = agl_link_shaders(vert_shader, frag_shader);
	dbg(2, "%u %u program=%u", vert_shader, frag_shader, program);

	glUseProgram(program);
	_program = program;

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
		g_error("problem compiling shader: '%s'\n", log);
		return 0;
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
   if (n > 0) {
      buffer[n] = 0;
      shader = agl_compile_shader_text(shaderType, buffer);
   }

   fclose(f);
   free(buffer);

   return shader;
}


void
agl_uniforms_init(GLuint program, AGlUniformInfo uniforms[])
{
	GLuint i;
	dbg(1, "program=%u", program);
	if(!uniforms) return;

	for (i = 0; uniforms[i].name; i++) {
		uniforms[i].location = glGetUniformLocation(program, uniforms[i].name);
		// note zero is a valid location number.
		if(uniforms[i].location < 0) gwarn("%s: location=%i", uniforms[i].name, uniforms[i].location);

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
agl_colour_rbga(uint32_t colour)
{
	float r = (colour & 0xff000000) >> 24;
	float g = (colour & 0x00ff0000) >> 16;
	float b = (colour & 0x0000ff00) >>  8;
	float a = (colour & 0x000000ff);

	glColor4f(r / 256.0, g / 256.0, b / 256.0, a / 256.0);
}


void
agl_bg_colour_rbga(uint32_t colour)
{
	float r = (colour & 0xff000000) >> 24;
	float g = (colour & 0x00ff0000) >> 16;
	float b = (colour & 0x0000ff00) >>  8;
	float a = (colour & 0x000000ff);

	glClearColor(r / 256.0, g / 256.0, b / 256.0, a / 256.0);
}


void
agl_rect(float x, float y, float w, float h)
{
	glRectf(x, y, x + w, y + h);
}


void
agl_rect_(AGlRect r)
{
	glRectf(r.x, r.y, r.x + r.w, r.y + r.h);
}


void
agl_irect(int x, int y, int w, int h)
{
	glRecti(x, y, x + w, y + h);
}


void
agl_textured_rect(guint texture, float x, float y, float w, float h, AGlQuad* _t)
{
	// to use the whole texture, pass NULL for _t

	agl_use_texture(texture);

	AGlQuad t = _t ? *_t : (AGlQuad){0.0, 0.0, 1.0, 1.0};

	glBegin(GL_QUADS);
	glTexCoord2d(t.x0, t.y0); glVertex2d(x,     y);
	glTexCoord2d(t.x1, t.y0); glVertex2d(x + w, y);
	glTexCoord2d(t.x1, t.y1); glVertex2d(x + w, y + h);
	glTexCoord2d(t.x0, t.y1); glVertex2d(x,     y + h);
	glEnd();
}


// TODO api to be reviewed (see similar fn above)
void
agl_texture_box(guint texture, uint32_t colour, double x, double y, double width, double height)
{
	if(agl->use_shaders){
		agl->shaders.texture->uniform.fg_colour = colour;
		agl_use_program((AGlShader*)agl->shaders.texture);
	}else{
		glColor4f(1.0, 1.0, 1.0, 1.0);
	}

	agl_use_texture(texture);

	glBegin(GL_QUADS);
	double top = y;
	double bot = y + height;
	double x1 = x;
	double x2 = x + width;
	glTexCoord2d(1.0, 1.0); glVertex2d(x1, top);
	glTexCoord2d(0.0, 1.0); glVertex2d(x2, top);
	glTexCoord2d(0.0, 0.0); glVertex2d(x2, bot);
	glTexCoord2d(1.0, 0.0); glVertex2d(x1, bot);
	glEnd();
}


/*
 *   Box outline
 *   The dimensions specify the outer size.
 */
void
agl_box(int s, float x, float y, float w, float h)
{
	agl_rect(x,         y,         w, s        ); // top
	agl_rect(x,         y + s,     s, h - 2 * s); // left
	agl_rect(x,         y + h - s, w, s        ); // bottom
	agl_rect(x + w - s, y + s,     s, h - 2 * s); // right
}


static PangoFontDescription* font_desc = NULL;
static gboolean renderer_inited = FALSE;

static PangoContext*
get_context()
{
	PangoGlRendererClass* renderer_init()
	{
		g_type_class_unref (g_type_class_ref (PANGO_TYPE_GL_RENDERER));

		PangoGlRendererClass* PGRC = g_type_class_peek(PANGO_TYPE_GL_RENDERER);

		if(PGRC && !PGRC->context){
			PangoFontMap* fontmap = pango_gl_font_map_new();
			//pango_gl_font_map_set_resolution (PANGO_GL_FONT_MAP(fontmap), 96.0);
			PGRC->context = pango_gl_font_map_create_context(PANGO_GL_FONT_MAP(fontmap));

			renderer_inited = TRUE;
		}

		return PGRC;
	}

	PangoGlRendererClass* PGRC;
	if(!renderer_inited) g_return_val_if_fail((PGRC = renderer_init()), NULL);
	else PGRC = g_type_class_peek(PANGO_TYPE_GL_RENDERER);
	return PGRC ? PGRC->context : NULL;
}


void
agl_set_font(char* family, int size, PangoWeight weight)
{
	PangoContext* context = get_context();

	if(font_desc) pango_font_description_free(font_desc);

	font_desc = pango_font_description_new();
	pango_font_description_set_family(font_desc, family);
	pango_font_description_set_size(font_desc, size * PANGO_SCALE);
	pango_font_description_set_weight(font_desc, weight);

	// for some reason there seems to be an issue with pixmap fonts
	if(!font_is_scalable(context, pango_font_description_get_family(font_desc))){
		pango_font_description_set_family(font_desc, family);
	}
}


void
agl_set_font_string(char* font_string)
{
	if(font_desc) pango_font_description_free(font_desc);
	font_desc = pango_font_description_from_string(font_string);

	dbg(2, "%s", font_string);
	// for some reason there seems to be an issue with pixmap fonts
	if(!font_is_scalable(get_context(), pango_font_description_get_family(font_desc))){
		pango_font_description_set_family(font_desc, "Sans");
	}
}


/*
 *  There is no need for the caller to set GL state.
 *  GL_TEXTURE_2D and GL_BLEND will be automatically enabled.
 */
void
agl_print(int x, int y, double z, uint32_t colour, const char *fmt, ...)
{
	if(!fmt) return;

	va_list args;
	va_start(args, fmt);
	gchar* text = g_strdup_vprintf(fmt, args);
	va_end(args); // text now contains the string.

	PangoGlRendererClass* PGRC = g_type_class_peek(PANGO_TYPE_GL_RENDERER);
#if 0
	if(!PGRC->context){
		PangoFontMap* fontmap = pango_gl_font_map_new();
		//pango_gl_font_map_set_resolution (PANGO_GL_FONT_MAP(fontmap), 96.0);
		PGRC->context = pango_gl_font_map_create_context(PANGO_GL_FONT_MAP(fontmap));
	}
#endif

	PangoLayout* layout = pango_layout_new (PGRC->context);
	pango_layout_set_text (layout, text, -1);
	g_free(text);

#if 0
	if(!font_desc){
		char font_string[64];
		get_font_string(font_string, -3);
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

	//pango_renderer_draw_layout (renderer, layout, 10 * PANGO_SCALE, -20 * PANGO_SCALE);
	pango_gl_render_layout (layout, x, y, z, (Colour32*)&colour, 0);
	g_object_unref(layout);

#ifdef TEST
	//prints the whole texture with all glyphs.
	pango_gl_debug_textures();
#endif

	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0); //reset back to the default value
}


/*
 *  A variation of agl_print that updates the y position after printing
 */
void
agl_print_with_cursor (int x, int* y, double z, uint32_t colour, const char* fmt, ...)
{
	if(!fmt) return;

	va_list args;
	va_start(args, fmt);
	gchar* text = g_strdup_vprintf(fmt, args);
	va_end(args); // text now contains the string.

	PangoGlRendererClass* PGRC = g_type_class_peek(PANGO_TYPE_GL_RENDERER);

	PangoLayout* layout = pango_layout_new (PGRC->context);
	pango_layout_set_text (layout, text, -1);
	g_free(text);

	agl_print_layout(x, *y, z, colour, layout);

	PangoRectangle irect = {0,}, lrect = {0,};
	pango_layout_get_pixel_extents (layout, &irect, &lrect);

	g_object_unref(layout);

	*y += irect.height;
}


void
agl_print_layout(int x, int y, double z, uint32_t colour, PangoLayout* layout)
{
	pango_layout_set_font_description(layout, font_desc);

	pango_gl_render_layout (layout, x, y, z, (Colour32*)&colour, 0);

	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0); //reset back to the default value
}


/**
 *  x, and y specify the top left corner of the background box.
 */
void
agl_print_with_background(int x, int y, double z, uint32_t colour, uint32_t bg_colour, const char *fmt, ...)
{
	#define PADDING 2

	if(!fmt) return;

	va_list args;
	va_start(args, fmt);
	gchar* text = g_strdup_vprintf(fmt, args);
	va_end(args); // text now contains the string.

	PangoGlRendererClass* PGRC = g_type_class_peek(PANGO_TYPE_GL_RENDERER);

	PangoLayout* layout = pango_layout_new (PGRC->context);
	pango_layout_set_text (layout, text, -1);

	pango_layout_set_font_description(layout, font_desc);

	PangoRectangle ink_rect, logical_rect;
	pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);
#if 0
	int text_width = ink_rect.width;
	int text_height = ink_rect.height;
	printf("   %i x %i  %i x %i\n", ink_rect.x, ink_rect.y, text_width, text_height);
#endif

	agl->shaders.plain->uniform.colour = bg_colour;
	agl_use_program((AGlShader*)agl->shaders.plain);
	agl_rect(x, y, ink_rect.width + 2 * PADDING, ink_rect.height + 2 * PADDING);

	pango_gl_render_layout (layout, x + PADDING, y + PADDING - ink_rect.y, z, (Colour32*)&colour, 0);
	g_object_unref(layout);
	g_free(text);

	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0); //reset back to the default value
}


static gboolean
font_is_scalable(PangoContext* context, const char* font_name)
{
	//scalable fonts dont list sizes, so if the font has a size list, we assume it is not scalable.
	//TODO surely there is a better way to find a font than iterating over every system font?

	g_return_val_if_fail(context, FALSE);

	gboolean scalable = TRUE;

	gchar* family_name = g_ascii_strdown(font_name, -1);
	dbg(3, "looking for: %s", family_name);

	PangoFontMap* fontmap = pango_context_get_font_map(context);
	PangoFontFamily** families = NULL;
	int n_families = 0;
	pango_font_map_list_families(fontmap, &families, &n_families);
	int i; for(i=0;i<n_families;i++){
		PangoFontFamily* family = families[i];
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
	g_free(families);
	g_free(family_name);
	dbg(3, "scalable=%i", scalable);
	return scalable;
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

	//it remains to be seen whether automatic setting of uniforms gives enough control.
	if(shader && shader->set_uniforms_) shader->set_uniforms_();
}


void
agl_use_texture(GLuint texture)
{
	//note: 2d texture

#if 1
	agl_enable(AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);
	/*
    if(!(__enable_flags & AGL_ENABLE_BLEND)){
		agl_enable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	*/
#else
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

	glBindTexture(GL_TEXTURE_2D, texture);
}


void
agl_enable_stencil(float x, float y, float w, float h)
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

	// draw only where stencil's value is 1
	glStencilFunc(GL_EQUAL, 1, 0xFF);
}


void
agl_disable_stencil()
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
	dbg (3, "%i -> %i", orig, 1 << i);
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


void
agl_print_stack_depths()
{
	GLint depth, max;
	printf("stack depths:\n");
	glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &depth);
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &max);
	printf("    ModelView:  %i / %i\n", depth, max);
	glGetIntegerv(GL_PROJECTION_STACK_DEPTH , &depth);
	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &max);
	printf("    Projection: %i / %i\n", depth, max);
	glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &depth);
	glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &max);
	printf("    Attribute:  %i / %i\n", depth, max);
}


void
agl_rgba_to_float(uint32_t rgba, float* r, float* g, float* b)
{
	double _r = (rgba & 0xff000000) >> 24;
	double _g = (rgba & 0x00ff0000) >> 16;
	double _b = (rgba & 0x0000ff00) >>  8;

	*r = _r / 0xff;
	*g = _g / 0xff;
	*b = _b / 0xff;
	dbg (3, "%08x --> %.2f %.2f %.2f", rgba, *r, *g, *b);
}


static AGlTextureUnit* active_texture_unit = NULL;


AGlTextureUnit*
agl_texture_unit_new(GLenum unit)
{
	AGlTextureUnit* texture_unit = g_new0(AGlTextureUnit, 1);
	texture_unit->unit = unit;
	return texture_unit;
}


void
agl_texture_unit_use_texture(AGlTextureUnit* unit, int texture)
{
	g_return_if_fail(unit);

	if(TRUE || active_texture_unit != unit){
		active_texture_unit = unit;
		glActiveTexture(unit->unit);
	}
	glBindTexture(GL_TEXTURE_1D, texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#if 1
	agl_enable(AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);
	/*
	if(!(__enable_flags & AGL_ENABLE_BLEND)){
		agl_enable(GL_BLEND);
	}
	*/
#else
	glEnable(GL_BLEND);
#endif
}


