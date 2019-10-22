/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2013-2017 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <glib.h>
#include "agl/ext.h"
#include "agl/pango_render.h"
#include "agl/shader.h"
#include "agl/utils.h"

static AGl* agl = NULL;

extern void wf_debug_printf (const char* func, int level, const char* format, ...);
#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define dbg(A, B, ...) wf_debug_printf(__func__, A, B, ##__VA_ARGS__)

static void agl_create_line_texture();

AGlMaterialClass aaline_class = {.init = agl_create_line_texture};


void
agl_use_material (AGlMaterial* material)
{
	agl_use_program(material->material_class->shader);
}


AGlMaterial*
agl_aa_line_new ()
{
	aaline_class.init();

	return AGL_NEW(AGlMaterial,
		.material_class = &aaline_class
	);
}


static void
agl_create_line_texture()
{
	if(aaline_class.texture) return;

	agl = agl_get_instance();

	aaline_class.shader = (AGlShader*)agl->shaders.text; // alpha shader

	glEnable(GL_TEXTURE_2D);

#if 1
	int width = 4;
	int height = 5;
	char* pbuf = g_new0(char, width * height);
	int y;
	//char vals[] = {0xff, 0xa0, 0x40};
	char vals[] = {0xff, 0x40, 0x10};
	int x; for(x=0;x<width;x++){
		y=0; *(pbuf + y * width + x) = vals[2];
		y=1; *(pbuf + y * width + x) = vals[1];
		y=2; *(pbuf + y * width + x) = vals[0];
		y=3; *(pbuf + y * width + x) = vals[1];
		y=4; *(pbuf + y * width + x) = vals[2];
	}
#else
	int width = 4;
	int height = 4;
	char* pbuf = g_new0(char, width * height);
	int y; for(y=0;y<height/2;y++){
		int x; for(x=0;x<width;x++){
			*(pbuf + y * width + x) = 0xff * (2*y)/height + 128;
			*(pbuf + (height -1 - y) * width + x) = 0xff * (2*y)/height + 128;
		}
	}
#endif

	glGenTextures(1, &aaline_class.texture);
	gl_warn("could not create line_texture");
	dbg(2, "line_texture=%u", aaline_class.texture);

	int pixel_format = GL_ALPHA;
	glBindTexture  (GL_TEXTURE_2D, aaline_class.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);
	gl_warn("error binding line texture");

	g_free(pbuf);
}

