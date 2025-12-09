/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform                                     |
 | https://github.com/ayyi/libwaveform                                  |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include "agl/typedefs.h"
#ifdef __gl_h_
#include "agl/shader.h"
#include "waveform/ui-typedefs.h"

struct _PeakShader {
	AGlShader shader;
	void      (*set_uniforms)(float peaks_per_pixel, float top, float bottom, uint32_t _fg_colour, int n_channels);
	struct {
		float peaks_per_pixel;
		float fg_colour[4];
		int   n_channels;
	}         uniform;
};

typedef struct {
	AGlShader shader;
	struct U {
		uint32_t fg_colour;
		float    top;
		float    bottom;
		int      n_channels;
		float    peaks_per_pixel;
	}         uniform;
} HiResShader;

enum {
	NG_U_TEX = 0,
	NG_U_TOP,
	NG_U_BOTTOM,
	NG_U_N_CHANNELS,
	NG_U_TEX_WIDTH,
	NG_U_TEX_HEIGHT,
	NG_U_MM_LEVEL,
	NG_U_VGAIN,
	NG_U_FG_COLOUR,
	NG_U_MAX
};

typedef struct {
	AGlShader shader;
	struct {
		uint32_t fg_colour;
		float    peaks_per_pixel;
	}         uniform;
} BloomShader;

struct _RulerShader {
	AGlShader    shader;
	struct {
		uint32_t fg_colour;
		float    beats_per_pixel;
		float    samples_per_pixel;
		float    viewport_left;
		int      markers[10];
	}            uniform;
};

typedef struct {
	AGlShader    shader;
	struct {
		uint32_t fg_colour;
		float    samples_per_pixel;
		float    pixels_per_unit;
		float    n_subs;
		float    viewport_left;
		int      markers[10];
	}            uniform;
} RulerFramesShader;

#ifdef MULTILINE_SHADER
typedef struct {
	AGlShader    shader;
	struct {
		uint32_t colour;
		int      n_channels;
		int      texture_width;
	}            uniform;
} LinesShader;
#endif

struct _ass_shader {
	AGlShader    shader;
	struct {
		uint32_t colour1;
		uint32_t colour2;
	}            uniform;
};

struct _CursorShader {
	AGlShader    shader;
	struct {
		float    width;
	}            uniform;
};
#ifndef __wf_shader_c__
extern CursorShader cursor;
#endif

#endif
