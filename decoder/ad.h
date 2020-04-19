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
#ifndef __ad_h__
#define __ad_h__
#include <unistd.h>
#include <stdint.h>
#include <glib.h>
#include "wf/typedefs.h"

typedef struct
{
   uint16_t   sample_rate;
   uint16_t   channels;
   int64_t    length;       // milliseconds
   int64_t    frames;       // total number of frames (eg a frame for 16bit stereo is 4 bytes).
   int16_t    bit_rate;
   int16_t    bit_depth;
   GPtrArray* meta_data;
} WfAudioInfo;

typedef struct _AdPlugin AdPlugin;

#ifndef __ad_plugin_c__
typedef struct
{
    WfAudioInfo     info;
    const AdPlugin* b;      // backend
    void*           d;      // private backend data
} WfDecoder;
#else
typedef struct _WfDecoder WfDecoder;
#endif

struct _AdPlugin
{
    int      (*eval)       (const char*);
    gboolean (*open)       (WfDecoder*, const char*);
    int      (*close)      (WfDecoder*);
    int      (*info)       (WfDecoder*);
    int64_t  (*seek)       (WfDecoder*, int64_t);
    ssize_t  (*read)       (WfDecoder*, float*, size_t);
    ssize_t  (*read_short) (WfDecoder*, WfBuf16*);
};


/* low level API */
gboolean ad_open          (WfDecoder*, const char*);
int      ad_close         (WfDecoder*);
int64_t  ad_seek          (WfDecoder*, int64_t);
ssize_t  ad_read          (WfDecoder*, float*, size_t);
ssize_t  ad_read_short    (WfDecoder*, WfBuf16*);
int      ad_info          (WfDecoder*);

void     ad_clear_nfo     (WfAudioInfo*);
void     ad_free_nfo      (WfAudioInfo*);

/* high level API - wrappers around low-level functions */
gboolean ad_finfo         (const char*, WfAudioInfo*);
void     ad_print_nfo     (int dbglvl, WfAudioInfo*);
ssize_t  ad_read_mono_dbl (WfDecoder*, double*, size_t);

/* hardcoded backends */
const AdPlugin* get_sndfile ();
#ifdef USE_FFMPEG
const AdPlugin* get_ffmpeg  ();
#endif

#define AD_FLOAT_TO_SHORT(A) (A * (1<<15));

#endif
