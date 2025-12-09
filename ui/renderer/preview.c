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

NGRenderer preview_renderer;


static Renderer*
preview_renderer__init ()
{
	g_return_val_if_fail(!preview_renderer.renderer.shader, NULL);

	preview_renderer.renderer.shader = &ng_shader;

	ng_make_lod_levels(&preview_renderer, PREVIEW_SIZE);

	return (Renderer*)&preview_renderer;
}


static void
preview_renderer_init (WaveformActor* actor)
{
	if (!preview_renderer.renderer.shader)
		preview_renderer__init();

	if (!ng_shader.program)
		agl_create_program(&ng_shader);
}


static void
preview_make_data (Renderer* renderer, WaveformActor* actor)
{
	// create buffer

	Waveform* waveform = actor->waveform;
	WaveformPrivate* w = waveform->priv;
	WfPreview* preview = w->preview;

	HiResNGWaveform** data = (HiResNGWaveform**)&preview->render_data;
	if (!*data) {
		int n_sections = 1;
		*(*data = g_malloc0(sizeof(HiResNGWaveform) + sizeof(Section) * n_sections)) = (HiResNGWaveform){
			.size = n_sections,
			.n_blocks = 1,
		};
		Section* section = &(*data)->section[0];
		section->buffer = g_malloc0(section->buffer_size = PREVIEW_SIZE * WF_PEAK_VALUES_PER_SAMPLE * ROWS_PER_PEAK_TYPE * sizeof(char) * waveform->n_channels);

		void preview_clear_render_data (WfPreview* preview)
		{
			preview_renderer.renderer.free((Renderer*)&preview_renderer, preview->waveform, &preview->render_data);
		}
		preview->clear_render_data = preview_clear_render_data;
	}
	Section* section = &(*data)->section[0];

	// fill buffer

	int mm_level = 0;
	int* lod_max = ((NGRenderer*)renderer)->mmidx_max;
	int* lod_min = ((NGRenderer*)renderer)->mmidx_min;

	for (int c=0;c<waveform->n_channels;c++) {
		int dest = (c * PREVIEW_SIZE * WF_PEAK_VALUES_PER_SAMPLE * 2);

		for(int t=0; t<PREVIEW_SIZE; t++){
			ng_gl2_set_(section, dest + lod_max[mm_level] + t, short_to_char( preview->data[c][t].positive));
			ng_gl2_set_(section, dest + lod_min[mm_level] + t, short_to_char(-preview->data[c][t].negative));
		}

		other_lods(renderer, section, dest);
	}

	// send to gpu

	if (!section->texture) {
		const int s = 0;
		section->texture = texture_cache_assign_new(GL_TEXTURE_2D, (WaveformBlock){waveform, (s * MAX_BLOCKS_PER_TEXTURE) | (renderer->mode == MODE_HI ? WF_TEXTURE_CACHE_HIRES_NG_MASK : 0)});
	}

	int width = PREVIEW_SIZE;
	int height = section->buffer_size / width;
	#define pixel_format GL_ALPHA
	agl_use_texture (section->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	dbg(1, "uploading texture: %i (%i x %i)", section->texture, width, height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, section->buffer);
}


static bool
preview_pre_render (Renderer* renderer, WaveformActor* actor)
{
	Waveform* w = actor->waveform;
	WfActorPriv* _a = actor->priv;

	if (!preview_renderer.renderer.shader)
		preview_renderer_init(actor);

	AGlShader* shader = renderer->shader;
	agl_use_program(shader);
	HiResNGWaveform** data = (HiResNGWaveform**)&w->priv->preview->render_data;
	if (!(*data)) preview_make_data(renderer, actor);
	Section* section = &(*data)->section[0];

	AGlUniformUnion* u = (AGlUniformUnion*)shader->uniforms;
	u[NG_U_FG_COLOUR].value.i[0] = (((AGlActor*)actor)->colour & 0xffffff00) + (unsigned)(0xff * _a->opacity);
	u[NG_U_TOP].value.f[0] = 0;
	u[NG_U_BOTTOM].value.f[0] = agl_actor__height((AGlActor*)actor);
	shader->uniforms[NG_U_N_CHANNELS].value[0] = w->n_channels;
	shader->uniforms[NG_U_TEX_WIDTH].value[0] = PREVIEW_SIZE;
	shader->uniforms[NG_U_TEX_HEIGHT].value[0] = section->buffer_size / PREVIEW_SIZE;
	shader->uniforms[NG_U_VGAIN].value[0] = actor->context->v_gain;

	int width = agl_actor__width((AGlActor*)actor);
	u[NG_U_MM_LEVEL].value.i[0] = width > 320 ? 0 : width > 160 ? 1 : 2;

	agl_translate(shader, 0, 0);
	shader->set_uniforms_(shader);

	glActiveTexture (GL_TEXTURE0);
	glBindBuffer (GL_ARRAY_BUFFER, agl->vbo);

	glEnableVertexAttribArray (0);
	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	return true;
}


static bool
preview_render (Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x)
{
	gl_warn("pre");

	Waveform* waveform = actor->waveform;

	HiResNGWaveform** data = (HiResNGWaveform**)&waveform->priv->preview->render_data;
	if (!(*data)) preview_make_data(renderer, actor);

	Section* section = &(*data)->section[0];

	TextureRange tex;
	WfSampleRegionf block;
	if (!wf_actor_get_quad_dimensions(actor, b, is_first, is_last, x, &tex, &block.start, &block.len, 0, 1)) return false;

	AGlQuad tex_rect = {tex.start, 0., tex.end, 0.001}; // the 0.001 prevents the wrong block being shown on some systems

	dbg(1, "t=%u x=%f-->%f", section->texture, tex.start, tex.end);

	agl_textured_rect_fast (section->texture, 0, 0, agl_actor__width((AGlActor*)actor), agl_actor__height((AGlActor*)actor), &tex_rect);

	return true;
}


static void
preview_renderer_update (Renderer* renderer, WaveformActor* actor, int x1, int x2)
{
	Waveform* waveform = actor->waveform;
	WaveformPrivate* w = waveform->priv;
	WfPreview* preview = w->preview;

	HiResNGWaveform** data = (HiResNGWaveform**)&preview->render_data;
	if (!(*data)) preview_make_data(renderer, actor);
	Section* section = &(*data)->section[0];

	int mm_level = 0;
	int* lod_max = ((NGRenderer*)renderer)->mmidx_max;
	int* lod_min = ((NGRenderer*)renderer)->mmidx_min;

	agl_use_texture (section->texture);

	#define ROWS_PER_CHANNEL 2
	for (int c=0;c<waveform->n_channels;c++) {
		int dest = (c * PREVIEW_SIZE * WF_PEAK_VALUES_PER_SAMPLE * ROWS_PER_CHANNEL);

		for (int t=x1; t<x2; t++) {
			ng_gl2_set_(section, dest + lod_max[mm_level] + t, short_to_char( preview->data[c][t].positive));
			ng_gl2_set_(section, dest + lod_min[mm_level] + t, short_to_char(-preview->data[c][t].negative));
		}

		other_lods(renderer, section, dest);

		WfPeak start = {
			dest + lod_max[mm_level] + x1,
			dest + lod_min[mm_level] + x1,
		};
		int len = x2 - x1;
		AGliPt t = { start.positive % PREVIEW_SIZE, start.positive / PREVIEW_SIZE };

		// mm level 0
		glTexSubImage2D(GL_TEXTURE_2D, 0, t.x, t.y    , len, 1, GL_ALPHA, GL_UNSIGNED_BYTE, &section->buffer[start.positive]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, t.x, t.y + 2, len, 1, GL_ALPHA, GL_UNSIGNED_BYTE, &section->buffer[start.negative]);

		// mm level 1
		glTexSubImage2D(GL_TEXTURE_2D, 0, t.x/2, t.y + 1, len, 1, GL_ALPHA, GL_UNSIGNED_BYTE, &section->buffer[dest + lod_max[1] + x1 / 2]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, t.x/2, t.y + 3, len, 1, GL_ALPHA, GL_UNSIGNED_BYTE, &section->buffer[dest + lod_min[1] + x1 / 2]);
	}
}


NGRenderer preview_renderer = {{0, preview_renderer_init, NULL, preview_pre_render, preview_render, ng_post_render, ng_free_waveform,
	.texture_size = PREVIEW_SIZE
}};
