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
#define __waveform_peak_c__
#define __wf_private__
#include "config.h"
#include "wf/waveform.h"
#include "wf/loaders/riff.h"

WF* wf = NULL;


WF*
wf_get_instance ()
{
	if(!wf){
		wf = WF_NEW(WF,
			.domain = "Libwaveform",
			.peak_cache = g_hash_table_new(g_direct_hash, g_direct_equal),
			.audio.cache = g_hash_table_new(g_direct_hash, g_direct_equal),
			.load_peak = wf_load_riff_peak, //set the default loader
		);
	}
	return wf;
}

