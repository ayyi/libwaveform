/*
  copyright (C) 2014-2017 Tim Orford <tim@orford.org>

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
	AGlShader*  shader;
} NGRenderer;


typedef struct {
   int       n_blocks;
   int       size;
   Section   section[];
} HiResNGWaveform;


static void ng_gl2_queue_clean(Renderer*);


static void
ng_gl2_finalize_notify(gpointer user_data, GObject* was)
{
	PF;
	Renderer* renderer = user_data;
	Waveform* waveform = (Waveform*)was;
	call(renderer->free, renderer, waveform);
}


#ifdef WF_DEBUG
static bool
ng_gl2_set(Section* section, int pos, char val)
{
	g_return_val_if_fail(section->buffer_size, false);
	g_return_val_if_fail(pos < section->buffer_size, false);
	section->buffer[pos] = val;
	return true;
}
#else
#define ng_gl2_set(section, pos, val) (section->buffer[pos] = val, true)
#define ng_gl2_set_(section, pos, val) (section->buffer[pos] = val)
#endif


static void
ng_gl2_load_block(Renderer* renderer, WaveformActor* actor, int b)
{
	NGRenderer* ng_renderer = (NGRenderer*)renderer;
	Waveform* waveform = actor->waveform;
	WaveformPriv* w = waveform->priv;

	#define get_block_size(ACTOR) (modes[renderer->mode].texture_size * waveform_get_n_channels(ACTOR->waveform) * WF_PEAK_VALUES_PER_SAMPLE * ROWS_PER_PEAK_TYPE)

	Section* add_section(Renderer* renderer, WaveformActor* actor, HiResNGWaveform* data, int s)
	{
		int block_size = get_block_size(actor);
		int buffer_size = block_size * MIN(MAX_BLOCKS_PER_TEXTURE, waveform_get_n_audio_blocks(waveform) - s * MAX_BLOCKS_PER_TEXTURE);
		dbg(1, "%s block_size=%ik section->buffer=%ik", modes[renderer->mode].name, block_size / 1024, buffer_size / 1024);

		Section* section = &data->section[s];
		section->buffer = g_malloc0(section->buffer_size = buffer_size);

		section->time_stamp = ((NGRenderer*)renderer)->time_stamp++;

		ng_gl2_queue_clean(renderer);

		return section;
	}

	bool section_is_complete(WaveformActor* actor, Section* section)
	{
		int max = MIN(waveform_get_n_audio_blocks(waveform), MAX_BLOCKS_PER_TEXTURE);
		int i;for(i=0;i<max;i++){
			if(!section->ready[i]) return false;
		}
		dbg(1, "complete");
		return section->completed = true;
	}

	void other_lods(Renderer* renderer, Section* section, int dest)
	{
		int* lod_max = ((NGRenderer*)renderer)->mmidx_max;
		int* lod_min = ((NGRenderer*)renderer)->mmidx_min;

		int m; for(m=1;m<N_LOD;m++){
			int mm_level = m;
			int mm = 1 << (mm_level - 1);
			int i,p; for(i=0, p=0; p<modes[renderer->mode].texture_size/mm; i++, p+=2){
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

	inline void lo_peakbuf_to_texture(Renderer* renderer, WaveformActor* actor, int b, Section* section, int n_chans, int block_size)
	{
		// borders: source data is not blocked so borders need to be added here.

		Waveform* waveform = actor->waveform;
		WaveformPriv* w = waveform->priv;
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

		int c; for(c=0;c<n_chans;c++){
			int src = WF_PEAK_VALUES_PER_SAMPLE * (b * B_SIZE - TEX_BORDER) * WF_PEAK_STD_TO_LO;
			int dest = _b * block_size + (c * block_size / 2);

			int t = 0;
			if(b == 0){
				for(t=0;t<TEX_BORDER;t++){
					ng_gl2_set_(section, dest + lod_max[mm_level] + t, 0);
					ng_gl2_set_(section, dest + lod_min[mm_level] + t, 0);
				}
				src = 0;
			}

			for(; t<stop; t++, src+=2*WF_PEAK_STD_TO_LO){
				WfPeakSample p = {0, 0};
				int j; for(j=0;j<2*WF_PEAK_STD_TO_LO;j+=WF_PEAK_VALUES_PER_SAMPLE){
					p.positive = MAX(p.positive, peak->buf[c][src + j    ]);
					p.negative = MIN(p.negative, peak->buf[c][src + j + 1]);
				}

				ng_gl2_set_(section, dest + lod_max[mm_level] + t, short_to_char( p.positive));
				ng_gl2_set_(section, dest + lod_min[mm_level] + t, short_to_char(-p.negative));
			}

			other_lods(renderer, section, dest);
		}
	}

	inline void med_peakbuf_to_texture(Renderer* renderer, WaveformActor* actor, int b, Section* section, int n_chans, int block_size)
	{
		// borders: source data is not blocked so borders are added here.

		Waveform* waveform = actor->waveform;
		WaveformPriv* w = waveform->priv;
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
			if(b == 0){
				for(t=0;t<TEX_BORDER;t++){
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

	inline void hi_audio_to_texture(Renderer* renderer, WaveformActor* actor, int b, Section* section, int n_chans, int block_size)
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
		int c; for(c=0;c<n_chans;c++){
			int B = _b * block_size + (c * block_size / 2);
			int mm_level = 0;
			int i, p; for(i=0, p=0; p<WF_PEAK_BLOCK_SIZE - DELAY; i++, p+= IO_RATIO){

				short* d = &audio_buf->buf[c][p];
				max[c] = 0;
				min[c] = 0;
				int k; for(k=0;k<IO_RATIO;k++){
					max[c] = (d[k + c] > max[c]) ? d[k + c] : max[c];
					min[c] = (d[k + c] < min[c]) ? d[k + c] : min[c];
				}

				bool ok = ng_gl2_set(section, B + ((NGRenderer*)renderer)->mmidx_max[mm_level] + ((int)TEX_BORDER_HI) + i, short_to_char(max[c]));
				if(!ok) gerr("max b=%i i=%i p=%i %i size=%i", _b, i, p, B + ((NGRenderer*)renderer)->mmidx_max[mm_level] + i, section->buffer_size);
				g_return_if_fail(ok);
				ok = ng_gl2_set(section, B + ((NGRenderer*)renderer)->mmidx_min[mm_level] + ((int)TEX_BORDER_HI) + i, short_to_char(-min[c]));
				if(!ok) gerr("min b=%i i=%i p=%i %i size=%i mm=%i", _b, i, p, b * block_size + ((NGRenderer*)renderer)->mmidx_min[mm_level] + i, section->buffer_size, ((NGRenderer*)renderer)->mmidx_min[mm_level]);
				g_return_if_fail(ok);
			}

			other_lods(renderer, section, B);
		}
	}

	int n_chans = waveform_get_n_channels(waveform);

	HiResNGWaveform** data = (HiResNGWaveform**)&w->render_data[renderer->mode];
#ifdef NG_HASHTABLE
#if WF_DEBUG
	{
		HiResNGWaveform* data1 = g_hash_table_lookup(((NGRenderer*)renderer)->ng_data, waveform);
		if((*data) != data1) gwarn("%i: wav=%p hash=%p %s", b, *data, data1, modes[renderer->mode].name);
		g_return_if_fail((*data) == data1);
	}
#endif
#endif
	if(!*data){
		int n_sections = waveform_get_n_audio_blocks(waveform) / MAX_BLOCKS_PER_TEXTURE + (waveform_get_n_audio_blocks(waveform) % MAX_BLOCKS_PER_TEXTURE ? 1 : 0);

		*(*data = g_malloc0(sizeof(HiResNGWaveform) + sizeof(Section) * n_sections)) = (HiResNGWaveform){
			.size = n_sections,
			.n_blocks = wf_actor_get_n_blocks(waveform, renderer->mode)
		};

		g_object_weak_ref((GObject*)waveform, ng_gl2_finalize_notify, renderer);
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
					//v_low_peakbuf_to_texture(renderer, actor, b, section, n_chans, block_size);
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

	int s; for(s=0;s<(*data)->size;s++){
		Section* section = &(*data)->section[s];
		if(!section->completed){
			if(texture_changed[s]){
				if(!section->texture){
					// note: for the WaveformBlock we use the first block for the section (WaveformBlock concept is broken in this context)
					section->texture = texture_cache_assign_new(GL_TEXTURE_2D, (WaveformBlock){waveform, (s * MAX_BLOCKS_PER_TEXTURE) | (renderer->mode == MODE_HI ? WF_TEXTURE_CACHE_HIRES_NG_MASK : 0)});
				}

				int width = modes[renderer->mode].texture_size;
				int height = section->buffer_size / width;
				int pixel_format = GL_ALPHA;
				glBindTexture  (GL_TEXTURE_2D, section->texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				// TODO it is quite common for this to be done several times in quick succession for the same texture with consecutive calls to ng_gl2_load_block
				dbg(1, "%i: uploading texture: %i x %i", s, width, height);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, section->buffer);
				gl_warn("error binding texture: %u", section->texture);
			}

			if(section_is_complete(actor, section)){
				g_free0(section->buffer); // all data has been sent to the gpu so can be freed.
			}
		}
	}
}


static void
ng_gl2_pre_render(Renderer* renderer, WaveformActor* actor)
{
	Waveform* w = actor->waveform;
	WfActorPriv* _a = actor->priv;
	RenderInfo* r  = &_a->render_info;

	if(r->mode >= MODE_V_HI) return; // TODO this will happen when falling through.

	HiResNGWaveform* data = (HiResNGWaveform*)w->priv->render_data[renderer->mode];
	HiResNGShader* shader = (HiResNGShader*)((NGRenderer*)renderer)->shader;
	if(!data || !shader) return; // this can happen when we fall through from v hi res.

	shader->uniform.fg_colour = (actor->fg_colour & 0xffffff00) + (unsigned)(0x100 * _a->animatable.opacity.val.f);
	shader->uniform.top = r->rect.top;
	shader->uniform.bottom = r->rect.top + r->rect.height;
	shader->uniform.n_channels = waveform_get_n_channels(w);
	shader->uniform.tex_width = modes[renderer->mode].texture_size;
	shader->uniform.tex_height = data->section[r->viewport_blocks.first / MAX_BLOCKS_PER_TEXTURE].buffer_size / modes[renderer->mode].texture_size;

	shader->uniform.mm_level = (renderer->mode == MODE_MED || renderer->mode == MODE_LOW)
		? (
		r->block_wid > 128
			? 0
			: r->block_wid > 64
				? 1
				: r->block_wid > 32
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

	agl_use_program(&shader->shader);
}


static bool
ng_gl2_render_block(Renderer* renderer, WaveformActor* actor, int b, gboolean is_first, gboolean is_last, double x)
{
	gl_warn("pre");

	int border = renderer->mode == MODE_HI ? TEX_BORDER_HI : TEX_BORDER;

	Waveform* waveform = actor->waveform; 
	WfActorPriv* _a = actor->priv;
	RenderInfo* r  = &_a->render_info;

	int  s = b / MAX_BLOCKS_PER_TEXTURE;
	int _b = b % MAX_BLOCKS_PER_TEXTURE;

	HiResNGWaveform* data = (HiResNGWaveform*)waveform->priv->render_data[renderer->mode];
	if(!data) return false; // this can happen when audio data not yet available.
	Section* section = &data->section[s];

	if(!_b && b != r->viewport_blocks.first){
		HiResNGShader* shader = (HiResNGShader*)((NGRenderer*)renderer)->shader;
		shader->uniform.tex_height = data->section[b / MAX_BLOCKS_PER_TEXTURE].buffer_size / modes[renderer->mode].texture_size;
		shader->shader.set_uniforms_(actor);
	}

																								glActiveTexture(GL_TEXTURE0);

	TextureRange tex;
	WfSampleRegionf block;
	if(!wf_actor_get_quad_dimensions(actor, b, is_first, is_last, x, &tex, &block.start, &block.len, border, 1)) return false;

	float n_rows = section->buffer_size / modes[renderer->mode].texture_size;
	float ty = (b % MAX_BLOCKS_PER_TEXTURE) * 4.0 * waveform->n_channels / n_rows; // this tells the shader which block to use.
	AGlQuad tex_rect = {tex.start, ty, tex.end, ty + 0.001}; // the 0.001 prevents the wrong block being shown on some systems

	//dbg(0, "b=%i %u n_rows=%f x=%f-->%f y=%f (%f)", b % MAX_BLOCKS_PER_TEXTURE, section->texture, n_rows, tex.start, tex.end, ty, ((float)(b % MAX_BLOCKS_PER_TEXTURE) * 4.0 * waveform->n_channels));

	agl_textured_rect(section->texture, block.start, r->rect.top, block.len, r->rect.height, &tex_rect);

	return true;
}


static void
ng_make_lod_levels(NGRenderer* renderer, Mode mode)
{
	int p = 0;
	int width = modes[mode].texture_size;
	int level_size = width;
	int i; for(i=0;i<N_LOD;i++){
		renderer->mmidx_max[i] = p;
		renderer->mmidx_min[i] = p + width * ROWS_PER_PEAK_TYPE;
		level_size = width / (1 << i);
		p += level_size;
	}
}


static void
ng_gl2_free_section(Renderer* renderer, Waveform* waveform, Section* section, int s)
{
	if(section){
		if(section->buffer) g_free0(section->buffer);
		if(section->texture){
			texture_cache_remove(GL_TEXTURE_2D, waveform, (s * MAX_BLOCKS_PER_TEXTURE) | (renderer->mode == MODE_HI ? WF_TEXTURE_CACHE_HIRES_NG_MASK : 0));
			section->texture = 0;
		}
		section->completed = false;
		memset(section->ready, 0, sizeof(bool) * MAX_BLOCKS_PER_TEXTURE);
	}
}


static void
ng_gl2_free_waveform(Renderer* renderer, Waveform* waveform)
{
	dbg(1, "%s", modes[renderer->mode].name);

#ifdef NG_HASHTABLE
	HiResNGWaveform* data = g_hash_table_lookup(((NGRenderer*)renderer)->ng_data, waveform);
#else
	HiResNGWaveform* data = (HiResNGWaveform*)waveform->priv->render_data[renderer->mode];
#endif
	if(data){
		// the sections must be freed before removing from the hashtable
		// so that the Waveform can be referenced.
		int s; for(s=0;s<data->size;s++){
			ng_gl2_free_section(renderer, waveform, &data->section[s], s);
		}
#ifdef NG_HASHTABLE
		// removing from the hash table will cause the item to be free'd.
		if(!g_hash_table_remove(((NGRenderer*)renderer)->ng_data, waveform)) dbg(1, "failed to remove render data");
#endif

		waveform->priv->render_data[renderer->mode] = NULL;
	}
}


static void
ng_gl2_queue_clean(Renderer* renderer)
{
	#define MAX_SECTIONS 1024

	static guint idle_id = 0;

	bool clean(gpointer user_data)
	{
		Renderer* renderer = user_data;

		static struct _oldest {
			Waveform*        waveform;
			HiResNGWaveform* data;
			int              section;
			int              time_stamp;
		} oldest;

		void __hi_find_oldest(gpointer key, gpointer value, gpointer _)
		{
			HiResNGWaveform* data = (HiResNGWaveform*)value;
			int s; for(s=0;s<data->size;s++){
				Section* section = &data->section[s];
				dbg(0, ">  %i", section->time_stamp);
				if(section->buffer && (section->time_stamp < oldest.time_stamp)){
					oldest = (struct _oldest){key, data, s, section->time_stamp};
				}
			}
		}

#ifdef NG_HASHTABLE
		GHashTable* table = ((NGRenderer*)renderer)->ng_data;
#endif

		dbg(1, "size=%i", g_hash_table_size(table));

		if(g_hash_table_size(table) > MAX_SECTIONS){
			int n_to_remove = g_hash_table_size(table) - MAX_SECTIONS;
			int i; for(i=0;i<n_to_remove;i++){
				oldest = (struct _oldest){NULL, 0, INT_MAX};
				g_hash_table_foreach(table, __hi_find_oldest, NULL);

				if(oldest.data){
					dbg(0, "removing: section=%i", oldest.section);
					ng_gl2_free_section(renderer, oldest.waveform, &oldest.data->section[oldest.section], oldest.section);
				}
			}
		}

		idle_id = 0;
		return G_SOURCE_REMOVE;
	}

	if(!idle_id) idle_id = g_idle_add_full(G_PRIORITY_LOW, clean, renderer, NULL);
}


