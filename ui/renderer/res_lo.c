/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2013-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

extern AGlShader ng_shader;

static void
low_new (WaveformActor* actor)
{
	WaveformPrivate* w = actor->waveform->priv;

	g_return_if_fail(!w->render_data[MODE_LOW]);

	Renderer* renderer = modes[MODE_LOW].renderer;
	if (!renderer->shader) {
		renderer->shader = &ng_shader;
		if (!ng_shader.program) agl_create_program(&ng_shader);
	}
}


NGRenderer lo_renderer = {{MODE_LOW, low_new, ng_load_block, ng_pre_render, ng_render_block, ng_post_render, ng_free_waveform,
	.texture_size = WF_PEAK_TEXTURE_SIZE,
}};


static Renderer*
lo_renderer_init ()
{
	g_return_val_if_fail(!med_renderer.ng_data, NULL);

	Renderer* _lo_renderer = (Renderer*)&lo_renderer;

	lo_renderer.ng_data = g_hash_table_new_full(g_direct_hash, g_int_equal, NULL, g_free);

	ng_make_lod_levels(&lo_renderer, _lo_renderer->texture_size);

	return (Renderer*)_lo_renderer;
}
