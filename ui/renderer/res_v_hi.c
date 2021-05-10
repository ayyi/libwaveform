/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
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

extern unsigned int vao;
extern unsigned int vbo;

static unsigned int vhi_vao;
static unsigned int tvbo;

typedef struct {
	struct {
		int l,r;
	} inner;
	struct {
		int l,r;
	} outer;
	int border;
} Range;

#define TWO_COORDS_PER_VERTEX 2

typedef struct {float x, y;} Vertex;
typedef struct {Vertex v0, v1, v2, v3;} Quad;


static void
v_hi_renderer_new (WaveformActor* actor)
{
	WaveformPrivate* w = actor->waveform->priv;

	g_return_if_fail(!w->render_data[MODE_V_HI]);

	agl = agl_get_instance();

	w->render_data[MODE_V_HI] = WF_NEW(WaveformModeRender,
		.n_blocks = w->n_blocks
	);

#if defined (MULTILINE_SHADER)
	if(agl->use_shaders){
		agl_create_program(&lines.shader);
	}
#endif

#if defined(ANTIALIASED_LINES)
	if(!agl->aaline) agl->aaline = agl_aa_line_new();

	modes[MODE_V_HI].renderer->shader = aaline_class.shader;
#endif

	glGenVertexArrays(1, &vhi_vao);
	glGenBuffers(1, &tvbo);
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


static void
_v_hi_set_gl_state (WaveformActor* actor)
{
	AGl* agl = agl_get_instance();
	const WaveformContext* wfc = actor->context;

#if defined (MULTILINE_SHADER)
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							if(wfc->use_1d_textures){
								glDisable(GL_TEXTURE_1D);
								_c->shaders.lines->uniform.colour = ((AGlActor*)actor)->colour;
								_c->shaders.lines->uniform.n_channels = w->n_channels;
							}
#elif defined(ANTIALIASED_LINES)
	if(wfc->use_1d_textures){
		agl->shaders.alphamap2->uniform.fg_colour = ((AGlActor*)actor)->colour;
		agl_use_material(agl->aaline);
		agl_enable(AGL_ENABLE_BLEND | AGL_ENABLE_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, aaline_class.texture);
	}else{
		agl_enable(AGL_ENABLE_BLEND | AGL_ENABLE_TEXTURE_2D);
	}
#else
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							if(wfc->use_1d_textures){
								glDisable(GL_TEXTURE_1D);
								agl->shaders.plain->uniform.colour = ((AGlActor*)actor)->colour;
								agl_use_program((AGlShader*)agl->shaders.plain);
							}
							glDisable(GL_TEXTURE_2D);
#endif
}


														#if 0
bool
draw_wave_buffer_v_hi_orig (Renderer* renderer, WaveformActor* actor, int block, bool is_first, bool is_last, double x_block0)
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
	if(!audio->n_blocks || w->offline) return false;
	WfBuf16* buf = audio->buf16[block];
	if(!buf) return false;

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
		? wfc->zoom->value.f * rect->len / (double)ri->region.len
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

#ifdef MULTILINE_SHADER
#elif defined(VERTEX_ARRAYS)
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	int n_lines = MIN(BIG_NUMBER, ri->viewport.right - b_rect.left); // TODO the arrays possibly can be smaller - dont use b_rect.left, use x0
	Quad quads[n_lines];
	Vertex texture_coords[n_lines * 4];
	for(int j=0;j<n_lines;j++){
		texture_coords[j * 4    ] = (Vertex){0.0, 0.0};
		texture_coords[j * 4 + 1] = (Vertex){1.0, 0.0};
		texture_coords[j * 4 + 2] = (Vertex){1.0, 1.0};
		texture_coords[j * 4 + 3] = (Vertex){0.0, 1.0};
	}
	glVertexPointer   (TWO_COORDS_PER_VERTEX, GL_FLOAT, 0, quads);
	glTexCoordPointer (TWO_COORDS_PER_VERTEX, GL_FLOAT, 0, texture_coords);
#endif

#ifndef MULTILINE_SHADER
	uint32_t rgba = ((AGlActor*)actor)->colour;
	const float r = ((float)((rgba >> 24)       ))/0x100;
	const float g = ((float)((rgba >> 16) & 0xff))/0x100;
	const float b = ((float)((rgba >>  8) & 0xff))/0x100;
	const float alpha = ((float)((rgba  ) & 0xff))/0x100;
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
	//dbg(0, "rect=%.2f-->%.2f b_region=%Lu-->%Lu viewport=%.1f-->%.1f zoom=%.3f", b_rect.left, b_rect.left + b_rect.len, b_region.start, b_region.start + ((uint64_t)b_region.len), ri->viewport.left, ri->viewport.right, zoom);

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
																						if(!buf->buf[c]){ pwarn("audio buf not set. c=%i", c); continue; }
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
		//int x; for(x = 0; x < BIG_NUMBER && /*rect->left +*/ x < viewport->right + border_right; x++, i++){
		// note that when using texture borders, at the viewport left edge x is NOT zero, it is TEX_BORDER_HI.
		int x; for(x = xr.inner.l; x < x_stop; x++, i++){
			double s_ = ((double)x - xr.inner.l) / zoom;
			double dist = s_ - s; // dist = distance in samples from one pixel to the next.
//if(i < 5) dbg(0, "x=%i s_=%.3f dist=%.2f", x, s_, dist);
			if (dist > 2.0) {
				//			if(dist > 5.0) pwarn("dist %.2f", dist);
				int ds = dist - 1;
				dist -= ds;
				s += ds;
			}
																						//if(c == 0 && i < 5) dbg(0, "  ss=%i", s0 + s);
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
																				//if(i < 10) printf("  x=%i s=%i %i y=%i dist=%.2f s=%i %.2f\n", x, s0 + s, d[s0 + s], y, dist, s, floor((x / zoom) - 1));

#if defined (MULTILINE_SHADER)
		if(i >= mls_tex_w){ pwarn("tex index out of range %i %i", i, mls_tex_w); break; }
		{
																				//int val = ((2*c + 1) * 128 + y) / w->n_channels;
																				//if(val < 0 || val > 256) pwarn("val out of range: %i", val); -- will be out of range when vgain is high.
			pbuf[c][i] = val0 + MIN(y, 63); // the 63 is to stop it wrapping - would be nice to remove this.
																				//if(i >= 0 && i < 10) dbg(0, "  s=%i y2=%.4f y=%i val=%i", s + s0, y2, y, ((2*c + 1) * 128 + y) / w->n_channels);
		}
#elif defined(VERTEX_ARRAYS)
		{
			//float ys = y * rect->height / 256.0;

			float x0 = oldx;
			float y0 = b_rect.top - oldy + b_rect.height / 2;
			float x1 = x;
			float y1 = b_rect.top -    y + b_rect.height / 2;

			float len = sqrtf(powf((y1 - y0), 2) + powf((x1 - x0), 2));
			float xoff = (y1 - y0) * 5.0 / len;
			float yoff = (x1 - x0) * 5.0 / len;

			quads[i] = (Quad){
				(Vertex){x0 - xoff/2, y0 + yoff/2},
				(Vertex){x1 - xoff/2, y1 + yoff/2},
				(Vertex){x1 + xoff/2, y1 - yoff/2},
				(Vertex){x0 + xoff/2, y0 - yoff/2},
			};
//if(i > 40 && i < 50) dbg(0, "len=%.2f xoff=%.2f yoff=%.2f (%.2f, %.2f) (%.2f, %.2f)", len, xoff, yoff, quads[i].v0.x, quads[i].v0.y, quads[i].v1.x, quads[i].v1.y);
		}
#else
		// draw straight line from old pos to new pos
		_draw_line(
			oldx,   b_rect.top - oldy + b_rect.height / 2,
			x,      b_rect.top -    y + b_rect.height / 2,
			r, g, b, alpha);
#endif

#ifndef MULTILINE_SHADER
		oldx = x;
		oldy = y;
#endif
	}
	dbg(2, "n_lines=%i x=%i-->%i", i, 0, x);

#if defined (MULTILINE_SHADER)
#elif defined(VERTEX_ARRAYS)
		glColor4f(r, g, b, alpha);
		glPushMatrix();
		glTranslatef(0.0, (w->n_channels == 2 ? -b_rect.height/4 : 0.0) + c * b_rect.height/2, 0.0);
		GLsizei count = (i - 0) * 4;
		glDrawArrays(GL_QUADS, 0, count);
		glPopMatrix();
#endif
	} // end channel

#if defined (MULTILINE_SHADER)
	#define TEXELS_PER_PIXEL 1.0
	guint texture =_wf_create_lines_texture(pbuf[0], t_width, mls_tex_h);
	float len = MIN(viewport->right - viewport->left, b_rect.len); //the texture contents are cropped by viewport so we should do the same when setting the rect width.
	AGlQuad t = {
		((float)TEX_BORDER_HI)/((float)t_width),
		0.0,
		((float)((x_stop) * TEXELS_PER_PIXEL)) / (float)t_width,
		1.0
	};
	wfc->priv->shaders.lines->uniform.texture_width = t_width;
	agl_use_program((AGlShader*)wfc->priv->shaders.lines); // to set the uniform

	glPushMatrix();
	glTranslatef(b_rect.left, 0.0, 0.0);
	agl_textured_rect_real(texture, 0.0, b_rect.top, len, b_rect.height, &t);
	glPopMatrix();

	g_free(_pbuf);

#elif defined(VERTEX_ARRAYS)
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

	// increment for next block
	vhr->block_region_v_hi.start = (vhr->block_region_v_hi.start / WF_PEAK_BLOCK_SIZE + 1) * WF_PEAK_BLOCK_SIZE;
	vhr->block_region_v_hi.len   = WF_PEAK_BLOCK_SIZE - vhr->block_region_v_hi.start % WF_PEAK_BLOCK_SIZE;

	return true;
}
																	#endif


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
	if(!audio->n_blocks || w->offline) return false;
	WfBuf16* buf = audio->buf16[block];
	if(!buf) return false;

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
		? wfc->zoom->value.f * rect->len / (double)ri->region.len
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

	int n_lines = MIN(BIG_NUMBER, ri->viewport.right - b_rect.left); // TODO the arrays possibly can be smaller - dont use b_rect.left, use x0
	Quad quads[n_lines];
	Vertex texture_coords[n_lines * 4];
	for(int j=0;j<n_lines;j++){
		texture_coords[j * 4    ] = (Vertex){0.0, 0.0};
		texture_coords[j * 4 + 1] = (Vertex){1.0, 0.0};
		texture_coords[j * 4 + 2] = (Vertex){1.0, 1.0};
		texture_coords[j * 4 + 3] = (Vertex){0.0, 1.0};
	}

	uint32_t rgba = ((AGlActor*)actor)->colour;
	const float r = ((float)((rgba >> 24)       ))/0x100;
	const float g = ((float)((rgba >> 16) & 0xff))/0x100;
	const float b = ((float)((rgba >>  8) & 0xff))/0x100;
	const float alpha = ((float)((rgba  ) & 0xff))/0x100;

	Range xr = {
		{0,},
		{0,},
		.border = 0
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
	//dbg(0, "rect=%.2f-->%.2f b_region=%Lu-->%Lu viewport=%.1f-->%.1f zoom=%.3f", b_rect.left, b_rect.left + b_rect.len, b_region.start, b_region.start + ((uint64_t)b_region.len), ri->viewport.left, ri->viewport.right, zoom);

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
																						if(!buf->buf[c]){ pwarn("audio buf not set. c=%i", c); continue; }
		if(!buf->buf[c]) continue;

		int oldx = -1;
		int oldy = 0;
		int s = 0;
		int i = 0;
		//int x; for(x = 0; x < BIG_NUMBER && /*rect->left +*/ x < viewport->right + border_right; x++, i++){
		// note that when using texture borders, at the viewport left edge x is NOT zero, it is TEX_BORDER_HI.
		int x; for(x = xr.inner.l; x < x_stop; x++, i++){
			double s_ = ((double)x - xr.inner.l) / zoom;
			double dist = s_ - s; // dist = distance in samples from one pixel to the next.
													//if(i < 5) dbg(0, "x=%i s_=%.3f dist=%.2f", x, s_, dist);
			if (dist > 2.0) {
				//			if(dist > 5.0) pwarn("dist %.2f", dist);
				int ds = dist - 1;
				dist -= ds;
				s += ds;
			}
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

			{
				//float ys = y * rect->height / 256.0;

				float x0 = oldx;
				float y0 = b_rect.top - oldy + b_rect.height / 2;
				float x1 = x;
				float y1 = b_rect.top -    y + b_rect.height / 2;

				float len = sqrtf(powf((y1 - y0), 2) + powf((x1 - x0), 2));
				float xoff = (y1 - y0) * 5.0 / len;
				float yoff = (x1 - x0) * 5.0 / len;

				quads[i] = (Quad){
					(Vertex){x0 - xoff/2, y0 + yoff/2},
					(Vertex){x1 - xoff/2, y1 + yoff/2},
					(Vertex){x1 + xoff/2, y1 - yoff/2},
					(Vertex){x0 + xoff/2, y0 - yoff/2},
				};
			}

			oldx = x;
			oldy = y;
		}
		dbg(2, "n_lines=%i x=%i-->%i", i, 0, x);

		glColor4f(r, g, b, alpha);
					glBindVertexArray(vhi_vao);

					// vertices
					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferData(GL_ARRAY_BUFFER, sizeof(quads), quads, GL_STATIC_DRAW);

					glEnableVertexAttribArray(0);
					glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Quad), NULL);

					// texture coords
					glBindBuffer(GL_ARRAY_BUFFER, tvbo);
					glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords), texture_coords, GL_STATIC_DRAW);
					glEnableVertexAttribArray(1);
					glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Quad), NULL);

		GLsizei count = (i - 0) * 4;
		glDrawArrays(GL_QUADS, 0, count);
	} // end channel

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
}


VHiRenderer v_hi_renderer = {{MODE_V_HI, v_hi_renderer_new, v_hi_load_block, v_hi_pre_render, draw_wave_buffer_v_hi, v_hi_free_waveform}};


