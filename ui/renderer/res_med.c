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

extern HiResNGShader hires_ng_shader;

typedef HiResNGWaveform NGWaveform;

#define RENDER_DATA_MED(A) ((NGWaveform*)A->render_data[MODE_MED])


static void
wf_texture_array_add_ch(WfGlBlock* textures, int c)
{
	dbg(2, "adding glbocks: c=%i num_blocks=%i", c, textures->size);
	unsigned* a = g_new0(unsigned, textures->size * 2); // single allocation for both pos and neg
	textures->peak_texture[c].main = a;
	textures->peak_texture[c].neg  = a + textures->size;
}


static WfGlBlock*
wf_texture_array_new(int size, int n_channels)
{
	WfGlBlock* textures = g_new0(WfGlBlock, 1);
	textures->size = size;
	dbg(1, "creating glbocks: num_blocks=%i n_ch=%i", textures->size, n_channels);
	int c; for(c=0;c<n_channels;c++){
		wf_texture_array_add_ch(textures, c);
	}
	textures->fbo = g_new0(AGlFBO*, textures->size); //note: only an array of _pointers_
#ifdef USE_FX
	textures->fx_fbo = g_malloc0(sizeof(AGlFBO*) * textures->size);
#endif
	return textures;
}


static void
med_renderer_new_gl2(WaveformActor* actor)
{
	AGlShader** shader = &((NGRenderer*)modes[MODE_MED].renderer)->shader;
	if(!*shader){
		*shader = &hires_ng_shader.shader;
		if(!(*shader)->program) agl_create_program(*shader);
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


static void
med_allocate_block_gl1(Renderer* renderer, WaveformActor* a, int b)
{
	// load resources (textures) required for display of the block.

	g_return_if_fail(b >= 0);

	Waveform* w = a->waveform;
	WaveformPrivate* _w = w->priv;
	WfGlBlock* blocks = (WfGlBlock*)(_w->render_data[MODE_MED]
		?  _w->render_data[MODE_MED]
		: (_w->render_data[MODE_MED] = (WaveformModeRender*)wf_texture_array_new(_w->n_blocks, w->n_channels)));
	WaveformBlock wb = {w, b};

	int c = WF_LEFT;
	if(blocks->peak_texture[c].main[b]
#ifdef WF_DEBUG
		&& glIsTexture(blocks->peak_texture[c].main[b])
#endif
	){
		texture_cache_freshen(GL_TEXTURE_2D, wb);
		return;
	}

	int texture_id = blocks->peak_texture[c].main[b] = texture_cache_assign_new(a->context->use_1d_textures ? GL_TEXTURE_1D : GL_TEXTURE_2D, wb);

	if(a->context->use_1d_textures){
		g_assert("NEVER GET HERE");
#if 0
		guint* peak_texture[4] = {
			&blocks->peak_texture[WF_LEFT ].main[b],
			&blocks->peak_texture[WF_LEFT ].neg[b],
			&blocks->peak_texture[WF_RIGHT].main[b],
			&blocks->peak_texture[WF_RIGHT].neg[b]
		};
		blocks->peak_texture[c].neg[b] = texture_cache_assign_new (GL_TEXTURE_1D, wb);

		if(a->waveform->priv->peak.buf[WF_RIGHT]){
			int i; for(i=2;i<4;i++){
				//g_return_if_fail(peak_texture[i]);
				*peak_texture[i] = texture_cache_assign_new (GL_TEXTURE_1D, (WaveformBlock){a->waveform, b});
			}
			dbg(1, "rhs: %i: texture=%i %i %i %i", b, *peak_texture[0], *peak_texture[1], *peak_texture[2], *peak_texture[3]);
		}else{
			dbg(1, "* %i: texture=%i %i (mono)", b, texture_id, *peak_texture[1]);
		}

		wf_actor_load_texture1d(w, MODE_MED, blocks, b);

#if defined (USE_FBO) && defined (multipass)
		if(!blocks->fbo[b]){
			block_to_fbo(a, b, blocks, 256);
			if(blocks->fbo[b]){
				// original texture no longer needed
				med_lo_clear_1d_textures(blocks, &wb);
			}
		}
#endif
#endif
	}else{
		wf_actor_load_texture2d(a, MODE_MED, texture_id, b);
	}

	gl_warn("(end)");
}


static void
low_allocate_block_gl1(Renderer* renderer, WaveformActor* a, int b)
{
	g_return_if_fail(b >= 0);

	Waveform* w = a->waveform;
	WfGlBlock* blocks = (WfGlBlock*)w->priv->render_data[renderer->mode];
	if(!blocks) return;
#ifdef WF_DEBUG
	g_return_if_fail(b < blocks->size);
#endif

	WaveformBlock wb = {w, b | (renderer->mode == MODE_LOW ? WF_TEXTURE_CACHE_LORES_MASK : WF_TEXTURE_CACHE_V_LORES_MASK)};

#ifdef USE_FBO
	if(blocks->fbo[b]) return;
#endif

	int c = WF_LEFT;
	if(blocks->peak_texture[c].main[b]
#ifdef WF_DEBUG
		&& glIsTexture(blocks->peak_texture[c].main[b])
#endif
	){
		//pwarn("waveform low-res texture already assigned for block %i: %i", b, blocks->peak_texture[c].main[b]);
		texture_cache_freshen(GL_TEXTURE_2D, wb);
		return;
	}

	int texture_id = blocks->peak_texture[c].main[b] = texture_cache_assign_new(GL_TEXTURE_2D, wb);

	dbg(1, "* %i: texture=%i", b, texture_id);
	AlphaBuf* alphabuf = wf_alphabuf_new(w, b, (renderer->mode == MODE_LOW ? WF_PEAK_STD_TO_LO : WF_MED_TO_V_LOW), false, TEX_BORDER);
	wf_canvas_load_texture_from_alphabuf(a->context, texture_id, alphabuf);
	wf_alphabuf_free(alphabuf);
}


static inline void
_med_lo_set_gl_state_for_block(WaveformContext* wfc, Waveform* w, WfGlBlock* textures, int b)
{
	g_return_if_fail(b < textures->size);

	if(wfc->use_1d_textures){
		g_assert("NEVER GET HERE");
		int c = 0;
		agl_texture_unit_use_texture(wfc->texture_unit[0], textures->peak_texture[c].main[b]);
		agl_texture_unit_use_texture(wfc->texture_unit[1], textures->peak_texture[c].neg[b]);

		if(textures->peak_texture[WF_RIGHT].main){
			agl_texture_unit_use_texture(wfc->texture_unit[2], textures->peak_texture[WF_RIGHT].main[b]);
			agl_texture_unit_use_texture(wfc->texture_unit[3], textures->peak_texture[WF_RIGHT].neg[b]);
		}

		glActiveTexture(WF_TEXTURE0);
	}else
		agl_use_texture(textures->peak_texture[0].main[b]);

	gl_warn("cannot bind texture: block=%i: %i", b, textures->peak_texture[0].main[b]);
}


static bool
med_lo_pre_render_gl1(Renderer* renderer, WaveformActor* actor)
{
	WfActorPriv* _a = actor->priv;

	agl_enable(AGL_ENABLE_BLEND); // TODO find out why GL_TEXTURE_2D needs to be flipped here.
	agl_enable(AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);

	AGlColourFloat fg; wf_colour_rgba_to_float(&fg, actor->fg_colour);

	glColor4f(fg.r, fg.g, fg.b, _a->opacity);

	return true;
}


static inline void
_draw_block_from_1d(float tex_start, float tex_pct, float x, float y, float width, float height, int t_size)
{
	// used by both med and hi_res modes.

	if(t_size > 256){
		// MODE_HI
		// no longer do any offsetting here - do the same for non MODE_HI also.
	}else{
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


static bool
med_lo_get_quad_dimensions(WaveformActor* actor, int b, bool is_first, bool is_last, double x, TextureRange* tex_, QuadExtent* qe_)
{
	// TODO this appears to not be using border offsetting. may need to copy from get_quad_dimensions() in hi_res.c

	WfActorPriv* _a = actor->priv;
	RenderInfo* r  = &_a->render_info;

#if 0 //TODO
	int border = TEX_BORDER;
	double usable_pct = (modes[r->mode].texture_size - 2.0 * border) / modes[r->mode].texture_size;
	double border_pct = (1.0 - usable_pct)/2;
#endif

	QuadExtent tex = {0.0, 1.0}; // use the whole texture
#ifdef RECT_ROUNDING
	QuadExtent quad = {x, round(r->block_wid)};
#else
	QuadExtent quad = {x, r->block_wid};
#endif

	if (is_first){
		tex.wid = 1.0 - ((double)r->first_offset) / r->samples_per_texture;
#ifdef RECT_ROUNDING
		quad.wid = round(r->block_wid * tex.wid); // this can be off by a pixel
#else
		quad.wid = r->block_wid * tex.wid;
#endif
		tex.start = 1.0 - tex.wid;
		dbg(3, "rect.left=%.2f region->start=%"PRIi64" first_offset=%i", r->rect.left, r->region.start, r->first_offset);
 		//if(r->first_offset) quad.start += r->first_offset_px;
		quad.start = x + r->block_wid - quad.wid; // align the block end to the start of the following one
	}

	if (is_last){
		if(b < r->region_end_block){
			//end is offscreen. last block is not smaller.
		}else{
			//end is trimmed
			double part_inset_px = wf_actor_samples2gl(r->zoom, r->region.start);
			double region_len_px = wf_actor_samples2gl(r->zoom, r->region.len);
																// TODO apply change to other renderers
			// note region may be smaller that the rect during transitions
			double distance_from_file_start_to_region_end = part_inset_px + MIN(r->rect.len, region_len_px);
			//                                                                               ^ does this one work in all cases?
			quad.wid = distance_from_file_start_to_region_end - b * r->block_wid;
#ifdef RECT_ROUNDING
			quad.wid = round(quad.wid);
#else
			dbg(3, " %i: inset=%.2f s->e=%.2f i*b=%.2f", b, part_inset_px, distance_from_file_start_to_region_end, b * r->block_wid);
			if(b * r->block_wid > distance_from_file_start_to_region_end){ pwarn("end error! %.2f %.2f", b * r->block_wid, distance_from_file_start_to_region_end); return false; }
#endif
		}

		tex.wid = quad.wid / r->block_wid;
	}

#if defined (USE_FBO) && defined (multipass)
	if(agl->use_shaders){
		tex.wid = MIN(0.995 - tex.start, tex.wid); // TODO remove
	}
#endif

	dbg (3, "%i: is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex.start=%.2f", b, is_last, x, quad.wid, r->block_wid, tex.wid, tex.start);
	if(tex.wid < 0.0 || tex.start + tex.wid > 1.0000001) pwarn("tex_pct out of range: %f %.20f", tex.wid, tex.start + tex.wid);

	*tex_ = (TextureRange){tex.start, tex.start + tex.wid};
	*qe_ = quad;
	return true;
}


static bool
med_lo_render_gl1(Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x)
{
	Waveform* w = actor->waveform; 
	WfActorPriv* _a = actor->priv;
	WaveformContext* wfc = actor->context;
	RenderInfo* r  = &_a->render_info;

	TextureRange tex;
	QuadExtent block;
	if(!med_lo_get_quad_dimensions(actor, b, is_first, is_last, x, &tex, &block)) return false;

	_med_lo_set_gl_state_for_block(wfc, w, (WfGlBlock*)w->priv->render_data[r->mode], b);

	glPushMatrix();
	glTranslatef(0, 0, actor->z);
	glBegin(GL_QUADS);
	_draw_block(tex.start, tex.end - tex.start, block.start, r->rect.top, block.wid, r->rect.height, wfc->v_gain);
	glEnd();
	glPopMatrix();
	gl_warn("block=%i", b);

#if 0
#ifdef WF_SHOW_RMS
	double bot = rect.top + rect.height;
	double top = rect->top;
	if(wfc->show_rms && w->textures->rms_texture){
		glBindTexture(GL_TEXTURE_2D, w->textures->rms_texture[b]);
#if 0
		if(!glIsTexture(w->textures->rms_texture[i])) pwarn ("texture not loaded. block=%i", i);
#endif
		AglColourFloat bg;
		wf_colour_rgba_to_float(&bg, actor->bg_colour);

		//note seems we have to do this after binding...
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
		glColor4f(bg.r, bg.g, bg.b, 0.5);

		dbg (2, "rms: %i: is_last=%i x=%.2f wid=%.2f tex_pct=%.2f", i, is_last, x, block_wid, tex_pct);
		glBegin(GL_QUADS);
		glTexCoord2d(tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       top);
		glTexCoord2d(tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, top);
		glTexCoord2d(tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, bot);
		glTexCoord2d(tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       bot);
		glEnd();
	}
#endif
#endif
	return true;
}


static void
med_lo_gl1_free_waveform(Renderer* renderer, Waveform* waveform)
{
	dbg(1, "%s", modes[renderer->mode].name);

	WaveformPrivate* w = waveform->priv;
	texture_cache_remove_waveform(waveform);

	WfGlBlock* textures = (WfGlBlock*)w->render_data[renderer->mode];
	if(textures){
		int c; for(c=0;c<WF_MAX_CH;c++){
			if(textures->peak_texture[c].main) g_free(textures->peak_texture[c].main); // pos and neg are a single allocation
		}
#ifdef USE_FBO
		if(textures->fbo){
			int b; for(b=0;b<textures->size;b++) if(textures->fbo[b]) agl_fbo_free(textures->fbo[b]);
			g_free(textures->fbo);
		}
#ifdef USE_FX
		if(textures->fx_fbo){
			int b; for(b=0;b<textures->size;b++) if(textures->fx_fbo[b]) agl_fbo_free(textures->fx_fbo[b]);
			g_free(textures->fx_fbo);
		}
#endif
#endif
		g_free(textures);

		w->render_data[renderer->mode] = NULL;
	}
}


void
med_lo_on_steal(WaveformBlock* wb, guint tex)
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
	if((p = find_texture_in_block(tex, wb, peak_texture)) < 0){
		pwarn("texture not found");
	}else{
		dbg(1, "clearing texture for block=%i %i ...", wb->block, p);
		*peak_texture[p] = 0;
	}
}


Renderer med_renderer_gl1 = {MODE_MED, NULL, med_allocate_block_gl1, med_lo_pre_render_gl1, med_lo_render_gl1, med_lo_gl1_free_waveform};
NGRenderer med_renderer_gl2 = {{MODE_MED, med_renderer_new_gl2, ng_gl2_load_block, ng_gl2_pre_render, ng_gl2_render_block, ng_gl2_free_waveform}};


static Renderer*
med_renderer_new()
{
	g_return_val_if_fail(!med_renderer_gl2.ng_data, NULL);

	static Renderer* med_renderer = (Renderer*)&med_renderer_gl2;

	med_renderer_gl2.ng_data = g_hash_table_new_full(g_direct_hash, g_int_equal, NULL, g_free);

	ng_make_lod_levels(&med_renderer_gl2, MODE_MED);

	return (Renderer*)med_renderer;
}


