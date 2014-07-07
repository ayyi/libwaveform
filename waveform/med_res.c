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


// temporary - performance testing
static void
use_texture_no_blend(GLuint texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glDisable(GL_BLEND);
}


static inline void
_set_gl_state_for_block(WaveformCanvas* wfc, Waveform* w, WfGlBlock* textures, int b, WfColourFloat fg, float alpha)
{
	g_return_if_fail(b < textures->size);

	if(wfc->use_1d_textures){
		int c = 0;
		//dbg(0, "tex=%i", textures->peak_texture[c].main[b]);
		//dbg(0, "%i %i %i", textures->peak_texture[c].main[0], textures->peak_texture[c].main[1], textures->peak_texture[c].main[2]);
		agl_texture_unit_use_texture(wfc->texture_unit[0], textures->peak_texture[c].main[b]);
		agl_texture_unit_use_texture(wfc->texture_unit[1], textures->peak_texture[c].neg[b]);

		if(w->textures->peak_texture[WF_RIGHT].main){
			agl_texture_unit_use_texture(wfc->texture_unit[2], textures->peak_texture[WF_RIGHT].main[b]);
			agl_texture_unit_use_texture(wfc->texture_unit[3], textures->peak_texture[WF_RIGHT].neg[b]);
		}

		glActiveTexture(WF_TEXTURE0);
	}else
		agl_use_texture(textures->peak_texture[0].main[b]);

	gl_warn("cannot bind texture: block=%i: %i", b, textures->peak_texture[0].main[b]);

	glColor4f(fg.r, fg.g, fg.b, alpha); //seems we have to set colour _after_ binding... ?

	gl_warn("gl error");
}


static void
med_lo_pre_render(Renderer* renderer, WaveformActor* actor)
{
	WaveformCanvas* wfc = actor->canvas;
	WfCanvasPriv* _c = wfc->priv;
	WfActorPriv* _a = actor->priv;
	RenderInfo* r  = &actor->priv->render_info;

#if defined (USE_FBO) && defined (multipass)
	if(agl->use_shaders){
			//set gl state
#ifdef USE_FX
			BloomShader* shader = _c->shaders.horizontal;
			shader->uniform.fg_colour = (actor->fg_colour & 0xffffff00) + MIN(0xff, 0x100 * _a->animatable.opacity.val.f);
			//shader->uniform.peaks_per_pixel = get_peaks_per_pixel(wfc, &r->region, &r->rect, r->mode) / 1.0;
			shader->uniform.peaks_per_pixel = r->peaks_per_pixel;
			agl_use_program(&shader->shader);
#else
			BloomShader* shader = wfc->priv->shaders.vertical;
			wfc->priv->shaders.vertical->uniform.fg_colour = (actor->fg_colour & 0xffffff00) + MIN(0xff, 0x100 * _a->animatable.opacity.val.f);
			wfc->priv->shaders.vertical->uniform.peaks_per_pixel = r->peaks_per_pixel_i;
			//TODO the vertical shader needs to check _all_ the available texture values to get true peak.
			agl_use_program(&shader->shader);
#endif // end USE_FX

					glDisable(GL_TEXTURE_1D);
					glEnable(GL_TEXTURE_2D);
					glActiveTexture(GL_TEXTURE0);
	}
#else
	if(wfc->use_1d_textures){
			agl_use_program((AGlShader*)wfc->priv->shaders.peak);

			//uniforms: (must be done on each paint because some vars are actor-specific)
			dbg(2, "vpwidth=%.2f region_len=%i region_n_peaks=%.2f peaks_per_pixel=%.2f", (viewport.right - viewport.left), r->region.len, ((float)r->region.len / WF_PEAK_TEXTURE_SIZE), r->peaks_per_pixel_i);
			float bottom = rect.top + rect.height;
			int n_channels = r->textures->peak_texture[WF_RIGHT].main ? 2 : 1;
			wfc->priv->shaders.peak->set_uniforms(r->peaks_per_pixel_i, rect.top, bottom, actor->fg_colour, n_channels);
	}
#endif // end FBO && multipass

	//check textures are loaded
	if(wf_debug > 1){ //textures may initially not be loaded, so don't show this warning too much
		int n = 0;
		int b; for(b=r->viewport_start_block;b<=r->viewport_end_block;b++){
			if(!r->textures->peak_texture[WF_LEFT].main[b]){ n++; gwarn("texture not loaded: b=%i", b); }
		}
		if(n) gwarn("%i textures not loaded", n);
	}

#if 0 // no longer needed - is done by agl_use_texture().
	glEnable(wfc->use_1d_textures ? GL_TEXTURE_1D : GL_TEXTURE_2D);
#endif
}


static bool
actor_render_med_lo(Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x)
{
	Waveform* w = actor->waveform; 
	WfActorPriv* _a = actor->priv;
	WaveformCanvas* wfc = actor->canvas;
	RenderInfo* r  = &_a->render_info;

	WfColourFloat fg;
	wf_colour_rgba_to_float(&fg, actor->fg_colour);
	WfColourFloat bg;
	wf_colour_rgba_to_float(&bg, actor->bg_colour);

	float alpha = _a->animatable.opacity.val.f;

					// this appears to not be using border offsetting. may need to copy from get_quad_dimensions() in hi_res.c

					;double block_wid = r->block_wid;
					double tex_pct = 1.0; //use the whole texture
					double tex_start = 0.0;
					if (is_first){
						if(r->first_offset) tex_pct = 1.0 - ((double)r->first_offset) / r->samples_per_texture;
						block_wid = r->block_wid * tex_pct;
						tex_start = 1 - tex_pct;
						dbg(3, "rect.left=%.2f region->start=%i first_offset=%i", r->rect.left, r->region.start, r->first_offset);
					}
					if (is_last){
						//if(x + r->block_wid < x0 + rect->len){
						if(b < r->region_end_block){
							//end is offscreen. last block is not smaller.
						}else{
							//end is trimmed
#if 0
							block_wid = rect->len - x0 - r->block_wid * (i - viewport_start_block); //last block is smaller
#else
							double part_inset_px = wf_actor_samples2gl(r->zoom, r->region.start);
							//double file_start_px = rect->left - part_inset_px;
							double distance_from_file_start_to_region_end = part_inset_px + r->rect.len;
							block_wid = distance_from_file_start_to_region_end - b * r->block_wid;
							dbg(3, " %i: inset=%.2f s->e=%.2f i*b=%.2f", b, part_inset_px, distance_from_file_start_to_region_end, b * r->block_wid);
							if(b * r->block_wid > distance_from_file_start_to_region_end){ gwarn("!!"); return false; }
#endif
						}

						if(b == w->textures->size - 1) dbg(2, "last sample block. fraction=%.2f", w->textures->last_fraction);
						//TODO check what happens here if we allow non-square textures
#if 0 // !! doesnt matter if its the last block or not, we must still take the region into account.
						tex_pct = (i == w->textures->size - 1)
							? (block_wid / r->block_wid) / w->textures->last_fraction    // block is at end of sample
															  // -cannot use block_wid / block_wid for last block because the last texture is smaller.
							: block_wid / r->block_wid;
#else
						tex_pct = block_wid / r->block_wid;
#endif
					}

					dbg (3, "%i: is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex_start=%.2f", b, is_last, x, block_wid, r->block_wid, tex_pct, tex_start);
					if(tex_pct > 1.0 || tex_pct < 0.0) gwarn("tex_pct! %.2f", tex_pct);
					double tex_x = x + ((is_first && r->first_offset) ? r->first_offset_px : 0);

#if defined (USE_FBO) && defined (multipass)
					if(agl->use_shaders){
						//rendering from 2d texture not 1d
						AglFBO* fbo = false
							? fbo_test
#ifdef USE_FX
							: r->textures->fx_fbo[b]
								? r->textures->fx_fbo[b]
#endif
								: r->textures->fbo[b];
						if(fbo){ //seems that the fbo may not be created initially...
							if(wfc->blend)
								agl_use_texture(fbo->texture);
							else
								use_texture_no_blend(fbo->texture);
						}
					}else{
						_set_gl_state_for_block(wfc, w, r->textures, b, fg, alpha);
					}
#else
					_set_gl_state_for_block(wfc, w, r->textures, b, fg, alpha);
#endif

					glPushMatrix();
					glTranslatef(0, 0, _a->animatable.z.val.f);
					glBegin(GL_QUADS);
#if defined (USE_FBO) && defined (multipass)
					if(false){
#else
					if(wfc->use_1d_textures){
#endif
						_draw_block_from_1d(tex_start, tex_pct, tex_x, r->rect.top, block_wid, r->rect.height, modes[r->mode].texture_size);
					}else{
						_draw_block(tex_start, tex_pct, tex_x, r->rect.top, block_wid, r->rect.height, wfc->v_gain);
					}
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


Renderer med_lo_renderer = {med_lo_pre_render, actor_render_med_lo};

