/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2015 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <GL/gl.h>
#include "waveform/waveform.h"
#include "waveform/actors/background.h"

static AGl* agl = NULL;


AGlActor*
background_actor(WaveformActor* view)
{
	agl = agl_get_instance();

	void agl_load_alphamap(char* buf, guint texture, int width, int height)
	{
		int pixel_format = GL_ALPHA;
		glBindTexture  (GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, buf);
#ifdef DEBUG
		gl_warn("binding bg texture");
#endif
	}

	void create_background(AGlActor* a)
	{
		//create an alpha-map gradient texture

		AGlTextureActor* ta = (AGlTextureActor*)a;

		if(ta->texture[0]) return;

		int width = 256;
		int height = 256;
		char* pbuf = g_new0(char, width * height);
#if 1
		int y; for(y=0;y<height;y++){
			int x; for(x=0;x<width;x++){
				*(pbuf + y * width + x) = ((x+y) * 0xff) / (width * 2);
			}
		}
#else
		// this gradient is brighter in the middle. It only works for stereo.
		int nc = 2;
		int c; for(c=0;c<nc;c++){
			int top = (height / nc) * c;
			int bot = height / nc;
			int mid = height / (2 * nc);
			int y; for(y=0;y<mid;y++){
				int x; for(x=0;x<width;x++){
					int y_ = top + y;
					int val = 0xff * (1.0 + sinf(((float)(-mid + 2 * y)) / mid));
					*(pbuf + y_ * width + x) = ((val) / 8 + ((x+y_) * 0xff) / (width * 2)) / 2;
					y_ = top + bot - y - 1;
					*(pbuf + (y_) * width + x) = ((val) / 8 + ((x+y_) * 0xff) / (width * 2)) / 2;
				}
			}
		} 
#endif

		agl_enable(AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);

		glGenTextures(1, ta->texture);
		if(glGetError() != GL_NO_ERROR){ gerr ("couldnt create bg_texture."); goto out; }

		agl_load_alphamap(pbuf, ta->texture[0], width, height);

	  out:
		g_free(pbuf);
	}

	void bg_actor_set_state(AGlActor* actor)
	{
		if(agl->use_shaders){
#if 0
			((AlphaMapShader*)actor->program)->uniform.fg_colour = 0x4488ffff; // TODO use theme colour, or pass as argument.
#else
			((AlphaMapShader*)actor->program)->uniform.fg_colour = 0x666666ff;
#endif
		}else{
			glColor4f(0.4, 0.4, 0.4, 1.0);
		}
	}

	bool bg_actor_paint(AGlActor* actor)
	{
		agl_textured_rect(((AGlTextureActor*)actor)->texture[0],
			0,
			0,
			agl_actor__width(actor->parent),
			actor->parent->region.y2,
			NULL
		);

		return true;
	}

	AGlTextureActor* ta = g_new0(AGlTextureActor, 1);
	AGlActor* actor = (AGlActor*)ta;
#ifdef AGL_DEBUG_ACTOR
	actor->name = "background";
#endif
	actor->init = create_background;
	actor->set_state = bg_actor_set_state;
	actor->paint = bg_actor_paint;
	actor->program = (AGlShader*)agl->shaders.alphamap;

	return actor;
}


