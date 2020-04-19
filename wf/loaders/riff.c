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
#define __wf_private__
#define ENABLE_CHECKS
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <glib.h>
#include <sndfile.h>
#include "agl/debug.h"
#include "wf/waveform.h"

#define peak_byte_depth 2 // value are stored in the peak file as int16.
#define WF_MAX_FRAMES 116121600000LL // 192kHz 7 days


/*
 *   Load the given peak_file into a buffer and return the number of channels loaded.
 */
int
wf_load_riff_peak(Waveform* wv, const char* peak_file)
{
	g_return_val_if_fail(wv, 0);
	PF2;

	WaveformPrivate* _w = wv->priv;

	SNDFILE* sndfile;
	SF_INFO sfinfo = { .format = 0 };

	if(!(sndfile = sf_open(peak_file, SFM_READ, &sfinfo))){
		if(!g_file_test(peak_file, G_FILE_TEST_EXISTS)){
#ifdef DEBUG
			if(wf_debug) gwarn("file open failure. file doesnt exist: %s", peak_file);
#endif
		}else{
			gwarn("file open failure.");
		}
		goto out;
	}
	if(!(sfinfo.format & SF_FORMAT_PCM_16)){
		gwarn("peakfile not 16 bit");
  		goto out_close;
	}
	if(!(sfinfo.format & SF_FORMAT_WAV)){
		gwarn("not wav format");
  		goto out_close;
	}
	if(sfinfo.channels > 2){
		gwarn("too many channels");
  		goto out_close;
	}

	const sf_count_t n_frames = sfinfo.frames / WF_PEAK_VALUES_PER_SAMPLE;
	dbg(2, "n_channels=%i n_frames=%"PRIi64" n_bytes=%"PRIi64" n_blocks=%i", sfinfo.channels, n_frames, sfinfo.frames * peak_byte_depth * sfinfo.channels, (int)(ceil((float)n_frames / WF_PEAK_TEXTURE_SIZE)));
	dbg(2, "secs=%.3f %.3f", ((float)(n_frames)) / 44100, ((float)(n_frames * WF_PEAK_RATIO)) / 44100);

	if(!n_frames){
		wv->renderable = false;
		goto out_close;
	}

	const sf_count_t max_frames = wv->n_frames ? (wv->n_frames / WF_PEAK_RATIO + (wv->n_frames % WF_PEAK_RATIO ? 1 : 0)) : WF_MAX_FRAMES;
#ifdef DEBUG
	if(sfinfo.frames / WF_PEAK_VALUES_PER_SAMPLE > max_frames) gwarn("peakfile is too long: %"PRIi64", expected %"PRIi64, sfinfo.frames / WF_PEAK_VALUES_PER_SAMPLE, max_frames);
#endif
	sf_count_t r_frames = MIN(sfinfo.frames, max_frames * WF_PEAK_VALUES_PER_SAMPLE);

	short* read_buf = (sfinfo.channels == 1)
		? waveform_peakbuf_malloc(wv, WF_LEFT, r_frames) // no deinterleaving required, can read directly into the peak buffer.
		: g_malloc(r_frames * peak_byte_depth * sfinfo.channels);

	//read the whole peak file into memory:
	int readcount_frames;
	if((readcount_frames = sf_readf_short(sndfile, read_buf, r_frames)) < r_frames){
		gwarn("unexpected EOF: %s - read %i of %"PRIi64" items", peak_file, readcount_frames, n_frames);
		//gerr ("read error. couldnt read %i bytes from %s", bytes, peak_file);
	}

	int ch_num = WF_LEFT; // TODO might not be WF_LEFT for split files
	if(sfinfo.channels == 1){
		_w->peak.buf[ch_num] = read_buf;
	}else if(sfinfo.channels == 2){
		short* buf[WF_MAX_CH] = {
			waveform_peakbuf_malloc(wv, 0, r_frames),
			waveform_peakbuf_malloc(wv, 1, r_frames)
		};
		int i; for(i=0;i<readcount_frames/2;i++){
			int c; for(c=0;c<sfinfo.channels;c++){
				int src = 2 * (i * sfinfo.channels + c);
				buf[c][2 * i    ] = read_buf[src    ]; // +
				buf[c][2 * i + 1] = read_buf[src + 1]; // -
			}
		}
		_w->peak.buf[ch_num    ] = buf[WF_LEFT ];
		_w->peak.buf[ch_num + 1] = buf[WF_RIGHT];

		g_free(read_buf);
	}
#ifdef ENABLE_CHECKS
	int k; for(k=0;k<10;k++){
		if(_w->peak.buf[0][2*k + 0] < 0.0){ gwarn("positive peak not positive"); break; }
		if(_w->peak.buf[0][2*k + 1] > 0.0){ gwarn("negative peak not negative"); break; }
	}
#endif
  out_close:
	sf_close(sndfile);

  out:
	return sfinfo.channels;
}

