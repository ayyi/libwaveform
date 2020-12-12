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
#define __waveform_peak_c__
#define __wf_private__
#include "config.h"
#include <fcntl.h>
#include <sys/stat.h>
#ifdef USE_SNDFILE
#include <sndfile.h>
#endif
#include <glib.h>
#include "inttypes.h"
#include "decoder/ad.h"
#include "wf/debug.h"
#include "wf/waveform.h"
#include "wf/loaders/ardour.h"
#include "wf/loaders/riff.h"
#include "wf/audio.h"
#include "wf/peakgen.h"
#include "wf/worker.h"
#include "wf/utils.h"

G_DEFINE_TYPE_WITH_PRIVATE (Waveform, waveform, G_TYPE_OBJECT);

enum  {
	WAVEFORM_DUMMY_PROPERTY,
	WAVEFORM_PROPERTY1
};

#define CHECKS_DONE(W) (w->priv->state & WAVEFORM_CHECKS_DONE)

extern WF* wf;
guint peak_idle = 0;

static void  waveform_finalize      (GObject*);
static void _waveform_get_property  (GObject*, guint property_id, GValue*, GParamSpec*);


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


/**
 *  waveform_new
 *
 *  Returns: waveform
 */
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
			if(wf_debug) pwarn("ignoring request to set same filename");
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
	if(g_hash_table_size(wf->peak_cache) && !g_hash_table_remove(wf->peak_cache, w) && wf_debug) pwarn("failed to remove waveform from peak_cache");

	int c; for(c=0;c<WF_MAX_CH;c++){
		if(_w->peak.buf[c]) g_free(_w->peak.buf[c]);
	}

	if(_w->peaks){
		if(!_w->peaks->is_resolved){
			// allow subscribers to free closure data
			am_promise_fail(_w->peaks, NULL);
		}
		am_promise_unref0(_w->peaks);
	}

	if(_w->hires_peaks){
		void** data = _w->hires_peaks->pdata;
		for(int i=0;i<_w->hires_peaks->len;i++){
			Peakbuf* p = data[i];
			waveform_peakbuf_free(p);
		}
		g_ptr_array_free (_w->hires_peaks, true);
	}

	for(int m=MODE_V_LOW;m<=MODE_HI;m++){
		if(_w->render_data[m]) pwarn("actor data not cleared");
	}

	if(w->free_render_data) w->free_render_data(w);
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
waveform_load (Waveform* w, WfCallback3 callback, gpointer user_data)
{
	WaveformPrivate* _w = w->priv;

	if(g_strrstr(w->filename, "%L")){
		char rhs[256] = {0};
		waveform_get_rhs(w->filename, rhs);
		if(g_file_test(rhs, G_FILE_TEST_EXISTS)){
			w->n_channels = 2;
		}
	}

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


bool
waveform_load_sync(Waveform* w)
{
	g_return_val_if_fail(w, false);

	WaveformPrivate* _w = w->priv;

	if(!_w->peaks){
		_w->peaks = am_promise_new(w);
	}

	char* peakfile = waveform_ensure_peakfile__sync(w);
	if(peakfile){
		bool loaded = waveform_load_peak(w, peakfile, 0);
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
			if(wf_debug) pwarn("file open failure. no such file: %s", w->filename);
		}else{
#ifdef USE_SNDFILE
			if(wf_debug) g_warning("file open failure (%s) \"%s\"\n", sf_strerror(NULL), w->filename);
#endif

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

			int diff0 = w->n_frames - _w->num_peaks * WF_PEAK_RATIO;
			int diff = diff0 / WF_PEAK_RATIO + (diff0 % WF_PEAK_RATIO ? 1 : 0);
			pwarn("peakfile is too short. maybe corrupted. len=%i expected=%"PRIi64" (short by %i) '%s'", _w->num_peaks, w->n_frames / WF_PEAK_RATIO + (diff0 % WF_PEAK_RATIO ? 1 : 0), diff, peakfile);

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


/*
 *  libwaveform can only handle mono or stereo files,
 *  so this will never return > 2 even if the file is
 *  multichannel.
 */
int
waveform_get_n_channels (Waveform* w)
{
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
			if(a != b){
				pwarn("got %"PRIi64" peaks, expected %"PRIi64, a, b);
				printf("\tn_frames=%i\n", (int)w->n_frames);
				printf("\tn_frames/WF_PEAK_RATIO=%i\n", (int)(w->n_frames / WF_PEAK_RATIO));
				printf("\tn_channels=%i\n", (int)w->n_channels);
				printf("\tpeak_file=%s\n", peak_file);
				printf("\texpected_peakfile_size=%i bytes\n", (int)((((int)w->n_frames) / WF_PEAK_RATIO) * w->n_channels * WF_PEAK_VALUES_PER_SAMPLE * sizeof(short)));
			}
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
		perr("file open");
		return 3;
	}
	write(fd, starting_string, strlen(starting_string));
	//get size of file
	if (fstat(fd, &buf) < 0) {
		perr("fstat error");
		return 4;
	}
	//create a buffer mapped to the file:
	if ((mmap_file = (char*) mmap(0, (size_t) buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == (void*)-1) {
		perr("mmap failure.");
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
									pwarn("bad channel number: %i", ch_num);
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
	if(!(fp = open(rms_file, O_RDONLY))){ pwarn ("file open failure."); goto out; }

	struct stat sinfo;
	if(stat(rms_file, &sinfo)){ pwarn ("rms file stat error. '%s'", rms_file); close(fp); goto out; }
	off_t fsize = sinfo.st_size;
	rb = WF_NEW(RmsBuf,
		.size = fsize,
		.buf = g_new(char, fsize)
	);

	//read the whole peak file into memory:
	if(read(fp, rb->buf, fsize) != fsize) perr ("read error. couldnt read %"PRIi64" bytes from %s", fsize, rms_file);

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
	g_return_val_if_fail(block_num < peaks->len, NULL);

	Peakbuf* peakbuf = g_ptr_array_index(peaks, block_num);

	dbg(2, "block_num=%i peaks->len=%i", block_num, peaks->len);

	return peakbuf;
}


void
waveform_peakbuf_assign (Waveform* w, int block_num, Peakbuf* peakbuf)
{
	g_return_if_fail(peakbuf);
	g_return_if_fail(block_num >= 0);
	g_return_if_fail(block_num < WF_MAX_AUDIO_BLOCKS);

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


/*
 *  Given a filename containing "%L", put the corresponding RHS filename into arg 2
 */
void
waveform_get_rhs (const char* left, char* rhs)
{
	g_strlcpy(rhs, left, 256);
	char* pos = g_strrstr(rhs, "%L") + 1;
	*pos = 'R';
}


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
