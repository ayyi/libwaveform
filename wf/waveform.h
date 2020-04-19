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
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __waveform_h__
#define __waveform_h__

#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include <glib-object.h>
#include "wf/typedefs.h"
#include "wf/utils.h"
#include "wf/promise.h"

G_BEGIN_DECLS

#define TYPE_WAVEFORM (waveform_get_type ())
#define WAVEFORM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_WAVEFORM, Waveform))
#define WAVEFORM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_WAVEFORM, WaveformClass))
#define IS_WAVEFORM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_WAVEFORM))
#define IS_WAVEFORM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_WAVEFORM))
#define WAVEFORM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_WAVEFORM, WaveformClass))

typedef struct _WaveformClass WaveformClass;

#define WF_PEAK_RATIO 256              // the number of samples per entry in the peakfile.
                                       // i.e. for every 4 bytes in peakfile, a 16bit audio file has 2*256 bytes
#define WF_PEAK_TEXTURE_SIZE 256       // the number of peakfile datapoints per texture.
                                       // i.e. 256 * 256 = 64k samples per texture, or 0.67 textures for 1 second of audio at 44.1k
#define WF_PEAK_VALUES_PER_SAMPLE 2    // one positive and one negative (unless using shaders).

#define WF_SHOW_RMS
#undef WF_SHOW_RMS

typedef int (*PeakLoader)(Waveform*, const char*);

typedef struct
{
	int64_t start;              // sample frames
	int64_t len;                // sample frames
} WfSampleRegion;

typedef struct
{
	double start;
	double len;
} WfSampleRegionf;

typedef struct
{
	float left;
	float top;
	float len;
	float height;
} WfRectangle;

struct _Waveform
{
	GObject            parent_instance;

	char*              filename;          // either full path, or relative to cwd.
	uint64_t           n_frames;          // audio file size
	int                n_channels;
	bool               is_split;          // true for split stereo files
	int                samplerate;

	bool               offline;
	bool               renderable;

	WfCallback4        free_render_data;

	WaveformPrivate*   priv;
};

struct _WaveformClass {
	GObjectClass parent_class;
};

struct _WfBuf16
{
    short*     buf[WF_STEREO];
    guint      size;                      // number of shorts allocated, NOT bytes. When accessing, note that the last block will likely not be full.
    uint32_t   stamp;                     // put here for now. can move a parallel array if neccesary.
#ifdef WF_DEBUG
    uint64_t   start_frame;
#endif
};

struct _buf
{
	char* buf;
	guint size;
};

//high level api
Waveform*  waveform_load_new             (const char* filename);
void       waveform_set_peak_loader      (PeakLoader);
uint64_t   waveform_get_n_frames         (Waveform*);
int        waveform_get_n_channels       (Waveform*);

//low level api
GType      waveform_get_type             () G_GNUC_CONST;
Waveform*  waveform_new                  (const char* filename);
Waveform*  waveform_construct            (GType);
#define    waveform_unref0(w)            (g_object_unref(w), w = NULL)
void       waveform_load                 (Waveform*, WfCallback3, gpointer);
gboolean   waveform_load_sync            (Waveform*);
void       waveform_set_file             (Waveform*, const char*);

bool       waveform_load_peak            (Waveform*, const char*, int ch_num);
bool       waveform_peak_is_loaded       (Waveform*, int ch_num);
RmsBuf*    waveform_load_rms_file        (Waveform*, int ch);

void       waveform_load_audio           (Waveform*, int block_num, int n_tiers_needed, WfAudioCallback, gpointer);
void       waveform_load_audio_sync      (Waveform*, int block_num, int n_tiers_needed);
short      waveform_find_max_audio_level (Waveform*);

int32_t    wf_get_peakbuf_len_frames     ();

#ifdef __wf_private__
typedef struct { WfCallback3 callback; gpointer user_data; } WfClosure;

#include "wf/private.h"
#endif

#ifndef __waveform_peak_c__
extern WF* wf;
#endif

#endif //__waveform_h__

#ifdef __cplusplus
}
#endif
