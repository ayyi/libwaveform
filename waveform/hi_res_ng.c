/*
  copyright (C) 2014 Tim Orford <tim@orford.org>

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

  ------------------------------------------------------------------------

  This HI_RES renderer is shader-only (GLSL 1.20) and passes peak data
  directly to the shader in a 2D texture.

  Texture format:
    - each texture can hold up to 32 blocks.
    - each 8bit value directly the stores the peak value (either max or min)
    - each block is stored in 4 rows per block per channel
    - row layout:
      - row 0: 4096 max values
      - row 1: max values repeated at progressively lower x resolution (mipmap-like)
      - row 2: 4096 min values
      - row 3: min values repeated at progressively lower x resolution

      - row 4...: for multi channel audio, the above is repeated for other channels.

  Texture size is 16k per block per channel

  A 10 hour 44100k mono audio file would use: 24000 blocks, 750 textures, 384MB.

  Memory usage:
    - textures are timestamped when used.
    - resources for each WfWaveform are removed when the waveform is free'd.
    - to handle cases such as in a DAW where there are very large numbers of
      audio files, a garbage collector is run after resources are added. The
      collector removes the textures that have been used least recently.

*/
#ifndef __actor_c__
#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <sndfile.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "waveform/utils.h"
#include "waveform/canvas.h"
#include "waveform/actor.h"
#include "waveform/peak.h"

extern int wf_debug;
#endif // __actor_c__

#define MAX_BLOCKS_PER_TEXTURE 32 // gives a texture size of 128k (256k stereo)

typedef struct {
   guchar* buffer;
   int     buffer_size;
   guint   texture;
   int     time_stamp;
   bool    completed;
   bool    ready[MAX_BLOCKS_PER_TEXTURE];
} Section;

typedef struct {
   int       size;
   Section   section[];
} HiResNGWaveform;

#define ROWS_PER_PEAK_TYPE 2

#define N_LOD 4
static int mmidx_max[N_LOD];
static int mmidx_min[N_LOD];

static GHashTable* hi_res_ng_data = NULL;
static int hi_time_stamp = 0;

static bool get_quad_dimensions (WaveformActor* actor, int b, bool is_first, bool is_last, double x, TextureRange*, double* tex_x_, double* block_wid_, int border, int multiplier);
static void hi_ng_queue_clean   ();
void        hi_ng_cache_print   ();

#define short_to_char(A) ((guchar)(A / 128))


static void
hi_ng_free_section(Waveform* waveform, Section* section, int s)
{
	if(section){
		if(section->buffer) g_free0(section->buffer);
		if(section->texture){
			texture_cache_remove(GL_TEXTURE_2D, waveform, (s * MAX_BLOCKS_PER_TEXTURE) | WF_TEXTURE_CACHE_HIRES_NG_MASK);
			section->texture = 0;
		}
		section->completed = false;
		memset(section->ready, 0, sizeof(bool) * MAX_BLOCKS_PER_TEXTURE);
	}
}


static void
hi_ng_free_waveform(Waveform* waveform)
{
	PF;

	HiResNGWaveform* data = g_hash_table_lookup(hi_res_ng_data, waveform);
	if(data){
		// the sections must be freed before removing from the hashtable
		// so that the Waveform can be referenced.
		int s; for(s=0;s<data->size;s++){
			hi_ng_free_section(waveform, &data->section[s], s);
		}
		// removing from the hash table will cause the item to be free'd.
		if(!g_hash_table_remove(hi_res_ng_data, waveform)) dbg(1, "failed to remove hi-res data");
	}
}


static void
hi_ng_free_item(/*Waveform* waveform, */gpointer _data)
{
	// this is called by the hash_table when an item is removed from hi_res_ng_data.

	// ** the item has already been removed from the hash_table
	//    so we have no way of referencing the Waveform.

	HiResNGWaveform* data = _data;

#if 0
	struct C {
		HiResNGWaveform* data;
		Waveform*        waveform;
	} c = {data, NULL};

	bool find_val(gpointer key, gpointer value, gpointer user_data)
	{
		// reverse lookup - find the key given the data.
		struct C* c = user_data;
		return (value == c->data)
			? c->waveform = key, true
			: false;
	}

	if(g_hash_table_find(hi_res_ng_data, find_val, &c)){

		int s; for(s=0;s<data->size;s++){
			hi_ng_free_section(c.waveform, &data->section[s], s);
		}
	}
	else gwarn("waveform not found");
#endif

	g_free(data);
}


static void
hi_ng_finalize_notify(gpointer user_data, GObject* was)
{
	PF;
	Waveform* waveform = (Waveform*)was;
	hi_ng_free_waveform(waveform);
}


#ifdef NOT_USED
static void hi_ng_uninit()
{
	g_hash_table_destroy(hi_res_ng_data);
	hi_res_ng_data = NULL;
}
#endif


#ifdef XDEBUG
static bool
ng_set(Section* section, int pos, char val)
{
	g_return_val_if_fail(section->buffer_size, false);
	g_return_val_if_fail(pos < section->buffer_size, false);
	section->buffer[pos] = val;
	return true;
}
#else
#define ng_set(section, pos, val) (section->buffer[pos] = val, true)
#endif


static void
hi_ng_load_block(Renderer* renderer, WaveformActor* actor, int b)
{
	Waveform* waveform = actor->waveform;

	int time_stamp = hi_time_stamp++;

	#define get_block_size(ACTOR) (modes[MODE_HI].texture_size * waveform_get_n_channels(ACTOR->waveform) * WF_PEAK_VALUES_PER_SAMPLE * ROWS_PER_PEAK_TYPE)

	Section* add_section(WaveformActor* actor, HiResNGWaveform* data, int s)
	{
		int block_size = get_block_size(actor);
		int buffer_size = block_size * MIN(MAX_BLOCKS_PER_TEXTURE, waveform_get_n_audio_blocks(waveform) - s * MAX_BLOCKS_PER_TEXTURE);
		dbg(1, "block_size=%ik section->buffer=%ik", block_size / 1024, buffer_size / 1024);

		Section* section = &data->section[s];
		section->buffer = g_malloc0(section->buffer_size = buffer_size);

		section->time_stamp = hi_time_stamp++;

		hi_ng_queue_clean();

		return section;
	}

	bool section_is_complete(WaveformActor* actor, Section* section)
	{
		int max = MIN(waveform_get_n_audio_blocks(waveform), MAX_BLOCKS_PER_TEXTURE);
		int i;for(i=0;i<max;i++){
			if(!section->ready[i]) return false;
		}
		dbg(1, "complete");
		return section->completed = true;
	}

	int n_chans = waveform_get_n_channels(waveform);

	HiResNGWaveform* data = g_hash_table_lookup(hi_res_ng_data, waveform);
	if(!data){
		int n_sections = waveform_get_n_audio_blocks(waveform) / MAX_BLOCKS_PER_TEXTURE + (waveform_get_n_audio_blocks(waveform) % MAX_BLOCKS_PER_TEXTURE ? 1 : 0);
		data = g_malloc0(sizeof(HiResNGWaveform) + sizeof(Section) * n_sections);
		data->size = n_sections;
		g_object_weak_ref((GObject*)waveform, hi_ng_finalize_notify, NULL);
		g_hash_table_insert(hi_res_ng_data, actor->waveform, data);

		int buffer_size = 0;
		int width = modes[MODE_HI].texture_size;
		int level_size = width;
		int i; for(i=0;i<N_LOD;i++){
			mmidx_max[i] = buffer_size;                                         // TODO move
			mmidx_min[i] = buffer_size + width * ROWS_PER_PEAK_TYPE;
			//dbg(0, "  %i %i", 1 << i, size0 / (1 << i));
			level_size = width / (1 << i);
			buffer_size += level_size;
		}
	}

	bool texture_changed[data->size];
	memset(texture_changed, 0, sizeof(bool) * data->size);

	int block_size = get_block_size(actor);
	{
		int s  = b / MAX_BLOCKS_PER_TEXTURE;
		int _b = b % MAX_BLOCKS_PER_TEXTURE;
		Section* section = &data->section[s];
															// TODO move timestamp to render
		section->time_stamp = time_stamp;
		if(section->completed) return;
		if(!section->buffer) section = add_section(actor, data, s);
		if(!section->ready[_b]){
			texture_changed[s] = true;
			// we are here as notification that audio has loaded so it is an error if not.
			g_return_if_fail(waveform->priv->audio_data);
			WfBuf16* audio_buf = waveform->priv->audio_data->buf16[b];
			g_return_if_fail(audio_buf);

			short max[n_chans];
			short min[n_chans];
			short short_max[n_chans][modes[MODE_HI].texture_size];
			short short_min[n_chans][modes[MODE_HI].texture_size];
			int c; for(c=0;c<n_chans;c++){
				int io_ratio = 16;
				int mm_level = 0;
				int i, p; for(i=0, p=0; p<WF_PEAK_BLOCK_SIZE; i++, p+= io_ratio){

					short* d = &audio_buf->buf[c][p];
					max[c] = 0;
					min[c] = 0;
					int k; for(k=0;k<io_ratio;k++){
						max[c] = (d[k + c] > max[c]) ? d[k + c] : max[c]; // why k+c ? doesnt make sense.
						min[c] = (d[k + c] < min[c]) ? d[k + c] : min[c];
					}
					short_max[c][i] = max[c];
					short_min[c][i] = min[c];

					bool ok = ng_set(section, _b * block_size + (c * block_size / 2) + mmidx_max[mm_level] + i, short_to_char(max[c]));
					if(!ok) gerr("max b=%i i=%i p=%i %i size=%i", _b, i, p, _b * block_size + mmidx_max[mm_level] + i, section->buffer_size);
					g_return_if_fail(ok);
					ok = ng_set(section, _b * block_size + (c * block_size / 2) + mmidx_min[mm_level] + i, short_to_char(-min[c]));
					if(!ok) gerr("min b=%i i=%i p=%i %i size=%i mm=%i", _b, i, p, b * block_size + mmidx_min[mm_level] + i, section->buffer_size, mmidx_min[mm_level]);
					g_return_if_fail(ok);
				}

				// other levels of detail:
				int m; for(m=1;m<N_LOD;m++){
					int mm_level = m;
					int mm = 1 << (mm_level - 1);
					for(i=0, p=0; p<modes[MODE_HI].texture_size/mm; i++, p+=2){
						short_max[c][i] = MAX(short_max[c][p], short_max[c][p+1]);
						short_min[c][i] = MIN(short_min[c][p], short_min[c][p+1]);
						ng_set(section, _b * block_size + (c * block_size / 2) + mmidx_max[mm_level] + i, short_to_char(short_max[c][i]));
						ng_set(section, _b * block_size + (c * block_size / 2) + mmidx_min[mm_level] + i, short_to_char(-short_min[c][i]));
					}
				}
			}
			section->ready[_b] = true;
		}
	}

	int s; for(s=0;s<data->size;s++){
		Section* section = &data->section[s];
		if(!section->completed){
			if(texture_changed[s]){
				if(!section->texture){
					// note: for the WaveformBlock we use the first block for the section (WaveformBlock concept is broken in this context)
					section->texture = texture_cache_assign_new(GL_TEXTURE_2D, (WaveformBlock){waveform, (s * MAX_BLOCKS_PER_TEXTURE) | WF_TEXTURE_CACHE_HIRES_NG_MASK});
				}

				int width = modes[MODE_HI].texture_size;
				int height = section->buffer_size / width;
				int pixel_format = GL_ALPHA;
				glBindTexture  (GL_TEXTURE_2D, section->texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				// TODO it is quite common for this to be done several times in quick succession for the same texture with consecutive calls to hi_ng_load_block
				dbg(1, "%i: uploading texture: %i x %i", s, width, height);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, section->buffer);
				gl_warn("error binding texture: %u", section->texture);
			}

			if(section_is_complete(actor, section)){
				g_free0(section->buffer); // all data has been sent to the gpu so can be freed.
			}
		}
	}
}


static void
hi_ng_pre_render(Renderer* renderer, WaveformActor* actor)
{
	WaveformCanvas* wfc = actor->canvas;
	Waveform* w = actor->waveform;
	WfActorPriv* _a = actor->priv;
	RenderInfo* r  = &_a->render_info;

	HiResNGWaveform* data = g_hash_table_lookup(hi_res_ng_data, w);
	if(!data) return; // this can happen when we fall through from v hi res.

	HiResNGShader* shader = wfc->priv->shaders.hires_ng;
	shader->uniform.fg_colour = (actor->fg_colour & 0xffffff00) + (unsigned)(0x100 * _a->animatable.opacity.val.f);
	shader->uniform.top = r->rect.top;
	shader->uniform.bottom = r->rect.top + r->rect.height;
	shader->uniform.n_channels = waveform_get_n_channels(w);
	shader->uniform.tex_height = data->section[0].buffer_size / modes[MODE_HI].texture_size;

	shader->uniform.mm_level = r->block_wid > 2048
		? 0
		: r->block_wid > 1024
			? 1
			: r->block_wid > 512
				? 2
				: 3;
				// TODO do we need levels 4-8 ?

	agl_use_program(&shader->shader);
}


static bool
block_hires_ng(Renderer* renderer, WaveformActor* actor, int b, gboolean is_first, gboolean is_last, double x)
{
	gl_warn("pre");

	int border = 0; // TODO

	Waveform* waveform = actor->waveform; 
	WfActorPriv* _a = actor->priv;
	RenderInfo* r  = &_a->render_info;

	HiResNGWaveform* data = g_hash_table_lookup(hi_res_ng_data, waveform);
	if(!data) return false; // this can happen when audio data not yet available.
	Section* section = &data->section[b / MAX_BLOCKS_PER_TEXTURE];

																								glActiveTexture(GL_TEXTURE0);

#if 0
	double tex_start;
	double tex_pct;
#else
	TextureRange tex;
#endif
	double tex_x;
	double block_wid;
	if(!get_quad_dimensions(actor, b, is_first, is_last, x, &tex, &tex_x, &block_wid, border, 1)) return false;

	float n_rows = section->buffer_size / modes[MODE_HI].texture_size;
	float ty = (b % MAX_BLOCKS_PER_TEXTURE) * 4.0 * waveform_get_n_channels(waveform) / n_rows; // this tells the shader which block to use.
	AGlQuad tex_rect = {tex.start, ty, tex.end, ty};

	agl_textured_rect(section->texture, tex_x, r->rect.top, block_wid, r->rect.height, &tex_rect);

	return true;
}


void
hi_ng_on_steal(WaveformBlock* wb, guint tex)
{
	HiResNGWaveform* data = g_hash_table_lookup(hi_res_ng_data, wb->waveform);
	if(data){
		int _s = wb->block & (~WF_TEXTURE_CACHE_HIRES_NG_MASK);
		int s = _s / MAX_BLOCKS_PER_TEXTURE;
#ifdef DEBUG
		g_return_if_fail(_s % MAX_BLOCKS_PER_TEXTURE == 0);
		g_return_if_fail(s < data->size);
#endif
		Section* section = &data->section[s];
		if(section){
			g_return_if_fail(tex == section->texture);
			section->texture = 0;
			section->completed = false;
			memset(section->ready, 0, sizeof(bool) * MAX_BLOCKS_PER_TEXTURE);
			dbg(0, "section %i cleared", s);
		}
	}
}


static void
hi_ng_queue_clean()
{
	#define MAX_SECTIONS 1024

	static guint idle_id = 0;

	bool clean(gpointer user_data)
	{
		static struct _oldest {
			Waveform*        waveform;
			HiResNGWaveform* data;
			int              section;
			int              time_stamp;
		} oldest;

		void __hi_find_oldest(gpointer key, gpointer value, gpointer _)
		{
			HiResNGWaveform* data = (HiResNGWaveform*)value;
			int s; for(s=0;s<data->size;s++){
				Section* section = &data->section[s];
				dbg(0, ">  %i", section->time_stamp);
				if(section->buffer && (section->time_stamp < oldest.time_stamp)){
					oldest = (struct _oldest){key, data, s, section->time_stamp};
				}
			}
		}

		dbg(1, "size=%i", g_hash_table_size(hi_res_ng_data));

		if(g_hash_table_size(hi_res_ng_data) > MAX_SECTIONS){
			int n_to_remove = g_hash_table_size(hi_res_ng_data) - MAX_SECTIONS;
			int i; for(i=0;i<n_to_remove;i++){
				oldest = (struct _oldest){NULL, 0, INT_MAX};
				g_hash_table_foreach(hi_res_ng_data, __hi_find_oldest, NULL);

				if(oldest.data){
					dbg(0, "removing: section=%i", oldest.section);
					hi_ng_free_section(oldest.waveform, &oldest.data->section[oldest.section], oldest.section);
				}
			}
		}

		idle_id = 0;
		return IDLE_STOP;
	}

	if(!idle_id) idle_id = g_idle_add_full(G_PRIORITY_LOW, clean, NULL, NULL);
}


// temporary
void
hi_ng_cache_print()
{
	static int n_textures; n_textures = 0;

	void _hi_ng_print(gpointer key, gpointer value, gpointer _)
	{
		HiResNGWaveform* data = (HiResNGWaveform*)value;
		int s, n=0; for(s=0;s<data->size;s++){
			Section* section = &data->section[s];
			if(section->buffer){
				//dbg(0, "  %2i: t=%u %i", s, section->texture, section->time_stamp);
				n++;
			}
			if(section->texture) n_textures++;
		}
		if(!n) dbg(0, "all sections EMPTY");
	}

	if(g_hash_table_size(hi_res_ng_data)){
		g_hash_table_foreach(hi_res_ng_data, _hi_ng_print, NULL);
		dbg(0, "n_textures=%i", n_textures);
	}else
		dbg(0, "EMPTY");
}


