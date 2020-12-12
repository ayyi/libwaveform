/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2011-2020 Tim Orford <tim@orford.org>                  |
* | copyright (C) 2011 Robin Gareus <robin@gareus.org>                   |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "config.h"
#include <inttypes.h>
#include <math.h>
#include <sndfile.h>
#include <glib.h>
#include "decoder/debug.h"
#include "decoder/ad.h"

extern void int16_to_float (float* out, int16_t* in, int n_channels, int n_frames, int out_offset);

typedef struct {
    SF_INFO  sfinfo;
    SNDFILE* sffile;
} SndfileDecoder;

struct _WfBuf16 // also defined in waveform.h
{
    short*     buf[WF_STEREO];
    guint      size;
    uint32_t   stamp;
#ifdef WF_DEBUG
    uint64_t   start_frame;
#endif
};


static int
parse_bit_depth (int format)
{
	/* see http://www.mega-nerd.com/libsndfile/api.html */

	switch (format & 0x0f) {
		case SF_FORMAT_PCM_S8: return 8;
		case SF_FORMAT_PCM_16: return 16; /* Signed 16 bit data */
		case SF_FORMAT_PCM_24: return 24; /* Signed 24 bit data */
		case SF_FORMAT_PCM_32: return 32; /* Signed 32 bit data */
		case SF_FORMAT_PCM_U8: return 8;  /* Unsigned 8 bit data (WAV and RAW only) */
		case SF_FORMAT_FLOAT : return 32; /* 32 bit float data */
		case SF_FORMAT_DOUBLE: return 64; /* 64 bit float data */
		default:
#ifdef DEBUG
			gwarn("missing format 0x%x", format);
#endif
			break;
	}
	return 0;
}


int
ad_info_sndfile (WfDecoder* d)
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


static bool
ad_open_sndfile (WfDecoder* decoder, const char* filename)
{
	SndfileDecoder* priv = decoder->d = g_new0(SndfileDecoder, 1);
	priv->sfinfo.format = 0;

	if(!(priv->sffile = sf_open(filename, SFM_READ, &priv->sfinfo))){
		dbg(1, "unable to open file '%s': %i: %s", filename, sf_error(NULL), sf_strerror(NULL));
		g_free(priv);
		return FALSE;
	}
	ad_info_sndfile(decoder);

	return TRUE;
}


int
ad_close_sndfile (WfDecoder* decoder)
{
	SndfileDecoder* priv = (SndfileDecoder*)decoder->d;
	if (!priv) return -1;

	if (!priv->sffile || sf_close(priv->sffile)) {
		perr("bad file close.\n");
		return -1;
	}
	g_free0(decoder->d);
	return 0;
}


int64_t
ad_seek_sndfile (WfDecoder* d, int64_t pos)
{
	SndfileDecoder* priv = (SndfileDecoder*)d->d;
	if (!priv) return -1;
	return sf_seek(priv->sffile, pos, SEEK_SET);
}


/*
 *  Output is interleaved float
 */
ssize_t
ad_read_sndfile (WfDecoder* d, float* out, size_t len)
{
	SndfileDecoder* priv = (SndfileDecoder*)d->d;
	if (!priv) return -1;

	switch(d->info.bit_depth){
		case 32:
			return sf_read_float (priv->sffile, out, len);
		case 16: {
				short* d16 = g_malloc0(len * sizeof(short));
				ssize_t r = sf_read_short (priv->sffile, d16, len);
				int16_to_float(out, d16, d->info.channels, len / d->info.channels, 0);
				g_free(d16);
				return r;
			}
		case 24:
			return sf_read_float(priv->sffile, out, len);
#ifdef DEBUG
		default:
			pwarn("unhandled bit depth: %i", d->info.bit_depth);
#endif
	}

	return -1;
}


ssize_t
ad_read_sndfile_short (WfDecoder* d, WfBuf16* buf)
{
	SndfileDecoder* sf = (SndfileDecoder*)d->d;

	switch(d->info.bit_depth){
		case 8:
		case 16: {
			if(d->info.channels == 1){
				return sf_readf_short(sf->sffile, buf->buf[0], buf->size);
			}else{
				short* data = g_malloc0(d->info.channels * buf->size * sizeof(short));
				ssize_t r = sf_read_short(sf->sffile, data, d->info.channels * buf->size);
				int i, f; for(i=0,f=0;i<r;i+=d->info.channels,f++){
					int c; for(c=0;c<d->info.channels;c++){
						buf->buf[c][f] = data[i];
					}
				}
				g_free(data);
				return f;
			}
		}
		case 24: {
			int* data = g_malloc0(d->info.channels * buf->size * sizeof(int));
			ssize_t r = sf_read_int(sf->sffile, data, d->info.channels * buf->size);
			int i, f; for(i=0,f=0;i<r;i+=d->info.channels,f++){
				int c; for(c=0;c<d->info.channels;c++){
					buf->buf[c][f] = data[i + c] >> 16;
				}
			}
			g_free(data);
			return f;
		}
		case 32: {
			float* data = g_malloc0(d->info.channels * buf->size * sizeof(float));
			ssize_t r = sf_read_float(sf->sffile, data, d->info.channels * buf->size);
			int i, f; for(i=0,f=0;i<r;i+=d->info.channels,f++){
				int c; for(c=0;c<d->info.channels;c++){
					buf->buf[c][f] = AD_FLOAT_TO_SHORT(data[i]);
				}
			}
			g_free(data);
			return r;
		}
		case 0:
			return -1;
#ifdef DEBUG
		default:
			dbg(0, "unhandled bit depth: %i", d->info.bit_depth);
#endif
	}
	return -1;
}


int
ad_eval_sndfile (const char *f)
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
	&ad_read_sndfile,
	&ad_read_sndfile_short
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
get_sndfile ()
{
	return &ad_sndfile;
}

