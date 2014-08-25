/*
  copyright (C) 2012-2014 Tim Orford <tim@orford.org>

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
#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "agl/ext.h"
#include "agl/utils.h"
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/gl_utils.h"
#include "waveform/canvas.h"
#include "waveform/shader.h"

#include "shaders/shaders.c"

static void  _peak_shader_set_uniforms (float peaks_per_pixel, float top, float bottom, uint32_t _fg_colour, int n_channels);
static void  _peak_nonscaling_set_uniforms ();
static void  _hires_set_uniforms       ();
static void  _hires_ng_set_uniforms    ();
static void  _vertical_set_uniforms    ();
static void  _horizontal_set_uniforms  ();
static void  _ass_set_uniforms         ();
static void  _ruler_set_uniforms       ();
static void  _lines_set_uniforms       ();

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

static AGlUniformInfo uniforms_hr[] = {
   {"tex1d",     1, GL_INT,   { 1, 0, 0, 0 }, -1},
   {"tex1d_neg", 1, GL_INT,   { 2, 0, 0, 0 }, -1},
   {"tex1d_3",   1, GL_INT,   { 3, 0, 0, 0 }, -1},
   {"tex1d_4",   1, GL_INT,   { 4, 0, 0, 0 }, -1},
   END_OF_UNIFORMS
};
HiResShader hires_shader = {{NULL, NULL, 0, uniforms_hr, _hires_set_uniforms, &hires_text}};

static AGlUniformInfo uniforms_hr_ng[] = {
   {"tex2d",     1, GL_INT,   { 0, 0, 0, 0 }, -1}, // 0 corresponds to glActiveTexture(GL_TEXTURE0);
   END_OF_UNIFORMS
};
HiResNGShader hires_ng_shader = {{NULL, NULL, 0, uniforms_hr_ng, _hires_ng_set_uniforms, &hires_ng_text}};

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

extern AlphaMapShader tex2d;

static AGlUniformInfo uniforms6[] = {
   {"tex2d",     1, GL_INT,   { 0, 0, 0, 0 }, -1},
   END_OF_UNIFORMS
};
AssShader ass = {{NULL, NULL, 0, uniforms6, _ass_set_uniforms, &ass_text}};

static AGlUniformInfo uniforms5[] = {
   END_OF_UNIFORMS
};
RulerShader ruler = {{NULL, NULL, 0, uniforms5, _ruler_set_uniforms, &ruler_text}};

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

static AGlUniformInfo uniforms7[] = {
   {"tex",     1, GL_INT,   { 0, 0, 0, 0 }, -1},
   END_OF_UNIFORMS
};
LinesShader lines = {{NULL, NULL, 0, uniforms7, _lines_set_uniforms, &lines_text}};


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

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(hires_shader.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(hires_shader.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "top"),             u->top);
	glUniform1f(glGetUniformLocation(shader->program, "bottom"),          u->bottom);
	glUniform1i(glGetUniformLocation(shader->program, "n_channels"),      u->n_channels);
	glUniform1f(glGetUniformLocation(shader->program, "peaks_per_pixel"), u->peaks_per_pixel);
}


static void
_hires_ng_set_uniforms()
{
	AGlShader* shader = &hires_ng_shader.shader;

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(hires_ng_shader.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(hires_ng_shader.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "top"),             hires_ng_shader.uniform.top);
	glUniform1f(glGetUniformLocation(shader->program, "bottom"),          hires_ng_shader.uniform.bottom);
	glUniform1i(glGetUniformLocation(shader->program, "n_channels"),      hires_ng_shader.uniform.n_channels);
	glUniform1f(glGetUniformLocation(shader->program, "tex_width"),       hires_ng_shader.uniform.tex_width);
	glUniform1f(glGetUniformLocation(shader->program, "tex_height"),      hires_ng_shader.uniform.tex_height);
	glUniform1i(glGetUniformLocation(shader->program, "mm_level"),        hires_ng_shader.uniform.mm_level);
}


static void
_vertical_set_uniforms()
{
	AGlShader* shader = &vertical.shader;
	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(vertical.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(vertical.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "peaks_per_pixel"), ((BloomShader*)shader)->uniform.peaks_per_pixel);
}


static void
_horizontal_set_uniforms()
{
	AGlShader* shader = &horizontal.shader;
	//dbg(0, "ppp=%.2f", ((BloomShader*)shader)->uniform.peaks_per_pixel);
	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(horizontal.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(horizontal.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "peaks_per_pixel"), ((BloomShader*)shader)->uniform.peaks_per_pixel);
}


static void
_ass_set_uniforms()
{
	float colour1[4] = {0.0, 0.0, 0.0, ((float)(ass.uniform.colour1 & 0xff)) / 0x100};
	agl_rgba_to_float(ass.uniform.colour1, &colour1[0], &colour1[1], &colour1[2]);
	glUniform4fv(glGetUniformLocation(ass.shader.program, "colour1"), 1, colour1);

	float colour2[4] = {0.0, 0.0, 0.0, ((float)(ass.uniform.colour2 & 0xff)) / 0x100};
	agl_rgba_to_float(ass.uniform.colour2, &colour2[0], &colour2[1], &colour2[2]);
	glUniform4fv(glGetUniformLocation(ass.shader.program, "colour2"), 1, colour2);
}


static void
_ruler_set_uniforms()
{
	AGlShader* shader = &ruler.shader;

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(ruler.uniform.fg_colour & 0xff)) / 0x100};
	agl_rgba_to_float(ruler.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(ruler.shader.program, "fg_colour"), 1, fg_colour);

	glUniform1iv(glGetUniformLocation(shader->program, "markers"), 10, ((RulerShader*)shader)->uniform.markers);
	glUniform1f(glGetUniformLocation(shader->program, "beats_per_pixel"), ((RulerShader*)shader)->uniform.beats_per_pixel);
	glUniform1f(glGetUniformLocation(shader->program, "viewport_left"), ((RulerShader*)shader)->uniform.viewport_left);
}


static void
_lines_set_uniforms()
{
	AGlShader* shader = &lines.shader;

	float colour[4] = {0.0, 0.0, 0.0, ((float)(lines.uniform.colour & 0xff)) / 0x100};
	agl_rgba_to_float(lines.uniform.colour, &colour[0], &colour[1], &colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "colour"), 1, colour);

	glUniform1f(glGetUniformLocation(shader->program, "texture_width"), (float)lines.uniform.texture_width);
	glUniform1i(glGetUniformLocation(shader->program, "n_channels"), lines.uniform.n_channels);
}


void
wf_shaders_init()
{
	agl_shaders_init();

	agl_create_program(&peak_shader.shader);
#ifdef USE_FBO
	agl_create_program(&peak_nonscaling.shader);
#endif
	agl_create_program(&hires_shader.shader);
	agl_create_program(&hires_ng_shader.shader);
	agl_create_program(&horizontal.shader);
	agl_create_program(&vertical.shader);
	agl_create_program(&ruler.shader);
	agl_create_program(&ass.shader);
	agl_create_program(&lines.shader);

	wf_shaders.ass = &ass;
}


