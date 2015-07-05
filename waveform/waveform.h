/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __waveform_h__
#define __waveform_h__

/*
#include "waveform/alphabuf.h"
*/

#include <stdint.h>
#include <glib.h>
#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "stdint.h"
#include "waveform/typedefs.h"
#ifdef USE_GDK_PIXBUF
#include <gtk/gtk.h>
//#else
//#ifndef GdkPixbuf
//#define GdkPixbuf void
//#endif
//#if !defined(GdkColor)
//#define GdkColor void
//#endif
#endif
#include "waveform/utils.h"

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

enum
{
	WF_MONO = 1,
	WF_STEREO,
};

typedef struct
{
	uint64_t start;              // sample frames
	uint64_t len;                // sample frames
} WfSampleRegion;

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
	gboolean           is_split;          // true for split stereo files

	gboolean           offline;
	gboolean           renderable;

	WaveformPriv*      priv;
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

gboolean   waveform_load_peak            (Waveform*, const char*, int ch_num);
gboolean   waveform_peak_is_loaded       (Waveform*, int ch_num);
RmsBuf*    waveform_load_rms_file        (Waveform*, int ch);

void       waveform_load_audio           (Waveform*, int block_num, int n_tiers_needed, WfAudioCallback, gpointer);
short      waveform_find_max_audio_level (Waveform*);

typedef void (WfPixbufCallback)(Waveform*, GdkPixbuf*, gpointer);

void       waveform_peak_to_pixbuf       (Waveform*, GdkPixbuf*, WfSampleRegion*, uint32_t colour, uint32_t bg_colour);
void       waveform_peak_to_pixbuf_async (Waveform*, GdkPixbuf*, WfSampleRegion*, uint32_t colour, uint32_t bg_colour, WfPixbufCallback, gpointer);
void       waveform_peak_to_pixbuf_full  (Waveform*, GdkPixbuf*, uint32_t src_inset, int* start, int* end, double samples_per_px, uint32_t colour, uint32_t bg_colour, float gain);
void       waveform_rms_to_pixbuf        (Waveform*, GdkPixbuf*, uint32_t src_inset, int* start, int* end, double samples_per_px, GdkColor*, uint32_t bg_colour, float gain);

int32_t    wf_get_peakbuf_len_frames     ();

typedef struct { WfCallback2 callback; gpointer user_data; } WfClosure;

#ifdef __wf_private__
#include "wf_private.h"
#endif

#ifdef __gl_h_
#include "waveform/actor.h"
#include "waveform/canvas.h"
#endif

#ifndef __waveform_peak_c__
extern WF* wf;
#endif

#endif //__waveform_h__

#ifdef __cplusplus
}
#endif
