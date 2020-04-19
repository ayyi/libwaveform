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
#ifndef __wf_audio_h__
#define __wf_audio_h__

#include "wf/waveform.h"

#define MAX_TIERS 8 //this is related to WF_PEAK_RATIO: WF_PEAK_RATIO = 2 ^ MAX_TIERS.

#ifdef __wf_private__
int wf_audio_cache_get_size();
#endif

#endif
