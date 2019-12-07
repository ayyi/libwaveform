/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2013-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __agl_utils_h__
#define __agl_utils_h__
#include <GL/glx.h>
#if defined(USE_GTK) || defined(__GTK_H__)
#include <gdk/gdkgl.h>
#include <gtk/gtkgl.h>
#endif
#include <pango/pango.h>
#include "agl/typedefs.h"
#include "agl/material.h"

typedef struct _agl               AGl;
typedef struct _AGlUniformInfo    AGlUniformInfo;

typedef struct _agl_shader_text
{
	char*        vert;
	char*        frag;
} AGlShaderText;

struct _AGlShader
{
	char*           vertex_file;
	char*           fragment_file;
	uint32_t        program;       // compiled program
	AGlUniformInfo* uniforms;
	void            (*set_uniforms_)();
	AGlShaderText*  text;
};

struct _AGlUniformInfo
{
   const char* name;
   GLuint      size;
   GLenum      type;      // GL_FLOAT or GL_INT
   GLfloat     value[4];
   GLint       location;  // filled in by agl_uniforms_init()
};

struct _texture_unit
{
   GLenum      unit;
   int         texture;
};

AGl*            agl_get_instance             ();
#if defined(USE_GTK) || defined(__GTK_H__)
GdkGLContext*   agl_get_gl_context           ();
#endif
void      agl_enable              (gulong flags);
void      agl_gl_init             ();
void      agl_free                ();
GLboolean agl_shaders_supported   ();
GLuint    agl_create_program      (AGlShader*);
GLuint    agl_compile_shader_text (GLenum shaderType, const char* text);
GLuint    agl_compile_shader_file (GLenum shaderType, const char* filename);
void      agl_uniforms_init       (GLuint program, AGlUniformInfo uniforms[]);
GLuint    agl_link_shaders        (GLuint vertShader, GLuint fragShader);
void      agl_use_program         (AGlShader*);
void      agl_use_texture         (GLuint texture);

AGlTextureUnit* agl_texture_unit_new         (GLenum unit);
void            agl_texture_unit_use_texture (AGlTextureUnit*, int texture);

void      agl_colour_rbga         (uint32_t);
void      agl_bg_colour_rbga      (uint32_t);

void      agl_rect                (float x, float y, float w, float h);
void      agl_rect_               (AGlRect);
void      agl_irect               (int x, int y, int w, int h);
void      agl_textured_rect       (guint texture, float x, float y, float w, float h, AGlQuad* tex_rect);
void      agl_texture_box         (guint texture, uint32_t colour, double x, double y, double w, double h); // to be reviewed
void      agl_box                 (int line_width, float x, float y, float w, float h);

void      agl_enable_stencil      (float x, float y, float w, float h);
void      agl_disable_stencil     ();
void      agl_print_error         (const char* func, int err, const char* format, ...);
void      agl_print_stack_depths  ();

void      agl_set_font            (char* family, int size, PangoWeight);
void      agl_set_font_string     (char* font_string);
void      agl_print               (int x, int y, double z, uint32_t colour, const char* fmt, ...);
void      agl_print_layout        (int x, int y, double z, uint32_t colour, PangoLayout*);
void      agl_print_with_cursor   (int x, int* y, double z, uint32_t colour, const char* fmt, ...);
void      agl_print_with_background(int x, int y, double z, uint32_t colour, uint32_t bg_colour, const char* fmt, ...);

int       agl_power_of_two        (guint);
void      agl_rgba_to_float       (uint32_t rgba, float* r, float* g, float* b);


typedef enum
{
	AGL_HAVE_NPOT_TEXTURES = 1,
	AGL_HAVE_STENCIL = 2,
} AGlHave;

struct _agl
{
	gboolean        pref_use_shaders;

	XVisualInfo*    xvinfo;
	Display*        xdisplay;

	gboolean        use_shaders;
	AGlHave         have;

	struct {
		AlphaMapShader* texture;
		AlphaMapShader* alphamap;
		PlainShader*    plain;
		AlphaMapShader* text;
	}               shaders;
	AGlMaterial*    aaline;

	int             debug;
};

#define END_OF_UNIFORMS   { NULL, 0, GL_NONE, { 0, 0, 0, 0 }, -1 }

extern GLenum _wf_ge;
#define gl_error ((_wf_ge = glGetError()) != GL_NO_ERROR)
#define gl_warn(A, ...) { \
		if(gl_error){ \
		agl_print_error(__func__, _wf_ge, A, ##__VA_ARGS__); \
	}}

#define AGL_NEW(T, ...) ({T* obj = g_new0(T, 1); *obj = (T){__VA_ARGS__}; obj;})

#ifdef DEBUG
#define AGL_DEBUG if(agl->debug)
#else
#define AGL_DEBUG if(false)
#endif

#endif
