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
//#include "waveform/peakgen.h"
#include "waveform/audio.h"

typedef struct {
	Waveform*        waveform;
	int              block_num;
	int              min_output_tiers;
}
PeakbufQueueItem;

typedef void   (*WfCallback)    (gpointer user_data);

typedef struct _queue_item
{
	WfCallback       callback;
	void*            user_data;
} QueueItem;

#define MAX_AUDIO_CACHE_SIZE (1 << 23) // words, NOT bytes.

//static void  process_audio         (short*, int count, int channels, long long pos, short* maxplus, short* maxmin);
static short*      audio_cache_malloc (Waveform*, int);
static void        audio_cache_free   (Waveform*, int block);
static void        audio_cache_print  ();
//static float int2db                (short);


void
wf_audio_free(Waveform* waveform)
{
	PF0;
	g_return_if_fail(waveform);

	WfAudioData* audio = waveform->priv->audio_data;
	if(audio){
		int b; for(b=0;b<audio->n_blocks;b++){
#if 0
			WfBuf* buf = audio->buf[b];
			if(buf){
				if(buf->buf[WF_LEFT]) g_free(buf->buf[WF_LEFT]);
				if(buf->buf[WF_RIGHT]) g_free(buf->buf[WF_RIGHT]);
				g_free(buf);
				audio->buf[b] = NULL;
			}
#endif
			WfBuf16* buf16 = audio->buf16[b];
			if(buf16){
				audio_cache_free(waveform, b);
				g_free(buf16);
				audio->buf16[b] = NULL;
			}
		}
#if 0
		g_free(audio->buf);
#endif
		g_free(audio->buf16);
		g_free0(waveform->priv->audio_data);
		//waveform->priv->audio_data = NULL;
	}
}


gboolean
wf_load_audio_block(Waveform* waveform, int block_num)
{
	//load a single audio block for the case where the audio is on a local filesystem.

	//TODO handle split stereo files

	g_return_val_if_fail(waveform, false);
	WfAudioData* audio = waveform->priv->audio_data;
	g_return_val_if_fail(audio->buf16, false);

	SF_INFO sfinfo;
	SNDFILE* sffile;
	sfinfo.format = 0;

	if(!(sffile = sf_open(waveform->filename, SFM_READ, &sfinfo))){
		gwarn ("not able to open input file %s.", waveform->filename);
		puts(sf_strerror(NULL));
		return false;
	}
	gboolean is_float = ((sfinfo.format & SF_FORMAT_SUBMASK) == SF_FORMAT_FLOAT);

	//int n_frames = sfinfo.frames;
	//guint n_peaks = ((n_frames * 1 ) / WF_PEAK_RATIO) << audio->n_tiers_present;

	// which parts of the audio file are present?
	//  tier 1:  0,             128
	//  tier 2:  0,     64,     128,     196
	//  tier 3:  0, 32, 64, 96, 128, ... 196
	//  tier 4:  0, 16, ...
	//  tier 5:  0,  8, ...
	//  tier 6:  0,  4, ...
	//  tier 7:  0,  2, ...
	//  tier 8:  0,  1, ...
	int spacing = WF_PEAK_RATIO >> (audio->n_tiers_present - 1);

	//dbg(1, "tot_frames=%i n_blocks=%i", sfinfo.frames, waveform_get_n_audio_blocks(waveform));
	uint64_t start_pos =  block_num      * WF_PEAK_BLOCK_SIZE;
	uint64_t end_pos   = (block_num + 1) * WF_PEAK_BLOCK_SIZE;
	if(start_pos > sfinfo.frames){ gerr("startpos too too high. %Li > %Li block=%i", start_pos, sfinfo.frames, block_num); return false; }
	if(end_pos > sfinfo.frames){ dbg(1, "*** last block?"); end_pos = sfinfo.frames; }
	sf_seek(sffile, start_pos, SEEK_SET);
	dbg(1, "block=%i/%i tiers_present=%i spacing=%i start=%Li end=%Li", block_num, waveform_get_n_audio_blocks(waveform), audio->n_tiers_present, spacing, start_pos, end_pos);

	int n_chans = waveform_get_n_channels(waveform);
	g_return_val_if_fail(n_chans, false);

	sf_count_t read_len = MIN(audio->buf16[block_num]->size, end_pos - start_pos); //1st of these isnt needed?
#if 0
	WfBuf* buf = audio->buf[block_num];
#endif
	WfBuf16* buf16 = audio->buf16[block_num];
	g_return_val_if_fail(buf16 && buf16->buf[WF_LEFT], false);

	sf_count_t n_frames = read_len;
	sf_count_t readcount;
	switch(sfinfo.channels){
		case WF_MONO:
			if(is_float){
				//FIXME temporary? sndfile is supposed to automatically convert between formats??!

				float readbuf[buf16->size];
				if((readcount = sf_readf_float(sffile, readbuf, n_frames)) < n_frames){
					gwarn("unexpected EOF: %s", waveform->filename);
					gwarn("                start_frame=%Li n_frames=%Lu/%Lu read=%Li", start_pos, n_frames, sfinfo.frames, readcount);
				}

				//convert to short
				int j; for(j=0;j<readcount;j++){
					buf16->buf[WF_LEFT][j] = readbuf[j] * (1 << 15);
				}

			}else{
				if((readcount = sf_readf_short(sffile, buf16->buf[WF_LEFT], n_frames)) < n_frames){
					gwarn("unexpected EOF: %s", waveform->filename);
					gwarn("                start_frame=%Li n_frames=%Lu/%Lu read=%Li", start_pos, n_frames, sfinfo.frames, readcount);
				}
			}
			/*
			int i; for(i=0;i<10;i++){
				printf("  %i\n", buf16->buf[WF_LEFT][i]);
			}
			*/
			break;
		case WF_STEREO:
			{
#if 0
			float read_buf[n_frames * WF_STEREO];

			if((readcount = sf_readf_float(sffile, read_buf, n_frames)) < n_frames){
#else
			short read_buf[n_frames * WF_STEREO];
			if((readcount = sf_readf_short(sffile, read_buf, n_frames)) < n_frames){
#endif
				gwarn("unexpected EOF: %s", waveform->filename);
				gwarn("                STEREO start_frame=%Lu n_frames=%Lu/%Lu read=%Lu", start_pos, n_frames, sfinfo.frames, readcount);
			}

			#if 0 //only useful for testing.
			memset(w->cache->buf->buf[0], 0, WF_CACHE_BUF_SIZE);
			memset(w->cache->buf->buf[1], 0, WF_CACHE_BUF_SIZE);
			#endif

			deinterleave16(read_buf, buf16->buf, n_frames);
			}
			break;
		default:
			break;
	}
	dbg(2, "read %Lu frames", read_len);
	if(sf_error(sffile)) gwarn("read error");
	if(sf_close(sffile)) gwarn ("bad file close.");

	//buffer size is the allocation size. To check if it is full, use w->samplecount
	//buf16->size = readcount; X

	return true;
}


static gboolean
peakbuf_is_present(Waveform* waveform, int block_num)
{
	Peakbuf* peakbuf = wf_get_peakbuf_n(waveform, block_num);
	if(!peakbuf){ dbg(2, "no"); return FALSE; }
	dbg(2, "%i: %s", block_num, peakbuf->buf[0] ? "yes" : "no");
	return (gboolean)peakbuf->buf[0];
}


gpointer
file_load_thread(gpointer data)
{
	//TODO as we use a blocking call on the async queue (g_async_queue_pop) we may no longer need a main loop.

	dbg(2, "new file load thread.");
	WF* wf = wf_get_instance();

	if(!wf->msg_queue){ perr("no msg_queue!\n"); return NULL; }

	g_async_queue_ref(wf->msg_queue);

	gboolean worker_timeout(gpointer data)
	{
		static GList* jobs = NULL;

		WF* wf = wf_get_instance();

		PeakbufQueueItem* peakbuf_queue_find(GList* jobs, PeakbufQueueItem* target)
		{
			GList* l = jobs;//wf_get_instance()->work_queue;
			for(;l;l=l->next){
				PeakbufQueueItem* item = l->data;
				if(item->waveform == target->waveform && item->block_num == target->block_num) return item;
			}
			return NULL;
		}

		gboolean do_callback(gpointer _item)
		{
			QueueItem* item = _item;
			call(item->callback, item->user_data);
			g_free(item);
			return IDLE_STOP;
		}

		//check for new work
		while(g_async_queue_length(wf->msg_queue)){
			QueueItem* message = g_async_queue_pop(wf->msg_queue); // blocks
			if(!peakbuf_queue_find(jobs, message->user_data)){
				jobs = g_list_append(jobs, message);
				dbg(2, "new message! %p", message);
			}
			else g_free(message->user_data); //not ideal.
		}

		while(jobs){
			dbg(1, "%i jobs remaining", g_list_length(jobs));
			QueueItem* item = g_list_first(jobs)->data;
			jobs = g_list_remove(jobs, item);

			g_idle_add(do_callback, item);
		}

		//TODO stop timer if no work?
		return TIMER_CONTINUE;
	}

	GMainContext* context = g_main_context_new();

	GSource* source = g_timeout_source_new(100);
	gpointer _data = NULL;
	g_source_set_callback(source, worker_timeout, _data, NULL);
	g_source_attach(source, context);

	g_main_loop_run (g_main_loop_new (context, 0));
	return NULL;
}


WfBuf16*
waveform_load_audio_async(Waveform* waveform, int block_num, int n_tiers_needed)
{
	//load part of the audio into a ram buffer.
	//-a signal will be emitted once the load is complete.

	//if the audio is already loaded, it will be returned imediately
	//and nothing further done.
	// *** Having two return paths is bad and is subject to change.

	//if the file is local we access it directly, otherwise send a msg.
	//-for now, we assume the file is local.

	//TODO should use same api as g_file_read_async ? uses GAsyncReadyCallback

	g_return_val_if_fail(waveform->priv->audio_data, NULL);
	g_return_val_if_fail(block_num < waveform_get_n_audio_blocks(waveform), NULL);
	WfAudioData* audio = waveform->priv->audio_data;

	if(audio->buf16){
		WfBuf16* buf = audio->buf16[block_num];
		if(buf) return buf;
	}

	waveform->priv->audio_data->n_tiers_present = MAX_TIERS;

	static gboolean have_thread = false;
	GError* error = NULL;
	if(!have_thread && !g_thread_create(file_load_thread, NULL, false, &error)){
		perr("error creating thread: %s\n", error->message);
		g_error_free(error);
		return NULL;
	}
	have_thread = true;

	void
	_queue_work(WfCallback callback, gpointer user_data)
	{
		QueueItem* item = g_new0(QueueItem, 1);
		item->callback = callback;
		item->user_data = user_data;

#if 0
		wf_get_instance()->work_queue = g_list_append(wf_get_instance()->work_queue, item);

		if(peak_idle) return;

		//we cant use a hi priority idle here as it prevents the canvas from being updated.
		peak_idle = g_idle_add((GSourceFunc)peakbuf_on_idle, NULL);
#else
		WF* wf = wf_get_instance();
#endif
		g_async_queue_push(wf->msg_queue, item);
	}

	void peakbuf_queue_for_regen(Waveform* waveform, int block_num, int min_output_tiers)
	{
		WF* wf = wf_get_instance();

		PeakbufQueueItem* item = g_new0(PeakbufQueueItem, 1);
		item->waveform         = waveform;
		item->block_num        = block_num;
		item->min_output_tiers = min_output_tiers;

		WfAudioData* audio = waveform->priv->audio_data;
		if(!audio->buf16){
			audio->buf16 = g_malloc0(sizeof(void*) * waveform_get_n_audio_blocks(waveform));
		}
		if(!audio->buf16[block_num]){
			audio->buf16[block_num] = g_new0(WfBuf16, 1);
			audio->buf16[block_num]->size = WF_PEAK_BLOCK_SIZE;
			int c; for(c=0;c<waveform_get_n_channels(waveform);c++){
				audio->buf16[block_num]->buf[c] = audio_cache_malloc(waveform, block_num);
				audio->buf16[block_num]->stamp = ++wf->audio.access_counter;
			}
		}
		dbg(1, "block=%i tot_audio_mem=%ukB", block_num, wf->audio.mem_size / 1024);

		void callback(gpointer item)
		{
			PeakbufQueueItem* peak = item;

			if(wf_load_audio_block(peak->waveform, peak->block_num)){
				wf_peakbuf_regen(peak->waveform, peak->block_num, peak->min_output_tiers);

				g_signal_emit_by_name(peak->waveform, "peakdata-ready", peak->block_num);
			}

			g_free(item);
		}
		_queue_work(callback, item);
	}

	if(!peakbuf_is_present(waveform, block_num)) peakbuf_queue_for_regen(waveform, block_num, n_tiers_needed);

	return NULL;
}


static int
wf_block_lookup_by_audio_buf(Waveform* w, WfBuf16* buf)
{
	g_return_val_if_fail(w && buf, -1);

	int b; for(b=0;b<waveform_get_n_audio_blocks(w);b++){
		if(w->priv->audio_data->buf16[b] == buf) return b;
	}
	return -1;
}


static short*
audio_cache_malloc(Waveform* w, int b)
{
	int size = WF_PEAK_BLOCK_SIZE;

	//dbg(1, "cache_size=%ik", MAX_AUDIO_CACHE_SIZE / 1024);
	WF* wf = wf_get_instance();
	if(wf->audio.mem_size + size > MAX_AUDIO_CACHE_SIZE){
		dbg(1, "**** cache full. looking for audio block to delete...");
		//what to delete?
		// - audio cache items can be in use, but only if we are at high zoom.
		//   But as there are many views, none of which we have knowledge of, we can't use this information.
		// Therefore we base decisions on 'least recently used'.
		WfBuf16* oldest = NULL;
		Waveform* oldest_waveform = NULL;
		GHashTableIter iter;
		gpointer key, value;
		g_hash_table_iter_init (&iter, wf->audio.cache);
		while (g_hash_table_iter_next (&iter, &key, &value)){ //need to check the stamp of each block and associate it with the waveform
			WfBuf16* buf = key;
			Waveform* w = value;
			//dbg(1, "  stamp=%u", buf->stamp);
			if(!oldest || buf->stamp < oldest->stamp){
				oldest = buf;
				oldest_waveform = w;
			}
		}
		if(oldest){
			dbg(1, "clearing buf with stamp=%i ...", oldest->stamp);
			audio_cache_free(oldest_waveform, wf_block_lookup_by_audio_buf(oldest_waveform, oldest));
		}
		if(wf->audio.mem_size + size > MAX_AUDIO_CACHE_SIZE){
			gerr("cant free space in audio cache");
		}
	}

	short* buf = g_malloc(sizeof(short) * size);
	wf->audio.mem_size += size;
	w->priv->audio_data->buf16[b]->size = size;
	dbg(1, "b=%i inserting: %p", b, buf);
	g_hash_table_insert(wf->audio.cache, w->priv->audio_data->buf16[b], w); //each channel has its own entry. however as channels are always accessed together, it might be better to have one entry per Buf16*
	audio_cache_print();
	return buf;
}


static void
audio_cache_free(Waveform* w, int block)
{
	//currently this ONLY frees the audio data, not the structs that contain it.

	//dbg(1, "b=%i", block);

	WF* wf = wf_get_instance();
	WfAudioData* audio = w->priv->audio_data;
	WfBuf16* buf16 = audio->buf16[block];
	if(buf16){
		if(!g_hash_table_remove(wf->audio.cache, buf16)) gwarn("failed to remove waveform from audio_cache");
		if(buf16->buf[WF_LEFT]){
			wf->audio.mem_size -= buf16->size;
			//dbg(1, "b=%i clearing left... size=%i", block, buf16->size);
			g_free0(buf16->buf[WF_LEFT]);
		}
		else { gwarn("left buffer empty"); }

		if(buf16->buf[WF_RIGHT]){
			wf->audio.mem_size -= buf16->size;
			dbg(1, "b=%i clearing right...", block);
			g_free0(buf16->buf[WF_RIGHT]);
		}
	}
	//audio_cache_print();
}


static void
audio_cache_print()
{
	WF* wf = wf_get_instance();

	char str[32];
	memset(str, ' ', 31);
	str[31] = '\0';

	int total_size = 0;
	int total_mem = 0;
	GHashTableIter iter;
	gpointer key, value;
	int i = 0;
	g_hash_table_iter_init (&iter, wf->audio.cache);
	while (g_hash_table_iter_next (&iter, &key, &value)){
		Waveform* w = value;
		WfBuf16* buf = key;
		//	printf("  -\n");
		g_return_if_fail(w && buf);
		int c; for(c=0;c<WF_STEREO;c++){
			//if(buf->buf[c]) printf("    %i %p\n", c, buf->buf[c]);
			if(buf->buf[c]) total_mem += WF_PEAK_BLOCK_SIZE;
		}
		if(i < 16 && buf->buf[WF_LEFT])  str[2*i    ] = 'L';
		if(i < 16 && buf->buf[WF_RIGHT]) str[2*i + 1] = 'R';

		total_size++;
		i++;
	}
	dbg(1, "size=%i mem=%ikB=%ikB %s", total_size, total_mem * sizeof(short) / 1024, wf->audio.mem_size * sizeof(short) / 1024, str);
}


#if UNUSED
static float
int2db(short x) // only used for debug output
{
	//converts a signed 16bit int to a dB value.

	float y;

	if(x != 0){
		y = -20.0 * log10(32768.0/abs(x));
		//printf("int2db: %f\n", 32768.0/abs(x));
	} else {
		y = -100.0;
	}

	return y;    
}
#endif


