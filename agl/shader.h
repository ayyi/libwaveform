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

#ifdef __agl_utils_c__
AGlUniformInfo agl_null_uniforms[] = {
   END_OF_UNIFORMS
};
#else
extern AGlUniformInfo agl_null_uniforms[];
#endif

#endif //__gl_h_
#endif //__agl_shader_h__
