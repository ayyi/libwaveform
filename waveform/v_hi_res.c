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

#define VERTEX_ARRAYS

#ifdef ANTIALIASED_LINES
GLuint _wf_create_line_texture();
#endif

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
v_hi_renderer_new(WaveformActor* actor)
{
	WaveformPriv* w = actor->waveform->priv;

	g_return_if_fail(!w->render_data[MODE_V_HI]);

	agl = agl_get_instance();
	w->render_data[MODE_V_HI] = g_new0(WaveformModeRender, 1);
	w->render_data[MODE_V_HI]->n_blocks = w->n_blocks;
}


static void
v_hi_pre_render(Renderer* renderer, WaveformActor* actor)
{
	RenderInfo* r  = &actor->priv->render_info;
	VHiRenderer* v_hi_renderer = (VHiRenderer*)renderer;

	// block_region_v_hi is actor specific so is only valid for current render.
	v_hi_renderer->block_region_v_hi = (WfSampleRegion){r->region.start, WF_PEAK_BLOCK_SIZE - r->region.start % WF_PEAK_BLOCK_SIZE};
}


static void
_v_hi_set_gl_state(WaveformActor* actor)
{
	AGl* agl = agl_get_instance();
	const WaveformCanvas* wfc = actor->canvas;

							//TODO might these prevent further blocks at different res? difficult to notice as they are usually the same.
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if defined (MULTILINE_SHADER)
							if(wfc->use_1d_textures){
								glDisable(GL_TEXTURE_1D);
								_c->shaders.lines->uniform.colour = actor->fg_colour;
								_c->shaders.lines->uniform.n_channels = w->n_channels;
							}
#elif defined(ANTIALIASED_LINES)
							if(wfc->use_1d_textures){
								glDisable(GL_TEXTURE_1D);
								agl->shaders.text->uniform.fg_colour = actor->fg_colour;
								agl_use_program((AGlShader*)agl->shaders.text); // alpha shader
							}
							glEnable(GL_TEXTURE_2D);
#else
							if(wfc->use_1d_textures){
								glDisable(GL_TEXTURE_1D);
								agl->shaders.plain->uniform.colour = actor->fg_colour;
								agl_use_program((AGlShader*)agl->shaders.plain);
							}
							glDisable(GL_TEXTURE_2D);
#endif
}


bool
draw_wave_buffer_v_hi(Renderer* renderer, WaveformActor* actor, int block, bool is_first, bool is_last, double x_block0)
{
	//for use at resolution 1, operates on audio data, NOT peak data.

	// @b_region - sample range within the current block. b_region.start is relative to the Waveform, not the block.
	// @rect     - the canvas area corresponding exactly to the WfSampleRegion.                                          XXX changed.

	// variable names: variables prefixed with x_ relate to screen coordinates (pixels), variables prefixed with s_ related to sample frames.

	const Waveform* w = actor->waveform;
	const WaveformCanvas* wfc = actor->canvas;
	const WfActorPriv* _a = actor->priv;
	const RenderInfo* ri  = &_a->render_info;
	VHiRenderer* vhr = (VHiRenderer*)renderer;
	const WfRectangle* rect = &ri->rect;

	WfAudioData* audio = w->priv->audio_data;
	if(!audio->n_blocks) return false;
	WfBuf16* buf = audio->buf16[block];
	if(!buf) return false;

	if(is_last) vhr->block_region_v_hi.len = (ri->region.start + ri->region.len) % WF_SAMPLES_PER_TEXTURE;

	//alternative calculation of block_region_v_hi - does it give same results? NO
	uint64_t st = MAX((uint64_t)(ri->region.start),                  (uint64_t)((block)     * ri->samples_per_texture));
	uint64_t e  = MIN((uint64_t)(ri->region.start + ri->region.len), (uint64_t)((block + 1) * ri->samples_per_texture));
	WfSampleRegion block_region_v_hi2 = {st, e - st};
	//dbg(0, "block_region_v_hi=%Lu(%Lu)-->%Lu len=%Lu (buf->size=%Lu r->region=%Lu-->%Lu)", st, (uint64_t)block_region_v_hi.start, e, (uint64_t)block_region_v_hi2.len, ((uint64_t)buf->size), ((uint64_t)region.start), ((uint64_t)region.start) + ((uint64_t)region.len));
	WfSampleRegion b_region = block_region_v_hi2;

	g_return_val_if_fail(b_region.len <= buf->size, false);

	if(rect->left + rect->len < ri->viewport.left){
		gerr("rect is outside viewport");
	}

	_v_hi_set_gl_state(actor);

	//#define BIG_NUMBER 4096
	#define BIG_NUMBER 8192    // temporarily increased pending cropping to viewport-left

	Range sr = {{0,0},{0,0}, 0}; //TODO check we are consistent in that these values are all *within the current block*
	Range xr = {{0,0},{0,0}, 0};

#ifdef ANTIALIASED_LINES
	_wf_create_line_texture(); // will be moved. TODO dont re-fill the texture if display is unchanged.
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, line_textures[0]);
#endif

	const double zoom = rect->len / (double)ri->region.len;

	const float _block_wid = WF_SAMPLES_PER_TEXTURE * zoom;
	float block_rect_start = is_first ? fmodf(rect->left, _block_wid) : x_block0; // TODO simplify. why 2 separate cases needed?  ** try just using the first case
	WfRectangle b_rect = {block_rect_start, rect->top, b_region.len * zoom, rect->height};

	if(!(ri->viewport.right > b_rect.left)) gwarn("outside viewport: vp.r=%.2f b_rect.l=%.2f", ri->viewport.right, b_rect.left);
	g_return_val_if_fail(ri->viewport.right > b_rect.left, false);
	if(!(b_rect.left + b_rect.len > ri->viewport.left)) gwarn("outside viewport: vp.l=%.1f b_rect.l=%.1f b_rect.len=%.1f", ri->viewport.left, b_rect.left, b_rect.len);
	g_return_val_if_fail(b_rect.left + b_rect.len > ri->viewport.left, false);

#ifdef MULTILINE_SHADER
#elif defined(VERTEX_ARRAYS)
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	int n_lines = MIN(BIG_NUMBER, ri->viewport.right - b_rect.left); // TODO the arrays possibly can be smaller - dont use b_rect.left, use x0
	Quad quads[n_lines];
	Vertex texture_coords[n_lines * 4];
	int j; for(j=0;j<n_lines;j++){
		texture_coords[j * 4    ] = (Vertex){0.0, 0.0};
		texture_coords[j * 4 + 1] = (Vertex){1.0, 0.0};
		texture_coords[j * 4 + 2] = (Vertex){1.0, 1.0};
		texture_coords[j * 4 + 3] = (Vertex){0.0, 1.0};
	}
	glVertexPointer   (TWO_COORDS_PER_VERTEX, GL_FLOAT, 0, quads);
	glTexCoordPointer (TWO_COORDS_PER_VERTEX, GL_FLOAT, 0, texture_coords);
#endif

#ifndef MULTILINE_SHADER
	uint32_t rgba = actor->fg_colour;
	const float r = ((float)((rgba >> 24)       ))/0x100;
	const float g = ((float)((rgba >> 16) & 0xff))/0x100;
	const float b = ((float)((rgba >>  8) & 0xff))/0x100;
	const float alpha = ((float)((rgba  ) & 0xff))/0x100;
#endif

#ifdef MULTILINE_SHADER
	xr.border = TEX_BORDER_HI;
#else
	xr.border = 0;
#endif
	sr.border = xr.border / zoom;
	sr.inner.l = b_region.start % WF_SAMPLES_PER_TEXTURE;
	//const int x0 = MAX(0, floor(viewport->left - rect->left) - 3);
	const int x0 = b_rect.left;
	//const int x0 = MIN(b_rect.left, viewport->left - xr.border);         // no, x and s should not be calculated independently.
	//						sr.inner.l = (x0 - x_block0) / zoom; //TODO crop to viewport-left
	const int s0 = sr.outer.l = sr.inner.l - sr.border;
	xr.inner.l = x0;
	xr.outer.l = x0 - xr.border;
	const int x_bregion_end = b_rect.left + (b_region.start + b_region.len) * zoom;
	xr.inner.r = MIN(MIN(MIN(
		x0 + BIG_NUMBER,
		(int)(b_rect.left + b_rect.len)),
		ri->viewport.right),
		x_bregion_end);
	const int x_stop = xr.inner.r + xr.border;
	/*
	if(x_stop < b_rect.left + b_rect.len){
		dbg(0, "stopping early. x_bregion_end=%i x0+B=%i", x_bregion_end, x0 + BIG_NUMBER);
	}
	*/
	//dbg(0, "rect=%.2f-->%.2f b_region=%Lu-->%Lu viewport=%.1f-->%.1f zoom=%.3f", b_rect.left, b_rect.left + b_rect.len, b_region.start, b_region.start + ((uint64_t)b_region.len), viewport->left, viewport->right, zoom);

#ifdef MULTILINE_SHADER
	int mls_tex_w = x_stop - x0 + 2 * TEX_BORDER_HI;
	int mls_tex_h = 2; // TODO if we really are not going to add the indirection for x (for zoom > 1), use a 1d texture.
	int t_width = agl_power_of_two(mls_tex_w);
	guchar* _pbuf = g_new0(guchar, t_width * mls_tex_h);
	guchar* pbuf[] = {_pbuf, _pbuf + t_width};
#endif
										sr.inner.r = s0 + (xr.inner.r - xr.inner.l) / zoom;
										sr.outer.r = s0 /* TODO should include border? */+ ((double)x_stop - x0) / zoom;
										//dbg(0, "x0=%i x_stop=%i s=%i,%i,%i,%i xre=%i", x0, x_stop, sr.outer.l, sr.inner.l, sr.inner.r, sr.outer.r, x_bregion_end);

										// because we access adjacent samples, s_max is the absolute maximum index into the sample buffer (must be less than).
										int s_max = /*s0 + */sr.outer.r + 4 + sr.border;
										if(s_max > buf->size){
											int over = s_max - buf->size;
											if(over < 4 + sr.border) dbg(0, "TODO block overlap - need to access next block");
											else gerr("error at block changeover. s_max=%i over=%i", s_max, over);
										}else{
											// its fairly normal to be limited by region, as the region can be set deliberately to match the viewport.
											// (the region can either correspond to a defined Section/Part of the waveform, or dynamically created with the viewport
											// -if it is a defined Section, we must not go beyond it. As we do not know which case we have, we must honour the region limit)
											//uint64_t b_region_end1 = (region.start + region.len) % buf->size; //TODO this should be the same as b_region_end2 ? but appears to be too short
											uint64_t b_region_end2 = (!((b_region.start + b_region.len) % WF_SAMPLES_PER_TEXTURE)) ? WF_SAMPLES_PER_TEXTURE : (b_region.start + b_region.len) % WF_SAMPLES_PER_TEXTURE;//buf->size;
											//if(s_max > b_region_end2) gwarn("limited by region length. region_end=%Lu %Lu", (uint64_t)((region.start + region.len) % buf->size), b_region_end2);
											//s_max = MIN(s_max, (region.start + region.len) % buf->size);
											s_max = MIN(s_max, b_region_end2);
										}
										s_max = MIN(s_max, buf->size);
										//note that there is never any need to be separately limited by b_region - that should be taken care of by buffer limitation?

	int c; for(c=0;c<w->n_channels;c++){
																						if(!buf->buf[c]){ gwarn("audio buf not set. c=%i", c); continue; }
		if(!buf->buf[c]) continue;
#ifdef MULTILINE_SHADER
		int val0 = ((2*c + 1) * 128) / w->n_channels;
		if(mls_tex_w > TEX_BORDER_HI + 7) //TODO improve this test - make sure index is not negative  --- may not be needed (texture is bigger now (includes borders))
		memset((void*)((uintptr_t)pbuf[c] + (uintptr_t)(mls_tex_w - TEX_BORDER_HI)) - 7, val0, TEX_BORDER_HI + 7); // zero the rhs border in case it is not filled with content.

																						memset((void*)((uintptr_t)pbuf[c]), val0, t_width);
#endif

#ifndef MULTILINE_SHADER
		int oldx = x0 - 1;
		int oldy = 0;
#endif
	int s = 0;
	int i = 0;
	//int x; for(x = x0; x < x0 + BIG_NUMBER && /*rect->left +*/ x < viewport->right + border_right; x++, i++){
	// note that when using texture borders, at the viewport left edge x is NOT zero, it is TEX_BORDER_HI.
	int x; for(x = xr.inner.l; x < x_stop; x++, i++){
		double s_ = ((double)x - xr.inner.l) / zoom;
		double dist = s_ - s; // dist = distance in samples from one pixel to the next.
//if(i < 5) dbg(0, "x=%i s_=%.3f dist=%.2f", x, s_, dist);
		if (dist > 2.0) {
			//			if(dist > 5.0) gwarn("dist %.2f", dist);
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
		if (s0 + s >= (int)buf->size ) { gwarn("end of block reached: b_region.start=%i b_region.end=%Lu %i", s0, b_region.start + ((uint64_t)b_region.len), buf->size); break; }
		/*
		if (s  + 3 >= b_region.len) {
			gwarn("end of b_region reached: b_region.len=%i x=%i s0=%i s=%i", b_region.len, x, s0, s);
			break;
		}
		*/

		short* d = buf->buf[c];
		double y1 = (s0 + s   < s_max) ? d[s0 + s  ] : 0; //TODO have a separately loop for the last 4 values.
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
		if(i >= mls_tex_w){ gwarn("tex index out of range %i %i", i, mls_tex_w); break; }
		{
																				//int val = ((2*c + 1) * 128 + y) / w->n_channels;
																				//if(val < 0 || val > 256) gwarn("val out of range: %i", val); -- will be out of range when vgain is high.
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
	dbg(2, "n_lines=%i x=%i-->%i", i, x0, x);

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
		((float)((x_stop - x0) * TEXELS_PER_PIXEL)) / (float)t_width,
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


static void
v_hi_load_block(Renderer* renderer, WaveformActor* a, int b)
{
	if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
}


#ifdef ANTIALIASED_LINES
GLuint
_wf_create_line_texture()
{
	if(line_textures[0]) return line_textures[0];

	glEnable(GL_TEXTURE_2D);

#if 1
	int width = 4;
	int height = 5;
	char* pbuf = g_new0(char, width * height);
	int y;
	//char vals[] = {0xff, 0xa0, 0x40};
	char vals[] = {0xff, 0x40, 0x10};
	int x; for(x=0;x<width;x++){
		y=0; *(pbuf + y * width + x) = vals[2];
		y=1; *(pbuf + y * width + x) = vals[1];
		y=2; *(pbuf + y * width + x) = vals[0];
		y=3; *(pbuf + y * width + x) = vals[1];
		y=4; *(pbuf + y * width + x) = vals[2];
	}
#else
	int width = 4;
	int height = 4;
	char* pbuf = g_new0(char, width * height);
	int y; for(y=0;y<height/2;y++){
		int x; for(x=0;x<width;x++){
			*(pbuf + y * width + x) = 0xff * (2*y)/height + 128;
			*(pbuf + (height -1 - y) * width + x) = 0xff * (2*y)/height + 128;
		}
	}
#endif

	glGenTextures(2, line_textures);
	if(glGetError() != GL_NO_ERROR){ gerr ("couldnt create line_textures."); return 0; }
	dbg(2, "line_textureis[0]=%i", line_textures[0]);

	int pixel_format = GL_ALPHA;
	glBindTexture  (GL_TEXTURE_2D, line_textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);
	gl_warn("error binding line texture!");

	g_free(pbuf);

	return line_textures[0];
}
#endif


VHiRenderer v_hi_renderer = {{MODE_V_HI, v_hi_renderer_new, v_hi_load_block, v_hi_pre_render, draw_wave_buffer_v_hi}};


