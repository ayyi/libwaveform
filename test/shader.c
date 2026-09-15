/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform                                     |
 | https://github.com/ayyi/libwaveform                                  |
 | copyright (C) 2020-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include "gdk/gdk.h"
#include "agl/ext.h"
#include "agl/x11.h"
#include "agl/debug.h"
#include "agl/behaviours/key.h"
#include "agl/text/text_input.h"
#include "ui/shader.h"
#define __glx_test__
#include "test/common2.h"

extern AGl _agl;
extern AGlShader ng_shader;

static AGlRootActor* scene;
static unsigned int texture;

static void create_texture ();


int
run (int argc, char *argv[])
{
	const int width = 400, height = 256;

	AGlWindow* window = agl_window("Text waveform shader", -1, -1, width, height, 0);
	XMapWindow(dpy, window->window);
	scene = window->scene;

	AGlActor* actor = (AGlActor*)scene;
	actor->program = &ng_shader;
	agl_create_program(actor->program);
	create_texture();

	bool paint (AGlActor* actor)
	{
		AGlShader* shader = actor->program;
		AGlUniform* u = shader->uniforms;

		agl_set_colour_uniform(&u[NG_U_FG_COLOUR], 0xff9933ff);
		u[NG_U_TOP].value.f[0] = 0;
		u[NG_U_BOTTOM].value.f[0] = agl_actor__height(actor);
		u[NG_U_N_CHANNELS].value.i[0] = 1;
		u[NG_U_TEX_WIDTH].value.f[0] = 256;
		u[NG_U_TEX_HEIGHT].value.f[0] = 256;
		u[NG_U_VGAIN].value.f[0] = 1.;
		u[NG_U_MM_LEVEL].value.i[0] = 0;

		agl_use_program(actor->program);

		{
			float x = 0;
			float y = 0;
			float w = agl_actor__width(actor);
			float h = agl_actor__height(actor);

			agl_use_texture(texture);

			AGlQuad t = (AGlQuad){0.0, 0.0, 1.0, 1.0};

			float vertices[] = {
				// position    // texture coord
				x,     y + h,  t.x0, t.y1,
				x + w, y,      t.x1, t.y0,
				x,     y,      t.x0, t.y0,

				x,     y + h,  t.x0, t.y1,
				x + w, y + h,  t.x1, t.y1,
				x + w, y,      t.x1, t.y0
			};

			agl_use_va(_agl.vao);
			glBindBuffer(GL_ARRAY_BUFFER, _agl.vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		return true;
	}

	actor->paint = paint;

	g_main_loop_run(agl_main_loop_new());

	agl_window_destroy(&window);
	XCloseDisplay(dpy);

	return 0;
}


static void
create_texture ()
{
	glGenTextures (1, &texture);

	int width = 256;
	int height = 256;
	char* pbuf = g_new0(char, width * height * 3);
	for (int y=0;y<height;y++) {
		for (int x=0;x<width;x++) {
			*(pbuf + y * (width*3) + (x*3))     = 0xff * sin(x+y);
			*(pbuf + y * (width*3) + (x*3) + 1) = 0xff * sin(x+y);
			*(pbuf + y * (width*3) + (x*3) + 2) = 0xff * sin(x+y);
		}
	}

	#define pixel_format GL_ALPHA
	agl_use_texture (texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);

	g_free (pbuf);
}

#include "test/x11.c"
