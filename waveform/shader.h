#ifndef __wf_shader_h__
#define __wf_shader_h__
#include "waveform/shaderutil.h"

struct _wf_shader
{
	char*        vertex_file;
	char*        fragment_file;
	uint32_t     program;       // compiled program
	UniformInfo* uniforms;
	void         (*set_uniforms_)();
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
	struct U {
		uint32_t fg_colour;
		float    top;
		float    bottom;
		int      n_channels;
		float    peaks_per_pixel;
	}         uniform;
} HiResShader;

typedef struct {
	Shader    shader;
	void      (*set_uniforms)();
	struct {
		uint32_t fg_colour;
		float    peaks_per_pixel;
	}         uniform;
} BloomShader;

typedef struct {
	WfShader     shader;
	struct {
		uint32_t fg_colour;
	}            uniform;
} AlphaMapShader;

typedef struct {
	WfShader     shader;
	struct {
		uint32_t fg_colour;
	}            uniform;
} RulerShader;
#endif

void wf_shaders_init();

#endif //__wf_shader_h__
