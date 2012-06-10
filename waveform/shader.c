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
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/gl_ext.h"
#include "waveform/gl_utils.h"
#include "waveform/canvas.h"
#include "waveform/shader.h"

#include "shaders/shaders.c"

static void  _peak_shader_set_uniforms (float peaks_per_pixel, float top, float bottom, uint32_t _fg_colour, int n_channels);
static void  _hires_set_uniforms       ();
static void  _vertical_set_uniforms    ();
static void  _alphamap_set_uniforms    ();
static void  _ruler_set_uniforms       ();

PeakShader peak_shader = {{NULL, NULL, 0, NULL, NULL, &peak_text}, _peak_shader_set_uniforms};
static UniformInfo uniforms[] = {
   {"tex1d",     1, GL_INT,   { 1, 0, 0, 0 }, -1}, // LHS +ve - 0 corresponds to glActiveTexture(WF_TEXTURE0);
   {"tex1d_neg", 1, GL_INT,   { 2, 0, 0, 0 }, -1}, // LHS -ve - 1 corresponds to glActiveTexture(WF_TEXTURE1);
   {"tex1d_3",   1, GL_INT,   { 3, 0, 0, 0 }, -1}, // RHS +ve WF_TEXTURE2
   {"tex1d_4",   1, GL_INT,   { 4, 0, 0, 0 }, -1}, // RHS -ve WF_TEXTURE3
   END_OF_UNIFORMS
};

HiResShader hires_shader = {{NULL, NULL, 0, NULL, _hires_set_uniforms, &hires_text}};
static UniformInfo uniforms_hr[] = {
   {"tex1d",     1, GL_INT,   { 1, 0, 0, 0 }, -1},
   {"tex1d_neg", 1, GL_INT,   { 2, 0, 0, 0 }, -1},
   {"tex1d_3",   1, GL_INT,   { 3, 0, 0, 0 }, -1},
   {"tex1d_4",   1, GL_INT,   { 4, 0, 0, 0 }, -1},
   END_OF_UNIFORMS
};

BloomShader horizontal = {{NULL, NULL, 0, NULL, NULL, &horizontal_text}};
static UniformInfo uniforms2[] = {
   {"tex2d",     1, GL_INT,   { 0, 0, 0, 0 }, -1}, // 0 corresponds to glActiveTexture(GL_TEXTURE0);
   END_OF_UNIFORMS
};

BloomShader vertical = {{NULL, NULL, 0, NULL, _vertical_set_uniforms, &vertical_text}};
static UniformInfo uniforms3[] = {
   {"tex2d",     1, GL_INT,   { 0, 0, 0, 0 }, -1}, // 0 corresponds to glActiveTexture(GL_TEXTURE0);
   END_OF_UNIFORMS
};

//used for background
AlphaMapShader tex2d = {{NULL, NULL, 0, NULL, _alphamap_set_uniforms, &alpha_map_text}};
static UniformInfo uniforms4[] = {
   {"tex2d",     1, GL_INT,   { 0, 0, 0, 0 }, -1},
   END_OF_UNIFORMS
};

RulerShader ruler = {{NULL, NULL, 0, NULL, _ruler_set_uniforms, &ruler_text}};
static UniformInfo uniforms5[] = {
   END_OF_UNIFORMS
};

static void
_peak_shader_set_uniforms(float peaks_per_pixel, float top, float bottom, uint32_t _fg_colour, int n_channels)
{
	//TODO this will generate GL_INVALID_OPERATION if the shader has never been assigned to an object

#if 0
	dbg(1, "peaks_per_pixel=%.2f top=%.2f bottom=%.2f n_channels=%i", peaks_per_pixel, top, bottom, n_channels);
#endif

	WfShader* shader = &peak_shader.shader;
	GLuint offsetLoc = glGetUniformLocation(shader->program, "peaks_per_pixel");
	glUniform1f(offsetLoc, peaks_per_pixel);

	offsetLoc = glGetUniformLocation(shader->program, "bottom");
	glUniform1f(offsetLoc, bottom);

	glUniform1f(glGetUniformLocation(shader->program, "top"), top);
	glUniform1i(glGetUniformLocation(shader->program, "n_channels"), n_channels);

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(_fg_colour & 0xff)) / 0x100};
	wf_rgba_to_float(_fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	gl_warn("");
}


static void
_hires_set_uniforms()
{
	WfShader* shader = &hires_shader.shader;
	struct U* u = &((HiResShader*)shader)->uniform;

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(hires_shader.uniform.fg_colour & 0xff)) / 0x100};
	wf_rgba_to_float(hires_shader.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "top"),             u->top);
	glUniform1f(glGetUniformLocation(shader->program, "bottom"),          u->bottom);
	glUniform1i(glGetUniformLocation(shader->program, "n_channels"),      u->n_channels);
	glUniform1f(glGetUniformLocation(shader->program, "peaks_per_pixel"), u->peaks_per_pixel);
}


static void
_vertical_set_uniforms()
{
	WfShader* shader = &vertical.shader;
	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(vertical.uniform.fg_colour & 0xff)) / 0x100};
	wf_rgba_to_float(vertical.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(shader->program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "peaks_per_pixel"), ((BloomShader*)shader)->uniform.peaks_per_pixel);
}


static void
_alphamap_set_uniforms()
{
	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(tex2d.uniform.fg_colour & 0xff)) / 0x100};
	wf_rgba_to_float(tex2d.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(tex2d.shader.program, "fg_colour"), 1, fg_colour);
}


static void
_ruler_set_uniforms()
{
	WfShader* shader = &ruler.shader;

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(ruler.uniform.fg_colour & 0xff)) / 0x100};
	wf_rgba_to_float(ruler.uniform.fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(ruler.shader.program, "fg_colour"), 1, fg_colour);

	glUniform1f(glGetUniformLocation(shader->program, "beats_per_pixel"), 0.10);
}


static GLuint
create_program(/*WaveformCanvas* wfc, */WfShader* sh, UniformInfo* uniforms)
{
	GLuint vert_shader = sh->vertex_file
		? compile_shader_file(GL_VERTEX_SHADER, sh->vertex_file)
		: compile_shader_text(GL_VERTEX_SHADER, sh->text->vert);
	GLuint frag_shader = sh->fragment_file
		? compile_shader_file(GL_FRAGMENT_SHADER, sh->fragment_file)
		: compile_shader_text(GL_FRAGMENT_SHADER, sh->text->frag);

	GLint status;
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE){
		printf("shader compile error! %i\n", status);
	}

	GLuint program = sh->program = link_shaders(vert_shader, frag_shader);
	dbg(2, "%u %u program=%u", vert_shader, frag_shader, program);

	glUseProgram(program);

	uniforms_init(program, uniforms);

	sh->uniforms = uniforms;

	return program;
}


void
wf_shaders_init()
{
	create_program(&peak_shader.shader, uniforms);
	create_program(&hires_shader.shader, uniforms_hr);
	create_program(&horizontal.shader, uniforms2);
	create_program(&vertical.shader, uniforms3);
	create_program(&tex2d.shader, uniforms4);
	create_program(&ruler.shader, uniforms5);
}


