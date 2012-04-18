#ifndef __wf_shader_h__
#define __wf_shader_h__
#include "waveform/shaderutil.h"

struct _wf_shader
{
};

#ifdef __gl_h_
typedef struct {
	Shader    shader;
	void      (*set_uniforms)(float peaks_per_pixel, float top, float bottom, uint32_t _fg_colour, int n_channels);
	struct {
		float peaks_per_pixel;
		float fg_colour[4];
	}         uniform;
} PeakShader;

typedef struct {
	Shader    shader;
	void      (*set_uniforms)();
	struct {
		uint32_t fg_colour;
		float    peaks_per_pixel;
	}         uniform;
} BloomShader;

typedef struct {
	Shader       shader;
	void         (*set_uniforms)();
	struct {
		uint32_t fg_colour;
	}            uniform;
} AlphaMapShader;
#endif

void wf_shaders_init();

#endif //__wf_shader_h__
