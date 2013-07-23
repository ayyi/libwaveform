#ifndef __wf_shader_h__
#define __wf_shader_h__
#include "agl/typedefs.h"
#ifdef __gl_h_
#include "agl/shader.h"

typedef struct {
	AGlShader shader;
	void      (*set_uniforms)(float peaks_per_pixel, float top, float bottom, uint32_t _fg_colour, int n_channels);
	struct {
		float peaks_per_pixel;
		float fg_colour[4];
		int   n_channels;
	}         uniform;
} PeakShader;

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

typedef struct {
	AGlShader shader;
	struct {
		uint32_t fg_colour;
		float    peaks_per_pixel;
	}         uniform;
} BloomShader;

typedef struct {
	AGlShader    shader;
	struct {
		uint32_t fg_colour;
		float    beats_per_pixel;
		float    viewport_left;
	}            uniform;
} RulerShader;

typedef struct {
	AGlShader    shader;
	struct {
		uint32_t colour;
		int      n_channels;
		int      texture_width;
	}            uniform;
} LinesShader;

struct _ass_shader {
	AGlShader    shader;
	struct {
		uint32_t colour1;
		uint32_t colour2;
	}            uniform;
};

#endif

struct _wf_shaders {
	AssShader* ass;
} wf_shaders;


void wf_shaders_init();

#endif //__wf_shader_h__
