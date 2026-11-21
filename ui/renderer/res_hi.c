/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#ifndef __actor_c__
#define __wf_private__
#include "config.h"
#include <sys/time.h>
#include <sndfile.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include "waveform/waveform.h"

#endif // __actor_c__

NGRenderer hi_renderer;

typedef struct
{
	Renderer       renderer;
	WfSampleRegion block_region;
} HiRenderer;

#define ANTIALIASED_LINES

#define RENDER_DATA_HI(W) ((WfTexturesHi*)W->render_data[MODE_HI])

static inline bool calc_render_info (WaveformActor* actor);


void
hi_init (WaveformActor* a)
{
	if (!hi_renderer.renderer.shader) {
		hi_renderer.renderer.shader = &ng_shader;
		if (!hi_renderer.renderer.shader->program)
			agl_create_program(&ng_shader);
	}
}


static void
hi_gl2_free_item (/*Waveform* waveform, */gpointer _data)
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
			ng_gl2_free_section(c.waveform, &data->section[s], s);
		}
	}
	else gwarn("waveform not found");
#endif

	g_free(data);
}


void
hi_gl2_on_steal (WaveformBlock* wb, guint tex)
{
	HiResNGWaveform* data = (HiResNGWaveform*)wb->waveform->priv->render_data[MODE_HI];
	if (data) {
		int _s = wb->block & (~WF_TEXTURE_CACHE_HIRES_NG_MASK);
		int s = _s / MAX_BLOCKS_PER_TEXTURE;
#ifdef DEBUG
		g_return_if_fail(_s % MAX_BLOCKS_PER_TEXTURE == 0);
		g_return_if_fail(s < data->size);
#endif
		Section* section = &data->section[s];
		if (section) {
			g_return_if_fail(tex == section->texture);
			section->texture = 0;
			section->completed = false;
			memset(section->ready, 0, sizeof(bool) * MAX_BLOCKS_PER_TEXTURE);
			dbg(0, "section %i cleared", s);
		}
	}
}


// temporary
void
hi_gl2_cache_print ()
{
	static int n_textures;
	n_textures = 0;

	void _hi_ng_print (gpointer key, gpointer value, gpointer _)
	{
		HiResNGWaveform* data = (HiResNGWaveform*)value;
		int s, n=0; for (s=0;s<data->size;s++) {
			Section* section = &data->section[s];
			if (section->buffer) {
				//dbg(0, "  %2i: t=%u %i", s, section->texture, section->time_stamp);
				n++;
			}
			if(section->texture) n_textures++;
		}
		if (!n) dbg(0, "all sections EMPTY");
	}

	if (g_hash_table_size(hi_renderer.ng_data)) {
		g_hash_table_foreach(hi_renderer.ng_data, _hi_ng_print, NULL);
		dbg(0, "n_textures=%i", n_textures);
	} else
		dbg(0, "EMPTY");
}


static void
hi_request_block (WaveformActor* a, int b)
{
	typedef struct {
		WaveformActor* actor;
		AGlWeakRef*    weakref;
	} C;

	void hi_request_block_done (Waveform* w, int b, gpointer user_data)
	{
		g_autofree C* c = user_data;

		if (!c->weakref->actor) {
			weak_ref_unref(c->weakref);
			return;
		}

		WaveformActor* a = c->actor;
		if (w == a->waveform) { // the actor may have a new waveform and requests are not cancellable.
			modes[MODE_HI].renderer->load_block(modes[MODE_HI].renderer, a, b);

			RenderInfo* r = &a->priv->render_info;
			if (!r->valid) calc_render_info(a);
			BlockRange blocks = a->priv->render_info.viewport_blocks;
			if (!r->valid || (b >= blocks.first && b <= blocks.last)) {
				if (((AGlActor*)a)->root && ((AGlActor*)a)->root->draw) wf_context_queue_redraw(a->context);
			}
		}
	}

	waveform_load_audio(a->waveform, b, HI_MIN_TIERS, hi_request_block_done, WF_NEW(C,
		.actor = a,
		.weakref = ({
			AGlWeakRef* weakref = ((WeakRefBehaviour*)((AGlActor*)a)->behaviours[WEAKREF])->ref;
			weakref->ref_count++;
			weakref;
		}),
	));
}


static void
make_texture_data_hi (Waveform* w, int ch, IntBufHi* buf, int blocknum)
{
	//data is transformed from the Waveform hi-res peakbuf into IntBufHi* buf.

	dbg(1, "b=%i", blocknum);
	int texture_size = modes[MODE_HI].renderer->texture_size;
	Peakbuf* peakbuf = waveform_get_peakbuf_n(w, blocknum);
	int o = TEX_BORDER_HI; for(;o<texture_size;o++){
		int i = (o - TEX_BORDER_HI) * WF_PEAK_VALUES_PER_SAMPLE;
		if (i >= peakbuf->size) {
			dbg(2, "end of peak: %i b=%i n_sec=%.3f", peakbuf->size, blocknum, ((float)((texture_size * blocknum + o) * WF_PEAK_RATIO))/44100); break;
		}

		short* p = peakbuf->buf[ch];
		buf->positive[o] =  p[i  ] >> 8;
		buf->negative[o] = -p[i+1] >> 8;
	}
#if 0
	int j; for(j=0;j<20;j++){
		printf("  %2i: %5i %5i %5u %5u\n", j, ((short*)peakbuf->buf[ch])[2*j], ((short*)peakbuf->buf[ch])[2*j +1], (guint)(buf->positive[j] * 0x100), (guint)(buf->negative[j] * 0x100));
	}
#endif
}


static bool
wf_actor_get_quad_dimensions (WaveformActor* actor, int b, bool is_first, bool is_last, double x, TextureRange* tex, double* tex_x_, double* block_wid_, int border, int multiplier)
{
	// multiplier is temporary and is used for HIRES_NONSHADER_TEXTURES

	// *** now contains BORDER offsetting which should be duplicated for MODE_MED in actor_render_med_lo().

	double tex_start;
	double tex_pct;
	double tex_x;
	double block_wid;

	WfActorPriv* _a = actor->priv;
	RenderInfo* r  = &_a->render_info;

	int samples_per_texture = r->samples_per_texture / multiplier;

	double usable_pct = (modes[r->mode].renderer->texture_size - 2.0 * border) / modes[r->mode].renderer->texture_size;
	double border_pct = (1.0 - usable_pct) / 2.0;

	block_wid = r->block_wid / multiplier;
	tex_pct = usable_pct; //use the whole texture
	tex_start = ((float)border) / modes[r->mode].renderer->texture_size;
	if (is_first) {
		double _tex_pct = 1.0;
		if (r->first_offset) {
			_tex_pct = 1.0 - ((double)r->first_offset) / samples_per_texture;
			tex_pct = tex_pct - (usable_pct) * ((double)r->first_offset) / samples_per_texture;
		}

#ifdef HIRES_NONSHADER_TEXTURES
		if (r->first_offset >= samples_per_texture) return false;
		if (r->first_offset) tex_pct = 1.0 - ((double)r->first_offset) / samples_per_texture;
#endif

		block_wid = (r->block_wid / multiplier) * _tex_pct;
		tex_start = 1.0 - border_pct - tex_pct;
		dbg(2, "rect.left=%.2f region->start=%"PRIi64" first_offset=%i", r->rect.left, r->region.start, r->first_offset);
	}
	if (is_last) {
		if (b < r->region_end_block) {
			//end is offscreen. last block is not smaller.
		} else {
			//end is trimmed
			WfSampleRegionf region_px = {
				.start = wf_actor_samples2gl(r->zoom, r->region.start),
				.len = wf_actor_samples2gl(r->zoom, r->region.len)
			};
#if 0
			// this correctly calculates the block width but is perhaps too complex
			double distance_from_file_start_to_region_end = region_px.start + MIN(r->rect.len, region_px.len);
			block_wid = distance_from_file_start_to_region_end - b * r->block_wid;
			dbg(2, " %i: inset=%.2f s->e=%.2f i*b=%.2f", b, region_px.start, distance_from_file_start_to_region_end, b * r->block_wid);
			if(b * r->block_wid > distance_from_file_start_to_region_end){ pwarn("!!"); return false; }
			block_wid = MIN(r->rect.len, block_wid);
#else
			WfdRange block_px = {
				.start = MAX(0.0, region_px.start - b * r->block_wid),
				.end = (region_px.start + region_px.len) - b * r->block_wid
			};
			block_wid = block_px.end - block_px.start;
#endif
		}

#if 0 // check if this is needed
#ifdef HIRES_NONSHADER_TEXTURES
		block_wid = MIN(block_wid, r->block_wid / multiplier);
#endif
#endif
		//TODO when non-square textures enabled, tex_pct can be wrong because the last texture is likely to be smaller
		//     (currently this only applies in non-shader mode)
		//tex_pct = block_wid / r->block_wid;
		tex_pct = (block_wid / r->block_wid) * multiplier * usable_pct;
	}

	dbg (2, "%i: %s x=%6.2f wid=%.2f/%.2f tex_pct=%.3f tex_start=%.3f", b, is_last ? "LAST" : "    ", x, block_wid, r->block_wid, tex_pct, tex_start);
if(tex_pct > usable_pct || tex_pct < 0.0){
dbg (0, "%i: is_first=%i is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex_start=%.3f", b, is_first, is_last, x, block_wid, r->block_wid, tex_pct, tex_start);
}
	if(tex_pct - 0.0001 > usable_pct || tex_pct < 0.0) pwarn("tex_pct > %.3f! %.3f (b=%i) %.3f --> %.3f", usable_pct, tex_pct, b, tex_start, tex_start + tex_pct);
	tex_x = x + ((is_first && r->first_offset) ? r->first_offset_px : 0);

	*tex = (TextureRange){tex_start, tex_start + tex_pct};
	*tex_x_ = tex_x;
	*block_wid_ = block_wid;
	return true;
}


NGRenderer hi_renderer = {{MODE_HI, hi_init, ng_load_block, ng_pre_render, ng_render_block, ng_post_render, ng_free_waveform,
	.texture_size = WF_PEAK_TEXTURE_SIZE * 16, // texture size chosen so that blocks are the same as in medium res
}};


static Renderer*
hi_renderer_new ()
{
	g_return_val_if_fail(!hi_renderer.ng_data, NULL);

	HiRenderer* _hi_renderer = (HiRenderer*)&hi_renderer;

	hi_renderer.ng_data = g_hash_table_new_full(g_direct_hash, g_int_equal, NULL, hi_gl2_free_item);

	ng_make_lod_levels(&hi_renderer, _hi_renderer->renderer.texture_size);

	return (Renderer*)_hi_renderer;
}
