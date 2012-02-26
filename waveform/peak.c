/*
  copyright (C) 2012 Tim Orford <tim@orford.org>

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
#define __waveform_peak_c__
#define __wf_private__
#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <sndfile.h>
#include <gtk/gtk.h>
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/loaders/ardour.h"
#include "waveform/loaders/riff.h"
#include "waveform/texture_cache.h"
#include "waveform/audio.h"
#include "waveform/peakgen.h"

static gpointer waveform_parent_class = NULL;
#define WAVEFORM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_WAVEFORM, WaveformPriv))
enum  {
	WAVEFORM_DUMMY_PROPERTY,
	WAVEFORM_PROPERTY1
};

//#define WF_TEXTURE_HEIGHT 128 //1024
#define WF_TEXTURE_HEIGHT 256 //intel 945 seems to work better with square textures
#define BITS_PER_PIXEL 8
typedef struct _drect { double x1, y1, x2, y2; } DRect;

WF* wf = NULL;
guint peak_idle = 0;
/*
		textures
		--------
		-need to know if they are in use.
		-if purged, need to clear all references.
		-need to link waveform -> texture

 */

static void  waveform_finalize     (GObject*);
static void _waveform_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec);

static void  wf_peak_to_alphabuf   (Waveform*, AlphaBuf*, int* start, int* end, GdkColor* colour, uint32_t colour_bg);
static void  wf_rms_to_alphabuf    (Waveform*, AlphaBuf*, int* start, int* end, double samples_per_px, GdkColor* colour, uint32_t colour_bg);
static void  wf_set_last_fraction  (Waveform*);
static int   get_n_textures        (WfGlBlocks*);

struct _buf_info
{
	short* buf;          //source buffer
	guint  len;
	guint  len_frames;
};

struct _rms_buf_info
{
	char*  buf;          //source buffer
	guint  len;
	guint  len_frames;
};



Waveform*
waveform_load_new(const char* filename)
{
	g_return_val_if_fail(filename, NULL);

	Waveform* w = waveform_new(filename);
	waveform_load(w);
	return w;
}


WF*
wf_get_instance()
{
	if(!wf){
		wf = g_new0(WF, 1);
		wf->peak_cache = g_hash_table_new(g_direct_hash, g_direct_equal);
		wf->audio.cache = g_hash_table_new(g_direct_hash, g_direct_equal);
		wf->load_peak = wf_load_riff_peak; //set the default loader
		wf->pref_use_shaders = true;
		wf->msg_queue = g_async_queue_new();

#ifdef WF_USE_TEXTURE_CACHE
		texture_cache_init();
#endif
	}
	return wf;
}


Waveform*
waveform_construct (GType object_type)
{
	Waveform* self = (Waveform*) g_object_new (object_type, NULL);
	return self;
}


Waveform*
waveform_new(const char* filename)
{
	wf_get_instance();

	Waveform* w = waveform_construct(TYPE_WAVEFORM);
	w->priv = g_new0(WaveformPriv, 1);
	w->filename = g_strdup(filename);
	w->hires_peaks = g_ptr_array_new();
	w->priv->audio_data = g_new0(WfAudioData, 1);
	return w;
}


static void
waveform_class_init (WaveformClass* klass)
{
	waveform_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof(WaveformPriv));
	G_OBJECT_CLASS (klass)->get_property = _waveform_get_property;
	G_OBJECT_CLASS (klass)->finalize = waveform_finalize;
	g_object_class_install_property (G_OBJECT_CLASS (klass), WAVEFORM_PROPERTY1, g_param_spec_int ("property1", "property1", "property1", G_MININT, G_MAXINT, 0, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
	g_signal_new ("peakdata_ready", TYPE_WAVEFORM, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}


static void
waveform_instance_init (Waveform* self)
{
	self->priv = WAVEFORM_GET_PRIVATE (self);
	self->priv->_property1 = 1;
}


static void
__finalize(Waveform* w)
{
	PF0;
	if(g_hash_table_size(wf->peak_cache) && !g_hash_table_remove(wf->peak_cache, w)) gwarn("failed to remove waveform from peak_cache");

	void wf_wav_cache_free(WfWavCache* cache)
	{
		if(!cache) return;

		if(cache->buf){
			int c; for(c=0;c<WF_STEREO;c++) if(cache->buf->buf[c]) g_free(cache->buf->buf[c]);
			g_free(cache->buf);
		}
		g_free(cache);
	}

	wf_wav_cache_free(w->cache);

	int i; for(i=0;i<WF_MAX_CH;i++){
		if(w->priv->peak.buf[i]) g_free(w->priv->peak.buf[i]);
	}
	if(w->gl_blocks){
		texture_cache_remove(w);

		int c; for(c=0;c<WF_MAX_CH;c++){
			if(w->gl_blocks->peak_texture[c].main) g_free(w->gl_blocks->peak_texture[c].main);
			if(w->gl_blocks->peak_texture[c].neg) g_free(w->gl_blocks->peak_texture[c].neg);
		}
		g_free(w->gl_blocks);
	}
	wf_audio_free(w);
	g_free(w->priv);
	g_free(w->filename);
}


static void
waveform_finalize (GObject* obj)
{
	PF0;
	Waveform* w = WAVEFORM(obj);
	__finalize(w);
	G_OBJECT_CLASS (waveform_parent_class)->finalize (obj);
	dbg(1, "done");
}


GType
waveform_get_type ()
{
	static volatile gsize waveform_type_id__volatile = 0;
	if (g_once_init_enter (&waveform_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (WaveformClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) waveform_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (Waveform), 0, (GInstanceInitFunc) waveform_instance_init, NULL };
		GType waveform_type_id = g_type_register_static (G_TYPE_OBJECT, "Waveform", &g_define_type_info, 0);
		g_once_init_leave (&waveform_type_id__volatile, waveform_type_id);
	}
	return waveform_type_id__volatile;
}


static void
_waveform_get_property (GObject* object, guint property_id, GValue* value, GParamSpec* pspec)
{
	//Waveform* w = WAVEFORM(object);
	switch (property_id){
		case WAVEFORM_PROPERTY1:
			//g_value_set_int(value, wf_get_property1(w));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}


gint
wf_get_property1(Waveform* self)
{
	g_return_val_if_fail (self, 0);
	return self->priv->_property1;
}


#if 0
void
wf_set_file(Waveform* w, const char* filename)
{
	PF0;
	w->n_frames = 0;
}
#endif


gboolean
waveform_load(Waveform* w)
{
	g_return_val_if_fail(w, false);

	char* peakfile = waveform_ensure_peakfile(w);
	if(peakfile){
		gboolean loaded = waveform_load_peak(w, peakfile, 0);
		g_free(peakfile);
		return loaded;
	}
	return false;
}


static void
waveform_get_sf_data(Waveform* w)
{
	SNDFILE* sndfile;
	SF_INFO sfinfo;
	sfinfo.format = 0;
	if(!(sndfile = sf_open(w->filename, SFM_READ, &sfinfo))){
		if(!g_file_test(w->filename, G_FILE_TEST_EXISTS)){
			gwarn("file open failure. file doesnt exist.");
		}else{
			gwarn("file open failure.");
		}
		w->offline = true;
	}
	sf_close(sndfile);

	w->n_frames = sfinfo.frames;
	w->n_channels = sfinfo.channels;
}


uint64_t
waveform_get_n_frames(Waveform* w)
{
	if(w->n_frames) return w->n_frames;

	waveform_get_sf_data(w);

	return w->n_frames;
}


int
waveform_get_n_channels(Waveform* w)
{
	g_return_val_if_fail(w, 0);

	if(w->n_frames) return w->n_channels;

	if(w->offline) return 0;

	waveform_get_sf_data(w);

	return w->n_channels;
}


/*
 *  @param ch_num - must be 0 or 1. Should be 0 unless loading rhs for split file.
 *
 *  Load the pre-existing peak file from disk into a buffer.
 *
 *  This function can be used to add an additional channel to an existing Waveform
 *  where the audio consists of split files.
 */
gboolean
waveform_load_peak(Waveform* w, const char* peak_file, int ch_num)
{
	g_return_val_if_fail(w, false);
	g_return_val_if_fail(ch_num <= 2, false);

	size_t size = 0;
	int n_channels = wf->load_peak(w, peak_file, size); //not currently passing the size. If we decide not to use it, this arg should be removed.

	w->num_peaks = w->priv->peak.size / WF_PEAK_VALUES_PER_SAMPLE;
	dbg(1, "ch=%i peak_buf=%p num_peaks=%i", ch_num, w->priv->peak.buf[ch_num], w->num_peaks);
#if 0
	int i; for (i=0;i<20;i++) printf("      %i %i\n", w->priv->peak.buf[0][2 * i], w->priv->peak.buf[0][2 * i + 1]);
#endif
	if(!w->gl_blocks){
		w->gl_blocks = g_new0(WfGlBlocks, 1);
		w->gl_blocks->size = w->num_peaks / WF_PEAK_TEXTURE_SIZE + ((w->num_peaks % WF_PEAK_TEXTURE_SIZE) ? 1 : 0);
		dbg(1, "creating glbocks... num_blocks=%i", w->gl_blocks->size);
		w->gl_blocks->peak_texture[0].main = g_new0(unsigned, w->gl_blocks->size);
		w->gl_blocks->peak_texture[0].neg = g_new0(unsigned, w->gl_blocks->size);
#ifdef WF_SHOW_RMS
		gerr("rms TODO");
		w->gl_blocks->rms_texture = g_new0(unsigned, w->gl_blocks->size);
#warning TODO where best to init rms textures? see peak_textures
		extern void glGenTextures(size_t n, uint32_t* textures);
		glGenTextures(w->gl_blocks->size, w->gl_blocks->rms_texture); //note currently doesnt use the texture_cache
#endif
	}

	wf_set_last_fraction(w);

	if(ch_num == 1 || n_channels == 2){
		w->gl_blocks->peak_texture[1].main = g_new0(unsigned, w->gl_blocks->size);
		w->gl_blocks->peak_texture[1].neg = g_new0(unsigned, w->gl_blocks->size);
	}

	return !!w->priv->peak.buf[ch_num];
}


gboolean
waveform_peak_is_loaded(Waveform* w, int ch_num)
{
	return !!w->priv->peak.buf[ch_num];
}


#if 0 //replaced with blocked version
gboolean
wf_load_chunk(Waveform* w, WfSampleRegion region)
{
	// non-audio-block version. possibly deprecated by the block version for performance.

	PF;
	g_return_val_if_fail(w, false);

	WfWavCache* cache = w->cache;

	if (cache->region.start != -1) {
		if ((region.start >= cache->region.start) && (region.start + region.len <= cache->region.start + cache->region.len)){
			dbg(1, "cache hit");
			return true; // already loaded
		}
	}
	gwarn("cache miss: %Lu --> %i", region.start, region.len);

	SNDFILE* sndfile;
	SF_INFO sfinfo;
	sfinfo.format = 0;
	if(!(sndfile = sf_open(w->filename, SFM_READ, &sfinfo))){
		if(!g_file_test(w->filename, G_FILE_TEST_EXISTS)){
			gwarn("file open failure. file doesnt exist.");
		}else{
			gwarn("file open failure.");
		}
		return false;
	}
	sf_count_t n_frames = MIN(sfinfo.frames, WF_CACHE_BUF_SIZE); //fill the buffer even if bigger than the request.
	//set the seek position so that the requested region is in the middle (so we can scroll backwards and forwards).
	uint64_t excess = cache->buf->size - region.len;
	uint64_t seek = region.start - MIN(region.start, excess / 2);
	//dbg(1, "size=%i len=%i", cache->buf->size, region.len);
	//dbg(1, "excess=%Lu seek=%Lu", excess, seek);

	sf_count_t pos = sf_seek(sndfile, seek, SEEK_SET);
	dbg(2, "pos=%Li n_frames=%Li", pos, n_frames);
	g_return_val_if_fail(pos > -1, false);

	int readcount;
	switch(sfinfo.channels){
		case WF_MONO:
			if((readcount = sf_readf_float(sndfile, w->cache->buf->buf[WF_LEFT], n_frames)) < n_frames){
				gwarn("unexpected EOF: %s", w->filename);
				gwarn("                start_frame=%Lu n_frames=%Lu/%Lu read=%i", region.start, n_frames, sfinfo.frames, readcount);
			}
			break;
		case WF_STEREO:
			{
			float read_buf[n_frames * WF_STEREO];

			if((readcount = sf_readf_float(sndfile, read_buf, n_frames)) < n_frames){
				gwarn("unexpected EOF: %s", w->filename);
				gwarn("                STEREO start_frame=%Lu n_frames=%Lu/%Lu read=%i", region.start, n_frames, sfinfo.frames, readcount);
			}

			#if 0 //only useful for testing.
			memset(w->cache->buf->buf[0], 0, WF_CACHE_BUF_SIZE);
			memset(w->cache->buf->buf[1], 0, WF_CACHE_BUF_SIZE);
			#endif

			deinterleave(read_buf, w->cache->buf->buf, n_frames);
			}
			break;
		default:
			break;
	}
	sf_close(sndfile);
	dbg(1, "readcount=%i", readcount);

	w->cache->region.start = pos;//region.start;
	w->cache->region.len = readcount;

	return true;
}
#endif


//dupe
static void
sort_(short* dest, const short* src, int size)
{
	//sort j into ascending order
	int i, j=0, min, new_min, top=size, p=0;
	guint16 n[4] = {0, 0, 0, 0};

	for(i=0;i<size;i++) n[i] = src[i]; //copy the source array

	for(i=0;i<size;i++){
		min = n[0];
		p=0;
		for(j=1;j<top;j++){
			new_min = MIN(min, n[j]);
			if(new_min < min){
				min = new_min;
				p = j;
			}
		}
		//printf("  %i min=%i p=%i", i, min, p);
		//p is the index to the value we have used, and need to remove.

		dest[i] = min;

		int m; for(m=p;m<top;m++) n[m] = n[m+1]; //move remaining entries down.
		//printf(" top=%i", top);
		top--;
		n[top] = 0;
	}
	//printf("  %i %i %i %i --> %i %i %i %i\n", src[0], src[1], src[2], src[3], dest[0], dest[1], dest[2], dest[3]);
}


#define MAX_PART_HEIGHT 1024 //FIXME
//dupe
struct _line{
	guchar a[MAX_PART_HEIGHT]; //alpha level for each pixel in the line.
};
static struct _line line[3];


//dupe
static void
line_write(struct _line* line, int index, guchar val)
{
	//remove fn once debugging complete?
	// __func__ is null!
	if(index < 0 || index >= MAX_PART_HEIGHT){ gerr ("y=%i", index); return; }
	line->a[index] = val;
}


//dupe
static void
line_clear(struct _line* line)
{
	int i;
	for(i=0;i<MAX_PART_HEIGHT;i++) line->a[i] = 0;
}


static void
alphabuf_draw_line(AlphaBuf* pixbuf, DRect* pts, double line_width, GdkColor* colour)
{
}


static inline gboolean
get_buf_info(const Waveform* w, int block_num, struct _buf_info* b, int ch)
{
	//buf_info is just a convenience. It is filled twice during a tile draw.

	gboolean hires_mode = (block_num > -1);

	b->buf        = w->priv->peak.buf[ch]; //source buffer.
	b->len        = w->priv->peak.size;
	b->len_frames = 0;
	if(hires_mode){
		WfAudioData* audio = w->priv->audio_data;
		b->buf = audio->buf16[block_num]->buf[ch];
		g_return_val_if_fail(b->buf, false);
		b->len = audio->buf16[block_num]->size;
		//dbg(2, "not empty. block=%i peaklevel=%.2f", peakbuf->block_num, int2db(peakbuf->maxlevel));
	}
	b->len_frames = b->len / WF_PEAK_VALUES_PER_SAMPLE;

	return true;
}


static inline gboolean
get_rms_buf_info(const char* buf, guint len, struct _rms_buf_info* b, int ch)
{
	//buf_info is just a convenience. It is filled twice during a tile draw.

	//gboolean hires_mode = (peakbuf != NULL);

	b->buf        = (char*)buf; //source buffer.
	b->len        = len;
	b->len_frames = b->len;// / WF_PEAK_VALUES_PER_SAMPLE;
	return TRUE;
}


static AlphaBuf*
_alphabuf_new(int width, int height)
{
	AlphaBuf* a = g_new0(AlphaBuf, 1);
	a->width = width;
	a->height = height;
	a->buf_size = width * height;
	a->buf = g_malloc(a->buf_size);
	return a;
}


static void
wf_set_last_fraction(Waveform* waveform)
{
	int width = WF_PEAK_TEXTURE_SIZE;
	int width_ = waveform->num_peaks % WF_PEAK_TEXTURE_SIZE;
	waveform->gl_blocks->last_fraction = (double)width_ / (double)width;
	//dbg(1, "num_peaks=%i last_fraction=%.2f", waveform->num_peaks, waveform->gl_blocks->last_fraction);
}


AlphaBuf*
wf_alphabuf_new(Waveform* waveform, int blocknum, gboolean is_rms)
{
	//copy part of the audio peakfile at 1:1 scale to a pixbuf suitable for use as a GL texture.
	// @param blocknum - if -1, use the whole peakfile.

	PF2;
	g_return_val_if_fail(waveform->num_peaks, NULL);
	GdkColor fg_colour = {0, 0xffff, 0xffff, 0xffff};

	int px_start;
	int px_stop;
	int width;
	if(blocknum == -1){
		px_start = 0;
		px_stop  = width = waveform->num_peaks;
	}else{
		int n_blocks = get_n_textures(waveform->gl_blocks);
		dbg(2, "block %i/%i", blocknum, n_blocks);
		gboolean is_last = (blocknum == n_blocks - 1);

		px_start =  blocknum      * WF_PEAK_TEXTURE_SIZE;
		px_stop  = (blocknum + 1) * WF_PEAK_TEXTURE_SIZE;

		width = WF_PEAK_TEXTURE_SIZE;
		if(is_last){
			int width_ = waveform->num_peaks % WF_PEAK_TEXTURE_SIZE;
			dbg(1, "is_last width_=%i", width_);
			width      = wf_power_of_two(width_);
			//_wf_set_last_fraction(waveform);
			//px_stop  = px_start + WF_PEAK_TEXTURE_SIZE * blocks->last_fraction;
			px_stop  = px_start + width_;
		}
		dbg (2, "block_num=%i width=%i px_stop=%i", blocknum, width, px_stop, is_last);
	}
	AlphaBuf* buf = _alphabuf_new(width, is_rms ? WF_TEXTURE_HEIGHT / 2: WF_TEXTURE_HEIGHT);

//dbg(1, "peakbuf_len=%i buf0=%p", peakbuf_len, f_item.ppppp.buf[0]);
	//f_item.hires_peaks = NULL; //FIXME
	if(is_rms){
		#define SCALE_BODGE 2;
		double samples_per_px = WF_PEAK_TEXTURE_SIZE * SCALE_BODGE;
		uint32_t bg_colour = 0x00000000;
		wf_rms_to_alphabuf(waveform, buf, &px_start, &px_stop, samples_per_px, &fg_colour, bg_colour);
	}else{

		uint32_t bg_colour = 0x00000000;
		wf_peak_to_alphabuf(waveform, buf, &px_start, &px_stop, &fg_colour, bg_colour);
	}

#if 0
	//put dots in corner for debugging:
	{
		buf->buf [0]                              = 0xff;
		buf->buf [buf->width   - 1]               = 0xff;
		buf->buf [buf->width * (buf->height - 1)] = 0xff;
		buf->buf [buf->buf_size -1]               = 0xff;
	}
#endif

	return buf;
}


void
wf_alphabuf_free(AlphaBuf* a)
{
	if(a){
		g_free(a->buf);
		g_free(a);
	}
}


GdkPixbuf*
wf_alphabuf_to_pixbuf(AlphaBuf* a)
{
	GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, BITS_PER_PIXEL, a->width, a->height);
	guchar* buf = gdk_pixbuf_get_pixels(pixbuf);
	int n = gdk_pixbuf_get_n_channels(pixbuf);

	int y; for(y=0;y<a->height;y++){
		int py = y * gdk_pixbuf_get_rowstride(pixbuf);
		//g_return_val_if_fail(py + (a->width -1) * n + 2 < (gdk_pixbuf_get_rowstride(pixbuf) * a->height), pixbuf);
		int x; for(x=0;x<a->width;x++){
			buf[py + x * n    ] = a->buf[y * a->width + x];
			buf[py + x * n + 1] = a->buf[y * a->width + x];
			buf[py + x * n + 2] = a->buf[y * a->width + x];
		}
	}
	return pixbuf;
}


short*
wf_peak_malloc(Waveform* w, uint32_t bytes)
{
	short* buf = g_malloc(bytes);
	wf->peak_mem_size += bytes;
	g_hash_table_insert(wf->peak_cache, w, w); //is removed in wf_unref()

	//(debug) check cache size
	int total_size = 0;
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init (&iter, wf->peak_cache);
	while (g_hash_table_iter_next (&iter, &key, &value)){
		Waveform* w = value;
		if(w->priv->peak.buf[0]) total_size += w->priv->peak.size;
		if(w->priv->peak.buf[1]) total_size += w->priv->peak.size;
	}
	dbg(2, "peak cache: size=%ik", total_size/1024);

	return buf;
}


static void
wf_peak_to_alphabuf(Waveform* w, AlphaBuf* a, int* start, int* end, GdkColor* colour, uint32_t colour_bg)
{
	/*
     renders a peakfile (loaded into the buffer given by waveform->buf) onto the given 8 bit alpha-map buffer.

	 -the given buffer is for a single block only.
	 Can be simplified. Eg, the antialiasing should be removed.

     @param start - if set, we start rendering at this value from the left of the pixbuf.
     @param end   - if set, num of pixels from left of pixbuf to stop rendering at.
                    Can be bigger than pixbuf width.

     further optimisation:
     -dont scan pixels above the peaks. Should we record the overall file peak level?
	*/

	g_return_if_fail(a);
	g_return_if_fail(w);
	g_return_if_fail(w->priv->peak.buf[0]);

	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);

	struct peak_sample sample;
	//short sample_positive;    //even numbered bytes in the src peakfile are positive peaks;
	//short sample_negative;    //odd  numbered bytes in the src peakfile are negative peaks;
	short min;                //negative peak value for each pixel.
	short max;                //positive peak value for each pixel.

	int n_chans     = waveform_get_n_channels(w);
	int width       = a->width;
	int height      = a->height;
	g_return_if_fail(width && height);
	guchar* pixels  = a->buf;
	int rowstride   = width;
	int n_channels  = 1;
	int ch_height   = height / n_chans;
	int vscale      = (256*128*2) / ch_height;

	int px=0;                    //pixel count starting at the lhs of the Part.
	int j,y;
	int src_start=0;             //index into the source buffer for each sample pt.
	int src_stop =0;   

	double xmag = 1; //making xmag smaller increases the visual magnification.
	                 //-the bigger the mag, the less samples we need to skip.
	                 //-as we're only dealing with smaller peak files, we need to adjust by the RATIO.

	int px_start = start ? *start : 0;
	int px_stop  = *end;//   ? MIN(*end, width) : width;
	dbg (2, "start=%i end=%i", px_start, px_stop);
	//dbg (1, "width=%i height=%i", width, height);

	if(width < px_stop - px_start){ gwarn("alphabuf too small? %i < %i", width, px_stop - px_start); return; }

	int ch; for(ch=0;ch<n_chans;ch++){

		struct _buf_info b; 
		get_buf_info(w, -1, &b, ch);
		g_return_if_fail(b.buf);
		g_return_if_fail(b.len < 1000000);

		line_clear(&line[0]);
		int line_index = 0;

		for(px=px_start;px<px_stop;px++){
			src_start = ((int)( px   * xmag));
			src_stop  = ((int)((px+1)* xmag));
			//printf("%i ", src_start);

			struct _line* previous_line = &line[(line_index  ) % 3];
			struct _line* current_line  = &line[(line_index+1) % 3];
			struct _line* next_line     = &line[(line_index+2) % 3];

			//arrays holding subpixel levels.
			short lmax[4] = {0,0,0,0};
			short lmin[4] = {0,0,0,0};
			short k   [4] = {0,0,0,0}; //sorted copy of l.

			int mid = ch_height / 2;

			if(src_stop < b.len/2){
				min = 0; max = 0;
				int n_sub_px = 0;
				for(j=src_start;j<src_stop;j++){ //iterate over all the source samples for this pixel.
					sample.positive = b.buf[2*j   ];
					sample.negative = b.buf[2*j +1];
					if(sample.positive > max) max = sample.positive;
					if(sample.negative < min) min = sample.negative;

					if(n_sub_px < 4){
						//FIXME these supixels are not evenly distributed when > 4 available, as we currently only use the first 4.
						lmax[n_sub_px] = (ch_height * sample.positive) / (256*128*2);
						lmin[n_sub_px] =-(ch_height * sample.negative) / (256*128*2); //lmin contains positive values.
					}
					n_sub_px++;
				}
				if(!n_sub_px) continue;

				if(!line_index){
					//first line - we also grab the previous sample for antialiasing.
					if(px){
						j = src_start - 1;
						sample.positive = b.buf[2*j   ];
						sample.negative = b.buf[2*j +1];
						max = sample.positive / vscale;
						min =-sample.negative / vscale;
						//printf(" j=%i max=%i min=%i\n", j, max, min);
						for(y=mid;y<mid+max;y++) line_write(previous_line, y, 0xff);
						for(y=mid;y<mid+max;y++) line_write(current_line, y, 0xff);
						for(y=mid;y>mid-max;y--) line_write(previous_line, y, 0xff);
						for(y=mid;y>mid-max;y--) line_write(current_line, y, 0xff);
					}
					else line_clear(current_line);
				}

				//scale the values to the part height:
				min = (min * ch_height) / (256*128*2);
				max = (max * ch_height) / (256*128*2);

				sort_(k, lmax, MIN(n_sub_px, 4));

				//note that we write into next_line, not current_line. There is a visible delay of one line.
				int alpha = 0xff;
				int v = 0;
				int s, a;
				for(s=0;s<MIN(n_sub_px, 4);s++){
					//printf(" v=%i->%i ", v, k[s]);
					for(y=v;y<k[s];y++){
						line_write(next_line, mid +y, alpha);
					}
					v = k[s];
					alpha = (alpha * 2) / 3;
				}
				line_write(next_line, mid+k[s-1], alpha/2); //blur!
				for(y=k[s-1]+1;y<mid;y++){
					next_line->a[mid + y] = 0;
				}

				//negative peak:
				sort_(k, lmin, MIN(n_sub_px, 4));
				alpha = 0xff;
				v = mid;
				for(s=0;s<MIN(n_sub_px, 4);s++){
					//for(y=v;y>mid-k[s];y--) next_line->a[y] = alpha;
					for(y=v;y>mid-k[s];y--) line_write(next_line, y, alpha);
					v=mid-k[s];
					alpha = (alpha * 2) / 3;
				}
				line_write(next_line, mid-k[s-1], alpha/2); //antialias the end of the line.
				for(y=mid-k[s-1]-1;y>0;y--) line_write(next_line, y, 0);
				//printf("%.1f %i ", xf, height/2+min);

				//debugging:
#if 0
				for(y=height/2;y<height;y++){
					int b = MIN((current_line->a[y] * 3)/4 + previous_line->a[y]/6 + next_line->a[y]/6, 0xff);
					if(!b){ dbg_max = MAX(y, dbg_max); break; }
				}
#endif

				//draw the lines:
				int blur = 6; //bigger value gives less blurring.
				for(y=0;y<ch_height;y++){
					int p = ch*ch_height*rowstride + (ch_height - y -1)*rowstride + n_channels*(px-px_start);
					if(p > rowstride*height || p < 0){ gerr ("p! %i > %i px=%i y=%i row=%i rowstride=%i=%i", p, 3*width*height, px, y, height-y-1, rowstride, 3*width); return; }

					a = MIN((current_line->a[y] * 3)/4 + previous_line->a[y]/blur + next_line->a[y]/blur, 0xff);
					pixels[p] = (int)(0xff * a) >> 8;
				}

			}else{
				//no more source data available - as the pixmap is clear, we have nothing much to do.
				//gdk_draw_line(GDK_DRAWABLE(pixmap), gc, px, 0, px, height);//x1, y1, x2, y2
				DRect pts = {px, 0, px, ch_height};
				alphabuf_draw_line(a, &pts, 1.0, colour);
			}
			//next = srcidx + 1;
			//xf += WF_PEAK_RATIO * WF_PEAK_VALUES_PER_SAMPLE / samples_per_px;
			//printf("%.1f ", xf);

			line_index++;
			//printf("line_index=%i %i %i %i\n", line_index, (line_index  ) % 3, (line_index+1) % 3, (line_index+2) % 3);
		}
	}

	//printf("%s(): done. drawn: %i of %i stop=%i\n", __func__, line_done_count, width, src_stop);

	gettimeofday(&time_stop, NULL);
	time_t secs = time_stop.tv_sec - time_start.tv_sec;
	suseconds_t usec;
	if(time_stop.tv_usec > time_start.tv_usec){
		usec = time_stop.tv_usec - time_start.tv_usec;
	}else{
		secs -= 1;
		usec = time_stop.tv_usec + 1000000 - time_start.tv_usec;
	}
	//printf("%s(): start=%03i: %lu:%06lu\n", __func__, px_start, secs, usec);
}


static void
wf_rms_to_alphabuf(Waveform* pool_item, AlphaBuf* pixbuf, int* start, int* end, double samples_per_px, GdkColor* colour, uint32_t colour_bg)
{
	/*

	temporary. will change to using 8bit greyscale image with no alpha channel.

	*/

	g_return_if_fail(pixbuf);
	g_return_if_fail(pool_item);

	/*
	int fg_red = colour->red   >> 8;
	int fg_grn = colour->green >> 8;
	int fg_blu = colour->blue  >> 8;
	*/

	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);

	if(samples_per_px < 0.001) gerr ("samples_per_pix=%f", samples_per_px);
	struct peak_sample sample;
	short min;                //negative peak value for each pixel.
	short max;                //positive peak value for each pixel.
//	if(!pool_item->valid) return;
//	if(!pool_item->source_id){ gerr ("bad core source id: %Lu.", pool_item->source_id[0]); return; }

	float gain       = 1.0;//ARRANGE_FIRST->peak_gain;
	dbg(3, "peak_gain=%.2f", gain);

		int
		_get_width(Waveform* w)
		{
		  //if(w->rms_buf1) return 2;
		  //if(w->rms_buf0) return 1;
		  if(w->priv->peak.buf[1]) return 2;
		  if(w->priv->peak.buf[0]) return 1;
		  return 0;
		}
	int n_chans      = _get_width(pool_item);
if(!n_chans){ gerr("n_chans"); n_chans = 1; }

	int width        = pixbuf->width;
	int height       = pixbuf->height;
	guchar* pixels   = pixbuf->buf;
	int rowstride    = pixbuf->width;
	int n_channels   = 1;
	if (height > MAX_PART_HEIGHT) gerr ("part too tall. not enough memory allocated.");
	int ch_height = height / n_chans;
	int vscale = (256*128*2) / ch_height;

	int px_start = start ? *start : 0;
	int px_stop  = end   ? MIN(*end, px_start + width) : px_start + width;
	dbg (2, "px_start=%i px_end=%i", px_start, px_stop);

	//xmag defines how many 'samples' we need to skip to get the next pixel.
	//making xmag smaller increases the visual magnification.
	//-the bigger the mag, the less samples we need to skip.
	//-as we're only dealing with smaller peak files, we need to adjust by WF_PEAK_RATIO.
	double xmag = samples_per_px / (WF_PEAK_RATIO * WF_PEAK_VALUES_PER_SAMPLE);

	dbg(2, "samples_per_px=%.2f", samples_per_px);

	int ch; for(ch=0;ch<n_chans;ch++){
		//we use the same part of Line for each channel, it is then render it to the pixbuf with a channel offset.
		dbg (2, "ch=%i", ch);

		RmsBuf* rb = pool_item->rms_buf0;
		if(!rb){
			if(!(rb = waveform_load_rms_file(pool_item, ch))) continue;
		}

		//-----------------------------------------

		struct _rms_buf_info b; 
		get_rms_buf_info(rb->buf, rb->size, &b, ch);
		g_return_if_fail(b.buf);

		int src_start=0;             // frames. multiply by 2 to get the index into the source buffer for each sample pt.
		int src_stop =0;   

		if(b.len > 10000000){ gerr ("buflen too big"); return; }

		line_clear(&line[0]);
		int line_index = 0;
		//struct _line* previous_line = &line[0];
		//struct _line* current_line  = &line[1]; //the line being displayed. Not the line being written to.
		//struct _line* next_line     = &line[2];

		int i  = 0;
		int px = 0;                    // pixel count starting at the lhs of the Part.
		for(px=px_start;px<px_stop;px++, i++){
			//note: units for src_start are *frames*.

			//subtract block using buffer index:
			src_start = ((int)((px  ) * xmag));
			src_stop  = ((int)((px+1) * xmag));
			//if(src_start == src_stop){ printf("^"); fflush(stdout); }
			//else { printf("_"); fflush(stdout); }
			dbg(4, "srcidx: %i - %i", src_start, src_stop);

			struct _line* previous_line = &line[(line_index  ) % 3];
			struct _line* current_line  = &line[(line_index+1) % 3];
			struct _line* next_line     = &line[(line_index+2) % 3];

			//arrays holding subpixel levels.
			short lmax[4] = {0,0,0,0};
			short lmin[4] = {0,0,0,0};
			short k   [4] = {0,0,0,0}; //sorted copy of l.

			int mid = ch_height / 2;

			int j,y;
			if(src_stop < b.len_frames){
				min = 0; max = 0;
				int sub_px = 0;

				if(src_start == src_stop) src_stop++; //repeat previous line. Temp, while we dont have hires rms info.

				for(j=src_start;j<src_stop;j++){ //iterate over all the source samples for this pixel.
					#define SHIFT_CHAR_TO_SHORT 8
					#define TEMP_V_SCALE 2
					sample.positive = ABS((b.buf[j] << (SHIFT_CHAR_TO_SHORT - TEMP_V_SCALE))/* * gain*/);
					//sample.negative = ABS(b.buf[2*j   ] * gain);
					//printf("%x ", sample.positive); fflush(stdout);
//				}
					if(sample.positive > max) max = sample.positive;
					//if(sample.negative < min) min = sample.negative;

					if(sub_px < 4){
						//FIXME these supixels are not evenly distributed when > 4 available.
						lmax[sub_px] = (ch_height * sample.positive) / (256*128*2);
						//lmin[sub_px] =-(ch_height * sample.negative) / (256*128*2); //lmin contains positive values.
						lmin[sub_px] = lmax[sub_px];
					}
					sub_px++;
				}
				if(!sub_px){ /*printf("*"); fflush(stdout);*/ continue; }

				if(!line_index){
					//first line - we also grab the previous sample for antialiasing.
					if(px){
						j = src_start - 1;
						sample.positive = b.buf[j   ];
						max = sample.positive / vscale;
						//min =-sample.negative / vscale;
						min = max;
						//printf(" j=%i max=%i min=%i\n", j, max, min);
						for(y=mid;y<mid+max;y++) line_write(previous_line, y, 0xff);
						for(y=mid;y<mid+max;y++) line_write(current_line, y, 0xff);
						for(y=mid;y>mid-max;y--) line_write(previous_line, y, 0xff);
						for(y=mid;y>mid-max;y--) line_write(current_line, y, 0xff);
					}
					else line_clear(current_line);
				}

				//scale the values to the part height:
				min = (ch_height * min) / (256*128*2);
				max = (ch_height * max) / (256*128*2);

				sort_(k, lmax, MIN(sub_px, 4));

				//note that we write into next_line, not current_line. There is a visible delay of one line.
				int alpha = 0xff;
				int v=0;
				int s,a;

				#define MIDLINE_OFFSET 3 //prevents the mid line being full strength for low rms values. Increasing the value reduces the effect

				//positive and negative peaks are the same so do them both at once:
				for(s=0;s<MIN(sub_px, 4);s++){
					//printf("\n");
					for(y=v;y<k[s];y++){
						//printf("  distance=%i %2i %i\n", k[s] - y, 2 * (k[s] - y) + 1, (0xff * (MIDLINE_OFFSET * (k[s] - y) - 1)) / (MIDLINE_OFFSET * k[s]));
						line_write(next_line, mid + y, (alpha * (MIDLINE_OFFSET * (k[s] - y) - 1)) / (MIDLINE_OFFSET * k[s]));
						line_write(next_line, mid - y, (alpha * (MIDLINE_OFFSET * (k[s] - y) - 1)) / (MIDLINE_OFFSET * k[s]));
					}
					v=k[s];
#ifdef PEAK_ANTIALIAS
					alpha = (alpha * 2) / 3;
#endif
				}
#ifdef PEAK_ANTIALIAS
				line_write(next_line, mid+k[s-1], alpha/2); //blur the line end.
#else
				line_write(next_line, mid+k[s-1], 0);
#endif
				for(y=k[s-1]+1;y<=mid;y++){ //this looks like y goes 1 too high, but the line isnt cleared otherwise.
					next_line->a[mid + y] = 0;
				}
				for(y=mid-k[s-1];y>=0;y--) line_write(next_line, y, 0);

#if 0
				//negative peak:
				sort_(k, lmin, MIN(sub_px, 4));
				alpha = 0xff;
				v=mid;
				for(s=0;s<MIN(sub_px, 4);s++){
					//printf("\n");
					for(y= v-1;y>mid-k[s];y--){
						//dbg(1, "s=%i y=%i ks=%i y-mid=%i val=%i", s, y, k[s], k[s] - (mid - y), (alpha * (k[s] - (mid -y ))) / k[s]);
						line_write(next_line, y, MIN(0xff, (alpha * (k[s] - (mid - y))) / k[s]));
					}
					v=mid-k[s];
#ifdef PEAK_ANTIALIAS
					alpha = (alpha * 2) / 3;
#endif
				}
#ifdef PEAK_ANTIALIAS
				line_write(next_line, mid-k[s-1], alpha/2); //antialias the end of the line.
#endif
				for(y=mid-k[s-1];y>=0;y--) line_write(next_line, y, 0);
				//printf("%.1f %i ", xf, height/2+min);
#endif

				//copy the line onto the pixbuf:
#ifdef PEAK_ANTIALIAS
				int blur = 6; //bigger value gives less blurring.
#else
				//int blur = 0xffff;
#endif
				for(y=0;y<ch_height;y++){
					int p = ch*ch_height*rowstride + (ch_height - y -1)*rowstride + n_channels*(px-px_start);
					if(p > rowstride*height || p < 0){ gerr ("p! %i > %i px=%i y=%i row=%i rowstride=%i=%i", p, 3*width*ch_height, px, y, ch_height-y-1, rowstride,3*width); return; }

#ifdef PEAK_ANTIALIAS
					a = MIN((current_line->a[y] * 2)/3 + previous_line->a[y]/blur + next_line->a[y]/blur, 0xff);
#else
					a = MIN(current_line->a[y], 0xff);
#endif
//					if(!a) continue; //testing

					pixels[p] = a;
//#warning put me back
//					pixels[p] = 0;
				}

			}else{
				//no more source data available - as the pixmap is clear, we have nothing much to do.
				//gdk_draw_line(GDK_DRAWABLE(pixmap), gc, px, 0, px, height);//x1, y1, x2, y2
				DRect pts = {px, 0, px, ch_height};
				alphabuf_draw_line(pixbuf, &pts, 1.0, colour);
//				warn_no_src_data(pool_item, b.len, src_stop);
				printf("*"); fflush(stdout);
			}
			//next = srcidx + 1;
			//xf += WF_PEAK_RATIO * WF_PEAK_VALUES_PER_SAMPLE / samples_per_px;
			//printf("%.1f ", xf);

			line_index++;
			//printf("line_index=%i %i %i %i\n", line_index, (line_index  ) % 3, (line_index+1) % 3, (line_index+2) % 3);
		}

		dbg (2, "done. xmag=%.2f drawn: %i of %i src=%i-->%i", xmag, line_index, width, ((int)( px_start * xmag * 0)), src_stop);
	} //end channel

	gettimeofday(&time_stop, NULL);
	time_t secs = time_stop.tv_sec - time_start.tv_sec;
	suseconds_t usec;
	if(time_stop.tv_usec > time_start.tv_usec){
		usec = time_stop.tv_usec - time_start.tv_usec;
	}else{
		secs -= 1;
		usec = time_stop.tv_usec + 1000000 - time_start.tv_usec;
	}
}


static int
get_n_textures(WfGlBlocks* blocks)
{
	if(blocks) return blocks->size;
	gerr("!! no glblocks\n");
	return -1;
}


//------------------------------------------------------------------------
// loaders

#warning TODO location of rms files and RHS.
RmsBuf*
waveform_load_rms_file(Waveform* waveform, int ch_num)
{
	//loads the rms cache file for the given poolitem into a buffer.
	//Unlike peakfiles, these are not stored. g_free() the returned buffer after use.

	RmsBuf* rb = NULL;

#ifdef RMS_MMAP
	int fd;
	char* mmap_file;
	struct stat buf;
	//create the file
	if ((fd = open(argv[1], O_CREAT | O_RDWR, 0666)) < 0) {
		gerr("file open");
		return 3;
	}
	write(fd, starting_string, strlen(starting_string));
	//get size of file
	if (fstat(fd, &buf) < 0) {
		gerr("fstat error");
		return 4;
	}
	//create a buffer mapped to the file:
	if ((mmap_file = (char*) mmap(0, (size_t) buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == (void*)-1) {
		gerr("mmap failure.");
		return;
	}
#endif

						char*
						wf_load_rms_get_full_path(Waveform* waveform, int ch_num)
						{
							//result must be free'd with g_free().

							char* src = NULL;

							switch(ch_num){
								case 0:
									src = waveform->filename;
									break;
								case 1:
									src = waveform->filename; //TODO
#if 0
									;AyyiFilesource* file = ayyi_song__filesource_at(pool_item->pod_index[0]);
									if(file){
										char f[256];
										ayyi_file_get_other_channel(file, f, 256);
										src = f;
									}
#endif
									break;
								default:
									gwarn("bad channel number: %i", ch_num);
									return NULL;
							}

							char* fullpath = NULL;
							if(src[0]=='/'){
								//absolute path.
								fullpath = g_strdup(src);
							}else{
								//relative paths are relative to the song default audio directory.
//								fullpath = g_strdup_printf("%s/%s", song->audiodir, src);
fullpath = g_strdup(src); //TODO
							}
							return fullpath;
						}

	char* fullpath = wf_load_rms_get_full_path(waveform, ch_num);

	//find the name of the peakfile:
	char rms_file[256];
					void _get_rms_filename(char* peakname, const char* filename, int ch_num)
					{
						dbg(2, "%i: %s", ch_num, filename);
						snprintf(peakname, 255, "%s.rms", filename);
						peakname[255] = '\0';
					}
	_get_rms_filename(rms_file, fullpath, ch_num);

	int fp = 0;
	if(!(fp = open(rms_file, O_RDONLY))){ gwarn ("file open failure."); goto out; }

	struct stat sinfo;
	if(stat(rms_file, &sinfo)){ gwarn ("rms file stat error. '%s'", rms_file); close(fp); goto out; }
	off_t fsize = sinfo.st_size;
	rb = g_new0(RmsBuf, 1);
	rb->size = fsize;
	rb->buf = g_new(char, fsize);

	//read the whole peak file into memory:
	if(read(fp, rb->buf, fsize) != fsize) gerr ("read error. couldnt read %Lu bytes from %s", fsize, rms_file);
  
	close(fp);
	//dbg (2, "done. %s: %isamples (%li beats / %.3f secs).", filename, pool_item->priv->peak.size, samples2beats(pool_item->priv->peak.size), samples2secs(pool_item->priv->peak.size*WF_PEAK_RATIO));

	g_free(fullpath); //FIXME handle other returns
	return rb;

  out:
	g_free(fullpath);
	return rb;
}


void
waveform_set_peak_loader(PeakLoader loader)
{
	wf_get_instance();
	wf->load_peak = loader;
}


WfWavCache*
wf_wav_cache_new(int n_channels)
{
	PF0;
	g_return_val_if_fail(n_channels <= WF_STEREO, NULL);

	WfWavCache* cache = g_new0(WfWavCache, 1);

	cache->buf = g_new0(WfBuf, 1);
	int c; for(c=0;c<n_channels;c++) cache->buf->buf[c] = g_malloc0(WF_CACHE_BUF_SIZE * sizeof(float));
	cache->buf->size = WF_CACHE_BUF_SIZE;

	cache->region.start = -1;
	cache->region.len = -1;
	return cache;
}


static Peakbuf*
wf_peakbuf_new(int block_num)
{
	Peakbuf* peakbuf = g_new0(Peakbuf, 1);
	peakbuf->block_num = block_num;

	return peakbuf;
}


#if 0
static gboolean
peakbuf_on_idle()
{
	//process a single block from the peakbuf draw queue.

	static int debug_rec_count = 0;
	dbg(1, "count=%i", debug_rec_count++);

	if (wf_get_instance()->work_queue) {
		QueueItem* item = wf_get_instance()->work_queue->data;
		g_return_val_if_fail(item, false);
		call(item->callback, item->user_data);

		wf_get_instance()->work_queue = g_list_remove(wf_get_instance()->work_queue, item);
		dbg(1, "done. remaining=%i", g_list_length(wf_get_instance()->work_queue));
	}

	if (!wf_get_instance()->work_queue){ peak_idle = 0; debug_rec_count--; return IDLE_STOP; }

	void peakbuf_print_queue()
	{
		int i = 0;
		GList* l = wf_get_instance()->work_queue;
		for(;l;l=l->next){
		/*
			QueueItem* item = l->data;
			printf("  %i %s\n", item->block_num, item->pool_item->leafname);
		*/
			i++;
		}
		printf("------------------- tot=%i\n", i);
	}
	//peakbuf_print_queue();

	debug_rec_count--;
	return true;
}
#endif


uint32_t
wf_peakbuf_get_max_size(int n_tiers)
{
	// the number of shorts in a full size buffer
	return (1 << (n_tiers-1)) * WF_PEAK_BLOCK_SIZE * WF_PEAK_VALUES_PER_SAMPLE;
}


int32_t
wf_get_peakbuf_len_frames()
{
	// the length in samples of the file-section that the peakbuf represents.

	return WF_PEAK_BLOCK_SIZE * WF_PEAK_RATIO;
}


Peakbuf*
wf_get_peakbuf_n(Waveform* w, int block_num)
{
	//an array entry will be created on demand, but it will not yet contain any audio data.

	g_return_val_if_fail(w, NULL);

	GPtrArray* peaks = w->hires_peaks;
	g_return_val_if_fail(peaks, NULL);
	Peakbuf* peakbuf = NULL;
	if(block_num >= peaks->len){
		//no peakbuf yet for this block. We need to create it.
		g_ptr_array_set_size(peaks, block_num + 1);
		void** array = peaks->pdata;
		peakbuf = array[block_num] = wf_peakbuf_new(block_num);
	}else{
		peakbuf = g_ptr_array_index(peaks, block_num);
		if(!peakbuf){
			void** array = peaks->pdata;
			peakbuf = array[block_num] = wf_peakbuf_new(block_num);
		}
	}
	dbg(2, "block_num=%i peaks->len=%i", block_num, peaks->len);

	//g_return_val_if_fail(peakbuf, NULL);
	if(!peakbuf){
		gwarn("peakbuf fail: b=%i n=%i", block_num, peaks->len);
		//print_ptr_array(peaks);
	}
	return peakbuf;
}


int
waveform_get_n_audio_blocks(Waveform* w)
{
	WfAudioData* audio = w->priv->audio_data;
	if(!audio->n_blocks){
		uint64_t n_frames = waveform_get_n_frames(w);

		int xtra = (w->n_frames % WF_PEAK_BLOCK_SIZE) ? 1 : 0;
		audio->n_blocks = n_frames / (WF_PEAK_BLOCK_SIZE/* * 256*/) + xtra;
		dbg(2, "remainder=%i xtra=%i", remainder, xtra);

		dbg(1, "setting samplecount=%Li xtra=%i n_blocks=%i",
		        n_frames,
		        xtra,
		        audio->n_blocks);
	}
	return audio->n_blocks;
}


short
waveform_find_max_audio_level(Waveform* w)
{
	int i;
	short max_level = 0;
	int c; for(c=0;c<2;c++){
		short* buf = w->priv->peak.buf[c];
		if(!buf) continue;

		for(i=0;i<w->priv->peak.size;i++){
			max_level = MAX(max_level, buf[i]);
		}
	}

	return max_level;
}


#ifdef USE_GDK_PIXBUF

static void
pixbuf_draw_line(cairo_t* cr, DRect* pts, double line_width, GdkColor* colour)
{
	if(pts->y1 == pts->y2) return;
	cairo_rectangle(cr, pts->x1, pts->y1, pts->x2 - pts->x1 + 1, pts->y2 - pts->y1 + 1);
	cairo_fill (cr);
	cairo_stroke (cr);
}


static void
warn_no_src_data(Waveform* waveform, int buflen, int src_stop)
{
	static int count = 0;
	if(count++ > 20) return;
	gwarn ("no src data in peak buffer! buflen=%i src_stop=%i", buflen, src_stop);
}


void
waveform_peak_to_pixbuf(Waveform* waveform, GdkPixbuf* pixbuf, uint32_t region_inset, int* start, int* end, double samples_per_px, GdkColor* colour, uint32_t colour_bg, float gain)
{
	/*
		renders part of a peakfile (loaded into the buffer given by waveform->buf) onto the given pixbuf.
		-the pixbuf must already be cleared, with the correct background colour.

		-*** although not explicitly block-based, it can be used with blocks by specifying start and end.

		@param pixbuf - pixbuf covers the whole Waveform. We only render to the bit between @start and @end.
		                                        |_ TODO no, this won't work 

		@param region_inset - (sample frames)
		@param start  - (pixels) if set, we start rendering at this value from the left of the pixbuf.
		@param end    - (pixels) if set, num of pixels from left of pixbuf to stop rendering at.
		                Can be bigger than pixbuf width.

		@param samples_per_px sets the magnification.

		further optimisation:
		-dont scan pixels above the peaks. Should we record the overall region/file peak level?
	*/

	g_return_if_fail(pixbuf);
	g_return_if_fail(waveform);
	dbg(2, "inset=%i", region_inset);

	gboolean hires_mode = ((samples_per_px / WF_PEAK_RATIO) < 1.0);

	int bg_red = (colour_bg & 0xff000000) >> 24;
	int bg_grn = (colour_bg & 0x00ff0000) >> 16;
	int bg_blu = (colour_bg & 0x0000ff00) >>  8;
	int fg_red = colour->red   >> 8;
	int fg_grn = colour->green >> 8;
	int fg_blu = colour->blue  >> 8;
	//printf("%s(): bg=%x r=%x g=%x b=%x \n", __func__, colour_bg, bg_red, bg_grn bg_blu);

#if 0
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
#endif

	if(samples_per_px < 0.001) gerr ("samples_per_pix=%f", samples_per_px);
	struct peak_sample sample;
	short min;                //negative peak value for each pixel.
	short max;                //positive peak value for each pixel.
#if 0
	if(!pool_item->valid) return;
	if(!pool_item->source_id){ gerr ("bad core source id: %Lu.", pool_item->source_id[0]); return; }
#endif

	dbg(1, "peak_gain=%.2f", gain);

	int n_chans      = waveform_get_n_channels(waveform);

	int width        = gdk_pixbuf_get_width(pixbuf);
	int height       = gdk_pixbuf_get_height(pixbuf);
	guchar* pixels   = gdk_pixbuf_get_pixels(pixbuf);
	int rowstride    = gdk_pixbuf_get_rowstride(pixbuf);
	cairo_surface_t* surface = cairo_image_surface_create_for_data(pixels, CAIRO_FORMAT_RGB24, width, height, rowstride);
	cairo_t* cairo = cairo_create(surface);
	if (height > MAX_PART_HEIGHT) gerr ("part too tall. not enough memory allocated.");
	int ch_height = height / n_chans;
dbg(1, "height=%i", ch_height);
	int vscale = (256*128*2) / ch_height;

	int px_start = start ? *start : 0;
	int px_stop  = end   ? MIN(*end, width) : width;
	dbg (1, "px_start=%i px_end=%i", px_start, px_stop);

	int hires_block = -1;
	int src_px_start = 0;
	if(hires_mode){
		uint64_t start_frame = px_start * samples_per_px;
		hires_block = start_frame / WF_PEAK_BLOCK_SIZE;
		src_px_start = (hires_block * wf_get_peakbuf_len_frames()) / samples_per_px; //if not 1st block, the src buffer address is smaller.
		dbg(1, "hires: offset=%i", src_px_start);
	}

	//xmag defines how many 'samples' we need to skip to get the next pixel.
	//making xmag smaller increases the visual magnification.
	//-the bigger the mag, the less samples we need to skip.
	//-as we're only dealing with smaller peak files, we need to adjust by WF_PEAK_RATIO.
	double xmag = samples_per_px / (WF_PEAK_RATIO * WF_PEAK_VALUES_PER_SAMPLE);

	int n_tiers = hires_mode ? /*peakbuf->n_tiers*/4 : 0; //TODO
	//note n_tiers is 1, not zero in lowres mode. (??!)
	dbg(1, "samples_per_px=%.2f", samples_per_px);
	dbg(1, "n_tiers=%i <<=%i", n_tiers, 1 << n_tiers);

	int ch; for(ch=0;ch<n_chans;ch++){
		//we use the same part of Line for each channel, it is then render it to the pixbuf with a channel offset.
		dbg (1, "ch=%i", ch);

		struct _buf_info b; 
		get_buf_info(waveform, hires_block, &b, ch);
		g_return_if_fail(b.buf);

		int src_start=0;             // frames. multiply by 2 to get the index into the source buffer for each sample pt.
		int src_stop =0;   

		if(b.len > 10000000){ gerr ("buflen too big"); return; }

		line_clear(&line[0]);
		int line_index = 0;
		//struct _line* previous_line = &line[0];
		//struct _line* current_line  = &line[1]; //the line being displayed. Not the line being written to.
		//struct _line* next_line     = &line[2];

		int block_offset = hires_mode ? wf_peakbuf_get_max_size(n_tiers) * (hires_block  ) / WF_PEAK_VALUES_PER_SAMPLE : 0;
		int block_offset2= hires_mode ? wf_peakbuf_get_max_size(n_tiers) * (hires_block+1) / WF_PEAK_VALUES_PER_SAMPLE : 0;

		int i  = 0;
		int px = 0;                    // pixel count starting at the lhs of the Part.
		for(px=px_start;px<px_stop;px++){
			i++;
			double xmag_ = hires_mode ? xmag * (1 << (n_tiers-1)) : xmag * 1.0;

			//note: units for src_start are *frames*.

			//src_start = ((int)( px   * xmag * (1 << n_tiers))) + region_inset; //warning ! n_tiers can be zero.
			//src_stop  = ((int)((px+1)* xmag * (1 << n_tiers))) + region_inset;

			//subtract block using pixel value:
			//src_start = ((int)((px-src_px_start  ) * xmag_)) + region_inset;// - peakbuf_get_len_samples() * peakbuf->block_num;
			//src_stop  = ((int)((px-src_px_start+1) * xmag_)) + region_inset;// - peakbuf_get_len_samples() * peakbuf->block_num;

			//subtract block using buffer index:
			src_start = ((int)((px  ) * xmag_)) + region_inset - block_offset;
			src_stop  = ((int)((px+1) * xmag_)) + region_inset - block_offset;
			if(src_start == src_stop){ printf("^"); fflush(stdout); }
			if((px==px_start) && hires_mode){
			double percent = 2 * 100 * (((int)(px * xmag_)) + region_inset - (wf_peakbuf_get_max_size(n_tiers) * hires_block) / WF_PEAK_VALUES_PER_SAMPLE) / b.len;
			dbg(2, "reading from buf=%i=%.2f%% stop=%i buflen=%i blocklen=%i", src_start, percent, src_stop, b.len, wf_peakbuf_get_max_size(n_tiers));
		}
		if(src_stop > b.len_frames && hires_mode){
			dbg(1, "**** block change needed!");
			extern Peakbuf* wf_get_peakbuf_n(Waveform*, int);
			Peakbuf* peakbuf = wf_get_peakbuf_n(waveform, hires_block + 1);
			g_return_if_fail(peakbuf);
			if(!get_buf_info(waveform, hires_block, &b, ch)){ break; }//TODO if this is multichannel, we need to go back to previous peakbuf - should probably have 2 peakbufs...
			//src_start = 0;
			block_offset = block_offset2;
			src_start = ((int)((px  ) * xmag_)) + region_inset - block_offset;
			src_stop  = ((int)((px+1) * xmag_)) + region_inset - block_offset;
		}
		dbg(3, "srcidx: %i - %i", src_start, src_stop);

		struct _line* previous_line = &line[(line_index  ) % 3];
		struct _line* current_line  = &line[(line_index+1) % 3];
		struct _line* next_line     = &line[(line_index+2) % 3];

		//arrays holding subpixel levels.
		short lmax[4] = {0,0,0,0};
		short lmin[4] = {0,0,0,0};
		short k   [4] = {0,0,0,0}; //sorted copy of l.

		int mid = ch_height / 2;

		int j,y;
		if(src_stop < b.len_frames){
			min = 0; max = 0;
			int sub_px = 0;
			for(j=src_start;j<src_stop;j++){ //iterate over all the source samples for this pixel.
				sample.positive = b.buf[2*j   ] * gain;
				sample.negative = b.buf[2*j +1] * gain;
				if(sample.positive > max) max = sample.positive;
				if(sample.negative < min) min = sample.negative;

				if(sub_px<4){
					//FIXME these supixels are not evenly distributed when > 4 available.
					lmax[sub_px] = (ch_height * sample.positive) / (256*128*2);
					lmin[sub_px] =-(ch_height * sample.negative) / (256*128*2); //lmin contains positive values.
				}
				sub_px++;
			}
			if(!sub_px){ /*printf("*"); fflush(stdout);*/ continue; }

			if(!line_index){
				//first line - we also grab the previous sample for antialiasing.
				if(px){
					j = src_start - 1;
					sample.positive = b.buf[2*j   ];
					sample.negative = b.buf[2*j +1];
					max = sample.positive / vscale;
					min =-sample.negative / vscale;
					//printf(" j=%i max=%i min=%i\n", j, max, min);
					for(y=mid;y<mid+max;y++) line_write(previous_line, y, 0xff);
					for(y=mid;y<mid+max;y++) line_write(current_line, y, 0xff);
					for(y=mid;y>mid-max;y--) line_write(previous_line, y, 0xff);
					for(y=mid;y>mid-max;y--) line_write(current_line, y, 0xff);
				}
				else line_clear(current_line);
			}

			//we dont need this? but we should check buffer lengh?
			/*
			if(px==px_stop-1){
				printf("%s(): last line.\n", __func__);
				//last line - we also need to grab the following sample for antialiasing.
				int src = ((int)((px+1)* xmag)) + start_offset + 1;
				if(src < buflen/2){
					sample.positive = buf[2*src   ];
					sample.negative = buf[2*src +1];
					max = (height * sample.positive) / (256*128*2);
					min =-(height * sample.negative) / (256*128*2);
					for(y=mid;y<mid+max;y++) line_write(next_line, y, 0xff);
					for(y=mid;y>mid-max;y--) line_write(next_line, y, 0xff);
				}
			}
			*/

			//scale the values to the part height:
			min = (ch_height * min) / (256*128*2);
			max = (ch_height * max) / (256*128*2);

			sort_(k, lmax, MIN(sub_px, 4));

			//note that we write into next_line, not current_line. There is a visible delay of one line.
			int alpha = 0xff;
			int v=0;
			int s,a;

			//positive peak:
			for(s=0;s<MIN(sub_px, 4);s++){
				//printf(" v=%i->%i ", v, k[s]);
				for(y=v;y<k[s];y++){
					line_write(next_line, mid +y, alpha);
				}
				v=k[s];
				alpha = (alpha * 2) / 3;
			}
			line_write(next_line, mid+k[s-1], alpha/2); //blur!
			for(y=k[s-1]+1;y<=mid;y++){ //this looks like y goes 1 too high, but the line isnt cleared otherwise.
				next_line->a[mid + y] = 0;
			}

			//negative peak:
			sort_(k, lmin, MIN(sub_px, 4));
			alpha = 0xff;
			v=mid;
			for(s=0;s<MIN(sub_px, 4);s++){
				//for(y=v;y>mid-k[s];y--) next_line->a[y] = alpha;
				for(y=v;y>mid-k[s];y--) line_write(next_line, y, alpha);
				v=mid-k[s];
				alpha = (alpha * 2) / 3;
			}
			line_write(next_line, mid-k[s-1], alpha/2); //antialias the end of the line.
			for(y=mid-k[s-1]-1;y>=0;y--) line_write(next_line, y, 0);
			//printf("%.1f %i ", xf, height/2+min);

			//draw the lines:
			int blur = 6; //bigger value gives less blurring.
			for(y=0;y<ch_height;y++){
				int p = ch*ch_height*rowstride + (ch_height - y -1)*rowstride + 3*px;
				if(p > rowstride*height || p < 0){ gerr ("p! %i > %i px=%i y=%i row=%i rowstride=%i=%i", p, 3*width*ch_height, px, y, ch_height-y-1, rowstride,3*width); return; }

				a = MIN((current_line->a[y] * 2)/3 + previous_line->a[y]/blur + next_line->a[y]/blur, 0xff);
				if(!a) continue; //testing
				pixels[p  ] = (int)( bg_red * (0xff - a) + fg_red * a) >> 8;
				pixels[p+1] = (int)( bg_grn * (0xff - a) + fg_grn * a) >> 8;
				pixels[p+2] = (int)( bg_blu * (0xff - a) + fg_blu * a) >> 8;
			}

		}else{
			//no more source data available - as the pixmap is clear, we have nothing much to do.
			//gdk_draw_line(GDK_DRAWABLE(pixmap), gc, px, 0, px, height);//x1, y1, x2, y2
			DRect pts = {px, 0, px, ch_height};
//#warning TODO - at least free
//								cairo_format_t format = CAIRO_FORMAT_ARGB32;
//								cairo_surface_t* surface = cairo_image_surface_create_for_data (gdk_pixbuf_get_pixels(pixbuf), format, gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf), gdk_pixbuf_get_rowstride(pixbuf));
//								cairo_t*         cr      = cairo_create(surface);
				pixbuf_draw_line(cairo, &pts, 1.0, colour);
				warn_no_src_data(waveform, b.len, src_stop);
				printf("*"); fflush(stdout);
			}
			//next = srcidx + 1;
			//xf += WF_PEAK_RATIO * WF_PEAK_VALUES_PER_SAMPLE / samples_per_px;
			//printf("%.1f ", xf);

			line_index++;
			//printf("line_index=%i %i %i %i\n", line_index, (line_index  ) % 3, (line_index+1) % 3, (line_index+2) % 3);
		}

		dbg (1, "done. xmag=%.2f drawn: %i of %i src=%i-->%i", xmag, line_index, width, ((int)( px_start * xmag * n_tiers)) + region_inset, src_stop);
	} //end channel

#if 0
	gettimeofday(&time_stop, NULL);
	time_t      secs = time_stop.tv_sec - time_start.tv_sec;
	suseconds_t usec;
	if(time_stop.tv_usec > time_start.tv_usec){
		usec = time_stop.tv_usec - time_start.tv_usec;
	}else{
		secs -= 1;
		usec = time_stop.tv_usec + 1000000 - time_start.tv_usec;
	}
	//printf("%s(): start=%03i: %lu:%06lu\n", __func__, px_start, secs, usec);
#endif

//#ifdef DBG_TILE_BOUNDARY
	GdkColor red = {0xffff, 0xffff, 0x0000, 0xffff};
	DRect pts = {px_start, 0, px_start, ch_height};
	pixbuf_draw_line(cairo, &pts, 1.0, &red);
	{
												pixels[0] = 0xff;
												pixels[1] = 0x00;
												pixels[2] = 0x00;
												pixels[3* px_stop + 0] = 0x00;
												pixels[3* px_stop + 1] = 0xff;
												pixels[3* px_stop + 2] = 0x00;
	}
//#endif
	cairo_surface_destroy(surface);
	cairo_destroy(cairo);
}
#endif


#undef PEAK_ANTIALIAS
void
waveform_rms_to_pixbuf(Waveform* w, GdkPixbuf* pixbuf, uint32_t src_inset, int* start, int* end, double samples_per_px, GdkColor* colour, uint32_t colour_bg, float gain)
{
	/*
     this fn is copied from peak_render_to_pixbuf(). It differs in the size of the src data width, and
     that plus and minus are the same. It also disables antialiasing.

     renders part of a peakfile (loaded into the buffer given by pool_item->buf) onto the given pixbuf.
     -the pixbuf must already be cleared, with the correct background colour.

     @peakbuf      - if this is present, we are in hires mode. THIS WILL BE TIDIED UP LATER

     @param pixbuf - pixbuf covers the whole Part. We only render to the bit between @start and @end.

     @src_inset    - usually the part inset. in peak units (ie samples / WF_PEAK_RATIO)

     @param start  - if set, we start rendering at this value from the left of the pixbuf.
     @param end    - if set, num of pixels from left of pixbuf to stop rendering at.
                     Can be bigger than pixbuf width.

     @param samples_per_px sets the magnification.
              -pool entries:   we scale to the given width using peak_render_pixmap_at_size().
              -part overviews: we need to use the scale from the arrangezoom.
	*/

	g_return_if_fail(pixbuf);
	g_return_if_fail(w);
	dbg(2, "inset=%i", src_inset);

	gboolean hires_mode = ((samples_per_px / WF_PEAK_RATIO) < 1.0);
	if(hires_mode) return;

	int bg_red = (colour_bg & 0xff000000) >> 24;
	int bg_grn = (colour_bg & 0x00ff0000) >> 16;
	int bg_blu = (colour_bg & 0x0000ff00) >>  8;
	int fg_red = colour->red   >> 8;
	int fg_grn = colour->green >> 8;
	int fg_blu = colour->blue  >> 8;
	//printf("%s(): bg=%x r=%x g=%x b=%x \n", __func__, colour_bg, bg_red, bg_grn bg_blu);

#if 0
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
#endif

	if(samples_per_px < 0.001) gerr ("samples_per_pix=%f", samples_per_px);
	struct peak_sample sample;
	short min;                //negative peak value for each pixel.
	short max;                //positive peak value for each pixel.
	//if(!pool_item->valid) return;
	//if(!pool_item->source_id){ gerr ("bad core source id: %Lu.", pool_item->source_id[0]); return; }

	int n_chans      = waveform_get_n_channels(w);

	int width        = gdk_pixbuf_get_width(pixbuf);
	int height       = gdk_pixbuf_get_height(pixbuf);
	guchar* pixels   = gdk_pixbuf_get_pixels(pixbuf);
	int rowstride    = gdk_pixbuf_get_rowstride(pixbuf);

	cairo_surface_t* surface = cairo_image_surface_create_for_data(pixels, CAIRO_FORMAT_RGB24, width, height, rowstride);
	cairo_t* cairo = cairo_create(surface);

	//if (height > MAX_PART_HEIGHT) gerr ("part too tall. not enough memory allocated.");
	int ch_height = height / n_chans;
	int vscale = (256*128*2) / ch_height;

	int px_start = start ? *start : 0;
	int px_stop  = end   ? MIN(*end, width) : width;
	dbg (2, "px_start=%i px_end=%i", px_start, px_stop);

/*
	int src_px_start = 0;
	if(hires_mode){
		src_px_start = (peakbuf->block_num * peakbuf_get_len_samples()) / samples_per_px; //if not 1st block, the src buffer address is smaller.
		//dbg(1, "offset=%i", src_px_start);
	}
*/
	int hires_block = -1;
	int src_px_start = 0;
	if(hires_mode){
		uint64_t start_frame = px_start * samples_per_px;
		hires_block = start_frame / WF_PEAK_BLOCK_SIZE;
		src_px_start = (hires_block * wf_get_peakbuf_len_frames()) / samples_per_px; //if not 1st block, the src buffer address is smaller.
		dbg(1, "hires: offset=%i", src_px_start);
	}

	//xmag defines how many 'samples' we need to skip to get the next pixel.
	//making xmag smaller increases the visual magnification.
	//-the bigger the mag, the less samples we need to skip.
	//-as we're only dealing with smaller peak files, we need to adjust by WF_PEAK_RATIO.
	double xmag = samples_per_px / (WF_PEAK_RATIO * WF_PEAK_VALUES_PER_SAMPLE);

	int n_tiers = hires_mode ? /*peakbuf->n_tiers*/4 : 0; //TODO
	//note n_tiers is 1, not zero in lowres mode. (??!)
	dbg(2, "samples_per_px=%.2f", samples_per_px);
	dbg(2, "n_tiers=%i <<=%i", n_tiers, 1 << n_tiers);

	int ch; for(ch=0;ch<n_chans;ch++){
		//we use the same part of Line for each channel, it is then render it to the pixbuf with a channel offset.
		dbg (2, "ch=%i", ch);

		//-----------------------------------------

#warning review
		/*
	we need some kind of cache.
      -the simplest approach would be to have 1 mmaped file for each audio src.
      -if we only want part of a file, the kernel will only load that part?
      -what does the kernel do with old mmaped files - does it put them in swap? 
      -store Age value to determine when to remove from cache?
      struct rms_cache {
        GList* items; //pointers to pool_item->rms
        guint clock;
      }
      struct cache_item {
        int age; //corresponds to cache->clock. For cache maintainence, iteration of whole list is needed.
                 // -or use a hashtable indexed by Age?
      }
		*/
		RmsBuf* rb = w->rms_buf0;
		if(!rb){
			if(!(rb = waveform_load_rms_file(w, ch))) continue;
		}

		//-----------------------------------------

		struct _rms_buf_info b; 
		get_rms_buf_info(rb->buf, rb->size, &b, ch);
		g_return_if_fail(b.buf);

		int src_start=0;             // frames. multiply by 2 to get the index into the source buffer for each sample pt.
		int src_stop =0;   

		if(b.len > 10000000){ gerr ("buflen too big"); return; }

		line_clear(&line[0]);
		int line_index = 0;
		//struct _line* previous_line = &line[0];
		//struct _line* current_line  = &line[1]; //the line being displayed. Not the line being written to.
		//struct _line* next_line     = &line[2];

		int block_offset = hires_mode ? wf_peakbuf_get_max_size(n_tiers) * hires_block / WF_PEAK_VALUES_PER_SAMPLE : 0;
		int block_offset2= hires_mode ? wf_peakbuf_get_max_size(n_tiers) * (hires_block+1) / WF_PEAK_VALUES_PER_SAMPLE : 0;

		int i  = 0;
		int px = 0;                    // pixel count starting at the lhs of the Part.
		for(px=px_start;px<px_stop;px++){
			i++;
			double xmag_ = hires_mode ? xmag * (1 << (n_tiers-1)) : xmag * 1.0;

			//note: units for src_start are *frames*.

			//subtract block using buffer index:
			src_start = ((int)((px  ) * xmag_)) + src_inset - block_offset;
			src_stop  = ((int)((px+1) * xmag_)) + src_inset - block_offset;
			//if(src_start == src_stop){ printf("^"); fflush(stdout); }
			//else { printf("_"); fflush(stdout); }
			if((px==px_start) && hires_mode){
				double percent = 2 * 100 * (((int)(px * xmag_)) + src_inset - (wf_peakbuf_get_max_size(n_tiers) * hires_block) / WF_PEAK_VALUES_PER_SAMPLE) / b.len;
				dbg(2, "reading from buf=%i=%.2f%% stop=%i buflen=%i blocklen=%i", src_start, percent, src_stop, b.len, wf_peakbuf_get_max_size(n_tiers));
			}
			if(src_stop > b.len_frames && hires_mode){
        dbg(1, "**** block change needed!");
        extern Peakbuf* wf_get_peakbuf_n(Waveform*, int);
        Peakbuf* peakbuf = wf_get_peakbuf_n(w, hires_block + 1);
        g_return_if_fail(peakbuf);
        if(!get_rms_buf_info(rb->buf, rb->size, &b, ch)){ break; }//TODO if this is multichannel, we need to go back to previous peakbuf - should probably have 2 peakbufs...
        //src_start = 0;
        block_offset = block_offset2;
        src_start = ((int)((px  ) * xmag_)) + src_inset - block_offset;
        src_stop  = ((int)((px+1) * xmag_)) + src_inset - block_offset;
      }
      dbg(4, "srcidx: %i - %i", src_start, src_stop);

      struct _line* previous_line = &line[(line_index  ) % 3];
      struct _line* current_line  = &line[(line_index+1) % 3];
      struct _line* next_line     = &line[(line_index+2) % 3];

      //arrays holding subpixel levels.
      short lmax[4] = {0,0,0,0};
      short lmin[4] = {0,0,0,0};
      short k   [4] = {0,0,0,0}; //sorted copy of l.

      int mid = ch_height / 2;

      int j,y;
      if(src_stop < b.len_frames){
        min = 0; max = 0;
        int sub_px = 0;

        if(src_start == src_stop) src_stop++; //repeat previous line. Temp, while we dont have hires rms info.

        for(j=src_start;j<src_stop;j++){ //iterate over all the source samples for this pixel.
          /*if(pool_item->peak_float && !hires_mode){
            //printf("%.1f ", buf_float[2*j]);
            sample.positive = (short)(b.buf_float[2*j +1] * (1 << 15) * gain); //ardour peak files have negative peak first.
            //sample.negative = (short)(b.buf_float[2*j   ] * (1 << 15))* gain;
          }else*/{
            #define SHIFT_CHAR_TO_SHORT 8
            #define TEMP_V_SCALE 2
            sample.positive = ABS((b.buf[j] << (SHIFT_CHAR_TO_SHORT - TEMP_V_SCALE))/* * gain*/);
            //sample.negative = ABS(b.buf[2*j   ] * gain);
            //printf("%x ", sample.positive); fflush(stdout);
          }
          if(sample.positive > max) max = sample.positive;
          //if(sample.negative < min) min = sample.negative;

          if(sub_px<4){
            //FIXME these supixels are not evenly distributed when > 4 available.
            lmax[sub_px] = (ch_height * sample.positive) / (256*128*2);
            //lmin[sub_px] =-(ch_height * sample.negative) / (256*128*2); //lmin contains positive values.
            lmin[sub_px] = lmax[sub_px];
          }
          sub_px++;
        }
        if(!sub_px){ /*printf("*"); fflush(stdout);*/ continue; }

        if(!line_index){
          //first line - we also grab the previous sample for antialiasing.
          if(px){
            j = src_start - 1;
            /*if(pool_item->peak_float && !hires_mode){
              sample.positive = (short)(b.buf_float[j] * (1 << 15)); //ardour peak files have negative peak first.
              //sample.negative = (short)(b.buf_float[2*j   ] * (1 << 15));
            }else*/{
              sample.positive = b.buf[j   ];
              //sample.negative = b.buf[2*j   ];
            }
            max = sample.positive / vscale;
            //min =-sample.negative / vscale;
            min = max;
            //printf(" j=%i max=%i min=%i\n", j, max, min);
            for(y=mid;y<mid+max;y++) line_write(previous_line, y, 0xff);
            for(y=mid;y<mid+max;y++) line_write(current_line, y, 0xff);
            for(y=mid;y>mid-max;y--) line_write(previous_line, y, 0xff);
            for(y=mid;y>mid-max;y--) line_write(current_line, y, 0xff);
          }
          else line_clear(current_line);
        }

        //scale the values to the part height:
        min = (ch_height * min) / (256*128*2);
        max = (ch_height * max) / (256*128*2);

        sort_(k, lmax, MIN(sub_px, 4));

        //note that we write into next_line, not current_line. There is a visible delay of one line.
        int alpha = 0xff;
        int v=0;
        int s,a;

        //positive peak:
        for(s=0;s<MIN(sub_px, 4);s++){
          //printf(" v=%i->%i ", v, k[s]);
          for(y=v;y<k[s];y++){
            line_write(next_line, mid +y, alpha);
          }
          v=k[s];
#ifdef PEAK_ANTIALIAS
          alpha = (alpha * 2) / 3; //hmm - why some values lighter than others??
#endif
        }
#ifdef PEAK_ANTIALIAS
        line_write(next_line, mid+k[s-1], alpha/2); //blur the line end.
#else
        line_write(next_line, mid+k[s-1], 0);
#endif
        for(y=k[s-1]+1;y<=mid;y++){ //this looks like y goes 1 too high, but the line isnt cleared otherwise.
          next_line->a[mid + y] = 0;
        }

        //negative peak:
        sort_(k, lmin, MIN(sub_px, 4));
        alpha = 0xff;
        v=mid;
        for(s=0;s<MIN(sub_px, 4);s++){
          //for(y=v;y>mid-k[s];y--) next_line->a[y] = alpha;
          for(y=v;y>mid-k[s];y--) line_write(next_line, y, alpha);
          v=mid-k[s];
#ifdef PEAK_ANTIALIAS
          alpha = (alpha * 2) / 3;
#endif
        }
#ifdef PEAK_ANTIALIAS
        line_write(next_line, mid-k[s-1], alpha/2); //antialias the end of the line.
#endif
        for(y=mid-k[s-1]-1;y>=0;y--) line_write(next_line, y, 0);
        //printf("%.1f %i ", xf, height/2+min);

        //copy the line onto the pixbuf:
#ifdef PEAK_ANTIALIAS
        int blur = 6; //bigger value gives less blurring.
#else
        //int blur = 0xffff;
#endif
        for(y=0;y<ch_height;y++){
          int p = ch*ch_height*rowstride + (ch_height - y -1)*rowstride + 3*px;
          if(p > rowstride*height || p < 0){ gerr ("p! %i > %i px=%i y=%i row=%i rowstride=%i=%i", p, 3*width*ch_height, px, y, ch_height-y-1, rowstride,3*width); return; }

#ifdef PEAK_ANTIALIAS
					a = MIN((current_line->a[y] * 2)/3 + previous_line->a[y]/blur + next_line->a[y]/blur, 0xff);
#else
					a = MIN(current_line->a[y], 0xff);
#endif
					if(!a) continue; //testing
					pixels[p  ] = (int)( bg_red * (0xff - a) + fg_red * a) >> 8;
					pixels[p+1] = (int)( bg_grn * (0xff - a) + fg_grn * a) >> 8;
					pixels[p+2] = (int)( bg_blu * (0xff - a) + fg_blu * a) >> 8;
				}

			}else{
				//no more source data available - as the pixmap is clear, we have nothing much to do.
				//gdk_draw_line(GDK_DRAWABLE(pixmap), gc, px, 0, px, height);//x1, y1, x2, y2
				DRect pts = {px, 0, px, ch_height};
				pixbuf_draw_line(cairo, &pts, 1.0, colour);
				warn_no_src_data(w, b.len, src_stop);
				printf("*"); fflush(stdout);
			}
			//next = srcidx + 1;
			//xf += WF_PEAK_RATIO * WF_PEAK_VALUES_PER_SAMPLE / samples_per_px;
			//printf("%.1f ", xf);

			line_index++;
			//printf("line_index=%i %i %i %i\n", line_index, (line_index  ) % 3, (line_index+1) % 3, (line_index+2) % 3);
		}

		dbg (2, "done. xmag=%.2f drawn: %i of %i src=%i-->%i", xmag, line_index, width, ((int)( px_start * xmag * n_tiers)) + src_inset, src_stop);
	} //end channel

#if 0
	gettimeofday(&time_stop, NULL);
	time_t      secs = time_stop.tv_sec - time_start.tv_sec;
	suseconds_t usec;
	if(time_stop.tv_usec > time_start.tv_usec){
		usec = time_stop.tv_usec - time_start.tv_usec;
	}else{
		secs -= 1;
		usec = time_stop.tv_usec + 1000000 - time_start.tv_usec;
	}
#endif
	cairo_surface_destroy(surface);
	cairo_destroy(cairo);
}


void
wf_print_blocks(Waveform* w)
{
	g_return_if_fail(w);

	WfGlBlocks* blocks = w->gl_blocks;
	int c = 0;
	printf("blocks:\n");
	int k; for(k=0;k<5;k++){
		printf("  %i %i\n", blocks->peak_texture[c].main[k], blocks->peak_texture[c].neg[k]);
	}
}


