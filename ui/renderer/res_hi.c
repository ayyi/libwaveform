/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2024 Tim Orford <tim@orford.org>                  |
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

typedef struct
{
	Renderer       renderer;
	WfSampleRegion block_region;
} HiRenderer;

#define ANTIALIASED_LINES

#define RENDER_DATA_HI(W) ((WfTexturesHi*)W->render_data[MODE_HI])


	static void hi_request_block_done(Waveform* w, int b, gpointer _a)
	{
		WaveformActor* a = _a;
		if(w == a->waveform){ // the actor may have a new waveform and there is currently no way to cancel old requests.
			modes[MODE_HI].renderer->load_block(modes[MODE_HI].renderer, a, b);

			//TODO check this block is within current viewport
			if(((AGlActor*)a)->root && ((AGlActor*)a)->root->draw) wf_context_queue_redraw(a->context);
		}
	}

static void
hi_request_block (WaveformActor* a, int b)
{
	waveform_load_audio(a->waveform, b, HI_MIN_TIERS, hi_request_block_done, a);
}


#if 0
static void
make_texture_data_hi(Waveform* w, int ch, IntBufHi* buf, int blocknum)
{
	//data is transformed from the Waveform hi-res peakbuf into IntBufHi* buf.

	dbg(1, "b=%i", blocknum);
	int texture_size = modes[MODE_HI].texture_size;
	Peakbuf* peakbuf = waveform_get_peakbuf_n(w, blocknum);
	int o = TEX_BORDER_HI; for(;o<texture_size;o++){
		int i = (o - TEX_BORDER_HI) * WF_PEAK_VALUES_PER_SAMPLE;
		if(i >= peakbuf->size){
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
#endif


#if 0
static void
_draw_line (int x1, int y1, int x2, int y2, float r, float g, float b, float a)
{
	glColor4f(r, g, b, a);
#ifdef ANTIALIASED_LINES
	//agl_textured_rect(line_textures[0], x1, y1, x2-x1, y2-y1, NULL);
	//glLineWidth(4);
	float w = 4.0;
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0); glVertex2d(x1, y1);
	glTexCoord2d(1.0, 0.0); glVertex2d(x2, y2);
	glTexCoord2d(1.0, 1.0); glVertex2d(x2 + w, y2);
	glTexCoord2d(0.0, 1.0); glVertex2d(x1 + w, y1);
	glEnd();
#else
	glLineWidth(1);

	glBegin(GL_LINES);
	//TODO 0.1 offset was added for intel 945 - test on other hardware (check 1st peak is visible in hi-res mode)
	glVertex2f(x1 + 0.1, y1); glVertex2f(x2 + 0.1, y2);
	glEnd();
#endif
}
#endif


#if 0
static void
_draw_line_f (float x1, int y1, float x2, int y2, float r, float g, float b, float a)
{
	glLineWidth(1);
	glColor4f(r, g, b, a);

	glBegin(GL_LINES);
	glVertex2f(x1, y1); glVertex2f(x2, y2);
	glEnd();
}
#endif


#ifdef TEMP
static void
_set_pixel(int x, int y, guchar r, guchar g, guchar b, guchar aa)
{
	glColor3f(((float)r)/0x100, ((float)g)/0x100, ((float)b)/0x100);
	glPushMatrix();
	glNormal3f(0, 0, 1);
	glDisable(GL_TEXTURE_2D);
	glPointSize(4.0);
	//glPointParameter(GL_POINT_DISTANCE_ATTENUATION, x, y, z); //xyz are 0.0 - 1.0

	//make pt rounded (doesnt work)
	/*
	glEnable(GL_POINT_SMOOTH); // opengl.org says not supported and not recommended.
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/

	glBegin(GL_POINTS);
	glVertex3f(x, y, 1);
	glEnd();
	glPopMatrix();
	glColor3f(1.0, 1.0, 1.0);
}
#endif


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

	double usable_pct = (modes[r->mode].texture_size - 2.0 * border) / modes[r->mode].texture_size;
	double border_pct = (1.0 - usable_pct) / 2.0;

	block_wid = r->block_wid / multiplier;
	tex_pct = usable_pct; //use the whole texture
	tex_start = ((float)border) / modes[r->mode].texture_size;
	if (is_first){
		double _tex_pct = 1.0;
		if(r->first_offset){
			_tex_pct = 1.0 - ((double)r->first_offset) / samples_per_texture;
			tex_pct = tex_pct - (usable_pct) * ((double)r->first_offset) / samples_per_texture;
		}

#ifdef HIRES_NONSHADER_TEXTURES
		if(r->first_offset >= samples_per_texture) return false;
		if(r->first_offset) tex_pct = 1.0 - ((double)r->first_offset) / samples_per_texture;
#endif

		block_wid = (r->block_wid / multiplier) * _tex_pct;
		tex_start = 1.0 - border_pct - tex_pct;
		dbg(2, "rect.left=%.2f region->start=%"PRIi64" first_offset=%i", r->rect.left, r->region.start, r->first_offset);
	}
	if (is_last){
		//if(x + r->block_wid < x0 + rect->len){
		if(b < r->region_end_block){
			//end is offscreen. last block is not smaller.
		}else{
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


#ifdef HIRES_NONSHADER_TEXTURES
static inline bool
hi_gl1_render_block (Renderer* renderer, WaveformActor* actor, int b, gboolean is_first, gboolean is_last, double x)
{
	//render the 2d peak texture onto a block.

	//if we dont subdivide the blocks, size will be 256 x 16 = 4096. TODO intel 945 has max size of 2048
	//-it works but is the large texture size causing performance issues?

	WaveformContext* wfc = actor->context;
	RenderInfo* r  = &actor->priv->render_info;
	WfRectangle* rect = &r->rect;

	#define HIRES_NONSHADER_TEXTURES_MULTIPLIER 2 // half size texture
	int texture_size = modes[MODE_HI].texture_size / HIRES_NONSHADER_TEXTURES_MULTIPLIER;

	gl_warn("pre");

									//int b_ = b | WF_TEXTURE_CACHE_HIRES_MASK;
	WfTextureHi* texture = g_hash_table_lookup(actor->waveform->textures_hi->textures, &b);
	if(!texture){
		dbg(1, "texture not available. b=%i", b);
		return false;
	}
	agl_use_texture(texture->t[WF_LEFT].main);
	gl_warn("texture assign");

	float texels_per_px = ((float)texture_size) / r->block_wid;
	#define EXTRA_PASSES 4 // empirically determined for visual effect.
	int texels_per_px_i = ((int)texels_per_px) + EXTRA_PASSES;

	AGlColourFloat fg;
	wf_colour_rgba_to_float(&fg, actor->fg_colour);
	float alpha = ((float)(actor->fg_colour & 0xff)) / 256.0;
	alpha /= (texels_per_px_i * 0.5); //reduce the opacity depending on how many texture passes we do, but not be the full amount (which looks too much).
	glColor4f(fg.r, fg.g, fg.b, alpha); //seems we have to set colour _after_ binding... ?

	//gboolean no_more = false;
	//#define RATIO 2
	//int r; for(r=0;r<RATIO;r++){ // no, this is horrible, we need to move this into the main block loop.

#if 0
	double tex_start;
	double tex_pct;
#else
	TextureRange tex;
#endif
	WfSampleRegionf block;
	if(!wf_actor_get_quad_dimensions(actor, b, is_first, is_last, x, &tex, &block.start, &block.len, TEX_BORDER_HI, HIRES_NONSHADER_TEXTURES_MULTIPLIER)) return false;

	glBegin(GL_QUADS);
	//#if defined (USE_FBO) && defined (multipass)
	//	if(false){
	//	if(true){    //fbo not yet implemented for hi-res mode.
	//#else
		if(wfc->use_1d_textures){
	//#endif
			_draw_block_from_1d(tex_start, tex_pct, block.start, r->rect.top, block.len, r->rect.height, modes[MODE_HI].texture_size);
		}else{
			dbg(0, "x=%.2f wid=%.2f tex_pct=%.2f", block.start, block.len, tex_pct);

			/*
			 * render the texture multiple times so that all peaks are shown
			 * -this looks quite nice, but without saturation, the peaks can be very faint.
			 *  (not possible to have saturation while blending over background)
			 */
			float texel_offset = 1.0 / ((float)texture_size);
			int i; for(i=0;i<texels_per_px_i;i++){
				dbg(0, "texels_per_px=%.2f %i texel_offset=%.3f tex_start=%.4f", texels_per_px, texels_per_px_i, (texels_per_px / 2.0) / ((float)texture_size), tex_start);
				glTexCoord2d(tex_start + 0.0,     0.0); glVertex2d(block.start + 0.0,       rect->top);
				glTexCoord2d(tex_start + tex_pct, 0.0); glVertex2d(block.start + block.len, rect->top);
				glTexCoord2d(tex_start + tex_pct, 1.0); glVertex2d(block.start + block.len, rect->top + rect->height);
				glTexCoord2d(tex_start + 0.0,     1.0); glVertex2d(block.start + 0.0,       rect->top + rect->height);

				tex_start += texel_offset;
			}
		}
	glEnd();

	return true;
}
#endif


NGRenderer hi_renderer_gl2 = {{MODE_HI, hi_gl2_new, ng_gl2_load_block, ng_pre_render0, ng_gl2_render_block, ng_gl2_post_render, ng_gl2_free_waveform}};

static Renderer*
hi_renderer_init ()
{
	g_return_val_if_fail(!hi_renderer_gl2.ng_data, NULL);

	hi_renderer_gl2.ng_data = g_hash_table_new_full(g_direct_hash, g_int_equal, NULL, hi_gl2_free_item);
	hi_renderer_gl2.renderer.shader = &hires_ng_shader.shader;

	ng_make_lod_levels(&hi_renderer_gl2, MODE_HI);

	return (Renderer*)&hi_renderer_gl2;
}
