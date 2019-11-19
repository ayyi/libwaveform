/*
  copyright (C) 2012-2019 Tim Orford <tim@orford.org>

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
#include <glib.h>
#ifdef USE_OPENGL
#include "agl/utils.h"
#else
#include "inttypes.h"
#endif
#include "decoder/ad.h"
#include "waveform/waveform.h"
#include "waveform/loaders/ardour.h"
#include "waveform/loaders/riff.h"
#include "waveform/texture_cache.h"
#include "waveform/audio.h"
#include "waveform/alphabuf.h"
#include "waveform/fbo.h"
#include "waveform/worker.h"
#include "waveform/peakgen.h"
#include "waveform/utils.h"

G_DEFINE_TYPE_WITH_PRIVATE (Waveform, waveform, G_TYPE_OBJECT);

enum  {
	WAVEFORM_DUMMY_PROPERTY,
	WAVEFORM_PROPERTY1
};

#define N_TIERS_NEEDED 3 // this will specify a hires peakbuf of minimum 16:1 resolution
#define CHECKS_DONE(W) (w->priv->state & WAVEFORM_CHECKS_DONE)

extern WF* wf;
guint peak_idle = 0;

static void  waveform_finalize      (GObject*);
static void _waveform_get_property  (GObject*, guint property_id, GValue*, GParamSpec*);

typedef struct _buf_info
{
    short* buf[2];       // source buffer
    guint  len;
    guint  len_frames;
    int    n_tiers;
} BufInfo;

struct _rms_buf_info
{
    char*  buf;          // source buffer
    guint  len;
    guint  len_frames;
};



Waveform*
waveform_construct (GType object_type)
{
	wf_get_instance();

	Waveform* w = (Waveform*) g_object_new (object_type, NULL);
	*w->priv = (WaveformPrivate){
		.hires_peaks = g_ptr_array_new(),
		.max_db = -1
	};
	return w;
}


Waveform*
waveform_load_new(const char* filename)
{
	g_return_val_if_fail(filename, NULL);

	Waveform* w = waveform_new(filename);
	waveform_load_sync(w);
	return w;
}


Waveform*
waveform_new (const char* filename)
{
	Waveform* w = waveform_construct(TYPE_WAVEFORM);
	w->filename = g_strdup(filename);
	w->renderable = true;
	return w;
}


void
waveform_set_file (Waveform* w, const char* filename)
{
	if(w->filename){
		if(filename && !strcmp(filename, w->filename)){
			// must bail otherwise peak job will not complete
			if(wf_debug) gwarn("ignoring request to set same filename");
			return;
		}
		g_free(w->filename);
	}

	w->filename = g_strdup(filename);
	w->renderable = true;
	am_promise_unref0(w->priv->peaks);
}


static void
waveform_class_init (WaveformClass* klass)
{
	waveform_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->get_property = _waveform_get_property;
	G_OBJECT_CLASS (klass)->finalize = waveform_finalize;
	g_object_class_install_property (G_OBJECT_CLASS (klass), WAVEFORM_PROPERTY1, g_param_spec_int ("property1", "property1", "property1", G_MININT, G_MAXINT, 0, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
	g_signal_new ("peakdata_ready", TYPE_WAVEFORM, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	g_signal_new ("hires_ready", TYPE_WAVEFORM, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}


static void
waveform_init (Waveform* self)
{
	self->priv = waveform_get_instance_private(self);
}


static void
waveform_finalize (GObject* obj)
{
	PF;
	Waveform* w = WAVEFORM(obj);
	WaveformPrivate* _w = w->priv;

#if 0 // the worker now uses a weak reference so there is no need to explictly cancel outstanding jobs
	wf_worker_cancel_jobs(&wf->audio_worker, w);
	waveform_peakgen_cancel(w);
#endif

	// the warning below occurs when the waveform is created and destroyed very quickly.
	if(g_hash_table_size(wf->peak_cache) && !g_hash_table_remove(wf->peak_cache, w) && wf_debug) gwarn("failed to remove waveform from peak_cache");

	int c; for(c=0;c<WF_MAX_CH;c++){
		if(_w->peak.buf[c]) g_free(_w->peak.buf[c]);
	}

	if(_w->hires_peaks){
		void** data = _w->hires_peaks->pdata;
		int i; for(i=0;i<_w->hires_peaks->len;i++){
			Peakbuf* p = data[i];
			waveform_peakbuf_free(p);
		}
		g_ptr_array_free (_w->hires_peaks, false);
	}

#ifdef USE_OPENGL
#ifdef DEBUG
	extern int texture_cache_count_by_waveform(Waveform*);
	if(texture_cache_count_by_waveform(w)){
		gerr("textures not cleared");
	}
#endif
#endif

	int m; for(m=MODE_V_LOW;m<=MODE_HI;m++){
		if(_w->render_data[m]) gwarn("actor data not cleared");
	}

#ifdef USE_OPENGL
	waveform_free_render_data(w);
#endif
	waveform_audio_free(w);
	g_free(w->filename);

	G_OBJECT_CLASS (waveform_parent_class)->finalize (obj);
	dbg(1, "done");
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


	typedef struct {
		WfCallback3 callback;
		gpointer user_data;
	} C;

	static void waveform_load_done(gpointer waveform, gpointer _c)
	{
		C* c = _c;
		if(c->callback) c->callback((Waveform*)waveform, ((Waveform*)waveform)->priv->peaks->error, c->user_data);
		g_free(c);
	}

	static void waveform_load_have_peak (Waveform* w, char* peakfile, gpointer _)
	{
		WaveformPrivate* _w = w->priv;

		_w->state &= ~WAVEFORM_LOADING;

		if(_w->peaks){ // the promise may have been removed indicating we are no longer interested in this peak
			if(peakfile){
				if(!_w->peaks->error){
					if(!waveform_load_peak(w, peakfile, 0)){
						//_w->peaks->error = g_error_new(g_quark_from_static_string(wf->domain), 1, "failed to load peak");
					}
				}
				g_signal_emit_by_name(w, "peakdata-ready");
			}
			am_promise_resolve(_w->peaks, NULL);
		}

		g_free0(peakfile);
	}

/*
 *  Load the peakdata for a waveform, and create a cached peakfile if not already existing.
 */
void
waveform_load(Waveform* w, WfCallback3 callback, gpointer user_data)
{
	WaveformPrivate* _w = w->priv;

	if(!_w->peaks){
		_w->peaks = am_promise_new(w);
	}

	am_promise_add_callback(
		_w->peaks,
		waveform_load_done,
		WF_NEW(C, .callback = callback, .user_data = user_data)
	);

	if(_w->peak.buf[0] || _w->state & WAVEFORM_LOADING){
		dbg(1, "subsequent load request");
		return;
	}

	_w->state |= WAVEFORM_LOADING;
	waveform_ensure_peakfile(w, waveform_load_have_peak, NULL);
}


gboolean
waveform_load_sync(Waveform* w)
{
	g_return_val_if_fail(w, false);

	WaveformPrivate* _w = w->priv;

	if(!_w->peaks){
		_w->peaks = am_promise_new(w);
	}

	char* peakfile = waveform_ensure_peakfile__sync(w);
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
	g_return_if_fail(w->filename);
	WaveformPrivate* _w = w->priv;

	if(w->offline) return;

	WfDecoder d = {{0,}};
	if(ad_open(&d, w->filename)){
		w->n_frames = d.info.frames; // for some filetypes this will be an estimate
		w->n_channels = w->n_channels ? w->n_channels : d.info.channels; // file info is not correct in the case of split stereo files.
		w->samplerate = d.info.sample_rate;

		ad_close(&d);
		ad_free_nfo(&d.info);
	}else{
		w->offline = true;

		if(!g_file_test(w->filename, G_FILE_TEST_EXISTS)){
			if(wf_debug) gwarn("file open failure. no such file: %s", w->filename);
		}else{
			if(wf_debug) g_warning("file open failure (%s) \"%s\"\n", sf_strerror(NULL), w->filename);

			// attempt to work with only a pre-existing peakfile in case file is temporarily unmounted
			if(waveform_load_sync(w)){
				w->n_channels = _w->peak.buf[1] ? 2 : 1;
				w->n_frames = _w->num_peaks * WF_PEAK_RATIO;
				dbg(1, "offline, have peakfile: n_frames=%Lu c=%i", w->n_frames, w->n_channels);
				return;
			}
		}
	}

	if(_w->num_peaks && !CHECKS_DONE(w)){
		if(w->n_frames > _w->num_peaks * WF_PEAK_RATIO){
			char* peakfile = waveform_ensure_peakfile__sync(w);
			gwarn("peakfile is too short. maybe corrupted. len=%i expected=%"PRIi64" '%s'", _w->num_peaks, w->n_frames / WF_PEAK_RATIO, peakfile);
			w->renderable = false;
			g_free(peakfile);
		}
		w->priv->state |= WAVEFORM_CHECKS_DONE;
	}
}


uint64_t
waveform_get_n_frames(Waveform* w)
{
	if(!w->n_frames) waveform_get_sf_data(w);

	return w->n_frames;
}


int
waveform_get_n_channels(Waveform* w)
{
	// libwaveform can only handle mono or stereo files,
	// so this will never return > 2 even if the file is
	// multichannel.

	g_return_val_if_fail(w, 0);

	if(w->n_frames) return MIN(2, w->n_channels);

	if(w->offline) return 0;

	waveform_get_sf_data(w);

	return MIN(2, w->n_channels);
}


/*
 *  Load the pre-existing peak file from disk into a buffer.
 *
 *  It is usually preferable to use waveform_load() instead,
 *  which will transparently manage the creation and loading of the peakfile.
 *  but this fn can be called explicitly if you have a pre-existing peakfile in
 *  a non-standard location.
 *
 *  Can be used to add an additional channel to an existing Waveform
 *  where the audio consists of split files.
 *
 *  @param ch_num - must be 0 or 1. Should be 0 unless loading rhs for split file.
 */
bool
waveform_load_peak (Waveform* w, const char* peak_file, int ch_num)
{
	g_return_val_if_fail(w, false);
	g_return_val_if_fail(ch_num <= WF_MAX_CH, false);
	WaveformPrivate* _w = w->priv;
	g_return_val_if_fail(!_w->peaks->error, false);

	// check is not previously loaded
	if(_w->peak.buf[ch_num]){
		dbg(2, "using existing peak data...");
		return true;
	}

	wf->load_peak(w, peak_file);

	if(ch_num) w->n_channels = MAX(w->n_channels, ch_num + 1); // for split stereo files

	_w->num_peaks = _w->peak.size / WF_PEAK_VALUES_PER_SAMPLE;
	_w->n_blocks = _w->num_peaks / WF_TEXTURE_VISIBLE_SIZE + ((_w->num_peaks % WF_TEXTURE_VISIBLE_SIZE) ? 1 : 0);
	dbg(1, "ch=%i num_peaks=%i", ch_num, _w->num_peaks);

	if(!_w->num_peaks){
		_w->peaks->error = g_error_new(g_quark_from_static_string(wf->domain), 1, "Failed to load peak");
	}

#ifdef DEBUG
	if(!g_str_has_suffix(w->filename, ".mp3")){
		if(wf_debug > -1 && w->n_frames){
			uint64_t a = _w->num_peaks;
			uint64_t b = w->n_frames / WF_PEAK_RATIO + (w->n_frames % WF_PEAK_RATIO ? 1 : 0);
			if(a != b) gwarn("got %"PRIi64" peaks, expected %"PRIi64, a, b);
		}
	}
#endif

	return !!w->priv->peak.buf[ch_num];
}


bool
waveform_peak_is_loaded(Waveform* w, int ch_num)
{
	return !!w->priv->peak.buf[ch_num];
}


static void
sort_(int ch, short dest[WF_MAX_CH][4], short src[WF_MAX_CH][4], int size)
{
	//sort j into ascending order

	int i, j=0, min, new_min, top=size, p=0;
	guint16 n[WF_MAX_CH][4] = {{0,}, {0,}};

	for(i=0;i<size;i++) n[ch][i] = src[ch][i]; // copy the source array

	for(i=0;i<size;i++){
		min = n[ch][0];
		p=0;
		for(j=1;j<top;j++){
			new_min = MIN(min, n[ch][j]);
			if(new_min < min){
				min = new_min;
				p = j;
			}
		}
		// p is the index to the value we have used, and need to remove.

		dest[ch][i] = min;

		int m; for(m=p;m<top;m++) n[ch][m] = n[ch][m+1]; // move remaining entries down.
		top--;
		n[ch][top] = 0;
	}
}


// deprecated
static void
sort_mono(short* dest, const short* src, int size)
{
	//sort j into ascending order

	int i, j=0, min, new_min, top=size, p=0;
	guint16 n[4] = {0, 0, 0, 0};

	for(i=0;i<size;i++) n[i] = src[i]; // copy the source array

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
		// p is the index to the value we have used, and need to remove.

		dest[i] = min;

		int m; for(m=p;m<top;m++) n[m] = n[m+1]; // move remaining entries down.
		top--;
		n[top] = 0;
	}
}


#define MAX_PART_HEIGHT 1024 //FIXME

typedef struct {
	guchar a[MAX_PART_HEIGHT]; //alpha level for each pixel in the line.
} Line;
static Line line[3];


static void
line_write(Line* line, int index, guchar val)
{
#ifdef DEBUG
	if(index < 0 || index >= MAX_PART_HEIGHT){ gerr ("y=%i", index); return; }
#endif
	line->a[index] = val;
}


static void
line_clear(Line* line)
{
	memset(line->a, 0, sizeof(guchar) * MAX_PART_HEIGHT);
}


static inline bool
get_buf_info(const Waveform* w, int block_num, BufInfo* b)
{
	//buf_info is just a convenience. It is filled twice during a tile draw.

	bool hires_mode = (block_num > -1);

	if(hires_mode){
		WfAudioData* audio = &w->priv->audio;
		g_return_val_if_fail(audio->buf16, false);
		Peakbuf* peakbuf = waveform_get_peakbuf_n((Waveform*)w, block_num);
		*b = (BufInfo){
			.buf[0] = peakbuf->buf[0],
			.buf[1] = peakbuf->buf[1],
			.len = peakbuf->size,
			.n_tiers = RESOLUTION_TO_TIERS(peakbuf->resolution)
		};
		g_return_val_if_fail(b->buf, false);
		//dbg(2, "HI block=%i len=%i %i", block_num, b->len, b->len / WF_PEAK_VALUES_PER_SAMPLE);
	}else{
		dbg(2, "MED len=%i %i (x256=%i)", b->len, b->len / WF_PEAK_VALUES_PER_SAMPLE, (b->len * 256) / WF_PEAK_VALUES_PER_SAMPLE);

		*b = (BufInfo){
			.buf[0]     = w->priv->peak.buf[0], // source buffer.
			.buf[1]     = w->priv->peak.buf[1],
			.len        = w->priv->peak.size,
			.len_frames = 0
		};
	}
	b->len_frames = b->len / WF_PEAK_VALUES_PER_SAMPLE;

	if(!b->buf) return false;
	g_return_val_if_fail(b->len < 10000000, false);

	return true;
}


static inline bool
get_rms_buf_info(const char* buf, guint len, struct _rms_buf_info* b, int ch)
{
	//buf_info is just a convenience. It is filled twice during a tile draw.

	//gboolean hires_mode = (peakbuf != NULL);

	b->buf        = (char*)buf; //source buffer.
	b->len        = len;
	b->len_frames = b->len;// / WF_PEAK_VALUES_PER_SAMPLE;
	return TRUE;
}


short*
waveform_peakbuf_malloc(Waveform* waveform, int ch, uint32_t size)
{
	WfPeakBuf* buf = &waveform->priv->peak;
	uint32_t bytes = size * sizeof(short);

	buf->buf[ch] = g_malloc(bytes);
	buf->size = size;

	wf->peak_mem_size += bytes;
	g_hash_table_insert(wf->peak_cache, waveform, waveform); // is removed in __finalize()

#ifdef DEBUG
	// check cache size
	if(wf_debug > 1){
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
	}
#endif

	return buf->buf[ch];
}


static void
alphabuf_draw_line(AlphaBuf* pixbuf, WfDRect* pts, double line_width, GdkColor* colour)
{
}


void
waveform_peak_to_alphabuf(Waveform* w, AlphaBuf* a, int scale, int* start, int* end, GdkColor* colour)
{
	/*
	 renders a peakfile (pre-loaded into WfPeakBuf* waveform->priv->peak) onto the given 8 bit alpha-map buffer.

	 -the given buffer is for a single block only.

	 @param start - if set, we start rendering at this value in the dest buffer.
	 @param end   - if set, dest buffer index to stop rendering at.
	                Is allowed to be bigger than the output buf width.

	 TODO
	   -can be simplified. Eg, the antialiasing should be removed.
	   -further optimisation: dont scan pixels above the peaks. Should we record the overall file peak level?
	*/

	g_return_if_fail(a);
	g_return_if_fail(w);
	g_return_if_fail(w->priv->peak.buf[0]);

#if 0
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
#endif

	WfPeakSample sample;
	short min;                //negative peak value for each pixel.
	short max;                //positive peak value for each pixel.

	int n_chans     = waveform_get_n_channels(w);
	int width       = a->width;
	int height      = a->height;
	g_return_if_fail(width && height);
	guchar* pixels  = a->buf;
	int rowstride   = a->width;
	int ch_height   = height / n_chans;
	int vscale      = (256*128*2) / ch_height;

	int px = 0;                  //pixel count starting at the lhs of the Part.
	int j, y;
	int src_start = 0;           //index into the source buffer for each sample pt.
	int src_stop  = 0;   

	double xmag = 1.0 * scale; //making xmag smaller increases the visual magnification.
	                 //-the bigger the mag, the less samples we need to skip.
	                 //-as we're only dealing with smaller peak files, we need to adjust by the RATIO.

	int src_offset = 0;
	int px_start = start ? *start : 0;
	int px_stop  = *end;//   ? MIN(*end, width) : width;
	dbg (2, "start=%i end=%i", px_start, px_stop);
	//dbg (1, "width=%i height=%i", width, height);

	if(width < px_stop - px_start){ gwarn("alphabuf too small? %i < %i", width, px_stop - px_start); return; }

//int last_src = 0;

	int ch; for(ch=0;ch<n_chans;ch++){

		BufInfo b = {0,};
		g_return_if_fail(get_buf_info(w, -1, &b));

		line_clear(&line[0]);
		int line_index = 0;

		for(px=px_start;px<px_stop;px++){
			src_start = ((int)( px   * xmag)) + src_offset;
			src_stop  = ((int)((px+1)* xmag));
//last_src = src_stop;
			//printf("%i ", src_start);
			if(src_start < 0){ dbg(2, "skipping line..."); continue; }

			Line* previous_line = &line[(line_index  ) % 3];
			Line* current_line  = &line[(line_index+1) % 3];
			Line* next_line     = &line[(line_index+2) % 3];

			//arrays holding subpixel levels.
			short lmax[4] = {0,0,0,0};
			short lmin[4] = {0,0,0,0};
			short k   [4] = {0,0,0,0}; //sorted copy of l.

			int mid = ch_height / 2;

			if(src_stop <= b.len/2){
				min = 0; max = 0;
				int n_sub_px = 0;
				for(j=src_start;j<src_stop;j++){ //iterate over all the source samples for this pixel.
					sample.positive = b.buf[ch][2*j   ];
					sample.negative = b.buf[ch][2*j +1];
					if(sample.positive > max) max = sample.positive;
					if(sample.negative < min) min = sample.negative;
//if((j > 240 && j<250) || j>490) dbg(0, "  s=%i %i %i", j, (int)max, (int)(-min));

					if(n_sub_px < 4){
						//FIXME these supixels are not evenly distributed when > 4 available, as we currently only use the first 4.
						lmax[n_sub_px] = sample.positive / vscale;
						lmin[n_sub_px] =-sample.negative / vscale; //lmin contains positive values.
					}
					n_sub_px++;
				}
				if(!n_sub_px) continue;

				if(!line_index){
					//first line - we also grab the previous sample for antialiasing.
					if(px){
						j = src_start - 1;
						sample.positive = b.buf[ch][2*j   ];
						sample.negative = b.buf[ch][2*j +1];
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
				min /= vscale;
				max /= vscale;

				sort_mono(k, lmax, MIN(n_sub_px, 4));

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
				sort_mono(k, lmin, MIN(n_sub_px, 4));
				alpha = 0xff;
				v = mid;
				for(s=0;s<MIN(n_sub_px, 4);s++){
					for(y=v;y>mid-k[s];y--) line_write(next_line, y, alpha);
					v=mid-k[s];
					alpha = (alpha * 2) / 3;
				}
				line_write(next_line, mid-k[s-1], alpha/2); //antialias the end of the line.
				for(y=mid-k[s-1]-1;y>0;y--) line_write(next_line, y, 0);
				//printf("%.1f %i ", xf, height/2+min);

#if 0
				//debugging:
				for(y=height/2;y<height;y++){
					int b = MIN((current_line->a[y] * 3)/4 + previous_line->a[y]/6 + next_line->a[y]/6, 0xff);
					if(!b){ dbg_max = MAX(y, dbg_max); break; }
				}
#endif

				//draw the lines:
				int blur = 6; //bigger value gives less blurring.
				for(y=0;y<ch_height;y++){
					int p = ch*ch_height*rowstride + (ch_height - y -1)*rowstride + (px - px_start)/* + border*/;
					if(p > rowstride*height || p < 0){ gerr ("p! %i > %i px=%i y=%i row=%i rowstride=%i=%i", p, 3*width*height, px, y, height-y-1, rowstride, 3*width); return; }

					a = MIN((current_line->a[y] * 3)/4 + previous_line->a[y]/blur + next_line->a[y]/blur, 0xff);
					pixels[p] = (int)(0xff * a) >> 8;
				}

			}else{
				//no more source data available - as the pixmap is clear, we have nothing much to do.
				//gdk_draw_line(GDK_DRAWABLE(pixmap), gc, px, 0, px, height);//x1, y1, x2, y2
				WfDRect pts = {px, 0, px, ch_height};
				alphabuf_draw_line(a, &pts, 1.0, colour);
//gwarn("!");
			}
			//next = srcidx + 1;
			//xf += WF_PEAK_RATIO * WF_PEAK_VALUES_PER_SAMPLE / samples_per_px;
			//printf("%.1f ", xf);

			line_index++;
			//printf("line_index=%i %i %i %i\n", line_index, (line_index  ) % 3, (line_index+1) % 3, (line_index+2) % 3);
		}
	}
//dbg(0, "last_src=%i x=%i", last_src, px);

	//printf("%s(): done. drawn: %i of %i stop=%i\n", __func__, line_done_count, width, src_stop);

#if 0
	struct timeval time_stop;
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
#endif
}


void
waveform_peak_to_alphabuf_hi(Waveform* w, AlphaBuf* a, int block, WfSampleRegion region, GdkColor* colour)
{
	//copy from hi-res peakfile to hi-res alphabuf
	//-region specifies the output range (not actually samples). TODO rename

	//TODO start is -2 so we probably need to load from 2 peakbufs?

#if 0
	void _draw_line(AlphaBuf* ab, int x, int y1, int y2, float a, int ch, int ch_height)
	{
		int rowstride = ab->width; //probably.

		int y; for(y=y1;y<y2;y++){
			int p = ch*ch_height*rowstride + (ch_height - y -1)*rowstride + x/* + border*/;
			if(p > rowstride*ab->height || p < 0){ gerr ("p! %i > %i px=%i y=%i row=%i rowstride=%i=%i", p, 3*ab->width*ab->height, x, y, ab->height-y-1, rowstride, 3*ab->width); return; }

			ab->buf[p] = (int)(0xff * a) >> 8;
		}
	}
#endif

	int chan = 0; //TODO
	int n_chans = 1; //TODO
	float v_gain = 1.0; // TODO

	float alpha = 1.0;

	Peakbuf* peakbuf = waveform_get_peakbuf_n(w, block);
	WfRectangle rect = {0.0, 0.0, a->width, a->height};
	int ch_height = WF_PEAK_TEXTURE_SIZE / n_chans;

	int64_t region_end = region.start + (int64_t)region.len;

	//float region_len = region.len;
	short* data = peakbuf->buf[chan];

	int io_ratio = (peakbuf->resolution == 16 || peakbuf->resolution == 8) ? 16 : 1; //TODO
io_ratio = 1;
	int x = 0;
	int p = 0;
	int p_ = region.start / io_ratio;
	//dbg(0, "width=%.2f region=%Li-->%Li xgain=%.2f resolution=%i io_ratio=%i", rect.len, region.start, region.start + (int64_t)region.len, rect.len / region_len, peakbuf->resolution, io_ratio);
	//dbg(0, "x: %.2f --> %.2f", (((double)0) / region_len) * rect.len, (((double)4095) / region_len) * rect.len);
	g_return_if_fail(region_end / io_ratio <= peakbuf->size / WF_PEAK_VALUES_PER_SAMPLE);
	while (p < a->width){
									if(2 * p_ >= peakbuf->size) gwarn("s_=%i size=%i", p_, peakbuf->size);
									g_return_if_fail(2 * p_ < peakbuf->size);
		//x = rect.left + (((double)p) / region_len) * rect.len * io_ratio;
//x = rect.left + (((double)p) / region_len) * rect.len * io_ratio;
		x = p;
//if(!p) dbg(0, "x=%i", x);
		if (x - rect.left >= rect.len) break;
//if(p < 10) printf("    x=%i %2i %2i\n", x, data[2 * p_], data[2 * p_ + 1]);

		double y1 = ((double)data[WF_PEAK_VALUES_PER_SAMPLE * p_    ]) * v_gain * (rect.height / 2.0) / (1 << 15);
		double y2 = ((double)data[WF_PEAK_VALUES_PER_SAMPLE * p_ + 1]) * v_gain * (rect.height / 2.0) / (1 << 15);

#if 0
		_draw_line(a, x, rect.top - y1 + rect.height / 2, rect.top - y2 + rect.height / 2, alpha, chan, ch_height);
#else
		int rowstride = a->width; //probably.

		int y1_ = rect.top - y1 + rect.height / 2;
		int y2_ = rect.top - y2 + rect.height / 2;
//if(p < 10) printf("      y=%i-->%i\n", y1_, y2_);
//int pp = p;
		//int y; for(y=0;y<ch_height;y++){
		int y; for(y=y1_;y<y2_;y++){
			int p = chan*ch_height*rowstride + (ch_height - y -1)*rowstride + x/* + border*/;
			if(p > rowstride*a->height || p < 0){ gerr ("p! %i > %i px=%i y=%i row=%i rowstride=%i=%i", p, 3*a->width*a->height, x, y, a->height-y-1, rowstride, 3*a->width); return; }

			a->buf[p] = (int)(0xff * alpha);
//if(pp < 10) printf("              y=%i \n", y);
		}
#endif
//if(p == 4095)
//		_draw_line(rect->left + x, 0, rect->left + x, rect->height, r, g, b, 1.0);

		p++;
		p_++;
	}
	dbg(2, "n_lines=%i x0=%.1f x=%i y=%.1f h=%.1f", p, rect.left, x, rect.top, rect.height);
}


void
waveform_rms_to_alphabuf(Waveform* waveform, AlphaBuf* pixbuf, int* start, int* end, double samples_per_px, GdkColor* colour, uint32_t colour_bg)
{
	/*

	temporary. will change to using 8bit greyscale image with no alpha channel.

	*/

	g_return_if_fail(pixbuf);
	g_return_if_fail(waveform);

	/*
	int fg_red = colour->red   >> 8;
	int fg_grn = colour->green >> 8;
	int fg_blu = colour->blue  >> 8;
	*/

#if 0
	struct timeval time_start;
	gettimeofday(&time_start, NULL);
#endif

	if(samples_per_px < 0.001) gerr ("samples_per_pix=%f", samples_per_px);
	WfPeakSample sample;
	short min;                //negative peak value for each pixel.
	short max;                //positive peak value for each pixel.

	float gain       = 1.0;
	dbg(3, "peak_gain=%.2f", gain);

	int n_chans      = waveform_get_n_channels(waveform);
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

		RmsBuf* rb = waveform->priv->rms_buf0;
		if(!rb){
			if(!(rb = waveform_load_rms_file(waveform, ch))) continue;
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
		//Line* previous_line = &line[0];
		//Line* current_line  = &line[1]; //the line being displayed. Not the line being written to.
		//Line* next_line     = &line[2];

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

			Line* previous_line = &line[(line_index  ) % 3];
			Line* current_line  = &line[(line_index+1) % 3];
			Line* next_line     = &line[(line_index+2) % 3];

			//arrays holding subpixel levels.
			short lmax[4] = {0,0,0,0};
#if 0
			short lmin[4] = {0,0,0,0};
#endif
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
#if 0
						//lmin[sub_px] =-(ch_height * sample.negative) / (256*128*2); //lmin contains positive values.
						lmin[sub_px] = lmax[sub_px];
#endif
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

				sort_mono(k, lmax, MIN(sub_px, 4));

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
				sort_mono(k, lmin, MIN(sub_px, 4));
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
				WfDRect pts = {px, 0, px, ch_height};
				alphabuf_draw_line(pixbuf, &pts, 1.0, colour);
//				warn_no_src_data(waveform, b.len, src_stop);
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

#if 0
	gettimeofday(&time_stop, NULL);
	time_t secs = time_stop.tv_sec - time_start.tv_sec;
	suseconds_t usec;
	if(time_stop.tv_usec > time_start.tv_usec){
		usec = time_stop.tv_usec - time_start.tv_usec;
	}else{
		secs -= 1;
		usec = time_stop.tv_usec + 1000000 - time_start.tv_usec;
	}
#endif
}


//------------------------------------------------------------------------
// loaders

//#warning TODO location of rms files and RHS.
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
	if(read(fp, rb->buf, fsize) != fsize) gerr ("read error. couldnt read %"PRIi64" bytes from %s", fsize, rms_file);
  
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


#if 0
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
waveform_get_peakbuf_n(Waveform* w, int block_num)
{
	g_return_val_if_fail(w, NULL);

	GPtrArray* peaks = w->priv->hires_peaks;
	g_return_val_if_fail(peaks, NULL);
	Peakbuf* peakbuf = NULL;
	if(block_num >= peaks->len){
	}else{
		peakbuf = g_ptr_array_index(peaks, block_num);
	}
	dbg(2, "block_num=%i peaks->len=%i", block_num, peaks->len);

	return peakbuf;
}


void
waveform_peakbuf_assign(Waveform* w, int block_num, Peakbuf* peakbuf)
{
	g_return_if_fail(peakbuf);

	GPtrArray* peaks = w->priv->hires_peaks;
	if(block_num >= peaks->len){
		g_ptr_array_set_size(peaks, block_num + 1);
	}
	peaks->pdata[block_num] = peakbuf;
}


int
waveform_get_n_audio_blocks(Waveform* w)
{
	WfAudioData* audio = &w->priv->audio;
	if(!audio->n_blocks){
		uint64_t n_frames = waveform_get_n_frames(w);

		int xtra = (n_frames % WF_SAMPLES_PER_TEXTURE) ? 1 : 0;
		audio->n_blocks = n_frames / WF_SAMPLES_PER_TEXTURE + xtra; // WF_SAMPLES_PER_TEXTURE takes border into account

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
	if(w->priv->max_db > -1) return w->priv->max_db;

	int i;
	short max_level = 0;
	int c; for(c=0;c<2;c++){
		short* buf = w->priv->peak.buf[c];
		if(!buf) continue;

		for(i=0;i<w->priv->peak.size;i++){
			max_level = MAX(max_level, buf[i]);
		}
	}

	return w->priv->max_db = max_level;
}


#ifdef USE_GDK_PIXBUF

static void
pixbuf_draw_line(cairo_t* cr, WfDRect* pts, double line_width, uint32_t colour)
{
	//TODO set colour, or remove arg
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
waveform_peak_to_pixbuf(Waveform* w, GdkPixbuf* pixbuf, WfSampleRegion* region, uint32_t colour, uint32_t bg_colour, bool single)
{
	g_return_if_fail(w && pixbuf);

	if(!region) region = &(WfSampleRegion){0, waveform_get_n_frames(w)};

	gdk_pixbuf_fill(pixbuf, bg_colour);
	double samples_per_px = region->len / gdk_pixbuf_get_width(pixbuf);
	waveform_peak_to_pixbuf_full(w, pixbuf, region->start, NULL, NULL, samples_per_px, colour, bg_colour, 1.0, single);
}


	typedef struct {
		Waveform*         waveform;
		WfSampleRegion    region;
		uint32_t          colour;
		uint32_t          bg_colour;
		GdkPixbuf*        pixbuf;
		WfPixbufCallback* callback;
		gpointer          user_data;
		int               n_blocks_total;
		int               n_blocks_done;
	} C2;

	void _waveform_peak_to_pixbuf__load_done(C2* c)
	{
		PF;
		double samples_per_px = c->region.len / gdk_pixbuf_get_width(c->pixbuf);
		gdk_pixbuf_fill(c->pixbuf, c->bg_colour);
		waveform_peak_to_pixbuf_full(c->waveform, c->pixbuf, c->region.start, NULL, NULL, samples_per_px, c->colour, c->bg_colour, 1.0, false);

		if(c->callback) c->callback(c->waveform, c->pixbuf, c->user_data);
		g_free(c);
	}

	void waveform_load_audio_done(Waveform* w, int block, gpointer _c)
	{
		dbg(3, "block=%i", block);
		C2* c = _c;
		g_return_if_fail(c);

		c->n_blocks_done++;
//TODO no, we cannot load the whole file at once!! render each block separately
		// call waveform_peak_to_pixbuf_full here for the block
		if(c->n_blocks_done >= c->n_blocks_total){
			_waveform_peak_to_pixbuf__load_done(c);
		}
	}

void
waveform_peak_to_pixbuf_async(Waveform* w, GdkPixbuf* pixbuf, WfSampleRegion* region, uint32_t colour, uint32_t bg_colour, WfPixbufCallback callback, gpointer user_data)
{
	g_return_if_fail(w && pixbuf && region);

					g_return_if_fail(region->start + region->len <= waveform_get_n_frames(w));
					g_return_if_fail(region->len > 0);

	C2* c = g_new0(C2, 1);
	*c = (C2){
		.waveform  = w,
		.region    = *region,
		.colour    = colour,
		.bg_colour = bg_colour,
		.pixbuf    = pixbuf,
		.callback  = callback,
		.user_data = user_data
	};

	double samples_per_px = region->len / gdk_pixbuf_get_width(pixbuf);
	bool hires_mode = ((samples_per_px / WF_PEAK_RATIO) < 1.0);
	if(hires_mode){
		int b0 = region->start / WF_SAMPLES_PER_TEXTURE;
		int b1 = (region->start + region->len) / WF_SAMPLES_PER_TEXTURE;
		c->n_blocks_total = b1 - b0 + 1;
		int b; for(b=b0;b<=b1;b++){
			waveform_load_audio(w, b, N_TIERS_NEEDED, waveform_load_audio_done, c);
		}
		return;
	}

	_waveform_peak_to_pixbuf__load_done(c);
}


typedef struct {
    int start, stop;
} iRange;

typedef struct {
	Line *previous, *current, *next;
} Lines;


void
waveform_peak_to_pixbuf_full(Waveform* waveform, GdkPixbuf* pixbuf, uint32_t region_inset, int* start, int* end, double samples_per_px, uint32_t colour, uint32_t colour_bg, float gain, bool single)
{
	/*
		renders part of a peakfile (loaded into the buffer given by waveform->buf) onto the given pixbuf.
		-the pixbuf must already be cleared, with the correct background colour.

		-although not explicitly block-based, it can be used with blocks by specifying start and end.

		-if the data is not imediately available it will not be waited for. For short samples or big pixbufs, caller should subscribe to the Waveform::peakdata_ready signal.

		@param pixbuf         - pixbuf covers the whole Waveform. We only render to the bit between @start and @end. 
		                        The simplicity of one pixbuf per waveform imposes limitations on file size and zooming.

		@param region_inset   - (sample frames)
		@param start          - (pixels) if set, we start rendering at this value from the left of the pixbuf.
		@param end            - (pixels) if set, num of pixels from left of pixbuf to stop rendering at.
		                        Can be bigger than pixbuf width.
		                        If samples_per_px is set it is not neccesary to specify the end.
		@param samples_per_px - sets the magnification.
		@param colour_bg      - 0xrrggbbaa. used for antialiasing.
		@param single         - all channels are mixed to a single waveform

		TODO
			v-high mode - currently fails if resolution is less than 1:16

			further optimisation:
				-dont scan pixels above the peaks. Should we record the overall region/file peak level?

			API:
				- why region_inset but not region end?
				- add WaveformPixbuf object?
	*/

	g_return_if_fail(pixbuf);
	g_return_if_fail(waveform);

	static Line line[WF_MAX_CH][3];

	gboolean hires_mode = ((samples_per_px / WF_PEAK_RATIO) < 1.0);

	int bg_red = (colour_bg & 0xff000000) >> 24;
	int bg_grn = (colour_bg & 0x00ff0000) >> 16;
	int bg_blu = (colour_bg & 0x0000ff00) >>  8;
	int fg_red = (colour    & 0xff000000) >> 24;
	int fg_grn = (colour    & 0x00ff0000) >> 16;
	int fg_blu = (colour    & 0x0000ff00) >>  8;

#if 0
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
#endif

	if(samples_per_px < 0.001) gerr ("samples_per_pix=%f", samples_per_px);
	WfPeakSample sample[WF_MAX_CH];
	WfPeakSample peak[WF_MAX_CH];

	dbg(3, "peak_gain=%.2f", gain);

	int n_chans      = waveform_get_n_channels(waveform);
	g_return_if_fail(n_chans);

	int n_chans_out  = single ? 1 : n_chans;
	int width        = gdk_pixbuf_get_width(pixbuf);
	int height       = gdk_pixbuf_get_height(pixbuf);
	guchar* pixels   = gdk_pixbuf_get_pixels(pixbuf);
	int rowstride    = gdk_pixbuf_get_rowstride(pixbuf);

#if 0
	cairo_surface_t* surface = cairo_image_surface_create_for_data(pixels, CAIRO_FORMAT_RGB24, width, height, rowstride);
	cairo_t* cairo = cairo_create(surface);
#endif

	if (height > MAX_PART_HEIGHT) gerr ("part too tall. not enough memory allocated.");
	int ch_height = height / n_chans_out;
	int vscale = (256 * 128 * 2) / ch_height;

	int px_start = start ? *start : 0;
	int px_stop  = end   ? MIN(*end, width) : width;
	dbg (3, "px_start=%i px_end=%i", px_start, px_stop);

	int hires_block = -1;
	int border = 0;

		//we use the same part of Line for each channel, it is then rendered to the pixbuf with a channel offset.

		if(hires_mode){
			hires_block = region_inset / WF_PEAK_BLOCK_SIZE;
			border = TEX_BORDER_HI;

			if(!waveform->priv->audio.buf16){
				waveform_load_audio_sync(waveform, hires_block, N_TIERS_NEEDED);
			}
		}

		BufInfo b = {0,};
		g_return_if_fail(get_buf_info(waveform, hires_block, &b));

		// xmag defines how many 'samples' we need to skip to get the next pixel.
		// making xmag smaller increases the visual magnification.
		// -the bigger the mag, the less samples we need to skip.
		// -as we're only dealing with smaller peak files, we need to adjust by WF_PEAK_RATIO.

		double xmag = samples_per_px / ((hires_mode ? (1 << b.n_tiers) : WF_PEAK_RATIO));

		iRange src = {0,};         // frames or peakbuf idx. multiply by 2 to get the index into the source buffer for each sample pt.

		line_clear(&line[WF_LEFT][0]);
		line_clear(&line[WF_RIGHT][0]);
		int line_index = 0;

		int block_offset = hires_mode ? hires_block * (b.len_frames - 2 * border) : 0;

		uint32_t region_inset_ = hires_mode ? region_inset / (1 << b.n_tiers): region_inset / WF_PEAK_RATIO;

		int px, i = 0; for(px=px_start; px<px_stop; px++, i++){

		//note: units for src.start are peakbuf *frames*.

		src.start = border + ((int)((px  ) * xmag)) + region_inset_ - block_offset;
		src.stop  = border + ((int)((px+1) * xmag)) + region_inset_ - block_offset;
		if(src.start == src.stop){ printf("^"); fflush(stdout); } // src data not hi enough resolution
		if(hires_mode){
			if(wf_debug && (px == px_start)){
				double percent = 2 * 100 * (((int)(px * xmag)) + region_inset - (wf_peakbuf_get_max_size(b.n_tiers) * hires_block) / WF_PEAK_VALUES_PER_SAMPLE) / b.len;
				dbg(2, "reading from buf=%i=%.2f%% stop=%i buflen=%i blocklen=%i", src.start, percent, src.stop, b.len, wf_peakbuf_get_max_size(b.n_tiers));
			}
			if(src.stop > b.len_frames - border/2){
				dbg(1, "**** block change needed!");
				hires_block++;
				Peakbuf* peakbuf = waveform_get_peakbuf_n(waveform, hires_block);
				g_return_if_fail(peakbuf);
				if(!get_buf_info(waveform, hires_block, &b)){ break; }
				block_offset = hires_block * (b.len_frames - 2 * border);

				// restart the iteration
				px--;
				i--;
				continue;
			}
		}

		Lines lines[WF_MAX_CH] = {
			{
				.previous = &line[WF_LEFT][(line_index  ) % 3],
				.current  = &line[WF_LEFT][(line_index+1) % 3], // the line being displayed. Not the line being written to.
				.next     = &line[WF_LEFT][(line_index+2) % 3]
			},
			{
				.previous = &line[WF_RIGHT][(line_index  ) % 3],
				.current  = &line[WF_RIGHT][(line_index+1) % 3],
				.next     = &line[WF_RIGHT][(line_index+2) % 3]
			}
		};

		//arrays holding subpixel levels.
		short lmax[WF_MAX_CH][4] = {{0,}, {0,}};
		short lmin[WF_MAX_CH][4] = {{0,}, {0,}};
		short k   [WF_MAX_CH][4] = {{0,}, {0,}}; // sorted copy of l.

		int mid = ch_height / 2;

		int j, y;
		if(src.stop < b.len_frames){
			int sub_px = 0;
			int ch; for(ch=0;ch<n_chans;ch++){
				sub_px = 0;
				peak[ch] = (WfPeakSample){0,};

				for(j=src.start;j<src.stop;j++){ //iterate over all the source samples for this pixel.
					sample[ch] = (WfPeakSample){
						.positive = b.buf[ch][2*j   ] * gain,
						.negative = b.buf[ch][2*j +1] * gain
					};
					peak[ch].positive = MAX(peak[ch].positive, sample[ch].positive);
					peak[ch].negative = MIN(peak[ch].negative, sample[ch].negative);

					if(sub_px < 4){
						//FIXME these supixels are not evenly distributed when > 4 available.
						lmax[ch][sub_px] = (ch_height * sample[ch].positive) / (256*128*2);
						lmin[ch][sub_px] =-(ch_height * sample[ch].negative) / (256*128*2); //lmin contains positive values.
					}
					sub_px++;
				}
			}
			if(!sub_px){ /*printf("*"); fflush(stdout);*/ continue; }

			if(!line_index){
				// first line - we also grab the previous sample for antialiasing.
				for(ch=0;ch<n_chans;ch++){
					if(px){
						j = src.start - 1;
						sample[ch] = (WfPeakSample){
							.positive = b.buf[ch][2*j   ],
							.negative = b.buf[ch][2*j +1]
						};
						peak[ch].positive = sample[ch].positive / vscale;
						peak[ch].negative =-sample[ch].negative / vscale;
																// TODO why peak.negative not used here?
						for(y=mid;y<mid+peak[ch].positive;y++) line_write(lines[ch].previous, y, 0xff);
						for(y=mid;y<mid+peak[ch].positive;y++) line_write(lines[ch].current, y, 0xff);
						for(y=mid;y>mid-peak[ch].positive;y--) line_write(lines[ch].previous, y, 0xff);
						for(y=mid;y>mid-peak[ch].positive;y--) line_write(lines[ch].current, y, 0xff);
					}
					else line_clear(lines[ch].current);
				}
			}

			//scale the values to the pixbuf height:
			for(ch=0;ch<n_chans;ch++){
				peak[ch].negative = (ch_height * peak[ch].negative) / (256*128*2);
				peak[ch].positive = (ch_height * peak[ch].positive) / (256*128*2);

				sort_(ch, k, lmax, MIN(sub_px, 4));

				//note that we write into line.next, not line.current. There is a visible delay of one line.
				int alpha = 0xff;
				int v = 0;

				//positive peak:
				int s; for(s=0;s<MIN(sub_px, 4);s++){
					for(y=v;y<k[ch][s];y++){
						line_write(lines[ch].next, mid +y, alpha);
					}
					v = k[ch][s];
					alpha = (alpha * 2) / 3;
				}
				line_write(lines[ch].next, mid + k[ch][s-1], alpha/2); //blur!
				for(y=k[ch][s-1]+1;y<=mid;y++){ //this looks like y goes 1 too high, but the line isnt cleared otherwise.
					lines[ch].next->a[mid + y] = 0;
				}

				//negative peak:
				sort_(ch, k, lmin, MIN(sub_px, 4));
				alpha = 0xff;
				v = mid;
				for(s=0;s<MIN(sub_px, 4);s++){
					for(y=v;y>mid-k[ch][s];y--) line_write(lines[ch].next, y, alpha);
					v = mid - k[ch][s];
					alpha = (alpha * 2) / 3;
				}
				line_write(lines[ch].next, mid-k[ch][s-1], alpha/2); //antialias the end of the line.
				for(y=mid-k[ch][s-1]-1;y>=0;y--) line_write(lines[ch].next, y, 0);

			} // end channel

			//draw the lines:
			#define blur 6 // bigger value gives less blurring.
			int n = single ? n_chans : 1;
			for(ch=0;ch<n_chans_out;ch++){
				for(y=0;y<ch_height;y++){
					int p = rowstride*(ch*ch_height + (ch_height - y -1)) + 3*px;
					if(p > rowstride*height || p < 0){ gerr ("p! %i > %i px=%i y=%i row=%i rowstride=%i=%i", p, 3*width*ch_height, px, y, ch_height-y-1, rowstride,3*width); return; }

					int a = 0;
					int c; for(c=0;c<n;c++){
						a += ((lines[c].current->a[y] * 2)/3 + lines[c].previous->a[y]/blur + lines[c].next->a[y]/blur) / n;
					}
					if(!a) continue;
					a = MIN(a, 0xff);
					pixels[p  ] = (int)(bg_red * (0xff - a) + fg_red * a) >> 8;
					pixels[p+1] = (int)(bg_grn * (0xff - a) + fg_grn * a) >> 8;
					pixels[p+2] = (int)(bg_blu * (0xff - a) + fg_blu * a) >> 8;
				}
			}
		}else{
			//no more source data available - as the pixmap is clear, we have nothing much to do.
			//gdk_draw_line(GDK_DRAWABLE(pixmap), gc, px, 0, px, height);//x1, y1, x2, y2
#if 0
			WfDRect pts = {px, 0, px, ch_height};
			pixbuf_draw_line(cairo, &pts, 1.0, colour);
#endif
			warn_no_src_data(waveform, b.len, src.stop);
			printf("*"); fflush(stdout);
		}

		line_index++;
	}

	dbg (1, "done. xmag=%.2f drawn: %i of %i src=%i-->%i", xmag, line_index, width, ((int)(px_start * xmag)) + region_inset, src.stop);

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
	dbg(0, "pxstart=%03i: %lu.%06lu s", px_start, secs, usec);
#endif

#ifdef DBG_TILE_BOUNDARY
	DRect pts = {px_start, 0, px_start, ch_height};
	pixbuf_draw_line(cairo, &pts, 1.0, 0xff0000ff);
	{
												pixels[0] = 0xff;
												pixels[1] = 0x00;
												pixels[2] = 0x00;
												pixels[3* px_stop + 0] = 0x00;
												pixels[3* px_stop + 1] = 0xff;
												pixels[3* px_stop + 2] = 0x00;
	}
#endif
#if 0
	cairo_surface_destroy(surface);
	cairo_destroy(cairo);
#endif
}


#undef PEAK_ANTIALIAS
void
waveform_rms_to_pixbuf(Waveform* w, GdkPixbuf* pixbuf, uint32_t src_inset, int* start, int* end, double samples_per_px, uint32_t colour, uint32_t colour_bg, float gain)
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
	dbg(3, "inset=%i", src_inset);

	gboolean hires_mode = ((samples_per_px / WF_PEAK_RATIO) < 1.0);
	if(hires_mode) return;

	int bg_red = (colour_bg & 0xff000000) >> 24;
	int bg_grn = (colour_bg & 0x00ff0000) >> 16;
	int bg_blu = (colour_bg & 0x0000ff00) >>  8;
	int fg_red = (colour & 0xff000000) >> 24;
	int fg_grn = (colour & 0x00ff0000) >> 16;
	int fg_blu = (colour & 0x0000ff00) >>  8;
	//printf("%s(): bg=%x r=%x g=%x b=%x \n", __func__, colour_bg, bg_red, bg_grn bg_blu);

#if 0
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
#endif

	if(samples_per_px < 0.001) gerr ("samples_per_pix=%f", samples_per_px);
	WfPeakSample sample;
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
	int vscale = (256 * 128 * 2) / ch_height;

	int px_start = start ? *start : 0;
	int px_stop  = end   ? MIN(*end, width) : width;
	dbg (3, "px_start=%i px_end=%i", px_start, px_stop);

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
		dbg(2, "hires: offset=%i", src_px_start);
	}

	int n_tiers = hires_mode ? /*peakbuf->n_tiers*/4 : 0; //TODO
	//note n_tiers is 1, not zero in lowres mode. (??!)
	dbg(3, "samples_per_px=%.2f", samples_per_px);
	dbg(3, "n_tiers=%i <<=%i", n_tiers, 1 << n_tiers);

	//xmag defines how many 'samples' we need to skip to get the next pixel.
	//making xmag smaller increases the visual magnification.
	//-the bigger the mag, the less samples we need to skip.
	//-as we're only dealing with smaller peak files, we need to adjust by WF_PEAK_RATIO.
	double xmag = samples_per_px / (WF_PEAK_RATIO);

	double xmag_ = hires_mode ? xmag * (1 << n_tiers) : xmag * 1.0;

	int ch; for(ch=0;ch<n_chans;ch++){
		//we use the same part of Line for each channel, it is then render it to the pixbuf with a channel offset.
		dbg (3, "ch=%i", ch);

		//-----------------------------------------

//#warning review
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
		RmsBuf* rb = w->priv->rms_buf0;
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
		//Line* previous_line = &line[0];
		//Line* current_line  = &line[1]; //the line being displayed. Not the line being written to.
		//Line* next_line     = &line[2];

		int block_offset = hires_mode ? wf_peakbuf_get_max_size(n_tiers) * hires_block / WF_PEAK_VALUES_PER_SAMPLE : 0;
		int block_offset2= hires_mode ? wf_peakbuf_get_max_size(n_tiers) * (hires_block+1) / WF_PEAK_VALUES_PER_SAMPLE : 0;

		int i  = 0;
		int px = 0;                    // pixel count starting at the lhs of the Part.
		for(px=px_start;px<px_stop;px++){
			i++;

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
        dbg(2, "**** block change needed!");
        Peakbuf* peakbuf = waveform_get_peakbuf_n(w, hires_block + 1);
        g_return_if_fail(peakbuf);
        if(!get_rms_buf_info(rb->buf, rb->size, &b, ch)){ break; }//TODO if this is multichannel, we need to go back to previous peakbuf - should probably have 2 peakbufs...
        //src_start = 0;
        block_offset = block_offset2;
        src_start = ((int)((px  ) * xmag_)) + src_inset - block_offset;
        src_stop  = ((int)((px+1) * xmag_)) + src_inset - block_offset;
      }
      dbg(4, "srcidx: %i - %i", src_start, src_stop);

      Line* previous_line = &line[(line_index  ) % 3];
      Line* current_line  = &line[(line_index+1) % 3];
      Line* next_line     = &line[(line_index+2) % 3];

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

        sort_mono(k, lmax, MIN(sub_px, 4));

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
        sort_mono(k, lmin, MIN(sub_px, 4));
        alpha = 0xff;
        v = mid;
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
				WfDRect pts = {px, 0, px, ch_height};
				pixbuf_draw_line(cairo, &pts, 1.0, 0xffff00ff);
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
#endif // USE_GDK_PIXBUF


#if 0
void
waveform_print_blocks(Waveform* w)
{
	g_return_if_fail(w);
#ifdef USE_OPENGL
	g_return_if_fail(!agl_get_instance()->use_shaders);
#endif

	printf("%s {\n", __func__);
	WfGlBlock* blocks = (WfGlBlock*)w->priv->render_data[MODE_MED];
	printf("        L+ L- R+ R-\n");
	printf("  std:\n");
	int b; for(b=0;b<MIN(5, blocks->size);b++){
		printf("    %i: %2i %2i %2i %2i\n", b,
			blocks->peak_texture[WF_LEFT].main[b],
			blocks->peak_texture[WF_LEFT].neg[b], 
			blocks->peak_texture[WF_RIGHT].main ? blocks->peak_texture[WF_RIGHT].main[b] : -1,
			blocks->peak_texture[WF_RIGHT].neg  ? blocks->peak_texture[WF_RIGHT].neg[b]  : -1);
	}
	int c = 0;
	blocks = (WfGlBlock*)w->priv->render_data[MODE_LOW];
	if(blocks){
		printf("  LOW:\n");
		for(b=0;b<5;b++){
			printf("    %i: %2i %2i\n", b, blocks->peak_texture[c].main[b], blocks->peak_texture[c].neg[b]);
		}
	} else printf("  LOW: not allocated\n");
	printf("}\n");
}
#endif


WfTextureHi*
waveform_texture_hi_new()
{
	return g_new0(WfTextureHi, 1);
}


void
waveform_texture_hi_free(WfTextureHi* th)
{
	g_return_if_fail(th);

	//int c; for(c=0;c<WF_MAX_CH;c++) g_free(&th->t[c]); no, these dont need to be free'd.
}


