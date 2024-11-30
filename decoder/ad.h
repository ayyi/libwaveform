/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2016-2024 Tim Orford <tim@orford.org>                  |
 | copyright (C) 2011 Robin Gareus <robin@gareus.org>                   |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include "wf/typedefs.h"

typedef struct
{
   uint32_t   sample_rate;
   uint16_t   channels;
   int64_t    length;       // milliseconds
   int64_t    frames;       // total number of frames (eg a frame for 16bit stereo is 4 bytes).
   int32_t    bit_rate;
   int16_t    bit_depth;
   GPtrArray* meta_data;
} WfAudioInfo;

typedef struct _AdPlugin AdPlugin;

typedef struct
{
    WfAudioInfo     info;
    const AdPlugin* b;      // impl
    void*           d;      // private impl data
} WfDecoder;

struct _AdPlugin
{
    int      (*eval)       (const char*);
    bool     (*open)       (WfDecoder*, const char*);
    int      (*close)      (WfDecoder*);
    int      (*info)       (WfDecoder*);
    int64_t  (*seek)       (WfDecoder*, int64_t);
    ssize_t  (*read)       (WfDecoder*, float*, size_t);
    ssize_t  (*read_short) (WfDecoder*, WfBuf16*);
    ssize_t  (*read_s32)   (WfDecoder*, int32_t*, size_t);
};


typedef struct
{
    int      width;
    int      height;
    int      row_stride;
    uint8_t* data;
} AdPicture;

/* low level API */
bool     ad_open          (WfDecoder*, const char*);
int      ad_close         (WfDecoder*);
void     ad_clear         (WfDecoder*);
int64_t  ad_seek          (WfDecoder*, int64_t);
ssize_t  ad_read          (WfDecoder*, float*, size_t);
ssize_t  ad_read_short    (WfDecoder*, WfBuf16*);
ssize_t  ad_read_s32      (WfDecoder*, int32_t*, size_t);
int      ad_info          (WfDecoder*);

void     ad_thumbnail     (WfDecoder*, AdPicture*);
void     ad_thumbnail_free(WfDecoder*, AdPicture*);

void     ad_clear_nfo     (WfAudioInfo*);
void     ad_free_nfo      (WfAudioInfo*);

/* high level API - wrappers around low-level functions */
bool     ad_finfo         (const char*, WfAudioInfo*);
void     ad_print_nfo     (int dbglvl, WfAudioInfo*);
ssize_t  ad_read_mono_dbl (WfDecoder*, double*, size_t);

/* hardcoded backends */
#ifdef USE_SNDFILE
const AdPlugin* get_sndfile ();
#endif
#ifdef USE_FFMPEG
const AdPlugin* get_ffmpeg  ();
#endif

#define ad_is_open(D) (D.d != NULL)

#define AD_FLOAT_TO_SHORT(A) round(A * 32767.f); // SHRT_MAX

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC(WfDecoder, ad_clear)
