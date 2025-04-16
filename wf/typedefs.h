/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

typedef struct _wf                  WF;
typedef struct _Waveform            Waveform;
typedef struct _WaveformPrivate     WaveformPrivate;
typedef struct _WfBuf16             WfBuf16;
typedef struct _WfPeakBuf           WfPeakBuf;
typedef struct _Peakbuf             Peakbuf;
typedef struct _buf                 RmsBuf;
typedef struct _WfAudioData         WfAudioData;
typedef struct _WfWorker            WfWorker;
typedef struct _texture_hi          WfTextureHi;
typedef struct _wf_texture_list     WfGlBlock;

typedef void   (*WfCallback)         (gpointer);
typedef void   (*WfCallback2)        (Waveform*, gpointer);
typedef void   (*WfCallback3)        (Waveform*, GError*, gpointer);
typedef void   (*WfCallback4)        (Waveform*);
typedef void   (*WfAudioCallback)    (Waveform*, int b, gpointer);
typedef void   (*WfPeakfileCallback) (Waveform*, char* peakfile_name, gpointer);

typedef struct { int64_t start, end; } WfFrRange;

enum
{
	WF_MONO = 1,
	WF_STEREO,
};

enum
{
	WF_LEFT = 0,
	WF_RIGHT,
	WF_MAX_CH
};
