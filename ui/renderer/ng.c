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
 | This renderer passes peak data directly to the shader in a 2D texture.
 |
 | Texture format:
 |    - each texture can hold up to 32 blocks.
 |    - each 8bit value directly the stores the peak value (either max or min)
 |    - each block is stored in 4 rows per block per channel
 |    - row layout:
 |      - row 0: max (positive) values
 |      - row 1: max values repeated at progressively lower x resolution (mipmap-like)
 |      - row 2: min (negative) values
 |      - row 3: min values repeated at progressively lower x resolution
 |
 |      - row 4...: for multi channel audio, the above is repeated for other channels.
 |
 |      ...(repeated for subsequent blocks up to 32)
 |
 |  Texture size is 16k per block per channel
 |
 |  A 10 hour 44100k mono audio file would use: 24000 blocks, 750 textures, 384MB.
 |
 |  Memory usage:
 |    - textures are timestamped when used.
 |    - resources for each WfWaveform are removed when the waveform is free'd.
 |    - to handle cases such as in a DAW where there are very large numbers of
 |      audio files, a garbage collector is run after resources are added. The
 |      collector removes the textures that have been used least recently.
 |
 */

#define NG_HASHTABLE
#define MAX_BLOCKS_PER_TEXTURE 32 // gives a texture size of 128k (256k stereo)
#define ROWS_PER_PEAK_TYPE 2
#define short_to_char(A) ((guchar)(A / 128))

typedef struct {
   guchar*   buffer;
   int       buffer_size;
   guint     texture;
   int       time_stamp;
   bool      completed;
   bool      ready[MAX_BLOCKS_PER_TEXTURE];
} Section;

typedef void (*WaveformActorBlockFn) (Renderer*, WaveformActor*, int b);

#define N_LOD 4
typedef struct {
	Renderer    renderer;
	WaveformActorBlockFn buf_to_tex;
#ifdef NG_HASHTABLE
	GHashTable* ng_data;          // TODO document what the purpose if any of the hash table is now that the waveform also has a reference.
	                              //      -it makes it slightly easier to iterate over all the waveforms/blocks to find the oldest.
#endif
	int         mmidx_max[N_LOD];
	int         mmidx_min[N_LOD];
	int         time_stamp;
} NGRenderer;


typedef struct {
   int       n_blocks;
   int       size;
   Section   section[];
} HiResNGWaveform;


static void ng_gl2_queue_clean (Renderer*);


static void
ng_finalize_notify (gpointer user_data, GObject* was)
{
	PF;
	Renderer* renderer = user_data;
	Waveform* waveform = (Waveform*)was;

	call(renderer->free, renderer, waveform, (void**)&waveform->priv->render_data[renderer->mode]);
}


#ifdef WF_DEBUG
static bool
ng_gl2_set (Section* section, int pos, char val)
{
	g_return_val_if_fail(section->buffer_size, false);
	g_return_val_if_fail(pos < section->buffer_size, false);
	section->buffer[pos] = val;
	return true;
}
static void
ng_gl2_set_ (Section* section, int pos, char val)
{
	g_return_if_fail(section->buffer_size);
	g_return_if_fail(pos < section->buffer_size);
	section->buffer[pos] = val;
}
#else
#define ng_gl2_set(section, pos, val) (section->buffer[pos] = val, true)
#define ng_gl2_set_(section, pos, val) (section->buffer[pos] = val)
#endif


static void
other_lods (Renderer* renderer, Section* section, int dest)
{
	int* lod_max = ((NGRenderer*)renderer)->mmidx_max;
	int* lod_min = ((NGRenderer*)renderer)->mmidx_min;

	for (int m=1;m<N_LOD;m++) {
		int mm_level = m;
		int mm = 1 << (mm_level - 1);
		int i,p; for (i=0, p=0; p<renderer->texture_size/mm; i++, p+=2) {
			ng_gl2_set_(section,
				dest + lod_max[mm_level] + i,
				MAX(
					section->buffer[dest + lod_max[mm_level - 1] + i * 2    ],
					section->buffer[dest + lod_max[mm_level - 1] + i * 2 + 1]
				)
			);
			ng_gl2_set_(section, dest + lod_min[mm_level] + i, MAX(
				section->buffer[dest + lod_min[mm_level - 1] + i * 2    ],
				section->buffer[dest + lod_min[mm_level - 1] + i * 2 + 1]
			));
		}
	}
}


static void
ng_gl2_load_block (Renderer* renderer, WaveformActor* actor, int b)
{
	NGRenderer* ng_renderer = (NGRenderer*)renderer;
	Waveform* waveform = actor->waveform;
	WaveformPrivate* w = waveform->priv;

	#define get_block_size(ACTOR) (renderer->texture_size * waveform_get_n_channels(ACTOR->waveform) * WF_PEAK_VALUES_PER_SAMPLE * ROWS_PER_PEAK_TYPE)

	Section* add_section (Renderer* renderer, WaveformActor* actor, HiResNGWaveform* data, int s)
	{
		int block_size = get_block_size(actor);
		int buffer_size = block_size * MIN(MAX_BLOCKS_PER_TEXTURE, waveform_get_n_audio_blocks(waveform) - s * MAX_BLOCKS_PER_TEXTURE);
		dbg(1, "%s %i block_size=%ik section->buffer=%ik", modes[renderer->mode].name, s, block_size / 1024, buffer_size / 1024);

		Section* section = &data->section[s];
		section->buffer = g_malloc0(section->buffer_size = buffer_size);

		section->time_stamp = ((NGRenderer*)renderer)->time_stamp++;

		ng_gl2_queue_clean(renderer);

		return section;
	}

	bool section_is_complete (WaveformActor* actor, Section* section)
	{
		int max = MIN(waveform_get_n_audio_blocks(waveform), MAX_BLOCKS_PER_TEXTURE);
		int i;for(i=0;i<max;i++){
			if(!section->ready[i]) return false;
		}
		dbg(1, "complete");
		return section->completed = true;
	}

	inline void lo_peakbuf_to_texture (Renderer* renderer, WaveformActor* actor, int b, Section* section, int n_chans, int block_size)
	{
		// borders: source data is not blocked so borders need to be added here.

		Waveform* waveform = actor->waveform;
		WaveformPrivate* w = waveform->priv;
		WfPeakBuf* peak = &w->peak;
		int _b = b % MAX_BLOCKS_PER_TEXTURE;

		int mm_level = 0;
		int* lod_max = ((NGRenderer*)renderer)->mmidx_max;
		int* lod_min = ((NGRenderer*)renderer)->mmidx_min;

		#define B_SIZE (WF_PEAK_TEXTURE_SIZE - 2 * TEX_BORDER)
		#define is_last_block(W, b) (b == ((HiResNGWaveform*)W->render_data[MODE_LOW])->n_blocks - 1)

		int stop = is_last_block(w, b)
			? peak->size / (WF_PEAK_VALUES_PER_SAMPLE * WF_PEAK_STD_TO_LO) + TEX_BORDER - B_SIZE * b
			: WF_PEAK_TEXTURE_SIZE;

		for (int c=0;c<n_chans;c++) {
			int src = WF_PEAK_VALUES_PER_SAMPLE * (b * B_SIZE - TEX_BORDER) * WF_PEAK_STD_TO_LO;
			int dest = _b * block_size + (c * block_size / 2);

			int t = 0;
			if (_b == 0) {
				for(t=0;t<TEX_BORDER;t++){
					ng_gl2_set_(section, dest + lod_max[mm_level] + t, 0);
					ng_gl2_set_(section, dest + lod_min[mm_level] + t, 0);
				}
				src = 0;
			}

			for (; t<stop; t++, src+=2*WF_PEAK_STD_TO_LO) {
				WfPeak p = {0, 0};
				for (int j=0;j<2*WF_PEAK_STD_TO_LO;j+=WF_PEAK_VALUES_PER_SAMPLE) {
					p.positive = MAX(p.positive, peak->buf[c][src + j    ]);
					p.negative = MIN(p.negative, peak->buf[c][src + j + 1]);
				}

				ng_gl2_set_(section, dest + lod_max[mm_level] + t, short_to_char( p.positive));
				ng_gl2_set_(section, dest + lod_min[mm_level] + t, short_to_char(-p.negative));
			}

			other_lods(renderer, section, dest);
		}
	}

	inline void med_peakbuf_to_texture (Renderer* renderer, WaveformActor* actor, int b, Section* section, int n_chans, int block_size)
	{
		// borders: source data is not blocked so borders are added here.

		g_return_if_fail(n_chans > -1);
		g_return_if_fail(n_chans <= 2);

		Waveform* waveform = actor->waveform;
		WaveformPrivate* w = waveform->priv;
		WfPeakBuf* peak = &w->peak;
		int _b = b % MAX_BLOCKS_PER_TEXTURE;

		int mm_level = 0;
		int* lod_max = ((NGRenderer*)renderer)->mmidx_max;
		int* lod_min = ((NGRenderer*)renderer)->mmidx_min;

		int stop = (b == waveform_get_n_audio_blocks(waveform) - 1)
			? peak->size / WF_PEAK_VALUES_PER_SAMPLE + TEX_BORDER - WF_TEXTURE_VISIBLE_SIZE * b
			: WF_PEAK_TEXTURE_SIZE;

		int c; for(c=0;c<n_chans;c++){
			int src = WF_PEAK_VALUES_PER_SAMPLE * (b * WF_TEXTURE_VISIBLE_SIZE - TEX_BORDER);
			int dest = _b * block_size + (c * block_size / 2);

			int t = 0;
			if (_b == 0) {
				for (t=0;t<TEX_BORDER;t++) {
					ng_gl2_set_(section, dest + lod_max[mm_level] + t, 0);
					ng_gl2_set_(section, dest + lod_min[mm_level] + t, 0);
				}
				src = 0;
			}

			for(; t<stop; t++, src+=2){
				ng_gl2_set_(section, dest + lod_max[mm_level] + t, short_to_char( peak->buf[c][src    ]));
				ng_gl2_set_(section, dest + lod_min[mm_level] + t, short_to_char(-peak->buf[c][src + 1]));
			}

			other_lods(renderer, section, dest);
		}
	}

	inline void hi_audio_to_texture (Renderer* renderer, WaveformActor* actor, int b, Section* section, int n_chans, int block_size)
	{
		// borders: Source data blocks are correctly sized but an offset needs to be added.
		//          The left hand border is currently empty which is ok if there is no texture post-processing.

		Waveform* waveform = actor->waveform;
		int _b = b % MAX_BLOCKS_PER_TEXTURE;

		// we are here as notification that audio has loaded so it is an error if not.
		g_return_if_fail(waveform->priv->audio.buf16);
		WfBuf16* audio_buf = waveform->priv->audio.buf16[b];
		g_return_if_fail(audio_buf);

		#define IO_RATIO 16
		#define DELAY ((int)(TEX_BORDER_HI * IO_RATIO))

		short max[n_chans];
		short min[n_chans];
		for (int c=0;c<n_chans;c++) {
			int B = _b * block_size + (c * block_size / 2);
			int mm_level = 0;
			int i, p; for(i=0, p=0; p<WF_PEAK_BLOCK_SIZE - DELAY; i++, p+= IO_RATIO){

				short* d = &audio_buf->buf[c][p];
				max[c] = 0;
				min[c] = 0;
				for (int k=0;k<IO_RATIO;k++) {
					max[c] = (d[k + c] > max[c]) ? d[k + c] : max[c];
					min[c] = (d[k + c] < min[c]) ? d[k + c] : min[c];
				}

				ng_gl2_set_(section, B + ((NGRenderer*)renderer)->mmidx_max[mm_level] + ((int)TEX_BORDER_HI) + i, short_to_char(max[c]));
				ng_gl2_set_(section, B + ((NGRenderer*)renderer)->mmidx_min[mm_level] + ((int)TEX_BORDER_HI) + i, short_to_char(-min[c]));
			}

			other_lods(renderer, section, B);
		}
	}

	int n_chans = waveform_get_n_channels(waveform);

	HiResNGWaveform** data = (HiResNGWaveform**)&w->render_data[renderer->mode];
#ifdef NG_HASHTABLE
#ifdef WF_DEBUG
	{
		HiResNGWaveform* data1 = g_hash_table_lookup(((NGRenderer*)renderer)->ng_data, waveform);
		if((*data) != data1) pwarn("%i: wav=%p hash=%p %s", b, *data, data1, modes[renderer->mode].name);
		g_return_if_fail((*data) == data1);
	}
#endif
#endif
	if(!*data){
#ifdef DEBUG
		if (!renderer->shader) {
			static bool done = false;
			if (!done) perr("renderer not initialised?");
			done = true;
		}
#endif
		int n_sections = waveform_get_n_audio_blocks(waveform) / MAX_BLOCKS_PER_TEXTURE + (waveform_get_n_audio_blocks(waveform) % MAX_BLOCKS_PER_TEXTURE ? 1 : 0);

		*(*data = g_malloc0(sizeof(HiResNGWaveform) + sizeof(Section) * n_sections)) = (HiResNGWaveform){
			.size = n_sections,
			.n_blocks = wf_actor_get_n_blocks(waveform, renderer->mode)
		};

		g_object_weak_ref((GObject*)waveform, ng_finalize_notify, renderer);
#ifdef NG_HASHTABLE
		g_hash_table_insert(((NGRenderer*)renderer)->ng_data, actor->waveform, *data);
#endif
	}

	bool texture_changed[(*data)->size];
	memset(texture_changed, 0, sizeof(bool) * (*data)->size);

	int block_size = get_block_size(actor);
	{
		int s  = b / MAX_BLOCKS_PER_TEXTURE;
		int _b = b % MAX_BLOCKS_PER_TEXTURE;
		Section* section = &(*data)->section[s];
															// TODO move timestamp to render
		section->time_stamp = ((NGRenderer*)renderer)->time_stamp++;
		if(section->completed) return;
		if(!section->buffer) section = add_section(renderer, actor, *data, s);
		if(!section->ready[_b]){
			texture_changed[s] = true;
			call(ng_renderer->buf_to_tex, renderer, actor, b);
			switch(renderer->mode){
				case MODE_LOW:
					lo_peakbuf_to_texture(renderer, actor, b, section, n_chans, block_size);
					break;
				case MODE_MED:
					med_peakbuf_to_texture(renderer, actor, b, section, n_chans, block_size);
					break;
				case MODE_HI:
					hi_audio_to_texture(renderer, actor, b, section, n_chans, block_size);
					break;
				case MODE_V_LOW:
					// texture data is added in NGRenderer.buf_to_tex
					;int c; for(c=0;c<n_chans;c++){
						int dest = _b * block_size + (c * block_size / 2);
						other_lods(renderer, section, dest);
					}
					break;
				default:
					break;
			}
			section->ready[_b] = true;
		}
	}

	for(int s=0;s<(*data)->size;s++){
		Section* section = &(*data)->section[s];
		if(!section->completed){
			if(texture_changed[s]){
				if(!section->texture){
					// note: for the WaveformBlock we use the first block for the section (WaveformBlock concept is broken in this context)
					section->texture = texture_cache_assign_new(GL_TEXTURE_2D, (WaveformBlock){waveform, (s * MAX_BLOCKS_PER_TEXTURE) | (renderer->mode == MODE_HI ? WF_TEXTURE_CACHE_HIRES_NG_MASK : 0)});
				}

				int width = renderer->texture_size;
				int height = section->buffer_size / width;
				#define pixel_format GL_ALPHA
				agl_use_texture (section->texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				// TODO it is quite common for this to be done several times in quick succession for the same texture with consecutive calls to ng_gl2_load_block
				dbg(1, "%i: uploading texture: %i x %i", s, width, height);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, section->buffer);
				gl_warn("error binding texture: %u", section->texture);
			}

			if(section_is_complete(actor, section)){
				g_free0(section->buffer); // all data has been sent to the gpu so can be freed.
			}
		}
	}
}


/*
 *  This is done only once per paint, it does not have to be done per block
 */
static bool
ng_pre_render (Renderer* renderer, WaveformActor* actor)
{
	Waveform* w = actor->waveform;
	WfActorPriv* _a = actor->priv;
	RenderInfo* r = &_a->render_info;

	if (r->mode >= MODE_V_HI) return false; // TODO this will happen when falling through.

	HiResNGWaveform* data = (HiResNGWaveform*)w->priv->render_data[renderer->mode];
	AGlShader* shader = renderer->shader;
	if (!data || !shader) return false; // this can happen when we fall through from v hi res.

	AGlUniformUnion* u = (AGlUniformUnion*)shader->uniforms;
	agl_set_colour_uniform(&shader->uniforms[NG_U_FG_COLOUR], (((AGlActor*)actor)->colour & 0xffffff00) + (unsigned)(0xff * _a->opacity));
	u[NG_U_TOP].value.f[0] = r->rect.top;
	u[NG_U_BOTTOM].value.f[0] = r->rect.top + r->rect.height;
	u[NG_U_N_CHANNELS].value.i[0] = waveform_get_n_channels(w);
	shader->uniforms[NG_U_TEX_WIDTH].value[0] = renderer->texture_size;
	shader->uniforms[NG_U_TEX_HEIGHT].value[0] = data->section[r->viewport_blocks.first / MAX_BLOCKS_PER_TEXTURE].buffer_size / renderer->texture_size;
	shader->uniforms[NG_U_VGAIN].value[0] = actor->context->v_gain;

	// trade off here between performance and sharpness
	// but the waveforms look much nicer using lower levels of detail.
	u[NG_U_MM_LEVEL].value.i[0] = (renderer->mode == MODE_MED || renderer->mode == MODE_LOW || renderer->mode == MODE_V_LOW)
		? (
		r->block_wid > 96
			? 0
			: r->block_wid > 48
				? 1
				: r->block_wid > 24
					? 2
					: 3
		)
		: (
		r->block_wid > 2048
			? 0
			: r->block_wid > 1024
				? 1
				: r->block_wid > 512
					? 2
					: 3
		);

	agl_scale (shader, 1., 1.);
	agl_translate (shader, -((AGlActor*)actor)->scrollable.x1, 0.);
	shader->set_uniforms_(shader);

	glActiveTexture (GL_TEXTURE0);
	glBindBuffer (GL_ARRAY_BUFFER, agl->vbo);

	glEnableVertexAttribArray (0);
	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	return true;
}


static bool
ng_gl2_render_block (Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x)
{
	gl_warn("pre");

	int border = renderer->mode == MODE_HI ? TEX_BORDER_HI : TEX_BORDER;

	Waveform* waveform = actor->waveform; 
	WfActorPriv* _a = actor->priv;
	RenderInfo* r = &_a->render_info;

	int  s = b / MAX_BLOCKS_PER_TEXTURE;
	int _b = b % MAX_BLOCKS_PER_TEXTURE;

	HiResNGWaveform* data = (HiResNGWaveform*)waveform->priv->render_data[renderer->mode];
	if (!data) return false; // this can happen when audio data not yet available.
	Section* section = &data->section[s];

	if (!_b && b != r->viewport_blocks.first) {
		AGlShader* shader = renderer->shader;
		shader->uniforms[NG_U_TEX_HEIGHT].value[0] = section->buffer_size / renderer->texture_size;
		shader->set_uniforms_((AGlShader*)shader);
	}

	TextureRange tex;
	WfSampleRegionf block;
	if (!wf_actor_get_quad_dimensions(actor, b, is_first, is_last, x, &tex, &block.start, &block.len, border, 1)) return false;

	float n_rows = section->buffer_size / renderer->texture_size;
	float ty = (b % MAX_BLOCKS_PER_TEXTURE) * 4.0 * waveform->n_channels / n_rows; // this tells the shader which block to use.
	AGlQuad tex_rect = {tex.start, ty, tex.end, ty + 0.001}; // the 0.001 prevents the wrong block being shown on some systems

	//dbg(0, "b=%i %u n_rows=%f x=%f-->%f y=%f (%f)", b % MAX_BLOCKS_PER_TEXTURE, section->texture, n_rows, tex.start, tex.end, ty, ((float)(b % MAX_BLOCKS_PER_TEXTURE) * 4.0 * waveform->n_channels));

	agl_textured_rect_fast (section->texture, block.start, r->rect.top, block.len, r->rect.height, &tex_rect);

	return true;
}


static void
ng_post_render (Renderer* renderer, WaveformActor* actor)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);  
}


static void
ng_make_lod_levels (NGRenderer* renderer, int texture_width)
{
	int p = 0;
	int level_size = texture_width;
	for (int i=0;i<N_LOD;i++){
		renderer->mmidx_max[i] = p;
		renderer->mmidx_min[i] = p + texture_width * ROWS_PER_PEAK_TYPE;
		level_size = texture_width / (1 << i);
		p += level_size;
	}
}


static void
ng_free_section (Renderer* renderer, Waveform* waveform, Section* section, int s)
{
	if (section) {
		g_clear_pointer(&section->buffer, g_free);
		if (section->texture) {
			texture_cache_remove(GL_TEXTURE_2D, waveform, (s * MAX_BLOCKS_PER_TEXTURE) | (renderer->mode == MODE_HI ? WF_TEXTURE_CACHE_HIRES_NG_MASK : 0));
			section->texture = 0;
		}
		section->completed = false;
		memset(section->ready, 0, sizeof(bool) * MAX_BLOCKS_PER_TEXTURE);
	}
}


/*
 *  Use __data where possible as it is more versatile than tying data to the Waveform
 */
static void
ng_free_waveform (Renderer* renderer, Waveform* waveform, void** __data)
{
	HiResNGWaveform** _data = (HiResNGWaveform**)__data;
	dbg(1, "%s", modes[renderer->mode].name);

#ifdef NG_HASHTABLE
	HiResNGWaveform* data = ((NGRenderer*)renderer)->ng_data
		? g_hash_table_lookup(((NGRenderer*)renderer)->ng_data, waveform)
		: _data
			? *_data
			: (HiResNGWaveform*)waveform->priv->render_data[renderer->mode];
#else
	HiResNGWaveform* data = _data ? *_data : (HiResNGWaveform*)waveform->priv->render_data[renderer->mode];
#endif
	if (data) {
		// the sections must be freed before removing from the hashtable
		// so that the Waveform can be referenced.
		for (int s=0;s<data->size;s++) {
			ng_free_section(renderer, waveform, &data->section[s], s);
		}
#ifdef NG_HASHTABLE
		// removing from the hash table will cause the item to be free'd.
		if (((NGRenderer*)renderer)->ng_data) {
			if (g_hash_table_remove(((NGRenderer*)renderer)->ng_data, waveform)) {
				if (_data) *_data = NULL;
			} else {
				dbg(1, "failed to remove render data");
			}
		}
#endif
	}

	if (_data)
		g_clear_pointer(_data, g_free);
	else
		waveform->priv->render_data[renderer->mode] = NULL;
}


	#define MAX_SECTIONS 1024

	static guint idle_id = 0;

		static struct _oldest {
			Waveform*        waveform;
			HiResNGWaveform* data;
			int              section;
			int              time_stamp;
		} oldest;

		static void __hi_find_oldest(gpointer key, gpointer value, gpointer _)
		{
			HiResNGWaveform* data = (HiResNGWaveform*)value;
			for (int s=0;s<data->size;s++) {
				Section* section = &data->section[s];
				dbg(0, ">  %i", section->time_stamp);
				if (section->buffer && (section->time_stamp < oldest.time_stamp)) {
					oldest = (struct _oldest){key, data, s, section->time_stamp};
				}
			}
		}

static gboolean
__clean (gpointer user_data)
{
	Renderer* renderer = user_data;

#ifdef NG_HASHTABLE
	GHashTable* table = ((NGRenderer*)renderer)->ng_data;
#endif

	dbg(1, "size=%i", g_hash_table_size(table));

	if (g_hash_table_size(table) > MAX_SECTIONS) {
		int n_to_remove = g_hash_table_size(table) - MAX_SECTIONS;
		for (int i=0;i<n_to_remove;i++) {
			oldest = (struct _oldest){NULL, 0, INT_MAX};
			g_hash_table_foreach(table, __hi_find_oldest, NULL);

			if (oldest.data) {
				dbg(0, "removing: section=%i", oldest.section);
				ng_free_section(renderer, oldest.waveform, &oldest.data->section[oldest.section], oldest.section);
			}
		}
	}

	idle_id = 0;
	return G_SOURCE_REMOVE;
}


static void
ng_gl2_queue_clean (Renderer* renderer)
{
	if (!idle_id) idle_id = g_idle_add_full(G_PRIORITY_LOW, __clean, renderer, NULL);
}
