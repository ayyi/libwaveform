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

extern AGlMaterialClass aaline_class;

#define VERTEX_ARRAYS
#define TWO_COORDS_PER_VERTEX 2

static unsigned int vao = 0;
static unsigned int vhivbo = 0;

typedef struct {int l, r;} Range;

typedef struct {
	Range inner;
	Range outer;
	int border;
} Extent;


static void
v_hi_renderer_new (WaveformActor* actor)
{
	WaveformPrivate* w = actor->waveform->priv;

	g_return_if_fail(!w->render_data[MODE_V_HI]);

	w->render_data[MODE_V_HI] = WF_NEW(WaveformModeRender,
		.n_blocks = w->n_blocks
	);

	if (!agl.aaline) agl.aaline = agl_aa_line_new();

	modes[MODE_V_HI].renderer->shader = aaline_class.shader;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vhivbo);
	agl_use_va(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vhivbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(AGlVertex), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(AGlVertex), (void*)sizeof(AGlVertex));
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
	ALPHAMAP_COLOUR() = ((AGlActor*)actor)->colour;
	agl_use_material(agl.aaline);
}


bool
draw_wave_buffer_v_hi (Renderer* renderer, WaveformActor* actor, int block, bool is_first, bool is_last, double x_block0)
{
	// For use at resolution 1, operates directly on audio data, NOT peak data.

	// variable names: variables prefixed with x_ relate to screen coordinates (pixels), variables prefixed with s_ related to sample frames.

	// drawing is done in regular local coordinate space (ie. relative to actor->scrollable.x1)

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
			.region = { .x1 = b_rect.left, .x2 = MIN(b_rect.left + b_rect.len, region.x2 - ((AGlActor*)actor)->scrollable.x1), .y2 = 10 },
			.parent = (AGlActor*)actor,
		},
		&cropped
	);

	const int x_stop = b_rect.left + cropped.x2;
	Extent xr = {
		.inner = {
			MAX(0, b_rect.left + cropped.x1),
			x_stop
		},
	};
	xr.outer = xr.inner;

	Extent sr = {
		.inner.l = is_first ? px_2_f(actor, ri->zoom, xr.inner.l + ((AGlActor*)actor)->region.x1) % WF_SAMPLES_PER_TEXTURE : 0,
	};
	sr.outer.l = sr.inner.l - sr.border;

	const int n_lines = MIN(MAX_SCREEN_SIZE, x_stop - xr.inner.l);
	g_return_val_if_fail(n_lines > 0, false);
	AGlTQuad quads[n_lines];

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
	for (int c=0;c<w->n_channels;c++) {
		if (!buf->buf[c]) continue;

		int oldx = xr.inner.l - 1;
		float oldy = 0;
		double ds = (1. / ri->zoom) * (x_stop - xr.inner.l + 1) / (x_stop - xr.inner.l);
		int s = -ds;
		int i = 0;
		int x;
		// note that when using texture borders, at the viewport left edge x is NOT zero, it is TEX_BORDER_HI.
		for (x = xr.inner.l; x < x_stop; x++, i++) {
			s = i * ds;

			if (s0 + s >= (int)buf->size ) { pwarn("end of block reached at s=%i: b_region%i..%i buf.size=%i ds=%.1f", s, s0, sr.inner.r, buf->size, 1. / ri->zoom); break; }

			short* d = buf->buf[c];

			float val[2] = {0.};
			for (int i=0;i<ds;i++) {
				val[0] = MAX(val[0], d[s0 + s + i]);
				val[1] = MIN(val[1], d[s0 + s + i]);
			}
			float y = (val[0] > -val[1] ? val[0] : val[1]) * gain;

			{
				float x0 = oldx;
				float y0 = rect->top - oldy + rect->height / 2;
				float x1 = x;
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
		}
		dbg(2, "%i: n_lines=%i x=%i-->%i", block, i, xr.inner.l, x);

		if (w->n_channels == 2) {
			agl_translate ((AGlShader*)agl.shaders.alphamap, 0., - rect->height/4. + c * rect->height/2.);
		}

		agl_use_va(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vhivbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quads), quads, GL_STATIC_DRAW);

		glDrawArrays(GL_TRIANGLES, 0, i * AGL_V_PER_QUAD);
	} // end channel

	// increment for next block
	vhr->block_region_v_hi.start = (vhr->block_region_v_hi.start / WF_PEAK_BLOCK_SIZE + 1) * WF_PEAK_BLOCK_SIZE;
	vhr->block_region_v_hi.len   = WF_PEAK_BLOCK_SIZE - vhr->block_region_v_hi.start % WF_PEAK_BLOCK_SIZE;

	return true;
}


static void
v_hi_load_block (Renderer* renderer, WaveformActor* a, int b)
{
	if (((AGlActor*)a)->root->draw) wf_context_queue_redraw(a->context);
}


static void
v_hi_free_waveform (Renderer* renderer, Waveform* w, void** data)
{
	g_clear_pointer(&w->priv->render_data[MODE_V_HI], g_free);
}


#ifdef USE_TEST
static void
v_hi_unref (Renderer* renderer)
{
	if (--renderer->ref_count < 1) {
		glDeleteBuffers (1, &vhivbo);
		vhivbo = 0;
	}
}
#endif


VHiRenderer v_hi_renderer = {{
	MODE_V_HI, v_hi_renderer_new, v_hi_load_block, v_hi_pre_render0, draw_wave_buffer_v_hi, NULL, v_hi_free_waveform,
#ifdef USE_TEST
	.unref = v_hi_unref
#endif
}};
