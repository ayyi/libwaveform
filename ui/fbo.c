/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include "agl/ext.h"
#include "wf/debug.h"
#include "wf/waveform.h"
#include "waveform/shader.h"
#include "waveform/fbo.h"

extern BloomShader vertical;

#ifdef USE_FBO


AGlFBO*
fbo_new_test()
{
#if 0
	GLuint _wf_create_background()
	{
		//create an alpha-map gradient texture for use as background

		glEnable(GL_TEXTURE_2D);

		int width = 256;
		int height = 256;
		char* pbuf = g_new0(char, width * height);
		int y; for(y=0;y<height;y++){
			int x; for(x=0;x<width;x++){
				*(pbuf + y * width + x) = ((x+y) * 0xff) / (width * 2);
			}
		}

		GLuint bg_textures;
		glGenTextures(1, &bg_textures);
		if(glGetError() != GL_NO_ERROR){ perr ("couldnt create bg_texture."); return 0; }
		dbg(2, "bg_texture=%i", bg_textures);

		int pixel_format = GL_ALPHA;
		agl_use_texture (bg_textures);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);
		if(glGetError() != GL_NO_ERROR) pwarn("gl error binding bg texture!");

		g_free(pbuf);

		return bg_textures;
	}
#endif

	GLuint _wf_create_background_rgba()
	{
		//create an alpha-map gradient texture for use as background

		glEnable(GL_TEXTURE_2D);

		int width = 256;
		int height = 256;
		int rowstride = 256 * 4;
		char* pbuf = g_new0(char, width * height * 4);
		int y; for(y=0;y<height;y++){
			int x; for(x=0;x<width;x++){
				*(pbuf + y * rowstride + 4 * x    ) = ((x+y) * 0xff) / (width * 2);
				*(pbuf + y * rowstride + 4 * x + 1) = ((x+y) * 0xff) / (width * 2);
				*(pbuf + y * rowstride + 4 * x + 2) = ((x+y) * 0xff) / (width * 2);
				*(pbuf + y * rowstride + 4 * x + 3) = ((x+y) * 0xff) / (width * 2);

				if(x == 64 || x == 65 || x == 66 || x == 67){
					*(pbuf + y * rowstride + 4 * x + 1) = 0;
					*(pbuf + y * rowstride + 4 * x + 2) = 0;
					*(pbuf + y * rowstride + 4 * x + 3) = 0;
				}
			}
		}

		GLuint bg_textures;
		glGenTextures(1, &bg_textures);
		if(glGetError() != GL_NO_ERROR){ perr ("couldnt create bg_texture."); return 0; }
		dbg(2, "bg_texture=%i", bg_textures);

		int pixel_format = GL_RGBA;
		agl_use_texture (bg_textures);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);
		if(glGetError() != GL_NO_ERROR) pwarn("gl error binding bg texture!");

		g_free(pbuf);

		return bg_textures;
	}
	//AGlFBO* fbo = agl_fbo_new(256, 256, _wf_create_background(), 0);
	AGlFBO* fbo = agl_fbo_new(256, 256, _wf_create_background_rgba(), 0);
	return fbo;
}

#endif //use_fbo

