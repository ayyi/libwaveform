#ifndef __waveform_peak_h__
#define __waveform_peak_h__
#include <glib.h>
#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "waveform/typedefs.h"

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

#define WF_CACHE_BUF_SIZE (1 << 15)

#define WF_SHOW_RMS
#undef WF_SHOW_RMS

#define WF_PEAK_BLOCK_SIZE (256 * 256)

#define WF_USE_TEXTURE_CACHE           // not currently possible to disable the texture cache

typedef int (*PeakLoader)(Waveform*, const char*, size_t);
typedef struct _texture_cache TextureCache;

enum
{
	WF_MONO = 1,
	WF_STEREO,
};

//a single hires peak block
struct _peakbuf {
	int        block_num;
	int        size;             // the number of shorts allocated. 2 shorts per value (plus + minus)
	int        res;
	int        n_tiers;          // deprecated. use resolution instead.
	int        resolution;       // 1 corresponds to full resolution (though peakbufs never have resolution of 1 as then the audio data is accessed directly)
	void*      buf[WF_STEREO];
	int        maxlevel;         // mostly just for debugging
};

struct _buf
{
	char* buf;
	guint size;
};

typedef struct _buf_stereo
{
	float*     buf[WF_STEREO];
	guint      size;             // number of floats, NOT bytes
} WfBuf;

typedef struct _buf_stereo_16
{
	short*     buf[WF_STEREO];
	guint      size;             // number of shorts allocated, NOT bytes. When accessing, note that the last block will likely not be full.
	uint32_t   stamp;            // put here for now. can move a parallel array if neccesary.
} WfBuf16;

struct _alpha_buf {
	int        width;
	int        height;
	guchar*    buf;
	int        buf_size;
};

typedef struct
{
	uint64_t start;              // sample frames
	uint32_t len;                // sample frames
} WfSampleRegion;

typedef struct _wav_cache {
	WfBuf*         buf;
	WfSampleRegion region;
} WfWavCache;

struct peak_sample{
	short positive;              // even numbered bytes in the src peakfile are positive peaks
	short negative;              // odd  numbered bytes in the src peakfile are negative peaks
};

struct _Waveform
{
	GObject            parent_instance;

	char*              filename;          // either full path, or relative to cwd.
	uint64_t           n_frames;          // audio file size
	int                n_channels;

	gboolean           offline;

	WfWavCache*        cache;
	GPtrArray*         hires_peaks;       // array of Peakbuf* TODO how much does audio_data deprecate this?
	int                num_peaks;         // peak_buflen / PEAK_VALUES_PER_SAMPLE
	RmsBuf*            rms_buf0;
	RmsBuf*            rms_buf1;

	WfGlBlocks*        gl_blocks;         // opengl textures.

	//float            max_db;

	WaveformPriv*      priv;
};

struct _WaveformClass {
	GObjectClass parent_class;
};

typedef struct _waveform_block
{
	Waveform*   waveform;
	int         block;
} WaveformBlock;

//high level api
Waveform*  waveform_load_new           (const char* filename);
void       waveform_set_peak_loader    (PeakLoader);
uint64_t   waveform_get_n_frames       (Waveform*);
int        waveform_get_n_channels     (Waveform*);

//low level api
GType      waveform_get_type           () G_GNUC_CONST;
Waveform*  waveform_new                (const char* filename);
Waveform*  waveform_construct          (GType);
#define    wf_unref0(w)                (g_object_unref(w), w = NULL)
gboolean   waveform_load               (Waveform*);

gboolean   waveform_load_peak          (Waveform*, const char*, int ch_num);
gboolean   waveform_peak_is_loaded     (Waveform*, int ch_num);
RmsBuf*    waveform_load_rms_file      (Waveform*, int ch);

WfBuf16*   waveform_load_audio_async   (Waveform*, int block_num, int n_tiers_needed);
int        waveform_get_n_audio_blocks (Waveform*);
short      waveform_find_max_audio_level(Waveform*);

AlphaBuf*  wf_alphabuf_new             (Waveform*, int blocknum, gboolean is_rms);
void       wf_alphabuf_free            (AlphaBuf*);
GdkPixbuf* wf_alphabuf_to_pixbuf       (AlphaBuf*);

#define USE_GDK_PIXBUF //TODO
#ifdef USE_GDK_PIXBUF
#include <gtk/gtk.h>
void       waveform_peak_to_pixbuf     (Waveform*, GdkPixbuf*, uint32_t src_inset, int* start, int* end, double samples_per_px, GdkColor*, uint32_t colour_bg, float gain);
void       waveform_rms_to_pixbuf      (Waveform*, GdkPixbuf*, uint32_t src_inset, int* start, int* end, double samples_per_px, GdkColor*, uint32_t colour_bg, float gain);
#endif
int32_t    wf_get_peakbuf_len_frames   ();

#ifdef __wf_private__
#include "wf_private.h"
#endif

#endif //__waveform_peak_h__
