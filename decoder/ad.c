/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2016 Tim Orford <tim@orford.org>                       |
* | copyright (C) 2011 Robin Gareus <robin@gareus.org>                   |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __ad_plugin_c__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "decoder/debug.h"
#include "decoder/ad.h"

#if 0
int      ad_eval_null  (const char* f)               { return -1; }
gboolean ad_open_null  (WfDecoder* d, const char* f)   { return  0; }
int      ad_close_null (WfDecoder* d)                  { return -1; }
int      ad_info_null  (WfDecoder* d)                  { return -1; }
int64_t  ad_seek_null  (void* sf, int64_t p)         { return -1; }
ssize_t  ad_read_null  (void* sf, float*d, size_t s) { return -1; }
#endif

struct _WfDecoder
{
    WfAudioInfo       info;
    const AdPlugin* b;      // backend
    void*           d;      // data
};


static AdPlugin const*
choose_backend(const char* fn)
{
	AdPlugin const* b = NULL;
	int max = 0;
	int score;

	if((score = get_sndfile()->eval(fn)) > max){
		max = score;
		b = get_sndfile();
	}

#ifdef USE_FFMPEG
	if((score = get_ffmpeg()->eval(fn)) > max){
		max = score;
		b = get_ffmpeg();
	}
#endif

	return b;
}


/*
 *  Opening will fill WfDecoder.info which the caller must free using ad_free_info()
 */
gboolean
ad_open(WfDecoder* d, const char* fname)
{
	ad_clear_nfo(&d->info);

	d->b = choose_backend(fname);
	if (!d->b) {
		return g_warning("no decoder backend available for filetype: '%s'", strrchr(fname, '.')), FALSE;
	}

	return d->b->open(d, fname);
}


int
ad_info(WfDecoder* d)
{
	if (!d) return -1;
	return d->b->info(d->d);
}


int
ad_close(WfDecoder* d)
{
	if (!d) return -1;
	return d->b->close(d);
}


int64_t
ad_seek(WfDecoder* d, int64_t pos)
{
	if (!d) return -1;
	return d->b->seek(d, pos);
}

ssize_t
ad_read(WfDecoder* d, float* out, size_t len)
{
	if (!d) return -1;
	return d->b->read(d, out, len);
}


ssize_t
ad_read_short(WfDecoder* d, WfBuf16* out)
{
	if (!d) return -1;
	return d->b->read_short(d, out);
}


/*
 *  For fftw clients that prefer data as double.
 *  side-effects: allocates buffer
 */
ssize_t
ad_read_mono_dbl(WfDecoder* d, double* data, size_t len)
{
	int c,f;
	int chn = d->info.channels;
	if (len < 1) return 0;

	static float *buf = NULL;
	static size_t bufsiz = 0;
	if (!buf || bufsiz != len*chn) {
		bufsiz = len * chn;
		buf = (float*) realloc((void*)buf, bufsiz * sizeof(float));
	}

	if((len = ad_read(d, buf, bufsiz)) > bufsiz) return 0;

	for (f=0;f<len/chn;f++) {
		double val = 0.0;
		for (c=0;c<chn;c++) {
			val += buf[f * chn + c];
		}
		data[f] = val / chn;
	}
	return len / chn;
}


gboolean
ad_finfo (const char* f, WfAudioInfo* nfo)
{
	ad_clear_nfo(nfo);
	WfDecoder d = {{0,}};
	if(ad_open(&d, f)){
		*nfo = d.info;
		ad_close(&d);
		return TRUE;
	}
	return FALSE;
}


void
ad_clear_nfo(WfAudioInfo* nfo)
{
	memset(nfo, 0, sizeof(WfAudioInfo));
}


void
ad_free_nfo(WfAudioInfo* nfo)
{
	if (nfo->meta_data) g_ptr_array_unref(nfo->meta_data);
}


void
ad_print_nfo(int dbglvl, WfAudioInfo* nfo)
{
#if 0
	dbg(dbglvl, "sample_rate: %u", nfo->sample_rate);
	dbg(dbglvl, "channels:    %u", nfo->channels);
	dbg(dbglvl, "length:      %"PRIi64" ms", nfo->length);
	dbg(dbglvl, "frames:      %"PRIi64, nfo->frames);
	dbg(dbglvl, "bit_rate:    %d", nfo->bit_rate);
	dbg(dbglvl, "bit_depth:   %d", nfo->bit_depth);
	dbg(dbglvl, "channels:    %u", nfo->channels);

	if(nfo->meta_data){
		dbg(dbglvl, "meta-data:");

		char** data = (char**)nfo->meta_data->pdata;
		int i; for(i=0;i<nfo->meta_data->len;i+=2){
			dbg(0, "  %s: %s", data[i], data[i+1]);
		}
	}
#endif
}


/*
 *  Input and output is both interleaved
 */
void
int16_to_float(float* out, int16_t* in, int n_channels, int n_frames, int out_offset)
{
	int f, c;
	for (f=0;f<n_frames;f++) {
		for (c=0;c<n_channels;c++) {
			out[(f+out_offset)*n_channels+c] = (float) in[f*n_channels+c] / 32768.0;
		}
	}
}

