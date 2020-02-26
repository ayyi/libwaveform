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
#ifndef __wf_peakgen_h__
#define __wf_peakgen_h__

void   waveform_ensure_peakfile       (Waveform*, WfPeakfileCallback, gpointer);
char*  waveform_ensure_peakfile__sync (Waveform*);
void   waveform_peakgen               (Waveform*, const char* peakfile, WfCallback3, gpointer);
void   waveform_peakgen_cancel        (Waveform*);

bool   wf_peakgen__sync               (const char* wav, const char* peakfile, GError**);

#endif
