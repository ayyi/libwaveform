/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2024 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include "glib.h"
#include "agl/typedefs.h"
#include "waveform/typedefs.h"

typedef struct _alpha_buf           AlphaBuf;
typedef struct _WaveformContext     WaveformContext;
typedef struct _WaveformContextClass WaveformContextClass;
typedef struct _WaveformActor       WaveformActor;
typedef struct _textures_hi         WfTexturesHi;
typedef struct _WfViewPort          WfViewPort; 
typedef struct _WaveformViewPlus    WaveformViewPlus;
typedef struct _wf_shaders          WfShaders;
typedef struct _ass_shader          AssShader;
typedef struct _PeakShader          PeakShader;
typedef struct _RulerShader         RulerShader;
typedef struct _CursorShader        CursorShader;
