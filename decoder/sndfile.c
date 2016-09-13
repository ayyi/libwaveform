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
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sndfile.h>

#include <gtk/gtk.h>
#include "decoder/debug.h"
#include "decoder/ad.h"

typedef struct {
    SF_INFO  sfinfo;
    SNDFILE* sffile;
} SndfileDecoder;


int
parse_bit_depth(int format)
{
	/* see http://www.mega-nerd.com/libsndfile/api.html */
	switch (format&0x0f) {
		case SF_FORMAT_PCM_S8: return 8;
		case SF_FORMAT_PCM_16: return 16; /* Signed 16 bit data */
		case SF_FORMAT_PCM_24: return 24; /* Signed 24 bit data */
		case SF_FORMAT_PCM_32: return 32; /* Signed 32 bit data */
		case SF_FORMAT_PCM_U8: return 8;  /* Unsigned 8 bit data (WAV and RAW only) */
		case SF_FORMAT_FLOAT : return 32; /* 32 bit float data */
		case SF_FORMAT_DOUBLE: return 64; /* 64 bit float data */
		default: break;
	}
	return 0;
}


int
ad_info_sndfile(WfDecoder* d)
{
	SndfileDecoder* sf = d->d;
	if (!sf) return -1;

	int16_t bit_depth = parse_bit_depth(sf->sfinfo.format);
	d->info = (WfAudioInfo){
		.channels    = sf->sfinfo.channels,
		.frames      = sf->sfinfo.frames,
		.sample_rate = sf->sfinfo.samplerate,
		.length      = sf->sfinfo.samplerate ? (sf->sfinfo.frames * 1000) / sf->sfinfo.samplerate : 0,
		.bit_depth   = bit_depth,
		.bit_rate    = bit_depth * sf->sfinfo.channels * sf->sfinfo.samplerate,
		.meta_data   = NULL,
	};
	return 0;
}


static gboolean
ad_open_sndfile(WfDecoder* decoder, const char* fn)
{
	SndfileDecoder* priv = decoder->d = g_new0(SndfileDecoder, 1);
	priv->sfinfo.format = 0;
	if(!(priv->sffile = sf_open(fn, SFM_READ, &priv->sfinfo))){
		dbg(1, "unable to open file '%s': %i: %s", fn, sf_error(NULL), sf_strerror(NULL));
		g_free(priv);
		return FALSE;
	}
	ad_info_sndfile(decoder);
	return TRUE;
}


int
ad_close_sndfile(WfDecoder* decoder)
{
	SndfileDecoder* priv = (SndfileDecoder*)decoder->d;
	if (!priv) return -1;
	if (!priv->sffile || sf_close(priv->sffile)) {
		perr("bad file close.\n");
		return -1;
	}
	return 0;
}


int64_t
ad_seek_sndfile(WfDecoder* d, int64_t pos)
{
	SndfileDecoder *priv = (SndfileDecoder*)d->d;
	if (!priv) return -1;
	return sf_seek(priv->sffile, pos, SEEK_SET);
}


ssize_t
ad_read_sndfile(WfDecoder* d, float* data, size_t len)
{
	SndfileDecoder* priv = (SndfileDecoder*)d->d;
	if (!priv) return -1;
	return sf_read_float (priv->sffile, data, len);
}


int
ad_eval_sndfile(const char *f)
{
	char *ext = strrchr(f, '.');
	if (!ext) return 5;
	/* see http://www.mega-nerd.com/libsndfile/ */
	if (!strcasecmp(ext, ".wav")) return 100;
	if (!strcasecmp(ext, ".aiff")) return 100;
	if (!strcasecmp(ext, ".aifc")) return 100;
	if (!strcasecmp(ext, ".snd")) return 100;
	if (!strcasecmp(ext, ".au")) return 100;
	if (!strcasecmp(ext, ".paf")) return 100;
	if (!strcasecmp(ext, ".iff")) return 100;
	if (!strcasecmp(ext, ".svx")) return 100;
	if (!strcasecmp(ext, ".sf")) return 100;
	if (!strcasecmp(ext, ".vcc")) return 100;
	if (!strcasecmp(ext, ".w64")) return 100;
	if (!strcasecmp(ext, ".mat4")) return 100;
	if (!strcasecmp(ext, ".mat5")) return 100;
	if (!strcasecmp(ext, ".pvf5")) return 100;
	if (!strcasecmp(ext, ".xi")) return 100;
	if (!strcasecmp(ext, ".htk")) return 100;
	if (!strcasecmp(ext, ".pvf")) return 100;
	if (!strcasecmp(ext, ".sd2")) return 100;
// libsndfile >= 1.0.18
	if (!strcasecmp(ext, ".flac")) return 80;
	if (!strcasecmp(ext, ".ogg")) return 80;
	return 0;
}


const static AdPlugin ad_sndfile = {
#if 1
	&ad_eval_sndfile,
	&ad_open_sndfile,
	&ad_close_sndfile,
	&ad_info_sndfile,
	&ad_seek_sndfile,
	&ad_read_sndfile
#else
	&ad_eval_null,
	&ad_open_null,
	&ad_close_null,
	&ad_info_null,
	&ad_seek_null,
	&ad_read_null
#endif
};


/* dlopen handler */
const AdPlugin*
get_sndfile()
{
	return &ad_sndfile;
}

