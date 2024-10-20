/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2014-2024 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */


static void
v_lo_new (WaveformActor* actor)
{
	Waveform* waveform = actor->waveform;
	WaveformPrivate* w = waveform->priv;
	Renderer* renderer = modes[MODE_V_LOW].renderer;

	int n_blocks = w->num_peaks / (WF_MED_TO_V_LOW * WF_TEXTURE_VISIBLE_SIZE) + ((w->num_peaks % (WF_MED_TO_V_LOW * WF_TEXTURE_VISIBLE_SIZE)) ? 1 : 0);

	HiResNGWaveform** data = (HiResNGWaveform**)&w->render_data[MODE_V_LOW];
	if (!*data) {
		int n_sections = waveform_get_n_audio_blocks(waveform) / MAX_BLOCKS_PER_TEXTURE + (waveform_get_n_audio_blocks(waveform) % MAX_BLOCKS_PER_TEXTURE ? 1 : 0);

		*(*data = g_malloc0(sizeof(HiResNGWaveform) + sizeof(Section) * n_sections)) = (HiResNGWaveform){
			.n_blocks = n_blocks,
			.size     = n_sections
		};

		g_object_weak_ref((GObject*)waveform, ng_gl2_finalize_notify, renderer);
		g_hash_table_insert(((NGRenderer*)renderer)->ng_data, waveform, *data);
	}

	AGlShader** shader = &renderer->shader;
	if (!*shader) {
		*shader = &hires_ng_shader.shader;
		if ((*shader)->program)
			((AGlActor*)actor)->program = *shader;
		else
			renderer_create_shader (renderer);
	}
}


static void
v_lo_new_gl1 (WaveformActor* actor)
{
	WaveformPrivate* w = actor->waveform->priv;

	waveform_load_sync(actor->waveform);

	if(!w->render_data[MODE_V_LOW]){
		int n_blocks = w->num_peaks / (WF_MED_TO_V_LOW * WF_TEXTURE_VISIBLE_SIZE) + ((w->num_peaks % (WF_MED_TO_V_LOW * WF_TEXTURE_VISIBLE_SIZE)) ? 1 : 0);
		w->render_data[MODE_V_LOW] = (WaveformModeRender*)wf_texture_array_new(n_blocks, actor->waveform->n_channels);
		w->render_data[MODE_V_LOW]->n_blocks = n_blocks;
	}
}


static void
v_lo_buf_to_tex (Renderer* renderer, WaveformActor* actor, int b)
{
	Waveform* waveform = actor->waveform;
	WaveformPrivate* w = waveform->priv;
	WfPeakBuf* peak = &w->peak;
	int s  = b / MAX_BLOCKS_PER_TEXTURE;
	int _b = b % MAX_BLOCKS_PER_TEXTURE;
	int block_size = get_block_size(actor);
	HiResNGWaveform* data = (HiResNGWaveform*)w->render_data[renderer->mode];
	Section* section = &data->section[s];
	int n_chans = waveform_get_n_channels(waveform);
	int n_blocks = waveform_get_n_audio_blocks(waveform) / WF_MED_TO_V_LOW + (waveform_get_n_audio_blocks(waveform) % WF_MED_TO_V_LOW ? 1 : 0);
	bool is_last_block = b == n_blocks - 1;

	int mm_level = 0;
	int* lod_max = ((NGRenderer*)renderer)->mmidx_max;
	int* lod_min = ((NGRenderer*)renderer)->mmidx_min;

	#define B_SIZE (WF_PEAK_TEXTURE_SIZE - 2 * TEX_BORDER)
	int stop = is_last_block
		? peak->size / (WF_MED_TO_V_LOW * WF_PEAK_VALUES_PER_SAMPLE) + TEX_BORDER - B_SIZE * b
		: WF_PEAK_TEXTURE_SIZE;

	int c; for(c=0;c<n_chans;c++){
		int64_t src = WF_PEAK_VALUES_PER_SAMPLE * WF_MED_TO_V_LOW * (_b * B_SIZE - TEX_BORDER);
		int dest = _b * block_size + (c * block_size / 2);

		int t = 0;
		if(b == 0){
			for(t=0;t<TEX_BORDER;t++){
				ng_gl2_set_(section, dest + lod_max[mm_level] + t, 0);
				ng_gl2_set_(section, dest + lod_min[mm_level] + t, 0);
			}
			src = 0;
		}

		for(; t<stop; t++, src+=2*(WF_MED_TO_V_LOW)){
			short max = 0, min = 0;

			int end = MIN(WF_MED_TO_V_LOW, peak->size - (src + 1));
			int i; for(i=0;i<end;i++){
				max = MAX(max,  peak->buf[c][src + i    ]);
				min = MIN(min, -peak->buf[c][src + i + 1]);
			}

			ng_gl2_set_(section, dest                     + t, short_to_char(max));
			ng_gl2_set_(section, dest + lod_min[mm_level] + t, short_to_char(-min));
		}

		if(is_last_block){
			for(t=stop;t<WF_PEAK_TEXTURE_SIZE;t++){
				ng_gl2_set_(section, dest + lod_max[mm_level] + t, 0);
				ng_gl2_set_(section, dest + lod_min[mm_level] + t, 0);
			}
		}
	}
}


#ifdef USE_TEST
static bool
v_lo_is_not_blank (Renderer* renderer, WaveformActor* actor)
{
	return true;
}
#endif


Renderer v_lo_renderer_gl1 = {MODE_V_LOW, v_lo_new_gl1, low_allocate_block_gl1, med_lo_pre_render_gl1, med_lo_render_gl1, NULL, med_lo_gl1_free_waveform};
NGRenderer v_lo_renderer_gl2 = {{MODE_V_LOW, v_lo_new, ng_gl2_load_block, ng_pre_render0, ng_gl2_render_block, ng_gl2_post_render, ng_gl2_free_waveform,
#ifdef USE_TEST
	.is_not_blank = v_lo_is_not_blank,
#endif
	}, v_lo_buf_to_tex,
};
Renderer v_lo_renderer;


static Renderer*
v_lo_renderer_init ()
{
	v_lo_renderer_gl2.ng_data = g_hash_table_new_full(g_direct_hash, g_int_equal, NULL, g_free);

	ng_make_lod_levels(&v_lo_renderer_gl2, MODE_V_LOW);

	return (Renderer*)&v_lo_renderer_gl2;
}
