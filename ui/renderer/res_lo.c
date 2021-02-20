/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
extern HiResNGShader hires_ng_shader;

static void
low_new_gl2 (WaveformActor* actor)
{
	WaveformPrivate* w = actor->waveform->priv;

	g_return_if_fail(!w->render_data[MODE_LOW]);

	Renderer* renderer = modes[MODE_LOW].renderer;
	if(!renderer->shader){
		renderer->shader = &hires_ng_shader.shader;
		if(!renderer->shader->program) agl_create_program(&hires_ng_shader.shader);
	}
}


static void
lo_new_gl1(WaveformActor* actor)
{
	// TODO this can be combined with v_lo_new_gl1

	Waveform* w = actor->waveform;
	WaveformPrivate* _w = w->priv;

	if(!_w->render_data[MODE_LOW]){
		//#warning check TEX_BORDER effect not multiplied in WF_PEAK_STD_TO_LO transformation
		int n_blocks = _w->num_peaks / (WF_PEAK_STD_TO_LO * WF_TEXTURE_VISIBLE_SIZE) + ((_w->num_peaks % (WF_PEAK_STD_TO_LO * WF_TEXTURE_VISIBLE_SIZE)) ? 1 : 0);

		_w->render_data[MODE_LOW] = (WaveformModeRender*)wf_texture_array_new(n_blocks, w->n_channels);
		_w->render_data[MODE_LOW]->n_blocks = n_blocks;
	}
}


Renderer lo_renderer_gl1 = {MODE_LOW, lo_new_gl1, low_allocate_block_gl1, med_lo_pre_render_gl1, med_lo_render_gl1, med_lo_gl1_free_waveform};
NGRenderer lo_renderer_gl2 = {{MODE_LOW, low_new_gl2, ng_gl2_load_block, ng_gl2_pre_render, ng_gl2_render_block, ng_gl2_free_waveform}};


static Renderer*
lo_renderer_new ()
{
	g_return_val_if_fail(!med_renderer_gl2.ng_data, NULL);

	static Renderer* lo_renderer = (Renderer*)&lo_renderer_gl2;

	lo_renderer_gl2.ng_data = g_hash_table_new_full(g_direct_hash, g_int_equal, NULL, g_free);

	ng_make_lod_levels(&lo_renderer_gl2, MODE_LOW);

	return (Renderer*)lo_renderer;
}
