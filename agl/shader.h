/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2017 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __agl_shader_h__
#define __agl_shader_h__
#ifdef __gl_h_
#include "agl/utils.h"

struct _plain_shader {
	AGlShader    shader;
	struct {
		uint32_t colour;
	}            uniform;
	struct {
		uint32_t colour;
	}            state;
};

struct _alphamap_shader {
	AGlShader    shader;
	struct {
		uint32_t fg_colour;
	}            uniform;
};

#ifdef __agl_utils_c__
AGlUniformInfo agl_null_uniforms[] = {
   END_OF_UNIFORMS
};
#else
extern AGlUniformInfo agl_null_uniforms[];
#endif

#endif //__gl_h_
#endif //__agl_shader_h__
