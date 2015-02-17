/*
  copyright (C) 2012-2014 Tim Orford <tim@orford.org>

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

typedef HiResNGWaveform NGWaveform;

#define RENDER_DATA_MED(A) ((NGWaveform*)A->render_data[MODE_MED])


static void
med_renderer_new_gl2(WaveformActor* actor)
{
	// nothing needed as is done in ng_renderer.load_block
}


static void
low_new_gl2(WaveformActor* actor)
{
	WaveformPriv* w = actor->waveform->priv;

	g_return_if_fail(!w->render_data[MODE_V_HI]);

}


// temporary - performance testing
static void
use_texture_no_blend(GLuint texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glDisable(GL_BLEND);
}


static void
med_lo_clear_1d_textures(WfGlBlock* blocks, WaveformBlock* wb)
{
	texture_cache_remove(GL_TEXTURE_1D, wb->waveform, wb->block);
	int b = wb->block & ~WF_TEXTURE_CACHE_LORES_MASK;
	int c; for(c=0;c<WF_RIGHT;c++) blocks->peak_texture[c].main[b] = blocks->peak_texture[c].neg[b] = 0;
}


static void
med_allocate_block_gl1(Renderer* renderer, WaveformActor* a, int b)
{
	// load resources (textures) required for display of the block.

	g_return_if_fail(b >= 0);

	Waveform* w = a->waveform;
	WaveformPriv* _w = w->priv;
	WfGlBlock* blocks = (WfGlBlock*)(_w->render_data[MODE_MED]
		?  _w->render_data[MODE_MED]
		: (_w->render_data[MODE_MED] = (WaveformModeRender*)wf_texture_array_new(_w->n_blocks, w->n_channels)));
	WaveformBlock wb = {w, b};

	int c = WF_LEFT;
	if(blocks->peak_texture[c].main[b]){
		if(glIsTexture(blocks->peak_texture[c].main[b])){
			//gwarn("waveform texture already assigned for block %i: %i", b, blocks->peak_texture[c].main[b]);
			return;
		}else{
			//if we get here something has gone badly wrong. Most likely unrecoverable.
			gwarn("removing invalid texture...");
			med_lo_clear_1d_textures(blocks, &wb);
		}
	}

	int texture_id = blocks->peak_texture[c].main[b] = texture_cache_assign_new(a->canvas->use_1d_textures ? GL_TEXTURE_1D : GL_TEXTURE_2D, wb);

	if(a->canvas->use_1d_textures){
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
	}else{
		wf_actor_load_texture2d(a, MODE_MED, texture_id, b);
	}

	gl_warn("(end)");
}


static void
low_allocate_block(Renderer* renderer, WaveformActor* a, int b)
{
	g_return_if_fail(b >= 0);

	Waveform* w = a->waveform;
	WfGlBlock* blocks = (WfGlBlock*)w->priv->render_data[renderer->mode];
	if(!blocks) return;
#ifdef WF_DEBUG
	g_return_if_fail(b < blocks->size);
#endif

	int texture_type = a->canvas->use_1d_textures ? GL_TEXTURE_1D : GL_TEXTURE_2D;
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
		//gwarn("waveform low-res texture already assigned for block %i: %i", b, blocks->peak_texture[c].main[b]);
		texture_cache_freshen(texture_type, wb);
																				// TODO do the same for MED
		return;
	}

	int texture_id = blocks->peak_texture[c].main[b] = texture_cache_assign_new(texture_type, wb);

	if(a->canvas->use_1d_textures){
		guint* peak_texture[4] = {
			&blocks->peak_texture[WF_LEFT ].main[b],
			&blocks->peak_texture[WF_LEFT ].neg[b],
			&blocks->peak_texture[WF_RIGHT].main[b],
			&blocks->peak_texture[WF_RIGHT].neg[b]
		};
		blocks->peak_texture[c].neg[b] = texture_cache_assign_new (GL_TEXTURE_1D, wb);

		if(a->waveform->priv->peak.buf[WF_RIGHT]){
			if(peak_texture[2]){
				int i; for(i=2;i<4;i++){
					*peak_texture[i] = texture_cache_assign_new (GL_TEXTURE_1D, wb);
				}
				dbg(1, "rhs: %i: texture=%i %i %i %i", b, *peak_texture[0], *peak_texture[1], *peak_texture[2], *peak_texture[3]);
			}
		}else{
			dbg(1, "* %i: textures=%i,%i (rhs peak.buf not loaded)", b, texture_id, *peak_texture[1]);
		}

		wf_actor_load_texture1d(w, MODE_LOW, (WfGlBlock*)w->priv->render_data[renderer->mode], b);
#if defined (USE_FBO) && defined (multipass)
		if(!blocks->fbo[b]){
			block_to_fbo(a, b, blocks, 1024);
			if(blocks->fbo[b]){
				// original texture no longer needed
				med_lo_clear_1d_textures(blocks, &wb);
			}
		}
#endif
	}else{
		dbg(1, "* %i: texture=%i", b, texture_id);
		AlphaBuf* alphabuf = wf_alphabuf_new(w, b, (renderer->mode == MODE_LOW ? WF_PEAK_STD_TO_LO : WF_MED_TO_V_LOW), false, TEX_BORDER);
		wf_canvas_load_texture_from_alphabuf(a->canvas, texture_id, alphabuf);
		wf_alphabuf_free(alphabuf);
	}
}


static inline void
_med_lo_set_gl_state_for_block(WaveformCanvas* wfc, Waveform* w, WfGlBlock* textures, int b)
{
	g_return_if_fail(b < textures->size);

	if(wfc->use_1d_textures){
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


static void
lo_pre_render_gl2(Renderer* renderer, WaveformActor* actor)
{
	WaveformCanvas* wfc = actor->canvas;
	WfCanvasPriv* _c = wfc->priv;
#if defined (USE_FBO) && defined (multipass)
	WfActorPriv* _a = actor->priv;
#endif
	RenderInfo* r = &actor->priv->render_info;

#if defined (USE_FBO) && defined (multipass)
#ifdef USE_FX
	BloomShader* shader = _c->shaders.horizontal;
	shader->uniform.fg_colour = (actor->fg_colour & 0xffffff00) + MIN(0xff, 0x100 * _a->animatable.opacity.val.f);
	//shader->uniform.peaks_per_pixel = get_peaks_per_pixel(wfc, &r->region, &r->rect, r->mode) / 1.0;
	shader->uniform.peaks_per_pixel = r->peaks_per_pixel;
	agl_use_program(&shader->shader);
#else
	BloomShader* shader = wfc->priv->shaders.vertical;
	_c->shaders.vertical->uniform.fg_colour = (actor->fg_colour & 0xffffff00) + MIN(0xff, 0x100 * _a->animatable.opacity.val.f);
	_c->shaders.vertical->uniform.peaks_per_pixel = r->peaks_per_pixel_i;
	//TODO the vertical shader needs to check _all_ the available texture values to get true peak.
	agl_use_program(&shader->shader);
#endif // end USE_FX

					glDisable(GL_TEXTURE_1D);
					glEnable(GL_TEXTURE_2D);
					glActiveTexture(GL_TEXTURE0);
#else
	if(wfc->use_1d_textures){
		agl_use_program((AGlShader*)_c->shaders.peak);

		//uniforms: (must be done on each paint because some vars are actor-specific)
		dbg(2, "vpwidth=%.2f region_len=%i region_n_peaks=%.2f peaks_per_pixel=%.2f", (r->viewport.right - r->viewport.left), r->region.len, ((float)r->region.len / WF_PEAK_TEXTURE_SIZE), r->peaks_per_pixel_i);
		int n_channels = r->textures->peak_texture[WF_RIGHT].main ? 2 : 1;
		_c->shaders.peak->set_uniforms(r->peaks_per_pixel_i, r->rect.top, r->rect.top + r->rect.height, actor->fg_colour, n_channels);
	}
#endif // end FBO && multipass

	//check textures are loaded
	if(wf_debug > 1){ //textures may initially not be loaded, so don't show this warning too much
		int n = 0;
		int b; for(b=r->viewport_blocks.first;b<=r->viewport_blocks.last;b++){
#if defined (USE_FBO) && defined (multipass)
			if(!((WfGlBlock*)actor->waveform->priv->render_data[MODE_LOW])->fbo[b]->texture)
#else
			if(!((WfGlBlock*)actor->waveform->priv->render_data[MODE_LOW])->peak_texture[WF_LEFT].main[b])
#endif
				{ n++; gwarn("texture not loaded: b=%i", b); }
		}
		if(n) gwarn("%i textures not loaded", n);
	}
}


static void
med_lo_pre_render_gl1(Renderer* renderer, WaveformActor* actor)
{
	WfActorPriv* _a = actor->priv;

	glEnable(GL_TEXTURE_2D);

	AGlColourFloat fg; wf_colour_rgba_to_float(&fg, actor->fg_colour);

	glColor4f(fg.r, fg.g, fg.b, _a->animatable.opacity.val.f);
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

	double block_wid = r->block_wid;
#ifdef RECT_ROUNDING
	block_wid = round(block_wid);
#endif
	double tex_x = x;
	QuadExtent tex = {0.0, 1.0}; // use the whole texture

	if (is_first){
		tex.wid = 1.0 - ((double)r->first_offset) / r->samples_per_texture;
#ifdef RECT_ROUNDING
		block_wid = round(r->block_wid * tex.wid); // this can be off by a pixel
#else
		block_wid = r->block_wid * tex.wid;
#endif
		tex.start = 1.0 - tex.wid;
		dbg(3, "rect.left=%.2f region->start=%i first_offset=%i", r->rect.left, r->region.start, r->first_offset);
 		//if(r->first_offset) tex_x += r->first_offset_px;
		tex_x = x + r->block_wid - block_wid; // align the block end to the start of the following one
	}

	if (is_last){
		if(b < r->region_end_block){
			//end is offscreen. last block is not smaller.
		}else{
			//end is trimmed
			double part_inset_px = wf_actor_samples2gl(r->zoom, r->region.start);
			//double file_start_px = rect->left - part_inset_px;
			double distance_from_file_start_to_region_end = part_inset_px + r->rect.len;
			block_wid = distance_from_file_start_to_region_end - b * r->block_wid;
#ifdef RECT_ROUNDING
			block_wid = round(block_wid);
#else
			dbg(3, " %i: inset=%.2f s->e=%.2f i*b=%.2f", b, part_inset_px, distance_from_file_start_to_region_end, b * r->block_wid);
			if(b * r->block_wid > distance_from_file_start_to_region_end){ gwarn("end error! %.2f %.2f", b * r->block_wid, distance_from_file_start_to_region_end); return false; }
#endif
		}

		tex.wid = block_wid / r->block_wid;
	}

#if defined (USE_FBO) && defined (multipass)
	if(agl->use_shaders){
		tex.wid = MIN(0.995 - tex.start, tex.wid); // TODO remove
	}
#endif

	dbg (3, "%i: is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex.start=%.2f", b, is_last, x, block_wid, r->block_wid, tex.wid, tex.start);
	if(tex.wid < 0.0 || tex.start + tex.wid > 1.0) gwarn("tex_pct out of range: %f %.20f", tex.wid, tex.start + tex.wid);

	*tex_ = (TextureRange){tex.start, tex.start + tex.wid};
	*qe_ = (QuadExtent){tex_x, block_wid};
	return true;
}


static bool
med_lo_render_gl1(Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x)
{
	Waveform* w = actor->waveform; 
	WfActorPriv* _a = actor->priv;
	WaveformCanvas* wfc = actor->canvas;
	RenderInfo* r  = &_a->render_info;

	TextureRange tex;
	QuadExtent block;
	if(!med_lo_get_quad_dimensions(actor, b, is_first, is_last, x, &tex, &block)) return false;

	_med_lo_set_gl_state_for_block(wfc, w, (WfGlBlock*)w->priv->render_data[r->mode], b);

	glPushMatrix();
	glTranslatef(0, 0, _a->animatable.z.val.f);
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
		if(!glIsTexture(w->textures->rms_texture[i])) gwarn ("texture not loaded. block=%i", i);
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


static bool
lo_render_gl2(Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x)
{
	WfActorPriv* _a = actor->priv;
	WaveformCanvas* wfc = actor->canvas;
	WaveformPriv* w = actor->waveform->priv;
	RenderInfo* r  = &_a->render_info;
	WfGlBlock* textures = (WfGlBlock*)w->render_data[MODE_LOW];

	TextureRange tex;
	QuadExtent block;
	if(!med_lo_get_quad_dimensions(actor, b, is_first, is_last, x, &tex, &block)) return false;


#if defined (USE_FBO) && defined (multipass)
	//rendering from 2d texture not 1d
	AGlFBO* fbo = false
		? fbo_test
#ifdef USE_FX
		: textures->fx_fbo[b]
			? textures->fx_fbo[b]
#endif
			: textures->fbo[b];
	if(fbo){
		if(wfc->blend)
			agl_use_texture(fbo->texture);
		else
			use_texture_no_blend(fbo->texture);
	}else{
		if(wf_debug) gwarn("%i: missing fbo", b);
		return false;
	}
#else
	_med_lo_set_gl_state_for_block(wfc, actor->waveform, textures, b);
#endif

	glPushMatrix();
	glTranslatef(0, 0, _a->animatable.z.val.f);
	glBegin(GL_QUADS);
#if defined (USE_FBO) && defined (multipass)
	_draw_block(tex.start, tex.end - tex.start, block.start, r->rect.top, block.wid, r->rect.height, wfc->v_gain);
#else
	_draw_block_from_1d(tex.start, tex.end - tex.start, block.start, r->rect.top, block.wid, r->rect.height, modes[r->mode].texture_size);
#endif
	glEnd();
	glPopMatrix();
	gl_warn("block=%i", b);

	return true;
}


void
med_lo_on_steal(WaveformBlock* wb, guint tex)
{
	bool lores = wb->block & WF_TEXTURE_CACHE_LORES_MASK;
	int b = wb->block & ~WF_TEXTURE_CACHE_LORES_MASK;
	WfGlBlock* blocks = (WfGlBlock*)wb->waveform->priv->render_data[lores ? MODE_LOW : MODE_MED];
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
		gwarn("texture not found");
	}else{
		dbg(1, "clearing texture for block=%i %i ...", wb->block, p);
		*peak_texture[p] = 0;
	}
}


Renderer med_renderer_gl1 = {MODE_MED, NULL, med_allocate_block_gl1, med_lo_pre_render_gl1, med_lo_render_gl1};
NGRenderer med_renderer_gl2 = {{MODE_MED, med_renderer_new_gl2, ng_gl2_load_block, ng_gl2_pre_render, hi_gl2_render_block, ng_gl2_free_waveform}};

Renderer lo_renderer_gl1 = {MODE_LOW, NULL, low_allocate_block, med_lo_pre_render_gl1, med_lo_render_gl1};
Renderer lo_renderer_gl2 = {MODE_LOW, low_new_gl2, low_allocate_block, lo_pre_render_gl2, lo_render_gl2};
Renderer lo_renderer;

static Renderer*
med_renderer_new()
{
	g_return_val_if_fail(!med_renderer_gl2.ng_data, NULL);

	static Renderer* med_renderer = (Renderer*)&med_renderer_gl2;

	med_renderer_gl2.ng_data = g_hash_table_new_full(g_direct_hash, g_int_equal, NULL, g_free);

	ng_make_lod_levels(&med_renderer_gl2, MODE_MED);

	return (Renderer*)med_renderer;
}


