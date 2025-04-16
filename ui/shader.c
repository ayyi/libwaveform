/*
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. https://www.ayyi.org          |
* | copyright (C) 2012-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

#define __wf_shader_c__
#define __wf_private__
#define __wf_canvas_priv__

#include "config.h"
#include "agl/ext.h"
#include "agl/utils.h"
#include "wf/waveform.h"
#include "waveform/shader.h"

#include "shaders/shaders.c"

#if 0
static void  _peak_shader_set_uniforms (float peaks_per_pixel, float top, float bottom, uint32_t _fg_colour, int n_channels);
static void  _peak_nonscaling_set_uniforms ();
#endif
static void  _hires_ng_set_uniforms    (AGlShader*);
#if 0
static void  _vertical_set_uniforms    ();
static void  _horizontal_set_uniforms  ();
#endif
static void  _ass_set_uniforms         (AGlShader*);
static void  _ruler_set_uniforms       (AGlShader*);
static void  _lines_set_uniforms       (AGlShader*);
static void  _cursor_set_uniforms      (AGlShader*);

#if 0
static AGlUniformInfo uniforms[] = {
   {"tex1d",     1, GL_INT,   { 1, 0, 0, 0 }, -1}, // LHS +ve - 0 corresponds to glActiveTexture(WF_TEXTURE0);
   {"tex1d_neg", 1, GL_INT,   { 2, 0, 0, 0 }, -1}, // LHS -ve - 1 corresponds to glActiveTexture(WF_TEXTURE1);
   {"tex1d_3",   1, GL_INT,   { 3, 0, 0, 0 }, -1}, // RHS +ve WF_TEXTURE2
   {"tex1d_4",   1, GL_INT,   { 4, 0, 0, 0 }, -1}, // RHS -ve WF_TEXTURE3
   END_OF_UNIFORMS
};
PeakShader peak_shader = {{NULL, NULL, 0, uniforms, NULL, &peak_text}, _peak_shader_set_uniforms};

#ifdef USE_FBO
static AGlUniformInfo uniforms1[] = {
   {"tex1d",     1, GL_INT,   { 1, 0, 0, 0 }, -1}, // LHS +ve - 0 corresponds to glActiveTexture(WF_TEXTURE0);
   {"tex1d_0neg",1, GL_INT,   { 2, 0, 0, 0 }, -1}, // LHS -ve - 1 corresponds to glActiveTexture(WF_TEXTURE1);
   {"tex1d_3",   1, GL_INT,   { 3, 0, 0, 0 }, -1}, // RHS +ve WF_TEXTURE2
   {"tex1d_4",   1, GL_INT,   { 4, 0, 0, 0 }, -1}, // RHS -ve WF_TEXTURE3
   END_OF_UNIFORMS
};
PeakShader peak_nonscaling = {{NULL, NULL, 0, uniforms1, _peak_nonscaling_set_uniforms, &peak_nonscaling_text}};
#endif
#endif

#if 0
static AGlUniformInfo uniforms_hr[] = {
   {"tex1d",     1, GL_INT,   { 1, 0, 0, 0 }, -1},
   {"tex1d_neg", 1, GL_INT,   { 2, 0, 0, 0 }, -1},
   {"tex1d_3",   1, GL_INT,   { 3, 0, 0, 0 }, -1},
   {"tex1d_4",   1, GL_INT,   { 4, 0, 0, 0 }, -1},
   END_OF_UNIFORMS
};
HiResShader hires_shader = {{NULL, NULL, 0, uniforms_hr, _hires_set_uniforms, &hires_text}};
#endif

static AGlUniformInfo uniforms_hr_ng[] = {
   {"tex2d",     1, GL_INT,   -1, { 0,  }}, // 0 corresponds to glActiveTexture(GL_TEXTURE0);
   {"top",       1, GL_FLOAT, -1, { 0., }},
   {"bottom",    1, GL_FLOAT, -1, { 0., }},
   {"n_channels",1, GL_INT,   -1, { 1,  }},
   {"tex_width", 1, GL_FLOAT, -1, { 0., }},
   {"tex_height",1, GL_FLOAT, -1, { 0., }},
   {"mm_level",  1, GL_INT,   -1, { 0,  }},
   {"v_gain",    1, GL_FLOAT, -1, { 1., }},
   {"fg_colour", 4, GL_FLOAT, -1,        },
   END_OF_UNIFORMS
};
HiResNGShader hires_ng_shader = {{NULL, NULL, 0, uniforms_hr_ng, _hires_ng_set_uniforms, &hires_ng_text}};

#if 0
static AGlUniformInfo uniforms2[] = {
   {"tex2d",     1, GL_INT,   { 0, 0, 0, 0 }, -1}, // 0 corresponds to glActiveTexture(GL_TEXTURE0);
   END_OF_UNIFORMS
};
BloomShader horizontal = {{NULL, NULL, 0, uniforms2, _horizontal_set_uniforms, &horizontal_text}};

static AGlUniformInfo uniforms3[] = {
   {"tex2d",     1, GL_INT,   { 0, 0, 0, 0 }, -1}, // 0 corresponds to glActiveTexture(GL_TEXTURE0);
   END_OF_UNIFORMS
};
BloomShader vertical = {{NULL, NULL, 0, uniforms3, _vertical_set_uniforms, &vertical_text}};
#endif

AssShader ass = {{
   NULL, NULL, 0,
   (AGlUniformInfo[]){
      {"tex2d", 1, GL_INT, -1,},
      END_OF_UNIFORMS
   },
   _ass_set_uniforms, &ass_text
}};


static AGlUniformInfo uniforms5[] = {
   END_OF_UNIFORMS
};
RulerShader ruler = {{.uniforms = uniforms5, _ruler_set_uniforms, &ruler_text}};


CursorShader cursor = {{
	.uniforms = (AGlUniformInfo[]){
   		{"colour", 4, GL_COLOR_ARRAY, -1,},
		END_OF_UNIFORMS
	},
	.set_uniforms_ = _cursor_set_uniforms,
	.text = &cursor_text
}};

#if 0
static void
_peak_shader_set_uniforms(float peaks_per_pixel, float top, float bottom, uint32_t _fg_colour, int n_channels)
{
	//TODO this will generate GL_INVALID_OPERATION if the shader has never been assigned to an object

#if 0
	dbg(1, "peaks_per_pixel=%.2f top=%.2f bottom=%.2f n_channels=%i", peaks_per_pixel, top, bottom, n_channels);
#endif

	AGlShader* shader = &peak_shader.shader;
	GLuint offsetLoc = glGetUniformLocation(shader->program, "peaks_per_pixel");
	glUniform1f(offsetLoc, peaks_per_pixel);

	offsetLoc = glGetUniformLocation(shader->program, "bottom");
	glUniform1f(offsetLoc, bottom);

	glUniform1f(glGetUniformLocation(shader->program, "top"), top);
	glUniform1i(glGetUniformLocation(shader->program, "n_channels"), n_channels);

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(_fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(_fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	gl_warn("");
}
#endif

static AGlUniformInfo uniforms7[] = {
   {"tex", 1, GL_INT, -1,},
   END_OF_UNIFORMS
};
LinesShader lines = {{NULL, NULL, 0, uniforms7, _lines_set_uniforms, &lines_text}};


#if 0
#ifdef USE_FBO
static void
_peak_nonscaling_set_uniforms()
{
	AGlShader* shader = &peak_nonscaling.shader;

	glUniform1i(glGetUniformLocation(shader->program, "n_channels"), ((PeakShader*)shader)->uniform.n_channels);
}
#endif


static void
_hires_set_uniforms()
{
	AGlShader* shader = &hires_shader.shader;
	struct U* u = &((HiResShader*)shader)->uniform;

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(hires_ng_shader.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(hires_shader.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "top"),             u->top);
	glUniform1f(glGetUniformLocation(shader->program, "bottom"),          u->bottom);
	glUniform1i(glGetUniformLocation(shader->program, "n_channels"),      u->n_channels);
	glUniform1f(glGetUniformLocation(shader->program, "peaks_per_pixel"), u->peaks_per_pixel);
}
#endif


static void
_hires_ng_set_uniforms (AGlShader* _shader)
{
	AGlShader* shader = &hires_ng_shader.shader;
	AGlUniformInfo* uniforms = shader->uniforms;

	// uniforms only updated if changed. reduces the number of gl calls but any difference is not readily apparent.
	static float top = 0.0;
	static float bottom = 0.0;
	static uint32_t _fg_colour;
	static int n_channels;
	static int mm_level;
	static float tex_width;
	static float tex_height;

	if(hires_ng_shader.uniform.fg_colour != _fg_colour){
		float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(hires_ng_shader.uniform.fg_colour & 0xff)) / 0x100};
		agl_rgba_to_float(hires_ng_shader.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
		glUniform4fv(uniforms[8].location, 1, fg_colour);
		_fg_colour = hires_ng_shader.uniform.fg_colour;
	}

	if(hires_ng_shader.uniform.top != top){
		glUniform1f(uniforms[1].location,                top = hires_ng_shader.uniform.top);
	}
	if(hires_ng_shader.uniform.bottom != bottom){
		glUniform1f(uniforms[2].location,                bottom = hires_ng_shader.uniform.bottom);
	}
	if(hires_ng_shader.uniform.n_channels != n_channels){
		glUniform1i(uniforms[3].location,                n_channels = hires_ng_shader.uniform.n_channels);
	}
	if(hires_ng_shader.uniform.tex_width != tex_width){
		glUniform1f(uniforms[4].location,                tex_width = hires_ng_shader.uniform.tex_width);
	}
	if(hires_ng_shader.uniform.tex_height != tex_height){
		glUniform1f(uniforms[5].location,                tex_height = hires_ng_shader.uniform.tex_height);
	}
	if(hires_ng_shader.uniform.mm_level != mm_level){
		glUniform1i(uniforms[6].location,                mm_level = hires_ng_shader.uniform.mm_level);
	}

	glUniform1f(uniforms[7].location, hires_ng_shader.uniform.v_gain);
}


#if 0
static void
_vertical_set_uniforms ()
{
	AGlShader* shader = &vertical.shader;
	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(vertical.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(vertical.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "peaks_per_pixel"), ((BloomShader*)shader)->uniform.peaks_per_pixel);
}


static void
_horizontal_set_uniforms ()
{
	AGlShader* shader = &horizontal.shader;
	//dbg(0, "ppp=%.2f", ((BloomShader*)shader)->uniform.peaks_per_pixel);
	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(horizontal.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(horizontal.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "peaks_per_pixel"), ((BloomShader*)shader)->uniform.peaks_per_pixel);
}
#endif


static void
_ass_set_uniforms (AGlShader* shader)
{
	float colour1[4] = {0.0, 0.0, 0.0, ((float)(ass.uniform.colour1 & 0xff)) / 0x100};
	agl_rgba_to_float(ass.uniform.colour1, &colour1[0], &colour1[1], &colour1[2]);
	glUniform4fv(glGetUniformLocation(ass.shader.program, "colour1"), 1, colour1);

	float colour2[4] = {0.0, 0.0, 0.0, ((float)(ass.uniform.colour2 & 0xff)) / 0x100};
	agl_rgba_to_float(ass.uniform.colour2, &colour2[0], &colour2[1], &colour2[2]);
	glUniform4fv(glGetUniformLocation(ass.shader.program, "colour2"), 1, colour2);
}


static void
_ruler_set_uniforms (AGlShader* _shader)
{
	AGlShader* shader = &ruler.shader;

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(ruler.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(ruler.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(ruler.shader.program, "fg_colour"), 1, fg_colour);

																		// TODO  glGetUniformLocation
	glUniform1iv(glGetUniformLocation(shader->program, "markers"), 10, ((RulerShader*)shader)->uniform.markers);
	glUniform1f(glGetUniformLocation(shader->program, "beats_per_pixel"), ((RulerShader*)shader)->uniform.beats_per_pixel);
	glUniform1f(glGetUniformLocation(shader->program, "samples_per_pixel"), ((RulerShader*)shader)->uniform.samples_per_pixel);
	glUniform1f(glGetUniformLocation(shader->program, "viewport_left"), ((RulerShader*)shader)->uniform.viewport_left);
}


static void
_lines_set_uniforms (AGlShader* _shader)
{
	AGlShader* shader = &lines.shader;

	float colour[4] = {0.0, 0.0, 0.0, ((float)(lines.uniform.colour & 0xff)) / 0x100};
	agl_rgba_to_float(lines.uniform.colour, &colour[0], &colour[1], &colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "colour"), 1, colour);

	glUniform1f(glGetUniformLocation(shader->program, "texture_width"), (float)lines.uniform.texture_width);
	glUniform1i(glGetUniformLocation(shader->program, "n_channels"), lines.uniform.n_channels);
}


static void
_cursor_set_uniforms (AGlShader* shader)
{
	//agl_set_colour_uniform (&cursor.shader.uniforms[0], cursor.shader.uniforms[0].value[0]);
	agl_set_uniforms(&cursor.shader);

	glUniform1f(3, (float)cursor.uniform.width);
}


#if 0
void
wf_shaders_init()
{
	// deprecated. actors should call agl_create_program in their init fn.

	agl_create_program(&peak_shader.shader);
#ifdef USE_FBO
	agl_create_program(&peak_nonscaling.shader);
#endif
	agl_create_program(&horizontal.shader);
	agl_create_program(&vertical.shader);
}
#endif


