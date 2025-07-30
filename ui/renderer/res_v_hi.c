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
extern LinesShader lines;
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

#if defined (MULTILINE_SHADER)
	if(agl->use_shaders){
		agl_create_program(&lines.shader);
		modes[MODE_V_HI].renderer->shader = &lines.shader;
	}
#else
	if(!agl->aaline) agl->aaline = agl_aa_line_new();

	modes[MODE_V_HI].renderer->shader = aaline_class.shader;
#endif

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &hivbo);
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
	renderer->pre_render = v_hi_pre_render;

	return v_hi_pre_render(renderer, actor);
}


static void
_v_hi_set_gl_state (WaveformActor* actor)
{
	const WaveformContext* wfc = actor->context;

#if defined (MULTILINE_SHADER)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (wfc->use_1d_textures) {
		lines.uniform.colour = ((AGlActor*)actor)->colour;
		lines.uniform.n_channels = actor->waveform->n_channels;
	}
#else
	AGl* agl = agl_get_instance();

	if (wfc->use_1d_textures) {
		agl->shaders.alphamap->uniform.fg_colour = ((AGlActor*)actor)->colour;
		agl_use_material(agl->aaline);
	} else {
		agl_enable(AGL_ENABLE_BLEND);
	}
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
	buf->stamp = ++wf->audio.access_counter;

	#define MAX_SCREEN_SIZE 8192

	_v_hi_set_gl_state(actor);

	float x_b0 = f_2_px(actor, ri->zoom, 0) - ((AGlActor*)actor)->region.x1;
	WfRectangle b_rect = {
		.left = x_b0 + block * ri->block_wid,
		.len = ri->block_wid,
	};

	AGlfRegion region = {
		.x2 = ((AGlActor*)actor)->region.x2 - ((AGlActor*)actor)->region.x1,
	};

	AGlfRegion cropped;
	agl_actor__calc_visible(
		&(AGlActor){
			.region = { .x1 = b_rect.left, .x2 = MIN(b_rect.left + b_rect.len, region.x2), .y2 = 10 },
			.parent = (AGlActor*)actor,
		},
		&cropped
	);

	const int x_stop = b_rect.left + cropped.x2;
	Range xr = {
		.inner = {
			MAX(0, b_rect.left + cropped.x1),
			x_stop
		},
		.outer = {
			MAX(0, b_rect.left + cropped.x1),
			x_stop
		},
#ifdef MULTILINE_SHADER
		.border = TEX_BORDER_HI
#endif
	};

	Range sr = {
		.inner.l = is_first ? px_2_f(actor, ri->zoom, xr.inner.l + ((AGlActor*)actor)->region.x1) % WF_SAMPLES_PER_TEXTURE : 0,
	};
	sr.outer.l = sr.inner.l - sr.border;

#ifndef MULTILINE_SHADER
	const int n_lines = MIN(MAX_SCREEN_SIZE, x_stop - xr.inner.l);
	g_return_val_if_fail(n_lines > 0, false);
	AGlTQuad quads[n_lines];
#endif

#ifdef MULTILINE_SHADER
	int mls_tex_w = x_stop - 2 * TEX_BORDER_HI;
	int mls_tex_h = 2; // TODO if we really are not going to add the indirection for x (for zoom > 1), use a 1d texture.
	int t_width = agl_power_of_two(mls_tex_w);
	guchar* _pbuf = g_new0(guchar, t_width * mls_tex_h);
	guchar* pbuf[] = {_pbuf, _pbuf + t_width};
#endif

	sr.inner.r = sr.outer.l + (xr.inner.r - xr.inner.l) / ri->zoom;
	sr.outer.r = sr.outer.l + ((double)x_stop) / ri->zoom;
	sr.outer.r = sr.outer.l + (xr.outer.r - xr.outer.l) / ri->zoom;

										// because we access adjacent samples, s_max is the absolute maximum index into the sample buffer (must be less than).
										int s_max = sr.outer.r + 4 + sr.border;
										if(s_max > buf->size){
											int over = s_max - buf->size;
											if (over < 4 + sr.border) {
												static bool done = false;
												if (!done) dbg(0, "TODO block overlap - need to access next block");
												done = true;
											}
											else perr("error at block changeover. s_max=%i over=%i", s_max, over);
										}
										g_return_val_if_fail(s_max <= buf->size, false);

	float gain = wfc->v_gain * (ri->rect.height / (2.0 * w->n_channels)) / (1 << 15);

	const int s0 = sr.outer.l;
	for(int c=0;c<w->n_channels;c++){
		if(!buf->buf[c]) continue;

#ifdef MULTILINE_SHADER
		int val0 = ((2*c + 1) * 128) / w->n_channels;
		if(mls_tex_w > TEX_BORDER_HI + 7) //TODO improve this test - make sure index is not negative  --- may not be needed (texture is bigger now (includes borders))
		memset((void*)((uintptr_t)pbuf[c] + (uintptr_t)(mls_tex_w - TEX_BORDER_HI)) - 7, val0, TEX_BORDER_HI + 7); // zero the rhs border in case it is not filled with content.

																						memset((void*)((uintptr_t)pbuf[c]), val0, t_width);
#endif

#ifndef MULTILINE_SHADER
		int oldx = xr.inner.l - 1;
		float oldy = 0;
#endif
		double ds = (1. / ri->zoom) * (x_stop - xr.inner.l + 1) / (x_stop - xr.inner.l);
		int s = -ds;
		int i = 0;
		int x;
		// note that when using texture borders, at the viewport left edge x is NOT zero, it is TEX_BORDER_HI.
		for(x = xr.inner.l; x < x_stop; x++, i++){
			s = i * ds;

#ifdef MULTILINE_SHADER
			if (s0 + s < 0){ // left border and no valid data.
				pbuf[c][i] = val0;
				continue;
			}
#endif
			if (s0 + s >= (int)buf->size ) { pwarn("end of block reached at s=%i: b_region%i..%i buf.size=%i ds=%.1f", s, s0, sr.inner.r, buf->size, 1. / ri->zoom); break; }

			short* d = buf->buf[c];

			float val[2] = {0.};
			for (int i=0;i<ds;i++) {
				val[0] = MAX(val[0], d[s0 + s + i]);
				val[1] = MIN(val[1], d[s0 + s + i]);
			}
			float y = (val[0] > -val[1] ? val[0] : val[1]) * gain;

#if defined (MULTILINE_SHADER)
			if(i >= mls_tex_w){ pwarn("tex index out of range %i (max %i)", i, mls_tex_w); break; }
			{
				pbuf[c][i] = val0 + MIN(y, 63); // the 63 is to stop it wrapping - would be nice to remove this.
			}
#else
			{
				float x0 = oldx - ((AGlActor*)actor)->scrollable.x1;
				float y0 = rect->top - oldy + rect->height / 2;
				float x1 = x - ((AGlActor*)actor)->scrollable.x1;
				float y1 = rect->top -    y + rect->height / 2;

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
		dbg(2, "%i: n_lines=%i x=%i-->%i", block, i, xr.inner.l, x);

		if (w->n_channels == 2) {
			agl_translate ((AGlShader*)agl->shaders.alphamap, 0., - rect->height/4. + c * rect->height/2.);
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


