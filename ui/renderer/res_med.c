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

extern AGlShader ng_shader;

typedef HiResNGWaveform NGWaveform;

#define RENDER_DATA_MED(A) ((NGWaveform*)A->render_data[MODE_MED])


static void
med_renderer_init (WaveformActor* actor)
{
	AGlShader** shader = &modes[MODE_MED].renderer->shader;
	if (!*shader) {
		*shader = &ng_shader;
		if (!(*shader)->program)
			agl_create_program(*shader);
	}

	// other data is created in ng_renderer.load_block
}


#if 0
static void
med_lo_clear_1d_textures(WfGlBlock* blocks, WaveformBlock* wb)
{
	texture_cache_remove(GL_TEXTURE_1D, wb->waveform, wb->block);
	int b = wb->block & ~WF_TEXTURE_CACHE_LORES_MASK;
	int c; for(c=0;c<WF_RIGHT;c++) blocks->peak_texture[c].main[b] = blocks->peak_texture[c].neg[b] = 0;
}
#endif


static inline void
_med_lo_set_gl_state_for_block (WaveformContext* wfc, Waveform* w, WfGlBlock* textures, int b)
{
	g_return_if_fail(b < textures->size);

	agl_use_texture(textures->peak_texture[0].main[b]);

	gl_warn("cannot bind texture: block=%i: %i", b, textures->peak_texture[0].main[b]);
}


static inline void
_draw_block_from_1d(float tex_start, float tex_pct, float x, float y, float width, float height, int t_size)
{
	// used by both med and hi_res modes.

	if (t_size > 256) {
		// MODE_HI
		// no longer do any offsetting here - do the same for non MODE_HI also.
	} else {
		float offset = TEX_BORDER;

		tex_start += offset / t_size;
		tex_pct *= (t_size - 2.0 * offset) / t_size;
	}

	float tex_end = tex_start + tex_pct;
	glMultiTexCoord2f(WF_TEXTURE0, tex_start, 1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start, 1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start, 1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start, 1.0); glVertex2d(x + 0.0,   y);
	glMultiTexCoord2f(WF_TEXTURE0, tex_end,   1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_end,   1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_end,   1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_end,   1.0); glVertex2d(x + width, y);
	glMultiTexCoord2f(WF_TEXTURE0, tex_end,   0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_end,   0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_end,   0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_end,   0.0); glVertex2d(x + width, y + height);
	glMultiTexCoord2f(WF_TEXTURE0, tex_start, 0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start, 0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start, 0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start, 0.0); glVertex2d(x + 0.0,   y + height);
}


void
med_lo_on_steal (WaveformBlock* wb, guint tex)
{
	Mode mode = wb->block & WF_TEXTURE_CACHE_V_LORES_MASK
		? MODE_V_LOW
		: wb->block & WF_TEXTURE_CACHE_LORES_MASK
			? MODE_LOW
			: MODE_MED;
	int b = wb->block & ~(WF_TEXTURE_CACHE_V_LORES_MASK | WF_TEXTURE_CACHE_LORES_MASK);
	WfGlBlock* blocks = (WfGlBlock*)wb->waveform->priv->render_data[mode];
	g_return_if_fail(blocks);
	guint* peak_texture[4] = {
		&blocks->peak_texture[0].main[b],
		&blocks->peak_texture[0].neg [b],
		&blocks->peak_texture[1].main[b],
		&blocks->peak_texture[1].neg [b]
	};

	int find_texture_in_block(guint tid, WaveformBlock* wb, guint* peak_texture[4])
	{
		// find which of the 4 textures was removed

		g_return_val_if_fail(wb->block < WF_MAX_AUDIO_BLOCKS, -1);

		int i; for(i=0;i<4;i++){
			if(peak_texture[i] && *peak_texture[i] == tid) return i;
		}
		return -1;
	}

	int p;
	if ((p = find_texture_in_block(tex, wb, peak_texture)) < 0) {
		pwarn("texture not found");
	} else {
		dbg(1, "clearing texture for block=%i %i ...", wb->block, p);
		*peak_texture[p] = 0;
	}
}


NGRenderer med_renderer = {{MODE_MED, med_renderer_init, ng_load_block, ng_pre_render, ng_render_block, ng_post_render, ng_free_waveform,
	.texture_size = WF_PEAK_TEXTURE_SIZE,
}};


static Renderer*
med_renderer_new ()
{
	g_return_val_if_fail(!med_renderer.ng_data, NULL);

	Renderer* _med_renderer = (Renderer*)&med_renderer;

	med_renderer.ng_data = g_hash_table_new_full(g_direct_hash, g_int_equal, NULL, g_free);

	ng_make_lod_levels(&med_renderer, _med_renderer->texture_size);

	return (Renderer*)_med_renderer;
}


