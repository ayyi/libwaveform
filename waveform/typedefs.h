/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __waveform_typedefs_h__
#define __waveform_typedefs_h__

#include "glib.h"
#include "agl/typedefs.h"
#include "wf/typedefs.h"

typedef struct _alpha_buf           AlphaBuf;
typedef struct _WaveformContext     WaveformContext;
typedef struct _WaveformContextClass WaveformContextClass;
typedef struct _WaveformActor       WaveformActor;
typedef struct _textures_hi         WfTexturesHi;
typedef struct _WfViewPort          WfViewPort; 
typedef struct _WaveformView        WaveformView;
typedef struct _WaveformViewPlus    WaveformViewPlus;
typedef struct _wf_shaders          WfShaders;
typedef struct _ass_shader          AssShader;
typedef struct _PeakShader          PeakShader;
typedef struct _RulerShader         RulerShader;
typedef struct _CursorShader        CursorShader;

#ifdef HAVE_GTK_2_22
#define KEY_Left     GDK_KEY_Left
#define KEY_Right    GDK_KEY_Right
#define KEY_KP_Left  GDK_KEY_KP_Left
#define KEY_KP_Right GDK_KEY_KP_Right
#define KEY_Up       GDK_KEY_Up
#define KEY_Down     GDK_KEY_Down
#define KEY_KP_Up    GDK_KEY_KP_Up
#define KEY_KP_Down  GDK_KEY_KP_Down
#define KEY_Home     GDK_KEY_KP_Home
#else
#define KEY_Left     GDK_Left
#define KEY_Right    GDK_Right
#define KEY_KP_Left  GDK_KP_Left
#define KEY_KP_Right GDK_KP_Right
#define KEY_Up       GDK_Up
#define KEY_Down     GDK_Down
#define KEY_KP_Up    GDK_KP_Up
#define KEY_KP_Down  GDK_KP_Down
#define KEY_Home     GDK_KP_Home
#endif

#endif //__waveform_typedefs_h__
