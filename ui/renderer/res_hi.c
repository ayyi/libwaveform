/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
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

static inline bool calc_render_info (WaveformActor* actor);

#ifdef DEBUG
static void  _wf_actor_print_hires_textures  (WaveformActor*);
#endif


void
hi_new_gl1 (WaveformActor* a)
{
	WaveformPrivate* _w = a->waveform->priv;

	g_return_if_fail(!_w->render_data[MODE_HI]);

	agl = agl_get_instance();
	if(!agl->use_shaders){
		_w->render_data[MODE_HI] = (WaveformModeRender*)g_new0(WfTexturesHi, 1);
		RENDER_DATA_HI(_w)->textures = g_hash_table_new(g_int_hash, g_int_equal);
	}
}


static void
hi_free_gl1 (Renderer* renderer, Waveform* w)
{
	if(!w->priv->render_data[MODE_HI]) return;

	WfTexturesHi* textures = (WfTexturesHi*)w->priv->render_data[MODE_HI];

	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init (&iter, textures->textures);
	while (g_hash_table_iter_next (&iter, &key, &value)){
		//int block = key;
		WfTextureHi* texture = value;
		waveform_texture_hi_free(texture);
	}

	g_hash_table_destroy(textures->textures);
	g_free0(w->priv->render_data[MODE_HI]);
}


	static void hi_request_block_done(Waveform* w, int b, gpointer _a)
	{
		WaveformActor* a = _a;
		if(w == a->waveform){ // the actor may have a new waveform and there is currently no way to cancel old requests.
			modes[MODE_HI].renderer->load_block(modes[MODE_HI].renderer, a, b);

			RenderInfo* r = &a->priv->render_info;
			if (!r->valid) calc_render_info(a);
			BlockRange blocks = a->priv->render_info.viewport_blocks;
			if (!r->valid || (b >= blocks.first && b <= blocks.last)) {
				if(((AGlActor*)a)->root && ((AGlActor*)a)->root->draw) wf_context_queue_redraw(a->context);
			}
		}
	}

static void
hi_request_block (WaveformActor* a, int b)
{
	waveform_load_audio(a->waveform, b, HI_MIN_TIERS, hi_request_block_done, a);
}


static void
hi_gl1_load_block (Renderer* renderer, WaveformActor* a, int block)
{
	// audio data for this block _must_ already be loaded

	void
	wf_actor_allocate_block_hi(WaveformActor* a, int b)
	{
		PF;
		WfTextureHi* texture = g_hash_table_lookup(RENDER_DATA_HI(a->waveform->priv)->textures, &b);

		int c = WF_LEFT;

		if(glIsTexture(texture->t[c].main)){
			pwarn("already assigned");
			return;
		}

	#ifdef HIRES_NONSHADER_TEXTURES
		texture->t[WF_LEFT].main = texture_cache_assign_new(GL_TEXTURE_2D, (WaveformBlock){a->waveform, b | WF_TEXTURE_CACHE_HIRES_MASK});
		dbg(0, "assigned texture=%u", texture->t[WF_LEFT].main);

		wf_actor_load_texture2d(a, MODE_HI, texture->t[c].main, b);
	#endif
	}

	ModeRange mode = mode_range(a);
	if(mode.lower == MODE_HI || mode.upper == MODE_HI){ // TODO presumably this check is no longer needed. test and remove.

		WfTextureHi* texture = g_hash_table_lookup(RENDER_DATA_HI(a->waveform->priv)->textures, &block);
		if(!texture){
			texture = waveform_texture_hi_new();
			dbg(1, "b=%i: inserting...", block);
			uint32_t* key = (uint32_t*)g_malloc(sizeof(uint32_t));
			*key = block;
			g_hash_table_insert(RENDER_DATA_HI(a->waveform->priv)->textures, key, texture);
			wf_actor_allocate_block_hi(a, block);
		}
		else dbg(1, "b=%i: already have texture. t=%i", block, texture->t[WF_LEFT].main);
#ifdef DEBUG
		if(_debug_ > 1) _wf_actor_print_hires_textures(a);
#endif

		//TODO check this block is within current viewport
		if(((AGlActor*)a)->root->draw) wf_context_queue_redraw(a->context);
	}
}


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
hi_gl1_pre_render (Renderer* renderer, WaveformActor* actor)
{
#ifndef HIRES_NONSHADER_TEXTURES
	RenderInfo* r  = &actor->priv->render_info;
	HiRenderer* hr = (HiRenderer*)renderer;

	//block_region specifies the sample range for that part of the waveform region that is within the current block
	//-note that the block region can exceed the range of the waveform region.
	hr->block_region = (WfSampleRegion){r->region.start % WF_SAMPLES_PER_TEXTURE, WF_SAMPLES_PER_TEXTURE - r->region.start % WF_SAMPLES_PER_TEXTURE};

#ifdef XANTIALIASED_LINES
	//TODO does antialiased look better? if so, must init line_textures
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, line_textures[0]);
#else
	//TODO blending is needed to support opacity, however the actual opacity currently varies depending on zoom due to overlapping.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glLineWidth(1);
#endif
	{
		uint32_t rgba = ((AGlActor*)actor)->colour;
		float r = ((float)((rgba >> 24)       ))/0x100;
		float g = ((float)((rgba >> 16) & 0xff))/0x100;
		float b = ((float)((rgba >>  8) & 0xff))/0x100;
		float alpha = ((float)((rgba  ) & 0xff))/0x100;
		glColor4f(r, g, b, alpha);
	}
#endif

	glEnable(GL_TEXTURE_2D);

	return true;
}


#ifndef HIRES_NONSHADER_TEXTURES
bool
draw_wave_buffer_hi_gl1 (Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x)
{
	void _draw_wave_buffer_hi_gl1(Waveform* w, WfSampleRegion region, WfRectangle* rect, Peakbuf* peakbuf, int chan, float v_gain, uint32_t rgba)
	{
		//for use with peak data of alternative plus and minus peaks.
		// -non-shader version

		// x is integer which means lines are not evenly spaced and causes problems setting alpha.
		// however using float x gives the same visual results (at least on intel 945)
		// -the solution to this is probably to use textures.

		int64_t region_end = region.start + (int64_t)region.len;

		short* data = peakbuf->buf[chan];

		int io_ratio = (peakbuf->resolution == 16 || peakbuf->resolution == 8) ? 16 : 1; //TODO
		int x = 0;
		int p = 0;
		int p_ = region.start / io_ratio;
		/*
		float region_len = region.len;
		dbg(0, "width=%.2f region=%Li-->%Li xgain=%.2f resolution=%i io_ratio=%i", rect->len, region.start, region.start + (int64_t)region.len, rect->len / region_len, peakbuf->resolution, io_ratio);
		dbg(0, "x: %.2f --> %.2f", (((double)0) / region_len) * rect->len, (((double)4095) / region_len) * rect->len);
		*/

		//TODO why is this needed ??? should not be
		glDisable(GL_TEXTURE_2D);

		/*
		if(!(region_end / io_ratio <= peakbuf->size / WF_PEAK_VALUES_PER_SAMPLE))
			pwarn("end/ratio=%i size=%i - region.end should never exceed %i", ((int)region_end / io_ratio), peakbuf->size / WF_PEAK_VALUES_PER_SAMPLE, io_ratio * peakbuf->size / WF_PEAK_VALUES_PER_SAMPLE);
		*/
		g_return_if_fail(region_end / io_ratio <= peakbuf->size / WF_PEAK_VALUES_PER_SAMPLE);
		while (p < region.len / io_ratio){
										if(2 * p_ >= peakbuf->size) pwarn("s_=%i size=%i", p_, peakbuf->size);
										g_return_if_fail(2 * p_ < peakbuf->size);
			x = rect->left + (((double)p) / ((double)region.len)) * rect->len * io_ratio;
			if (x - rect->left >= rect->len) break;

			double y1 = ((double)data[WF_PEAK_VALUES_PER_SAMPLE * p_    ]) * v_gain * (rect->height / 2.0) / (1 << 15);
			double y2 = ((double)data[WF_PEAK_VALUES_PER_SAMPLE * p_ + 1]) * v_gain * (rect->height / 2.0) / (1 << 15);

#if 0
			_draw_line(x, rect->top - y1 + rect->height / 2, x, rect->top - y2 + rect->height / 2, r, g, b, alpha);
#else
			// this assumes that we want un-antialiased lines.

			glBegin(GL_LINES);
			// note: 0.1 offset was added for intel 945.
			glVertex2f(x + 0.1, rect->top - y1 + rect->height / 2);
			glVertex2f(x + 0.1, rect->top - y2 + rect->height / 2);
			glEnd();
#endif

			p++;
			p_++;
		}
		dbg(2, "n_lines=%i x0=%.1f x=%i y=%.1f h=%.1f", p, rect->left, x, rect->top, rect->height);
	}

	Waveform* w = actor->waveform;
	WfActorPriv* _a = actor->priv;
	WaveformContext* wfc = actor->context;
	RenderInfo* r  = &_a->render_info;
	HiRenderer* hr = (HiRenderer*)renderer;
	WfRectangle* rect = &r->rect;

	Peakbuf* peakbuf = waveform_get_peakbuf_n(w, b);
	if(!peakbuf) return false;

	dbg(2, "  b=%i x=%.2f", b, x);

	WfRectangle block_rect = {
		is_first
			? x + (hr->block_region.start - r->region.start % WF_SAMPLES_PER_TEXTURE) * r->zoom
			: x,
		rect->top,
		r->block_wid,
		rect->height / w->n_channels
	};
	if(is_first){
		float first_fraction =((float)hr->block_region.len) / WF_SAMPLES_PER_TEXTURE;
		block_rect.left += (WF_SAMPLES_PER_TEXTURE - WF_SAMPLES_PER_TEXTURE * first_fraction) * r->zoom;
	}
	//WfRectangle block_rect = {x + (region.start % WF_PEAK_BLOCK_SIZE) * r->zoom, rect.top + c * rect.height/2, r->block_wid, rect.height / w->n_channels};
	dbg(2, "  HI: %i: rect=%.2f-->%.2f", b, block_rect.left, block_rect.left + block_rect.len);

	if(is_last){
		if(b < r->region_end_block){
			// end is offscreen. last block is not smaller.
			// reducing the block size here would be an optimisation rather than essential
		}else{
			// last block of region (not merely truncated by viewport).
			//block_region.len = r->region.len - b * WF_PEAK_BLOCK_SIZE;
			//block_region.len = (r->region.start - block_region.start + r->region.len) % WF_PEAK_BLOCK_SIZE;// - hr->block_region.start;
			if(is_first){
				hr->block_region.len = r->region.len % WF_SAMPLES_PER_TEXTURE;
			}else{
				hr->block_region.len = (r->region.start + r->region.len) % WF_SAMPLES_PER_TEXTURE;
			}
			dbg(2, "REGIONLAST: %i/%i region.len=%"PRIi64" ratio=%.2f rect=%.2f %.2f", b, r->region_end_block, hr->block_region.len, ((float)hr->block_region.len) / WF_PEAK_BLOCK_SIZE, block_rect.left, block_rect.len);
		}
	}
	block_rect.len = hr->block_region.len * r->zoom; // always

	int c; for(c=0;c<w->n_channels;c++){
		if(peakbuf->buf[c]){
			//dbg(1, "peakbuf: %i:%i: %i", b, c, ((short*)peakbuf->buf[c])[0]);

			block_rect.top = rect->top + c * rect->height/2;

			_draw_wave_buffer_hi_gl1(w, hr->block_region, &block_rect, peakbuf, c, wfc->v_gain, ((AGlActor*)actor)->colour);
		}
		else dbg(1, "buf not ready: %i", c);
	}

	// increment for next block
	hr->block_region.start = 0; //all blocks except first start at 0
	hr->block_region.len = WF_PEAK_BLOCK_SIZE;

	return true;
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


#ifdef DEBUG
static void
_wf_actor_print_hires_textures (WaveformActor* a)
{
	PF0;

	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init (&iter, RENDER_DATA_HI(a->waveform->priv)->textures);
	while (g_hash_table_iter_next (&iter, &key, &value)){
		int b = *((int*)key);
		WfTextureHi* th = value;
		printf("  b=%i t=%i\n", b, th->t[WF_LEFT].main);
	}
}
#endif


NGRenderer hi_renderer_gl2 = {{MODE_HI, hi_gl2_init, ng_gl2_load_block, ng_pre_render, ng_gl2_render_block, ng_gl2_post_render, ng_gl2_free_waveform}};

HiRenderer hi_renderer_gl1 = {{MODE_HI, hi_new_gl1, hi_gl1_load_block, hi_gl1_pre_render,
#ifdef HIRES_NONSHADER_TEXTURES
				hi_gl1_render_block,
#else
				// without shaders, each sample line is drawn directly without using textures, so performance will be relatively poor.
				draw_wave_buffer_hi_gl1,
#endif
				NULL, hi_free_gl1
}};


static Renderer*
hi_renderer_new ()
{
	g_return_val_if_fail(!hi_renderer_gl2.ng_data, NULL);

	static HiRenderer* hi_renderer = (HiRenderer*)&hi_renderer_gl2;

	hi_renderer_gl2.ng_data = g_hash_table_new_full(g_direct_hash, g_int_equal, NULL, hi_gl2_free_item);

	ng_make_lod_levels(&hi_renderer_gl2, MODE_HI);

	return (Renderer*)hi_renderer;
}

