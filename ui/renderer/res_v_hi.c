/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. https://www.ayyi.org          |
* | copyright (C) 2012-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

extern LinesShader lines;
extern AGlMaterialClass aaline_class;

#define VERTEX_ARRAYS

/*
 *  The multiline shader is a performance related experiment,
 *  but is unlikely to offer much improvement if any.
 *  It has less draw calls but the gpu is doing more work.
 */
#define MULTILINE_SHADER
#undef MULTILINE_SHADER

#define TWO_COORDS_PER_VERTEX 2

static unsigned int vao = 0;
static unsigned int hivbo = 0;

typedef struct {
	struct {
		int l,r;
	} inner;
	struct {
		int l,r;
	} outer;
	int border;
} Range;

#ifdef MULTILINE_SHADER
static GLuint lines_texture[8] = {0};

GLuint _wf_create_lines_texture (guchar* pbuf, int width, int height);
#endif


static void
v_hi_renderer_new (WaveformActor* actor)
{
	WaveformPrivate* w = actor->waveform->priv;

	g_return_if_fail(!w->render_data[MODE_V_HI]);

	agl = agl_get_instance();

	w->render_data[MODE_V_HI] = WF_NEW(WaveformModeRender,
		.n_blocks = w->n_blocks
	);
}


static bool
v_hi_pre_render (Renderer* renderer, WaveformActor* actor)
{
	RenderInfo* r = &actor->priv->render_info;
	VHiRenderer* v_hi_renderer = (VHiRenderer*)renderer;

	// block_region_v_hi is actor specific so is only valid for current render.
	v_hi_renderer->block_region_v_hi = (WfSampleRegion){r->region.start, WF_PEAK_BLOCK_SIZE - r->region.start % WF_PEAK_BLOCK_SIZE};

	return true;
}


static bool
v_hi_pre_render0 (Renderer* renderer, WaveformActor* actor)
{
#if defined (MULTILINE_SHADER)
	agl_create_program(&lines.shader);
	modes[MODE_V_HI].renderer->shader = &lines.shader;
#else
	if (!agl->aaline) agl->aaline = agl_aa_line_new();

	modes[MODE_V_HI].renderer->shader = aaline_class.shader;
	((AGlActor*)actor)->program = renderer->shader;
#endif

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &hivbo);

	renderer->pre_render = v_hi_pre_render;

	return v_hi_pre_render(renderer, actor);
}


static void
_v_hi_set_gl_state (WaveformActor* actor)
{
#if defined (MULTILINE_SHADER)
	const WaveformContext* wfc = actor->context;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (wfc->use_1d_textures) {
		lines.uniform.colour = ((AGlActor*)actor)->colour;
		lines.uniform.n_channels = actor->waveform->n_channels;
	}
#else
	AGl* agl = agl_get_instance();

	agl->shaders.alphamap->uniform.fg_colour = ((AGlActor*)actor)->colour;
	agl_use_material(agl->aaline);
#endif
}


bool
draw_wave_buffer_v_hi (Renderer* renderer, WaveformActor* actor, int block, bool is_first, bool is_last, double x_block0)
{
	// For use at resolution 1, operates directly on audio data, NOT peak data.

	// variable names: variables prefixed with x_ relate to screen coordinates (pixels), variables prefixed with s_ related to sample frames.

	const Waveform* w = actor->waveform;
	const WaveformContext* wfc = actor->context;
	const WfActorPriv* _a = actor->priv;
	const RenderInfo* ri  = &_a->render_info;
	VHiRenderer* vhr = (VHiRenderer*)renderer;
	const WfRectangle* rect = &ri->rect;

	WfAudioData* audio = &w->priv->audio;
	if (!audio->n_blocks || w->offline) return false;
	WfBuf16* buf = audio->buf16[block];
	if (!buf) {
#ifdef DEBUG
		actor->render_result = RENDER_RESULT_NO_AUDIO_DATA;
#endif
		return false;
	}

	if(is_last) vhr->block_region_v_hi.len = (ri->region.start + ri->region.len) % WF_SAMPLES_PER_TEXTURE;

	// Alternative calculation of block_region_v_hi - does it give same results? NO
	uint64_t st = MAX((uint64_t)(ri->region.start),                  (uint64_t)((block)     * ri->samples_per_texture));
	uint64_t e  = MIN((uint64_t)(ri->region.start + ri->region.len), (uint64_t)((block + 1) * ri->samples_per_texture));
	WfSampleRegion block_region_v_hi2 = {st, e - st};
	//dbg(0, "block_region_v_hi=%Lu(%Lu)-->%Lu len=%Lu (buf->size=%Lu r->region=%Lu-->%Lu)", st, (uint64_t)vhr->block_region_v_hi.start, e, (uint64_t)block_region_v_hi2.len, ((uint64_t)buf->size), ((uint64_t)ri->region.start), ((uint64_t)ri->region.start) + ((uint64_t)ri->region.len));
	WfSampleRegion b_region = block_region_v_hi2;

	g_return_val_if_fail(b_region.len <= buf->size, false);

#ifdef DEBUG
	if(rect->left + rect->len < ri->viewport.left){
		perr("rect is outside viewport");
	}
#endif

	_v_hi_set_gl_state(actor);

	//#define BIG_NUMBER 4096
	#define BIG_NUMBER 8192    // temporarily increased pending cropping to viewport-left

	const double zoom = wfc->scaled
		? wfc->zoom->value.f / wfc->samples_per_pixel
		: rect->len / (double)ri->region.len;

	const float _block_wid = WF_SAMPLES_PER_TEXTURE * zoom;
	WfRectangle b_rect = {
		is_first ? fmodf(rect->left, _block_wid) : x_block0, // TODO simplify. why 2 separate cases needed?  ** try just using the first case
		rect->top,
		b_region.len * zoom,
		rect->height
	};

	if(!(ri->viewport.right > b_rect.left)) pwarn("outside viewport: vp.r=%.2f b_rect.l=%.2f", ri->viewport.right, b_rect.left);
	g_return_val_if_fail(ri->viewport.right > b_rect.left, false);
	if(!(b_rect.left + b_rect.len > ri->viewport.left)) pwarn("outside viewport: vp.l=%.1f b_rect.l=%.1f b_rect.len=%.1f", ri->viewport.left, b_rect.left, b_rect.len);
	g_return_val_if_fail(b_rect.left + b_rect.len > ri->viewport.left, false);

#ifndef MULTILINE_SHADER
	const int n_lines = MIN(BIG_NUMBER, ri->viewport.right - b_rect.left); // TODO the arrays possibly can be smaller - dont use b_rect.left, use x0
	AGlTQuad quads[n_lines];
#endif

	Range xr = {
		{0,},
		{0,},
#ifdef MULTILINE_SHADER
		.border = TEX_BORDER_HI
#else
		.border = 0
#endif
	};

	Range sr = {
		.inner.l = b_region.start % WF_SAMPLES_PER_TEXTURE,
		.border = xr.border / zoom
	};                                       //TODO check we are consistent in that these values are all *within the current block*

	//						sr.inner.l = (0 - x_block0) / zoom; //TODO crop to viewport-left
	const int s0 = sr.outer.l = sr.inner.l - sr.border;
	xr.outer.l = xr.border;
	const int x_bregion_end = b_rect.left + (b_region.start + b_region.len) * zoom;

	xr.inner.r = MIN(
		MIN(
			MIN(
				BIG_NUMBER,
				(int)(b_rect.left + b_rect.len)
			),
			ri->viewport.right
		),
		x_bregion_end
	);
	const int x_stop = xr.inner.r + xr.border - b_rect.left;

	/*
	if(x_stop < b_rect.left + b_rect.len){
		dbg(0, "stopping early. x_bregion_end=%i 0+B=%i", x_bregion_end, BIG_NUMBER);
	}
	*/

#ifdef MULTILINE_SHADER
	int mls_tex_w = x_stop - 2 * TEX_BORDER_HI;
	int mls_tex_h = 2; // TODO if we really are not going to add the indirection for x (for zoom > 1), use a 1d texture.
	int t_width = agl_power_of_two(mls_tex_w);
	guchar* _pbuf = g_new0(guchar, t_width * mls_tex_h);
	guchar* pbuf[] = {_pbuf, _pbuf + t_width};
#endif
										sr.inner.r = s0 + (xr.inner.r - xr.inner.l) / zoom;
										sr.outer.r = s0 /* TODO should include border? */+ ((double)x_stop) / zoom;

										// because we access adjacent samples, s_max is the absolute maximum index into the sample buffer (must be less than).
										int s_max = /*s0 + */sr.outer.r + 4 + sr.border;
										if(s_max > buf->size){
											int over = s_max - buf->size;
											if(over < 4 + sr.border) dbg(0, "TODO block overlap - need to access next block");
											else perr("error at block changeover. s_max=%i over=%i", s_max, over);
										}else{
											// its fairly normal to be limited by region, as the region can be set deliberately to match the viewport.
											// (the region can either correspond to a defined Section/Part of the waveform, or dynamically created with the viewport
											// -if it is a defined Section, we must not go beyond it. As we do not know which case we have, we must honour the region limit)
											//uint64_t b_region_end1 = (region.start + region.len) % buf->size; //TODO this should be the same as b_region_end2 ? but appears to be too short
											uint64_t b_region_end2 = (!((b_region.start + b_region.len) % WF_SAMPLES_PER_TEXTURE)) ? WF_SAMPLES_PER_TEXTURE : (b_region.start + b_region.len) % WF_SAMPLES_PER_TEXTURE;//buf->size;
											//if(s_max > b_region_end2) pwarn("limited by region length. region_end=%Lu %Lu", (uint64_t)((region.start + region.len) % buf->size), b_region_end2);
											//s_max = MIN(s_max, (region.start + region.len) % buf->size);
											s_max = MIN(s_max, b_region_end2);
										}
										s_max = MIN(s_max, buf->size);
										//note that there is never any need to be separately limited by b_region - that should be taken care of by buffer limitation?

	for(int c=0;c<w->n_channels;c++){
		if(!buf->buf[c]) continue;

#ifdef MULTILINE_SHADER
		int val0 = ((2*c + 1) * 128) / w->n_channels;
		if(mls_tex_w > TEX_BORDER_HI + 7) //TODO improve this test - make sure index is not negative  --- may not be needed (texture is bigger now (includes borders))
		memset((void*)((uintptr_t)pbuf[c] + (uintptr_t)(mls_tex_w - TEX_BORDER_HI)) - 7, val0, TEX_BORDER_HI + 7); // zero the rhs border in case it is not filled with content.

																						memset((void*)((uintptr_t)pbuf[c]), val0, t_width);
#endif

#ifndef MULTILINE_SHADER
		int oldx = -1;
		int oldy = 0;
#endif
		int s = 0;
		int i = 0;
		int x;
		//for(x = 0; x < BIG_NUMBER && /*rect->left +*/ x < viewport->right + border_right; x++, i++){
		// note that when using texture borders, at the viewport left edge x is NOT zero, it is TEX_BORDER_HI.
		for(x = xr.inner.l; x < x_stop; x++, i++){
			double s_ = ((double)x - xr.inner.l) / zoom;
			double dist = s_ - s; // dist = distance in samples from one pixel to the next.
													//if(i < 5) dbg(0, "x=%i s_=%.3f dist=%.2f", x, s_, dist);
			if (dist > 2.0) {
				//			if(dist > 5.0) pwarn("dist %.2f", dist);
				int ds = dist - 1;
				dist -= ds;
				s += ds;
			}

#ifdef MULTILINE_SHADER
			if (s0 + s < 0){ // left border and no valid data.
				pbuf[c][i] = val0;
				continue;
			}
#endif
			if (s0 + s >= (int)buf->size ) { pwarn("end of block reached: b_region.start=%i b_region.end=%"PRIi64" %i", s0, b_region.start + ((uint64_t)b_region.len), buf->size); break; }
			/*
			if (s  + 3 >= b_region.len) {
				pwarn("end of b_region reached: b_region.len=%i x=%i s0=%i s=%i", b_region.len, x, s0, s);
				break;
			}
			*/

			short* d = buf->buf[c];
			double y1 = (s0 + s   < s_max) ? d[s0 + s  ] : 0; //TODO have a separate loop for the last 4 values.
			double y2 = (s0 + s+1 < s_max) ? d[s0 + s+1] : 0;
			double y3 = (s0 + s+2 < s_max) ? d[s0 + s+2] : 0;
			double y4 = (s0 + s+3 < s_max) ? d[s0 + s+3] : 0;

			double d0 = dist;
			double d1 = dist - 1.0;
			double d2 = dist - 2.0;
			double d3 = dist - 3.0;

																				//TODO for MULTILINE_SHADER we probably dont want b_rect.height to affect y.
			int y = (int)(
				(
					- (d1 * d2 * d3 * y1) / 6
					+ (d0 * d2 * d3 * y2) / 2
					- (d0 * d1 * d3 * y3) / 2
					+ (d0 * d1 * d2 * y4) / 6
				)
				* wfc->v_gain * (b_rect.height / (2.0 * w->n_channels )) / (1 << 15)
			);
#if defined (MULTILINE_SHADER)
			if(i >= mls_tex_w){ pwarn("tex index out of range %i (max %i)", i, mls_tex_w); break; }
			{
				pbuf[c][i] = val0 + MIN(y, 63); // the 63 is to stop it wrapping - would be nice to remove this.
			}
#else
			{
				float x0 = oldx;
				float y0 = b_rect.top - oldy + b_rect.height / 2;
				float x1 = x;
				float y1 = b_rect.top -    y + b_rect.height / 2;

				float len = sqrtf(powf((y1 - y0), 2) + powf((x1 - x0), 2));
				float xoff = (y1 - y0) * 5.0 / len;
				float yoff = (x1 - x0) * 5.0 / len;

				quads[i] = (AGlTQuad){
					{(AGlVertex){x0 - xoff/2, y0 + yoff/2}, (AGlVertex){0.0, 0.0}},
					{(AGlVertex){x1 - xoff/2, y1 + yoff/2}, (AGlVertex){1.0, 0.0}},
					{(AGlVertex){x1 + xoff/2, y1 - yoff/2}, (AGlVertex){1.0, 1.0}},
					{(AGlVertex){x0 - xoff/2, y0 + yoff/2}, (AGlVertex){0.0, 0.0}},
					{(AGlVertex){x1 + xoff/2, y1 - yoff/2}, (AGlVertex){1.0, 1.0}},
					{(AGlVertex){x0 + xoff/2, y0 - yoff/2}, (AGlVertex){0.0, 1.0}}
				};
			}

			oldx = x;
			oldy = y;
#endif
		}
		dbg(2, "n_lines=%i x=%i-->%i", i, 0, x);

		if(w->n_channels == 2){
			agl_translate ((AGlShader*)agl->shaders.alphamap, 0., - b_rect.height/4. + c * b_rect.height/2.);
		}

#ifndef MULTILINE_SHADER
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, hivbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quads), quads, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

		glDrawArrays(GL_TRIANGLES, 0, i * AGL_V_PER_QUAD);
#endif
	} // end channel

#if defined (MULTILINE_SHADER)
	#define TEXELS_PER_PIXEL 1.0
	guint texture =_wf_create_lines_texture(pbuf[0], t_width, mls_tex_h);
	float len = MIN(agl_actor__width((AGlActor*)actor), b_rect.len); // the texture contents are cropped by viewport so we should do the same when setting the rect width.

	AGlQuad t = {
		((float)TEX_BORDER_HI)/((float)t_width),
		0.0,
		((float)((x_stop) * TEXELS_PER_PIXEL)) / (float)t_width,
		1.0
	};

	lines.uniform.texture_width = t_width;
	agl_use_program((AGlShader*)&lines); // to set the uniform

	glPushMatrix();
	glTranslatef(b_rect.left, 0.0, 0.0);
	agl_textured_rect(texture, 0.0, b_rect.top, len, b_rect.height, &t);
	glPopMatrix();

	g_free(_pbuf);
#endif

	// increment for next block
	vhr->block_region_v_hi.start = (vhr->block_region_v_hi.start / WF_PEAK_BLOCK_SIZE + 1) * WF_PEAK_BLOCK_SIZE;
	vhr->block_region_v_hi.len   = WF_PEAK_BLOCK_SIZE - vhr->block_region_v_hi.start % WF_PEAK_BLOCK_SIZE;

	return true;
}


static void
v_hi_load_block (Renderer* renderer, WaveformActor* a, int b)
{
	if(((AGlActor*)a)->root->draw) wf_context_queue_redraw(a->context);
}


static void
v_hi_free_waveform (Renderer* renderer, Waveform* w)
{
	g_clear_pointer(&w->priv->render_data[MODE_V_HI], g_free);
#if 0
	glDeleteBuffers (1, &vbo);
	vbo = 0;
#endif
}


#ifdef MULTILINE_SHADER
GLuint
_wf_create_lines_texture (guchar* pbuf, int width, int height)
{
	/*
	 * This texture is used as data for the shader.
	 * Each column represents a vertex.
	 * Each row contains the following values:
	 *   0: x value // .. or maybe not
	 *   1: y value
	 *
	 */
	glEnable(GL_TEXTURE_2D);

	if(!lines_texture[0]){
		glGenTextures(8, lines_texture);
		if(glGetError() != GL_NO_ERROR){ perr ("couldnt create lines_texture."); return 0; }
		dbg(2, "lines_texture=%i", lines_texture[0]);
	}

	/*
	int y; for(y=0;y<height/2;y++){
		int x; for(x=0;x<width;x++){
			y=0; *(pbuf + y * width + x) = 0x40;
			y=1; *(pbuf + y * width + x) = 0xa0;
			y=2; *(pbuf + y * width + x) = 0xff;
			y=3; *(pbuf + y * width + x) = 0xa0;
			y=4; *(pbuf + y * width + x) = 0x40;
		}
	}
	*/

	// currently we rotate through a fixed number of textures. TODO needs improvement - fixed number will either be too high or too low depending on the application.
	static int t_idx = 0;
	int t = t_idx;

	int pixel_format = GL_ALPHA;
	glBindTexture  (GL_TEXTURE_2D, lines_texture[t]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);
	gl_warn("error binding lines texture");

	t_idx = (t_idx + 1) % 8;

	return lines_texture[t];
}
#endif


VHiRenderer v_hi_renderer = {{MODE_V_HI, v_hi_renderer_new, v_hi_load_block, v_hi_pre_render0, draw_wave_buffer_v_hi, NULL, v_hi_free_waveform}};


