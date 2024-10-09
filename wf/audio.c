/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2022 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+

  Borders:

  Audio blocks overlap so that there is a 1:1 block relationship all
  the way through the rendering chain.
  But note that the block starts are NOT offset, so when rendered they
  need to be delayed by the border size.

*/
#define __waveform_peak_c__
#include "config.h"
#include <glib.h>
#define __wf_private__
#include "decoder/ad.h"
#include "debug/debug.h"
#include "wf/waveform.h"
#define __wf_worker_private__
#include "wf/worker.h"
#include "wf/audio.h"

typedef struct {
	int              block_num;
	int              min_output_tiers;
	struct {
	    WfBuf16*     buf16;
	    Peakbuf*     peakbuf;
	}                out;
	WfAudioCallback  done;
	gpointer         user_data;
} PeakbufQueueItem;

#define MAX_AUDIO_CACHE_SIZE (1 << 23) // words, NOT bytes.

static void        audio_cache_insert (Waveform*, WfBuf16*, int);
static void        audio_cache_free   (Waveform*, int block);
#if 0
static void        audio_cache_print  ();
#endif

static WF* wf = NULL;


void
waveform_audio_free (Waveform* waveform)
{
	PF;
	g_return_if_fail(waveform);

	WfAudioData* audio = &waveform->priv->audio;
	if(audio && audio->buf16){
		int b; for(b=0;b<audio->n_blocks;b++){
			WfBuf16* buf16 = audio->buf16[b];
			if(buf16){
				audio_cache_free(waveform, b);
			}
		}
		g_free0(audio->buf16);
	}
}


/*
 *  Load a single audio block for the case where the audio is on a local filesystem.
 *  For thread-safety, the Waveform is not modified.
 *  Usually called by a worker. Not intended to be used directly.
 */
static bool
waveform_load_audio_block (Waveform* waveform, WfBuf16* buf16, int block_num)
{
	g_return_val_if_fail(waveform, false);
	g_return_val_if_fail(buf16 && buf16->buf[WF_LEFT], false);

	uint64_t start_pos = block_num * (WF_PEAK_BLOCK_SIZE - 2.0 * TEX_BORDER * 256.0);

	int n_chans = waveform_get_n_channels(waveform);
	g_return_val_if_fail(n_chans, false);

	// TODO hold this open for subsequent blocks so we dont have to seek
	WfDecoder f = {{0,}};

	if(!ad_open(&f, waveform->filename)){
		pwarn ("not able to open input file %s.", waveform->filename);
		return false;
	}

	if(ad_seek(&f, start_pos) < 0){
		ad_close(&f);
		return false;
	}

#ifdef WF_DEBUG
	buf16->start_frame = start_pos;
#endif

#if 0
	uint64_t end_pos = MIN(start_pos + WF_PEAK_BLOCK_SIZE, waveform->n_frames - 1);

	int64_t n_frames = end_pos - start_pos;

	bool ff_read_short(FF* f, WfBuf16* buf, int ch, sf_count_t n_frames)
	{
		int64_t readcount;
		if((readcount = f->read(f, buf, n_frames)) < n_frames){
			pwarn("unexpected EOF: %s", waveform->filename);
			pwarn("                start_frame=%"PRIi64" expected=%"PRIi64" got=%"PRIi64, start_pos, n_frames, readcount);
			return false;
		}

		return true;
	}
#endif

#if 0
	bool ff_read_float_to_short(FF* f, WfBuf16* buf, int ch, sf_count_t n_frames)
	{
		float readbuf[buf->size];
		int64_t readcount;
		if((readcount = wf_ff_read(f, readbuf, n_frames)) < n_frames){
			pwarn("unexpected EOF: %s", waveform->filename);
			pwarn("                start_frame=%Li n_frames=%Lu read=%Li", start_pos, n_frames, readcount);
			return false;
		}

		//convert to short
		int j; for(j=0;j<readcount;j++){
			buf->buf[ch][j] = readbuf[j] * (1 << 15);
		}

		return true;
	}

	bool ff_read_float_to_short2(FF* f, short* buf, int64_t n_frames)
	{
		float readbuf[n_frames];
		int64_t readcount;
		if((readcount = wf_ff_read(f, readbuf, n_frames)) < n_frames){
			pwarn("unexpected EOF: %s", waveform->filename);
			pwarn("                start_frame=%Li n_frames=%Lu (%Lu) read=%Li", start_pos, n_frames, start_pos + n_frames, readcount);
			return false;
		}

		//convert to short
		int j; for(j=0;j<readcount;j++){
			buf[j] = readbuf[j] * (1 << 15);
		}

		return true;
	}
#endif

	ad_read_short(&f, buf16);

//#warning FIXME split files
	if(waveform->is_split){
		switch(n_chans){
			case WF_MONO:
				;bool is_float = f.info.bit_depth == 4;//f.codec_context->sample_fmt == AV_SAMPLE_FMT_FLT || f.codec_context->sample_fmt == AV_SAMPLE_FMT_FLTP;
				if(is_float){
					dbg(2, "is_split! file=%s", waveform->filename);
					char rhs[256];
					if(wf_get_filename_for_other_channel(waveform->filename, rhs, 256)){
						dbg(3, "  %s", rhs);

						ad_close(&f);
						if(!ad_open(&f, rhs)){
							pwarn ("not able to open input file %s.", rhs);
							return false;
						}
						ad_seek(&f, start_pos);

#if 0
						ff_read_short(f.d, buf16, WF_RIGHT, n_frames);
#else
						// we want to write to WF_RIGHT here but are overwriting WF_LEFT
						ad_read_short(&f, buf16);
#endif
					}
				}
				break;
			case WF_STEREO:
				break;
		}
	}

	ad_close(&f);

	return true;
}


#ifdef NOT_USED
static gboolean
peakbuf_is_present (Waveform* waveform, int block_num)
{
	Peakbuf* peakbuf = waveform_get_peakbuf_n(waveform, block_num);
	if(!peakbuf){ dbg(2, "no"); return FALSE; }
	dbg(2, "%i: %s", block_num, peakbuf->buf[0] ? "yes" : "no");
	return (gboolean)peakbuf->buf[0];
}
#endif

#define INPUT_RESOLUTION TIERS_TO_RESOLUTION(MAX_TIERS)
#define OUTPUT_RESOLUTION WF_PEAK_RATIO                 // output_resolution goes from 1 meaning full resolution, to 256 for normal lo-res peakbuf
#define IO_RATIO (OUTPUT_RESOLUTION / INPUT_RESOLUTION) // number of input bytes for 1 byte of output


static void
waveform_load_audio_run_job (Waveform* waveform, gpointer _pjob)
{
	// Runs in the worker thread.
	// Does not modify the waveform
	// (though it does insert into the wf->audio.cache hash table)

	PeakbufQueueItem* pjob = _pjob;
	if(!waveform) return;

	pjob->out.buf16 = WF_NEW(WfBuf16,
		.size = WF_PEAK_BLOCK_SIZE
	);
	for(int c=0;c<waveform_get_n_channels(waveform);c++){
		pjob->out.buf16->buf[c] = g_malloc0(sizeof(short) * WF_PEAK_BLOCK_SIZE);
		pjob->out.buf16->stamp = ++wf->audio.access_counter;
	}
	pjob->out.peakbuf = WF_NEW(Peakbuf, .block_num = pjob->block_num);
	pjob->out.peakbuf->size = pjob->out.buf16->size * WF_PEAK_VALUES_PER_SAMPLE / IO_RATIO;

	dbg(1, "block=%i tot_audio_mem=%ukB", pjob->block_num, wf->audio.mem_size / 1024);

	if(waveform_load_audio_block(waveform, pjob->out.buf16, pjob->block_num)){

		waveform_peakbuf_regen(waveform, pjob->out.buf16, pjob->out.peakbuf, pjob->block_num, pjob->min_output_tiers);
	}
}


static void
waveform_load_audio_post (Waveform* waveform, GError* error, gpointer _pjob)
{
	// Runs in the main thread

	PeakbufQueueItem* pjob = _pjob;

	if (waveform) {
		audio_cache_insert(waveform, pjob->out.buf16, pjob->block_num);

		WfAudioData* audio = &waveform->priv->audio;
		GPtrArray* peaks = waveform->priv->hires_peaks;
		if (audio->buf16[pjob->block_num]) {
			// this is unexpected. If the data is obsolete, it should probably be cleared imediately.
			pwarn("overwriting old audio buffer");
			audio_cache_free(waveform, pjob->block_num);
		}
		audio->buf16[pjob->block_num] = pjob->out.buf16;

		g_assert(!peaks->pdata || pjob->block_num >= peaks->len || !peaks->pdata[pjob->block_num]);
		waveform_peakbuf_assign(waveform, pjob->block_num, pjob->out.peakbuf);

		if (pjob->done) pjob->done(waveform, pjob->block_num, pjob->user_data);
		dbg(2, "--->");
		g_signal_emit_by_name(waveform, "hires-ready", pjob->block_num);

	} else {
		if (pjob->out.buf16) {
			for (int c=0;c<2;c++) {
				if (pjob->out.buf16->buf[c])
					wf_free0(pjob->out.buf16->buf[c]);
			}
			wf_free0(pjob->out.buf16);
		}
	}
}


/*
 * Load part of the audio into a ram buffer.
 * -a signal will be emitted once the load is complete.
 *
 * If the audio is already loaded, the callback is called imediately
 * and its cache timestamp will be updated to prevent it being purged.
 *
 * A hi resolution buffer of peak data is also built. The resolution of
 * the peak buffer is defined by @n_tiers_needed. Use a value of 3 to
 * obtain a peakbuf of resolution 16:1.
 *
 * warning: a vary large file cannot be loaded into ram all at once.
 * if the audio file is too big for the cache, then multiple parallel calls may fail.
 * -requests should be done sequentially to avoid this so that processing can be
 *  completed before it is purged.
 *
 */
void
waveform_load_audio (Waveform* waveform, int block_num, int n_tiers_needed, WfAudioCallback done, gpointer user_data)
{
	// if the file is local we access it directly, otherwise send a msg.
	// -for now, we assume the file is local.

	PF2;
	g_return_if_fail(block_num < waveform_get_n_audio_blocks(waveform));
	WfAudioData* audio = &waveform->priv->audio;
	wf = wf_get_instance();

	if(audio->buf16){
		WfBuf16* buf = audio->buf16[block_num];
		if(buf){
			buf->stamp = ++wf->audio.access_counter;
			if(done) done(waveform, block_num, user_data);
			return;
		}
	}

	audio->n_tiers_present = MAX_TIERS;

	if(!wf->audio_worker.msg_queue) wf_worker_init(&wf->audio_worker);

	bool is_queued(Waveform* waveform, int block_num)
	{
		GList* l = wf->audio_worker.jobs;
		for(;l;l=l->next){
			QueueItem* i = l->data;
			if(i->cancelled) continue;
			PeakbufQueueItem* item = i->user_data;
			Waveform* w = g_weak_ref_get(&i->ref);
			if(w){
				if(w == waveform && item->block_num == block_num){
					dbg(2, "already queued");
					// it is possible to get here while zooming in/out fast
					// or when there are lots of views of the same waveform
					g_object_unref(w);
					return true;
				}
				g_object_unref(w);
			}
		}
		return false;
	}

	void wf_peakbuf_queue_for_regen (Waveform* waveform, int block_num, int min_output_tiers, WfAudioCallback done, gpointer user_data)
	{
		dbg(1, "%i", block_num);
		g_return_if_fail(block_num >= 0);

		if(is_queued(waveform, block_num)) return;

		WfAudioData* audio = &waveform->priv->audio;
		if(!audio->buf16) audio->buf16 = g_malloc0(sizeof(void*) * waveform_get_n_audio_blocks(waveform));

		GPtrArray* peaks = waveform->priv->hires_peaks;
		if(peaks->pdata && block_num < peaks->len && peaks->pdata[block_num])
			peaks->pdata[block_num] = NULL; // disconnect until job finished

		wf_worker_push_job(
			&wf->audio_worker,
			waveform,
			waveform_load_audio_run_job,
			waveform_load_audio_post,
			wf_free,
			WF_NEW(PeakbufQueueItem,
				.done = done,
				.user_data = user_data,
				.block_num = block_num,
				.min_output_tiers = min_output_tiers
			)
		);
	}

	// if(!peakbuf_is_present(waveform, block_num))        -- if v_hi_mode, the peakbuf is not enough, we need the actual audio.
		wf_peakbuf_queue_for_regen(waveform, block_num, n_tiers_needed, done, user_data);
}


void
waveform_load_audio_sync (Waveform* waveform, int block_num, int n_tiers_needed)
{
	PF2;
	g_return_if_fail(block_num < waveform_get_n_audio_blocks(waveform));
	WfAudioData* audio = &waveform->priv->audio;
	wf = wf_get_instance();

	if(audio->buf16){
		WfBuf16* buf = audio->buf16[block_num];
		if(buf){
			buf->stamp = ++wf->audio.access_counter;
			return;
		}
	}

	audio->n_tiers_present = MAX_TIERS;

	dbg(1, "%i", block_num);

	if(!audio->buf16) audio->buf16 = g_malloc0(sizeof(void*) * waveform_get_n_audio_blocks(waveform));

	PeakbufQueueItem* item = WF_NEW(PeakbufQueueItem,
		.block_num = block_num,
		.min_output_tiers = n_tiers_needed
	);

	GPtrArray* peaks = waveform->priv->hires_peaks;
	if(peaks->pdata && peaks->pdata[block_num]) peaks->pdata[block_num] = NULL; // disconnect until job finished

	waveform_load_audio_run_job(waveform, item);

	waveform_load_audio_post(waveform, NULL, item);

	wf_free(item);
}


static int
wf_block_lookup_by_audio_buf (Waveform* w, WfBuf16* buf)
{
	g_return_val_if_fail(w && buf, -1);

	for (int b=0;b<waveform_get_n_audio_blocks(w);b++) {
		if (w->priv->audio.buf16[b] == buf)
			return b;
	}
	return -1;
}


static void
audio_cache_insert (Waveform* w, WfBuf16* buf16, int b)
{
	if (wf->audio.mem_size + buf16->size > MAX_AUDIO_CACHE_SIZE) {
		dbg(2, "cache full. looking for audio block to delete...");
		//what to delete?
		// - audio cache items can be in use, but only if we are at high zoom.
		//   But as there are many views, none of which we have knowledge of, we can't use this information.
		// Therefore we base decisions on 'least recently used'.
		WfBuf16* oldest = NULL;
		Waveform* oldest_waveform = NULL;
		GHashTableIter iter;
		gpointer key, value;
		g_hash_table_iter_init (&iter, wf->audio.cache);
		while (g_hash_table_iter_next (&iter, &key, &value)) { //need to check the stamp of each block and associate it with the waveform
			WfBuf16* buf = key;
			Waveform* w = value;
			if (!oldest || buf->stamp < oldest->stamp) {
				oldest = buf;
				oldest_waveform = w;
			}
		}
		if (oldest) {
			dbg(2, "*** cache full: clearing buf with stamp=%i ...", oldest->stamp);
			audio_cache_free(oldest_waveform, wf_block_lookup_by_audio_buf(oldest_waveform, oldest));
		}
		if (wf->audio.mem_size + buf16->size > MAX_AUDIO_CACHE_SIZE) {
			perr("cant free space in audio cache");
		}
	}

	wf->audio.mem_size += buf16->size * (buf16->buf[WF_RIGHT] ? 2 : 1);
	g_hash_table_insert(wf->audio.cache, buf16, w); //each channel has its own entry. however as channels are always accessed together, it might be better to have one entry per Buf16*
}


static void
audio_cache_free (Waveform* w, int block)
{
	// this now frees both the audio data, and the structs that contain it.

	WfAudioData* audio = &w->priv->audio;
	WfBuf16* buf16 = audio->buf16[block];
	if (buf16) {
		// the cache may already have been cleared if the cache is full
		if (!g_hash_table_remove(wf->audio.cache, buf16)) dbg(2, "%i: failed to remove waveform block from audio_cache", block);
		if (buf16->buf[WF_LEFT]) {
			wf->audio.mem_size -= buf16->size;
			wf_free0(buf16->buf[WF_LEFT]);
		}
		else { dbg(2, "%i: left buffer empty", block); }

		if (buf16->buf[WF_RIGHT]) {
			wf->audio.mem_size -= buf16->size;
			dbg(2, "b=%i clearing right...", block);
			wf_free0(buf16->buf[WF_RIGHT]);
		}
		wf_free0(audio->buf16[block]);
	}
	//audio_cache_print();
}


int
wf_audio_cache_get_size ()
{
	// return the number of blocks that can be held by the cache.

	return MAX_AUDIO_CACHE_SIZE / WF_PEAK_BLOCK_SIZE;
}


#if UNUSED
static void
audio_cache_print ()
{
	#define STRLEN 64 // this is not long enough to show the whole cache.
	char str[STRLEN];
	memset(str, ' ', STRLEN - 1);
	str[STRLEN - 1] = '\0';

	int total_size = 0;
	int total_mem = 0;
	GHashTableIter iter;
	gpointer key, value;
	int i = 0;
	g_hash_table_iter_init (&iter, wf->audio.cache);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		Waveform* w = value;
		WfBuf16* buf = key;
		g_return_if_fail(w && buf);
		for(int c=0;c<WF_STEREO;c++){
			if (buf->buf[c]) total_mem += WF_PEAK_BLOCK_SIZE;
		}
		if (i < STRLEN / 2 - 1) {
			if (buf->buf[WF_LEFT])  str[2*i    ] = 'L';
			if (buf->buf[WF_RIGHT]) str[2*i + 1] = 'R';
		}

		total_size++;
		i++;
	}
	dbg(1, "size=%i mem=%ikB=%ikB %s", total_size, total_mem * sizeof(short) / 1024, wf->audio.mem_size * sizeof(short) / 1024, str);
}
#endif


