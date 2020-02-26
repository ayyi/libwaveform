/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define _XOPEN_SOURCE 500
#define ENABLE_CHECKS
#define __wf_private__
#include "config.h"
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/time.h>
#include <glib.h>
#include "agl/debug.h"
#include "waveform/waveform.h"

static int peak_byte_depth = 4; //ardour peak files use floats.

static size_t get_n_words(Waveform*, const char* peakfile);


/*
 *  load the contents of a peak file from an Ardour project.
 */
int
wf_load_ardour_peak (Waveform* wv, const char* peak_file)
{
	g_return_val_if_fail(wv, 0);

	int fp = open(peak_file, O_RDONLY);
	if(!fp){ gwarn ("file open failure."); goto out; }
	dbg(2, "%s", peak_file);

	size_t n_frames = get_n_words(wv, peak_file);

	uint32_t bytes = n_frames * peak_byte_depth * WF_PEAK_VALUES_PER_SAMPLE;

	// read the whole peak file into memory
	float* read_buf = g_malloc(bytes);
	if(read(fp, read_buf, bytes) != bytes) perr ("read error. couldnt read %i bytes from %s", bytes, peak_file);
	close(fp);

	// convert from float to short
	short* buf = waveform_peakbuf_malloc(wv, WF_LEFT, n_frames * WF_PEAK_VALUES_PER_SAMPLE);
	int i; for(i=0;i<n_frames;i++){
		// ardour peak files have negative peak first. ABS is used because values occasionally have incorrect sign.
		buf[2 * i    ] =   ABS(read_buf[2 * i + 1]  * (1 << 15));
		buf[2 * i + 1] = -(ABS(read_buf[2 * i    ]) * (1 << 15));
	}

	g_free(read_buf);

	int ch_num = wv->priv->peak.buf[WF_LEFT] ? 1 : 0; //this makes too many assumptions. better to pass explicitly as argument.
	wv->priv->peak.buf[ch_num] = buf;

#ifdef ENABLE_CHECKS
	int k; for(k=0;k<n_frames;k++){
		if(wv->priv->peak.buf[0][2*k + 0] < 0.0){ gwarn("positive peak not positive"); break; }
		if(wv->priv->peak.buf[0][2*k + 1] > 0.0){ gwarn("negative peak not negative"); break; }
	}
#endif
  out:

	return 1;
}


static size_t
get_n_words (Waveform* wv, const char* peakfile)
{
	// ardour peak files are oversized. To get the useable size, we need to refer to the original file.

	int64_t n_frames = waveform_get_n_frames(wv);

	dbg(3, "n_frames=%"PRIi64" n_peaks=%zu", n_frames, (size_t)(ceil(((float)n_frames) / WF_PEAK_RATIO)));

	return ceil(((float)n_frames) / WF_PEAK_RATIO);
}

