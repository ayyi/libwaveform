/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include <gdk/gdk.h>
#include "waveform/waveform.h"

typedef void (WfGdkTextureCallback) (Waveform*, GdkTexture*, gpointer);

typedef struct {
	WfSampleRegion* region;
	int             width;
	int             height;
	bool            single_channel;
	uint32_t        colour;
	uint32_t        bgcolour;
} WfPeakParams;

void        waveform_peak_to_texture      (Waveform*, WfPeakParams*, WfGdkTextureCallback, gpointer);
GdkTexture* waveform_peak_to_texture_sync (Waveform*, WfPeakParams*);
