/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2012-2019 Tim Orford <tim@orford.org>                  |
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
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <glib.h>
#ifdef USE_SNDFILE
#include <sndfile.h>
#else
#include "decoder/ad.h"
#endif
#include "waveform/waveform.h"

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

	// it seems this may be needed
	waveform_get_n_frames(wv);

	WaveformPrivate* _w = wv->priv;

#ifdef USE_SNDFILE
	SNDFILE* sndfile;
	SF_INFO sfinfo = { .format = 0 };
#else
	WfDecoder decoder = {0,};
#endif

#ifdef USE_SNDFILE
	if(!(sndfile = sf_open(peak_file, SFM_READ, &sfinfo))){
#else
	if(!ad_open(&decoder, peak_file)){
#endif
		if(!g_file_test(peak_file, G_FILE_TEST_EXISTS)){
#ifdef DEBUG
			if(wf_debug) gwarn("file open failure. file doesnt exist: %s", peak_file);
#endif
		}else{
			gwarn("file open failure (%s)", peak_file);
		}
		goto out;
	}

#ifdef USE_SNDFILE
	if(!(sfinfo.format & SF_FORMAT_PCM_16)){
#else
	if(false){
#endif
		gwarn("peakfile not 16 bit");
  		goto out_close;
	}
#ifdef USE_SNDFILE
	if(!(sfinfo.format & SF_FORMAT_WAV)){
		gwarn("not wav format");
  		goto out_close;
	}
#endif
#ifdef USE_SNDFILE
	if(sfinfo.channels > 2){
#else
	if(decoder.info.channels > 2){
#endif
		gwarn("too many channels");
  		goto out_close;
	}

#ifdef USE_SNDFILE
	const int64_t n_frames = sfinfo.frames / WF_PEAK_VALUES_PER_SAMPLE;
#else
	const int64_t n_frames = decoder.info.frames / WF_PEAK_VALUES_PER_SAMPLE;
#endif
#ifdef USE_SNDFILE
	dbg(2, "n_channels=%i n_frames=%Li n_bytes=%Li n_blocks=%i", sfinfo.channels, n_frames, sfinfo.frames * peak_byte_depth * sfinfo.channels, (int)(ceil((float)n_frames / WF_PEAK_TEXTURE_SIZE)));
#else
	dbg(2, "n_channels=%i n_frames=%Li n_bytes=%Li n_blocks=%i", decoder.info.channels, n_frames, decoder.info.frames * peak_byte_depth * decoder.info.channels, (int)(ceil((float)n_frames / WF_PEAK_TEXTURE_SIZE)));
#endif
	dbg(2, "secs=%.3f %.3f", ((float)(n_frames)) / 44100, ((float)(n_frames * WF_PEAK_RATIO)) / 44100);

	const int64_t max_frames = wv->n_frames ? (wv->n_frames / WF_PEAK_RATIO + (wv->n_frames % WF_PEAK_RATIO ? 1 : 0)) : WF_MAX_FRAMES;
#ifdef DEBUG
#ifdef USE_SNDFILE
	if(sfinfo.frames / WF_PEAK_VALUES_PER_SAMPLE > max_frames) gwarn("peakfile is too long: %"PRIi64", expected %"PRIi64, sfinfo.frames / WF_PEAK_VALUES_PER_SAMPLE, max_frames);
#else
	if(decoder.info.frames % WF_PEAK_VALUES_PER_SAMPLE) gwarn("peakfile not even length: %"PRIi64, decoder.info.frames);
	if(n_frames > max_frames) gwarn("peakfile is too long: %"PRIi64", expected %"PRIi64, decoder.info.frames / WF_PEAK_VALUES_PER_SAMPLE, max_frames);
	if(n_frames < max_frames && max_frames != WF_MAX_FRAMES) gwarn("peakfile is too short: %"PRIi64", expected %"PRIi64, n_frames, max_frames);
#endif
#endif
#ifdef USE_SNDFILE
	sf_count_t r_frames = MIN(sfinfo.frames, max_frames * WF_PEAK_VALUES_PER_SAMPLE);
#else
	int64_t r_frames = MIN(decoder.info.frames, max_frames * WF_PEAK_VALUES_PER_SAMPLE);
#endif

#ifdef USE_SNDFILE
	short* read_buf = (sfinfo.channels == 1)
		? waveform_peakbuf_malloc(wv, WF_LEFT, r_frames) // no deinterleaving required, can read directly into the peak buffer.
		: g_malloc(r_frames * peak_byte_depth * sfinfo.channels);
#endif

	// Read the whole peak file into memory
	int readcount_frames;
#ifdef USE_SNDFILE
	if((readcount_frames = sf_readf_short(sndfile, read_buf, r_frames)) < r_frames){
#else
	WfBuf16 buf = decoder.info.channels == 1
		? (WfBuf16){
			.buf = {waveform_peakbuf_malloc(wv, WF_LEFT, r_frames),},
			.size = r_frames
		}
		: (WfBuf16){
			.buf = {
				waveform_peakbuf_malloc(wv, WF_LEFT, r_frames),
				waveform_peakbuf_malloc(wv, WF_RIGHT, r_frames)
			},
			.size = r_frames
		};
	if((readcount_frames = ad_read_peak(&decoder, &buf)) < r_frames){
#endif
		int shortfall = n_frames * 2 - readcount_frames;
#ifndef USE_SNDFILE
		if(shortfall){
			memset(buf.buf[0] + readcount_frames, 0, shortfall);
			memset(buf.buf[1] + readcount_frames, 0, shortfall);
		}
#endif
		if(shortfall < 100){
			gwarn("shortfall");
		}else{
			gwarn("unexpected EOF: read %i of %"PRIi64" (%"PRIi64") (short by %i) %s", readcount_frames, n_frames * 2, n_frames, shortfall, peak_file);
		}
	}

#ifdef USE_SNDFILE
	int ch_num = WF_LEFT; // TODO might not be WF_LEFT for split files
	if(sfinfo.channels == 1){
		_w->peak.buf[ch_num] = read_buf;
	}else if(sfinfo.channels == 2){
		// de-interleave
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
#else
	switch(decoder.info.channels){
		case 1:
			break;
		case 2:
			break;
		default:
			break;
	}
#endif
#ifdef ENABLE_CHECKS
	int k; for(k=0;k<10;k++){
		if(_w->peak.buf[0][2*k + 0] < 0.0 || _w->peak.buf[1][2*k + 0] < 0.0){ gwarn("positive peak not positive"); break; }
		if(_w->peak.buf[0][2*k + 1] > 0.0 || _w->peak.buf[1][2*k + 1] > 0.0){ gwarn("negative peak not negative"); break; }
	}
#endif
  out_close:
#ifdef USE_SNDFILE
	sf_close(sndfile);
#else
	ad_close(&decoder);
#endif

  out:
#ifdef USE_SNDFILE
	return sfinfo.channels;
#else
	return decoder.info.channels;
#endif
}

