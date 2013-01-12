#ifndef __agl_shader_h__
#define __agl_shader_h__
#ifdef __gl_h_
#include "agl/utils.h"

struct _alphamap_shader {
	AGlShader    shader;
	struct {
		uint32_t fg_colour;
	}            uniform;
};

#endif //__gl_h_
#endif //__agl_shader_h__
