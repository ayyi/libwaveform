/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform https://github.com/ayyi/libwaveform |
 | copyright (C) 2012-2024 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include "agl/fbo.h"
#include "waveform/ui-typedefs.h"

//TODO refactor based on _texture_hi (eg reverse order of indirection)
struct _wf_texture_list                   // WfGlBlock - used at MED and LOW resolutions in gl1 mode.
{
	int             size;
	struct {
		unsigned*   main;                 // array of texture id
		unsigned*   neg;                  // array of texture id - only used in shader mode.
	}               peak_texture[WF_MAX_CH];
	AGlFBO**        fbo;
#ifdef USE_FX
	AGlFBO**        fx_fbo;
#endif
#ifdef WF_SHOW_RMS
	unsigned*       rms_texture;
#endif
};

WaveformActor* wf_actor_new        (Waveform*, WaveformContext*);
