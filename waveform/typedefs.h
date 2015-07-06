/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifndef __waveform_typedefs_h__
#define __waveform_typedefs_h__
#include "agl/typedefs.h"

typedef struct _wf                  WF;
typedef struct _Waveform            Waveform;
typedef struct _Peakbuf             Peakbuf;
typedef struct _alpha_buf           AlphaBuf;
typedef struct _WfPeakBuf           WfPeakBuf;
typedef struct _buf                 RmsBuf;
typedef struct _WfBuf16             WfBuf16;
typedef struct _waveform_canvas     WaveformCanvas;
typedef struct _WaveformCanvasClass WaveformCanvasClass;
typedef struct _waveform_actor      WaveformActor;
typedef struct _wf_texture_list     WfGlBlock;
typedef struct _textures_hi         WfTexturesHi;
typedef struct _texture_hi          WfTextureHi;
typedef struct _WaveformPriv        WaveformPriv;
typedef struct _WfAudioData         WfAudioData;
typedef struct _vp                  WfViewPort; 
typedef struct _WaveformView        WaveformView;
typedef struct _WaveformViewPlus    WaveformViewPlus;
typedef struct _wf_shaders          WfShaders;
typedef struct _ass_shader          AssShader;
typedef struct _WfWorker            WfWorker;

typedef void   (*WfCallback)         (gpointer);
typedef void   (*WfCallback2)        (Waveform*, gpointer);
typedef void   (*WfCallback3)        (Waveform*, GError*, gpointer);
typedef void   (*WfPeakfileCallback) (Waveform*, char* peakfile_name, gpointer);
typedef void   (*WfAudioCallback)    (Waveform*, int b, gpointer);

#ifdef HAVE_GTK_2_22
#define KEY_Left     GDK_KEY_Left
#define KEY_Right    GDK_KEY_Right
#define KEY_KP_Left  GDK_KEY_KP_Left
#define KEY_KP_Right GDK_KEY_KP_Right
#define KEY_Up       GDK_KEY_Up
#define KEY_Down     GDK_KEY_Down
#define KEY_KP_Up    GDK_KEY_KP_Up
#define KEY_KP_Down  GDK_KEY_KP_Down
#else
#define KEY_Left     GDK_Left
#define KEY_Right    GDK_Right
#define KEY_KP_Left  GDK_KP_Left
#define KEY_KP_Right GDK_KP_Right
#define KEY_Up       GDK_Up
#define KEY_Down     GDK_Down
#define KEY_KP_Up    GDK_KP_Up
#define KEY_KP_Down  GDK_KP_Down
#endif

#endif //__waveform_typedefs_h__
