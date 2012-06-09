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
#include <glib.h>
#include "waveform/peak.h"
#include "waveform/utils.h"
#include "waveform/loaders/ardour.h"
#include "waveform/loaders/riff.h"
#include "waveform/texture_cache.h"
#include "waveform/audio.h"
#include "waveform/alphabuf.h"
#include "waveform/fbo.h"
#include "waveform/peakgen.h"

static gpointer waveform_parent_class = NULL;
#define WAVEFORM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_WAVEFORM, WaveformPriv))
enum  {
	WAVEFORM_DUMMY_PROPERTY,
	WAVEFORM_PROPERTY1
};

extern WF* wf;
guint peak_idle = 0;

static void  waveform_finalize          (GObject*);
static void _waveform_get_property      (GObject*, guint property_id, GValue*, GParamSpec*);

static void  waveform_set_last_fraction (Waveform*);

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


Waveform*
waveform_construct (GType object_type)
{
	Waveform* self = (Waveform*) g_object_new (object_type, NULL);
	return self;
}


Waveform*
waveform_new (const char* filename)
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
__finalize (Waveform* w)
{
	PF0;
	if(g_hash_table_size(wf->peak_cache) && !g_hash_table_remove(wf->peak_cache, w)) gwarn("failed to remove waveform from peak_cache");

	int i; for(i=0;i<WF_MAX_CH;i++){
		if(w->priv->peak.buf[i]) g_free(w->priv->peak.buf[i]);
	}
#if 0
	if(w->textures){
		texture_cache_remove(w);

		int c; for(c=0;c<WF_MAX_CH;c++){
			if(w->textures->peak_texture[c].main) g_free(w->textures->peak_texture[c].main);
			if(w->textures->peak_texture[c].neg) g_free(w->textures->peak_texture[c].neg);
		}
		g_free(w->textures);
	}
#else
	if(w->textures) texture_cache_remove_waveform(w); //TODO this doesnt yet clear lo-res textures.

	void free_textures(WfGlBlock** _textures)
	{
		WfGlBlock* textures = *_textures;
		if(textures){
			int c; for(c=0;c<WF_MAX_CH;c++){
				if(textures->peak_texture[c].main) g_free(textures->peak_texture[c].main);
				if(textures->peak_texture[c].neg) g_free(textures->peak_texture[c].neg);
			}
#ifdef USE_FBO
			if(textures->fbo){
																						dbg(0, "textures=%p textures->fbo=%p", textures, textures->fbo);
				int b; for(b=0;b<textures->size;b++) if(textures->fbo[b]) fbo_free(textures->fbo[b]);
				g_free(textures->fbo);
			}
#endif
			g_free(textures);
			*_textures = NULL;
		}
	}
	free_textures(&w->textures);
	free_textures(&w->textures_lo);

	void free_textures_hi(Waveform* w)
	{
		if(!w->textures_hi) return;

		GHashTableIter iter;
		gpointer key, value;
		g_hash_table_iter_init (&iter, w->textures_hi->textures);
		while (g_hash_table_iter_next (&iter, &key, &value)){
			//int block = key;
			WfTextureHi* texture = value;
			waveform_texture_hi_free(texture);
		}

		g_hash_table_destroy(w->textures_hi->textures);
		g_free0(w->textures_hi);
	}
	free_textures_hi(w);
#endif

	waveform_audio_free(w);
	g_free(w->priv);
	g_free(w->filename);
}


static void
waveform_finalize (GObject* obj)
{
	PF;
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


WfGlBlock*
wf_texture_array_new(int size, int n_channels)
{
	WfGlBlock* textures = g_new0(WfGlBlock, 1);
	textures->size = size;
	dbg(1, "creating glbocks... num_blocks=%i", textures->size);
	textures->peak_texture[0].main = g_new0(unsigned, textures->size);
	textures->peak_texture[0].neg = g_new0(unsigned, textures->size);
	if(n_channels > 1){
		textures->peak_texture[WF_RIGHT].main = g_new0(unsigned, textures->size);
		textures->peak_texture[WF_RIGHT].neg = g_new0(unsigned, textures->size);
	}
	textures->fbo = g_new0(WfFBO*, textures->size); //note: only an array of _pointers_
	return textures;
}


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
	//sfinfo.channels = 0;
memset(&sfinfo, 0, sizeof(SF_INFO));
	if(!(sndfile = sf_open(w->filename, SFM_READ, &sfinfo))){
		if(!g_file_test(w->filename, G_FILE_TEST_EXISTS)){
			gwarn("file open failure. no such file: %s", w->filename);
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
	dbg(1, "ch=%i num_peaks=%i", ch_num, w->num_peaks);
#if 0
	int i; for (i=0;i<20;i++) printf("      %i %i\n", w->priv->peak.buf[0][2 * i], w->priv->peak.buf[0][2 * i + 1]);
#endif
	if(!w->textures){
		w->textures = wf_texture_array_new(w->num_peaks / WF_PEAK_TEXTURE_SIZE + ((w->num_peaks % WF_PEAK_TEXTURE_SIZE) ? 1 : 0), (ch_num == 1 || n_channels == 2) ? 2 : 1);
#ifdef WF_SHOW_RMS
		gerr("rms TODO");
		w->textures->rms_texture = g_new0(unsigned, w->textures->size);
#warning TODO where best to init rms textures? see peak_textures
		extern void glGenTextures(size_t n, uint32_t* textures);
		glGenTextures(w->textures->size, w->textures->rms_texture); //note currently doesnt use the texture_cache
#endif
		w->textures_hi = g_new0(WfTexturesHi, 1);
		w->textures_hi->textures = g_hash_table_new(g_int_hash, g_int_equal);
	}

	waveform_set_last_fraction(w);

	return !!w->priv->peak.buf[ch_num];
}


gboolean
waveform_peak_is_loaded(Waveform* w, int ch_num)
{
	return !!w->priv->peak.buf[ch_num];
}


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

struct _line{
	guchar a[MAX_PART_HEIGHT]; //alpha level for each pixel in the line.
};
static struct _line line[3];


static void
line_write(struct _line* line, int index, guchar val)
{
	//remove fn once debugging complete?
	// __func__ is null!
	if(index < 0 || index >= MAX_PART_HEIGHT){ gerr ("y=%i", index); return; }
	line->a[index] = val;
}


static void
line_clear(struct _line* line)
{
	int i;
	for(i=0;i<MAX_PART_HEIGHT;i++) line->a[i] = 0;
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
		if(audio->buf16){
			b->buf = audio->buf16[block_num]->buf[ch];
			g_return_val_if_fail(b->buf, false);
			b->len = audio->buf16[block_num]->size;
			//dbg(2, "HI block=%i len=%i %i", block_num, b->len, b->len / WF_PEAK_VALUES_PER_SAMPLE);
		}else{
			gerr("using hires mode but audio->buf not allocated");
		}
	}else{
		dbg(2, "STD len=%i %i (x256=%i)", b->len, b->len / WF_PEAK_VALUES_PER_SAMPLE, (b->len * 256) / WF_PEAK_VALUES_PER_SAMPLE);
	}
	b->len_frames = b->len / WF_PEAK_VALUES_PER_SAMPLE;

	if(!b->buf) return false;
	if(b->len > 10000000){ gerr ("buflen too big"); return false; }

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


static void
waveform_set_last_fraction(Waveform* waveform)
{
	int width = WF_PEAK_TEXTURE_SIZE;
	int width_ = waveform->num_peaks % WF_PEAK_TEXTURE_SIZE;
	waveform->textures->last_fraction = (double)width_ / (double)width;
	//dbg(1, "num_peaks=%i last_fraction=%.2f", waveform->num_peaks, waveform->textures->last_fraction);
}


short*
waveform_peak_malloc(Waveform* w, uint32_t bytes)
{
	short* buf = g_malloc(bytes);
	wf->peak_mem_size += bytes;
	g_hash_table_insert(wf->peak_cache, w, w); //is removed in __finalize()

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
alphabuf_draw_line(AlphaBuf* pixbuf, DRect* pts, double line_width, GdkColor* colour)
{
}


void
waveform_peak_to_alphabuf(Waveform* w, AlphaBuf* a, int scale, int* start, int* end, GdkColor* colour, uint32_t colour_bg, int border)
{
	/*
	 renders a peakfile (loaded into the buffer given by waveform->buf) onto the given 8 bit alpha-map buffer.

	 -the given buffer is for a single block only.

	 @param start - if set, we start rendering at this value in the source buffer.
	 @param end   - if set, source buffer index to stop rendering at.
	                Is allowed to be bigger than the output buf width.
	 @border      - nothing will be drawn within this number of pixels from the alphabuf left and right edges.
TODO wrong - should be filled with data from adjacent blocks
	                To facilitate drawing the border, the Alphabuf must be 2xborder wider than the requested draw range.
	                TODO ensure that the data to fill the RHS border is loaded (only applies to hi-res mode)

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

//------------------------------
	//if start if not specified, we start at 0.
	//       -however in this case, the output is N pixels to the right?
	//        ie the source is N earlier?
	//if start _is_ specified, we start at s or s-4 ? ***
					int src_offset = start ? 0 : -TEX_BORDER;
	int px_start = start ? *start : 0;
	int px_stop  = *end;//   ? MIN(*end, width) : width;
//------------------------------
	dbg (1, "start=%i end=%i", px_start, px_stop);
	//dbg (1, "width=%i height=%i", width, height);

	if(width < px_stop - px_start){ gwarn("alphabuf too small? %i < %i", width, px_stop - px_start); return; }

	int ch; for(ch=0;ch<n_chans;ch++){

		struct _buf_info b; 
		g_return_if_fail(get_buf_info(w, -1, &b, ch));

		line_clear(&line[0]);
		int line_index = 0;

		for(px=px_start;px<px_stop;px++){
			src_start = ((int)( px   * xmag)) + src_offset;
			src_stop  = ((int)((px+1)* xmag));
			//printf("%i ", src_start);
			if(src_start < 0){ dbg(0, "skipping..."); continue; }

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
				min /= vscale;
				max /= vscale;

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
waveform_rms_to_alphabuf(Waveform* pool_item, AlphaBuf* pixbuf, int* start, int* end, double samples_per_px, GdkColor* colour, uint32_t colour_bg)
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

#if 0
	struct timeval time_start;
	gettimeofday(&time_start, NULL);
#endif

	if(samples_per_px < 0.001) gerr ("samples_per_pix=%f", samples_per_px);
	WfPeakSample sample;
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
waveform_get_peakbuf_n(Waveform* w, int block_num)
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
pixbuf_draw_line(cairo_t* cr, DRect* pts, double line_width, uint32_t colour)
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


#ifdef USE_GDK_PIXBUF
void
waveform_peak_to_pixbuf(Waveform* w, GdkPixbuf* pixbuf, WfSampleRegion* region, uint32_t colour, uint32_t bg_colour)
{
	g_return_if_fail(w && pixbuf && region);

	gdk_pixbuf_fill(pixbuf, bg_colour);
	double samples_per_px = region->len / gdk_pixbuf_get_width(pixbuf);
	waveform_peak_to_pixbuf_full(w, pixbuf, region->start, NULL, NULL, samples_per_px, colour, bg_colour, 1.0);
}


void
waveform_peak_to_pixbuf_async(Waveform* w, GdkPixbuf* pixbuf, WfSampleRegion* region, uint32_t colour, uint32_t bg_colour, WfPixbufCallback callback, gpointer user_data)
{
	g_return_if_fail(w && pixbuf && region);

					g_return_if_fail(region->start + region->len <= waveform_get_n_frames(w));
					g_return_if_fail(region->len > 0);

	typedef struct {
		Waveform*         waveform;
		WfSampleRegion    region;
		uint32_t          colour;
		uint32_t          bg_colour;
		GdkPixbuf*        pixbuf;
		WfPixbufCallback* callback;
		gpointer          user_data;
		guint             ready_handler;
		int               n_blocks_done;
	} C;
	C* c = g_new0(C, 1);
	c->waveform  = w;
	c->region    = *region;
	c->colour    = colour;
	c->bg_colour = bg_colour;
	c->pixbuf    = pixbuf;
	c->callback  = callback;
	c->user_data = user_data;

	void _waveform_peak_to_pixbuf__done(C* c)
	{
		PF;
		double samples_per_px = c->region.len / gdk_pixbuf_get_width(c->pixbuf);
		gdk_pixbuf_fill(c->pixbuf, c->bg_colour);
		waveform_peak_to_pixbuf_full(c->waveform, c->pixbuf, c->region.start, NULL, NULL, samples_per_px, c->colour, c->bg_colour, 1.0);

		if(c->callback) c->callback(c->waveform, c->pixbuf, c->user_data);
		g_free(c);
	}

	void _on_peakdata_ready(Waveform* w, int block, gpointer _c)
	{
		dbg(3, "block=%i", block);
		C* c = _c;
		g_return_if_fail(c);

		c->n_blocks_done++;
//TODO no, we cannot load the whole file at once!! render each block separately
		if(c->n_blocks_done >= waveform_get_n_audio_blocks(c->waveform)){
			g_signal_handler_disconnect((gpointer)c->waveform, c->ready_handler);
			_waveform_peak_to_pixbuf__done(c);
		}
	}

	double samples_per_px = region->len / gdk_pixbuf_get_width(pixbuf);
	gboolean hires_mode = ((samples_per_px / WF_PEAK_RATIO) < 1.0);
	if(hires_mode){
		WfAudioData* audio = w->priv->audio_data;
		if(!audio->buf16){
			c->ready_handler = g_signal_connect (w, "peakdata-ready", (GCallback)_on_peakdata_ready, c);
			int n_tiers_needed = 3; //TODO
			int b; for(b=0;b<waveform_get_n_audio_blocks(w);b++){
				WfBuf16* buf = waveform_load_audio_async(w, b, n_tiers_needed);
				if(buf) _on_peakdata_ready(w, b, c);
			}
			return;
		}
	}

	_waveform_peak_to_pixbuf__done(c);
}


void
waveform_peak_to_pixbuf_full(Waveform* waveform, GdkPixbuf* pixbuf, uint32_t region_inset, int* start, int* end, double samples_per_px, uint32_t colour, uint32_t colour_bg, float gain)
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
		@param samples_per_px - sets the magnification.
		@param colour_bg      - 0xrrggbbaa. used for antialiasing.

		TODO
			further optimisation:
				-dont scan pixels above the peaks. Should we record the overall region/file peak level?

			API:
				- why region_inset but not region end?
				- add WaveformPixbuf object?
	*/

	g_return_if_fail(pixbuf);
	g_return_if_fail(waveform);
	dbg(2, "inset=%i", region_inset);

	gboolean hires_mode = ((samples_per_px / WF_PEAK_RATIO) < 1.0);

	int bg_red = (colour_bg & 0xff000000) >> 24;
	int bg_grn = (colour_bg & 0x00ff0000) >> 16;
	int bg_blu = (colour_bg & 0x0000ff00) >>  8;
	int fg_red = (colour    & 0xff000000) >> 24;
	int fg_grn = (colour    & 0x00ff0000) >> 16;
	int fg_blu = (colour    & 0x0000ff00) >>  8;
	//printf("%s(): bg=%x r=%x g=%x b=%x \n", __func__, colour_bg, bg_red, bg_grn bg_blu);

#if 0
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
#endif

	if(samples_per_px < 0.001) gerr ("samples_per_pix=%f", samples_per_px);
	WfPeakSample sample;
	short min;                //negative peak value for each pixel.
	short max;                //positive peak value for each pixel.
#if 0
	if(!pool_item->valid) return;
	if(!pool_item->source_id){ gerr ("bad core source id: %Lu.", pool_item->source_id[0]); return; }
#endif

	dbg(2, "peak_gain=%.2f", gain);

	int n_chans      = waveform_get_n_channels(waveform);

	int width        = gdk_pixbuf_get_width(pixbuf);
	int height       = gdk_pixbuf_get_height(pixbuf);
	guchar* pixels   = gdk_pixbuf_get_pixels(pixbuf);
	int rowstride    = gdk_pixbuf_get_rowstride(pixbuf);

	//TODO
	cairo_surface_t* surface = cairo_image_surface_create_for_data(pixels, CAIRO_FORMAT_RGB24, width, height, rowstride);
	cairo_t* cairo = cairo_create(surface);

	if (height > MAX_PART_HEIGHT) gerr ("part too tall. not enough memory allocated.");
	int ch_height = height / n_chans;
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

		WfAudioData* audio = waveform->priv->audio_data;
		if(!audio->buf16){
			int n_tiers_needed = 3; //TODO
			waveform_load_audio_async(waveform, hires_block, n_tiers_needed);
			return;
		}
	}

	//xmag defines how many 'samples' we need to skip to get the next pixel.
	//making xmag smaller increases the visual magnification.
	//-the bigger the mag, the less samples we need to skip.
	//-as we're only dealing with smaller peak files, we need to adjust by WF_PEAK_RATIO.
	double xmag = samples_per_px / (WF_PEAK_RATIO * WF_PEAK_VALUES_PER_SAMPLE);

	int n_tiers = hires_mode ? /*peakbuf->n_tiers*/4 : 0; //TODO
	//note n_tiers is 1, not zero in lowres mode. (??!)
	dbg(1, "samples_per_px=%.2f", samples_per_px);
	dbg(2, "n_tiers=%i <<=%i", n_tiers, 1 << n_tiers);

	int ch; for(ch=0;ch<n_chans;ch++){
		//we use the same part of Line for each channel, it is then render it to the pixbuf with a channel offset.
		dbg (2, "ch=%i", ch);

		struct _buf_info b; 
		g_return_if_fail(get_buf_info(waveform, hires_block, &b, ch));

		int src_start=0;             // frames or peakbuf idx. multiply by 2 to get the index into the source buffer for each sample pt.
		int src_stop =0;   

		line_clear(&line[0]);
		int line_index = 0;
		//struct _line* previous_line = &line[0];
		//struct _line* current_line  = &line[1]; //the line being displayed. Not the line being written to.
		//struct _line* next_line     = &line[2];

		int block_offset = hires_mode ? wf_peakbuf_get_max_size(n_tiers) * (hires_block  ) / WF_PEAK_VALUES_PER_SAMPLE : 0;
		int block_offset2= hires_mode ? wf_peakbuf_get_max_size(n_tiers) * (hires_block+1) / WF_PEAK_VALUES_PER_SAMPLE : 0;

		uint32_t region_inset_ = hires_mode ? region_inset : region_inset / WF_PEAK_RATIO;

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
			src_start = ((int)((px  ) * xmag_)) + region_inset_ - block_offset;
			src_stop  = ((int)((px+1) * xmag_)) + region_inset_ - block_offset;
			if(src_start == src_stop){ printf("^"); fflush(stdout); }
			if((px==px_start) && hires_mode){
				double percent = 2 * 100 * (((int)(px * xmag_)) + region_inset - (wf_peakbuf_get_max_size(n_tiers) * hires_block) / WF_PEAK_VALUES_PER_SAMPLE) / b.len;
				dbg(2, "reading from buf=%i=%.2f%% stop=%i buflen=%i blocklen=%i", src_start, percent, src_stop, b.len, wf_peakbuf_get_max_size(n_tiers));
			}
			if(src_stop > b.len_frames && hires_mode){
				dbg(2, "**** block change needed!");
				Peakbuf* peakbuf = waveform_get_peakbuf_n(waveform, hires_block + 1);
				g_return_if_fail(peakbuf);
				if(!get_buf_info(waveform, hires_block, &b, ch)){ break; }//TODO if this is multichannel, we need to go back to previous peakbuf - should probably have 2 peakbufs...
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

			int j, y;
//src_stop must not be in audio_frames at STD res!
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

		dbg (1, "done. xmag=%.2f drawn: %i of %i src=%i-->%i", xmag, line_index, width, ((int)(px_start * xmag * n_tiers)) + region_inset, src_stop);
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
        Peakbuf* peakbuf = waveform_get_peakbuf_n(w, hires_block + 1);
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


void
waveform_print_blocks(Waveform* w)
{
	g_return_if_fail(w);

	printf("%s {\n", __func__);
	WfGlBlock* blocks = w->textures;
	int c = 0;
	printf("  std:\n");
	int b; for(b=0;b<5;b++){
		printf("    %i: %2i %2i\n", b, blocks->peak_texture[c].main[b], blocks->peak_texture[c].neg[b]);
	}
	blocks = w->textures_lo;
	if(blocks){
		printf("  LOW:\n");
		for(b=0;b<5;b++){
			printf("    %i: %2i %2i\n", b, blocks->peak_texture[c].main[b], blocks->peak_texture[c].neg[b]);
		}
	} else printf("  LOW: not allocated\n");
	printf("}\n");
}


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


