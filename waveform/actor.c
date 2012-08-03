/*
  copyright (C) 2012 Tim Orford <tim@orford.org>

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

  ---------------------------------------------------------------

  WaveformActor draws a Waveform object onto a shared opengl drawable.

*/
#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "waveform/typedefs.h"
#include "waveform/utils.h"
#include "waveform/gl_utils.h"
#include "waveform/peak.h"
#include "waveform/audio.h"
#include "waveform/texture_cache.h"
#include "waveform/animator.h"
#include "waveform/shaderutil.h"
#include "waveform/hi_res.h"
#include "waveform/alphabuf.h"
#include "waveform/fbo.h"
#include "waveform/actor.h"
#include "waveform/gl_ext.h"
#ifdef USE_FBO
#define multipass
#endif

#define WF_SAMPLES_PER_TEXTURE (WF_PEAK_RATIO * WF_PEAK_TEXTURE_SIZE - TEX_BORDER)

/*
	Mipmapping is needed (when shaders not available) because without it,
	peaks can be missed at small zoom, and can be too blurry.

	However, it is problematic:
	- doesnt work with non-square textures (is supposed to; possible driver issues)
	- small x size reduces y resolution - doesnt look good.
 */
#define USE_MIPMAPPING
#undef USE_MIPMAPPING

extern WfShader sh_main;
extern WfShader shader2;
extern BloomShader vertical;

struct _actor_priv
{
	struct {
		WfAnimatable  start;     // (int) region
		WfAnimatable  len;       // (int) region
		WfAnimatable  rect_left; // (float)
		WfAnimatable  rect_len;  // (float)
	}               animatable;
	GList*          transitions; // list of WfAnimation*

	gulong          peakdata_ready_handler;
};

typedef enum
{
	MODE_LOW = 0,
	MODE_MED,
	MODE_HI,
	MODE_V_HI,
	N_MODES
} Mode;

typedef struct _a
{
	guchar positive[WF_PEAK_TEXTURE_SIZE * 16];
	guchar negative[WF_PEAK_TEXTURE_SIZE * 16];
} IntBufHi;

typedef void (MakeTextureData)(Waveform*, int ch, IntBufHi*, int blocknum);
static MakeTextureData
	make_texture_data_hi;

#warning texture size 4096 will be too high for Intel
struct _draw_mode
{
	char             name[4];
	int              resolution;
	int              texture_size;      // mostly applies to 1d textures. 2d textures have non-square issues.
	MakeTextureData* make_texture_data; // might not be needed after all
} modes[N_MODES] = {
	{"LOW", 1024, WF_PEAK_TEXTURE_SIZE,      NULL},
	{"MED",  256, WF_PEAK_TEXTURE_SIZE,      NULL},
	{"HI",    16, WF_PEAK_TEXTURE_SIZE * 16, NULL}, // texture size chosen so that blocks are the same as in medium res
	{"V_HI",   1, WF_PEAK_TEXTURE_SIZE,      NULL},
};
#define HI_RESOLUTION modes[MODE_HI].resolution
#define RES_MED modes[MODE_MED].resolution

static void   wf_actor_get_viewport          (WaveformActor*, WfViewPort*);
static void   wf_actor_start_transition      (WaveformActor*, WfAnimatable*);
static void   wf_actor_on_animation_finished (WaveformActor*, WfAnimation*);
static void   wf_actor_allocate_block_hi     (WaveformActor*, int b);
static void   wf_actor_allocate_block_med    (WaveformActor*, int b);
static void   wf_actor_allocate_block_low    (WaveformActor*, int b);
static void  _wf_actor_load_missing_blocks   (WaveformActor*);
static void   wf_actor_load_texture1d        (Waveform*, Mode, WfGlBlock*, int b);
static void  _wf_actor_load_texture_hi       (WaveformActor*, int b);
static void  _wf_actor_print_hires_textures  (WaveformActor*);
static inline int   get_resolution           (double zoom);
static inline float get_peaks_per_pixel      (WaveformCanvas*, WfSampleRegion*, WfRectangle*, int mode);
#if defined (USE_FBO) && defined (multipass)
static void   block_to_fbo                   (WaveformActor*, int b, WfGlBlock*, int resolution);
#endif

static WfFBO* fbo_test = NULL;

void
wf_actor_init()
{
	get_gl_extensions();

	modes[MODE_HI].make_texture_data = make_texture_data_hi;
}


WaveformActor*
wf_actor_new(Waveform* w)
{
	if(wf_debug > 1) gwarn("%s----------------------%s", "\x1b[1;33m", "\x1b[0;39m");

	uint32_t get_frame(int time)
	{
		return 0;
	}

	WaveformActor* a = g_new0(WaveformActor, 1);
	a->bg_colour = 0x000000ff;
	a->fg_colour = 0xffffffff;
	a->waveform = g_object_ref(w);

	a->priv = g_new0(WfActorPriv, 1);
	WfAnimatable* animatable = &a->priv->animatable.start;
	animatable->model_val.i = (uint32_t*)&a->region.start; //TODO add uint64_t to union
	animatable->start_val.i = a->region.start;
	animatable->val.i       = a->region.start;
	//animatable->min.i       = 0;
	animatable->type        = WF_INT;

	animatable = &a->priv->animatable.len;
	animatable->model_val.i = &a->region.len;
	animatable->start_val.i = a->region.len;
	animatable->val.i       = a->region.len;
	//animatable->min.i       = 1;
	animatable->type        = WF_INT;

	animatable = &a->priv->animatable.rect_left;
	animatable->model_val.f = &a->rect.left;
	animatable->start_val.f = a->rect.left;
	animatable->val.i       = a->rect.left;
	animatable->type        = WF_FLOAT;

	animatable = &a->priv->animatable.rect_len;
	animatable->model_val.f = &a->rect.len;
	animatable->start_val.f = a->rect.len;
	animatable->val.i       = a->rect.len;
	animatable->type        = WF_FLOAT;

	void _wf_actor_on_peakdata_available(Waveform* waveform, int block, gpointer _actor)
	{
		// because there can be many actors showing the same waveform
		// this can be called multiple times, but the texture must only
		// be updated once.
		// if the waveform has changed, the existing data must be cleared first.

		dbg(1, "block=%i", block);
		_wf_actor_load_texture_hi((WaveformActor*)_actor, block);
	}
	a->priv->peakdata_ready_handler = g_signal_connect (w, "peakdata-ready", (GCallback)_wf_actor_on_peakdata_available, a);
	return a;
}


void
wf_actor_free(WaveformActor* a)
{
	PF;
	g_return_if_fail(a);

	if(a->waveform){
		g_signal_handler_disconnect((gpointer)a->waveform, a->priv->peakdata_ready_handler);
		waveform_unref0(a->waveform);
	}
	g_free(a->priv);
	g_free(a);
}


void
wf_actor_set_region(WaveformActor* a, WfSampleRegion* region)
{
	g_return_if_fail(a);
	g_return_if_fail(region);
	WfActorPriv* _a = a->priv;
	dbg(1, "region_start=%Lu region_end=%Lu wave_end=%Lu", region->start, (uint64_t)(region->start + region->len), waveform_get_n_frames(a->waveform));
	if(!region->len){ gwarn("invalid region: len not set"); return; }
	if(region->start + region->len > waveform_get_n_frames(a->waveform)){ gwarn("invalid region: too long: %Lu len=%u n_frames=%Lu", region->start, region->len, waveform_get_n_frames(a->waveform)); return; }

	if(a->region.len < 2){
		a->region.len = _a->animatable.len.val.i = region->len; //dont animate on initial region set.
	}

	gboolean start = (region->start != a->region.start);
	gboolean end   = (region->len   != a->region.len);

	a->priv->animatable.start.start_val.i = a->region.start;
	a->priv->animatable.len.start_val.i = MAX(1, a->region.len);

	a->region = *region;

	if(!start && !end) return;

	if(a->rect.len > 0.00001) _wf_actor_load_missing_blocks(a);

	if(!a->canvas->draw){
		a->priv->animatable.start.val.i = MAX(1, a->region.start);
		a->priv->animatable.len.val.i = MAX(1, a->region.len);
		return; //no animations
	}

	GList* l = _a->transitions;
	if(l) dbg(2, "transitions=%i", g_list_length(l));
	for(;l;l=l->next){                                 //TODO this loop is also inside wf_animation_remove_animatable()
		WfAnimation* animation = l->data;

		GList* m = animation->members;
		dbg(2, "  animation=%p n_members=%i", animation, g_list_length(m));
		for(;m;m=m->next){
			if(start){
				wf_animation_remove_animatable(animation, &a->priv->animatable.start);
			}
			if(end){
				wf_animation_remove_animatable(animation, &a->priv->animatable.len);
			}
		}
	}

	WfAnimation* animation = wf_animation_add_new(wf_actor_on_animation_finished);
	_a->transitions = g_list_append(_a->transitions, animation);

	GList* animatables = NULL;
	if(start){
		wf_actor_start_transition(a, &a->priv->animatable.start);
		animatables = g_list_append(animatables, &a->priv->animatable.start);
	}
	if(end){
		wf_actor_start_transition(a, &a->priv->animatable.len);
		animatables = g_list_append(animatables, &a->priv->animatable.len);
	}
	wf_transition_add_member(animation, a, animatables);
	wf_animation_start(animation);
}


void
wf_actor_set_colour(WaveformActor* a, uint32_t fg_colour, uint32_t bg_colour)
{
	dbg(2, "0x%08x", fg_colour);
	a->fg_colour = fg_colour;
	a->bg_colour = bg_colour;
}


static double wf_actor_samples2gl(double zoom, uint32_t n_samples)
{
	//zoom is pixels per sample
	return n_samples * zoom;
}


static int
wf_actor_get_first_visible_block(WfSampleRegion* region, double zoom, WfRectangle* rect, WfViewPort* viewport_px)
{
	int resolution = get_resolution(zoom);
	int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (resolution == 1024 ? WF_PEAK_STD_TO_LO : 1);

	double part_inset_px = wf_actor_samples2gl(zoom, region->start);
	double file_start_px = rect->left - part_inset_px;
	double block_wid = wf_actor_samples2gl(zoom, samples_per_texture);

	int part_start_block = region->start / samples_per_texture;
	int part_end_block = (region->start + region->len) / samples_per_texture;
	int b; for(b=part_start_block;b<=part_end_block;b++){
		int block_pos_px = file_start_px + b * block_wid;
		dbg(3, "block_pos_px=%i", block_pos_px);
		double block_end_px = block_pos_px + block_wid;
		if(block_end_px >= viewport_px->left) return b;
	}

	dbg(1, "region outside viewport? vp_left=%.2f", viewport_px->left);
	return 10000;
}


static int
wf_actor_get_last_visible_block(WfSampleRegion* region, WfRectangle* rect, double zoom, WfViewPort* viewport_px, WfGlBlock* textures)
{
	//the region, rect and viewport are passed explictly because different users require slightly different values during transitions.

	int resolution = get_resolution(zoom);
	int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (resolution == 1024 ? WF_PEAK_STD_TO_LO : 1);

	g_return_val_if_fail(textures, -1);
	g_return_val_if_fail(viewport_px->right - viewport_px->left > 0.01, -1);

	//dbg(1, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

	double part_inset_px = wf_actor_samples2gl(zoom, region->start);
	double file_start_px = rect->left - part_inset_px;
	double block_wid = wf_actor_samples2gl(zoom, samples_per_texture);
	//dbg(1, "vp->right=%.2f", viewport_px->right);
	int region_start_block = region->start / samples_per_texture;
	float _end_block = ((float)(region->start + region->len)) / samples_per_texture;
	dbg(2, "%s region_start=%Li region_end=%i start_block=%i end_block=%.2f(%.i) n_peak_frames=%i gl->size=%i", resolution == 1024 ? "LOW" : resolution == 256 ? "STD" : "HI", region->start, ((int)region->start) + region->len, region_start_block, _end_block, (int)ceil(_end_block), textures->size * 256, textures->size);

	//we round _down_ as the block corresponds to the _start_ of a section.
	int region_end_block = MIN(_end_block, textures->size - 1);

	if(region_end_block >= textures->size) gwarn("!!");

	//crop to viewport:
	int b; for(b=region_start_block;b<=region_end_block-1;b++){ //note we dont check the last block which can be partially outside the viewport
		float block_end_px = file_start_px + (b + 1) * block_wid;
		//dbg(1, " %i: block_px: %.1f --> %.1f", b, block_end_px - (int)block_wid, block_end_px);
		if(block_end_px > viewport_px->right) dbg(2, "end %i clipped by viewport at block %i. vp.right=%.2f block_end=%.1f", region_end_block, MAX(0, b/* - 1*/), viewport_px->right, block_end_px);
		if(block_end_px > viewport_px->right) return MAX(0, b/* - 1*/);

#if 0
		if(rect.len > 0.0)
		if(file_start_px + (b) * block_wid > rect.left + rect.len){
			gerr("block too high: block_start=%.2f rect.len=%.2f", file_start_px + (b) * block_wid, rect.len);
			return b - 1;
		}
#endif
	}

	dbg(2, "end not outside viewport. vp_right=%.2f last=%i", viewport_px->right, region_end_block);
	return region_end_block;
}


#if defined (USE_FBO) && defined (multipass)
static void
block_to_fbo(WaveformActor* a, int b, WfGlBlock* blocks, int resolution)
{
#if 0 //nothing special to do
	if(resolution == 1024){ //LO mode
	}
#endif

	g_return_if_fail(!blocks->fbo[b]);
	blocks->fbo[b] = fbo_new(0);
	{
		WaveformCanvas* wfc = a->canvas;
		WaveformActor* actor = a;
		WfGlBlock* textures = blocks;
		WfSampleRegion region = {actor->priv->animatable.start.val.i, actor->priv->animatable.len.val.i}; //tmp
		WfRectangle rect = {actor->rect.left, actor->rect.top, actor->priv->animatable.rect_len.val.f, actor->rect.height};
		WfViewPort viewport; wf_actor_get_viewport(actor, &viewport); //TODO is just 0-256?

		if(a->canvas->use_1d_textures){
			WfFBO* fbo = blocks->fbo[b];
			if(fbo){
				draw_to_fbo(fbo) {
					WfColourFloat fg; wf_colour_rgba_to_float(&fg, actor->fg_colour);
					glClearColor(fg.r, fg.g, fg.b, 0.0); //background colour must be same as foreground for correct antialiasing
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

					{
						//set shader program
						PeakShader* peak_shader = wfc->priv->shaders.peak;
						wf_canvas_use_program(wfc, peak_shader->shader.program);

						//uniforms:
						float peaks_per_pixel = 1.0;//get_peaks_per_pixel(wfc, &region, &rect, get_resolution(rect.len / region.len));
								viewport.right = 256;
						dbg(2, "vpwidth=%.2f region_len=%i region_n_peaks=%.2f peaks_per_pixel=%.2f", (viewport.right - viewport.left), region.len, ((float)region.len / WF_PEAK_TEXTURE_SIZE), peaks_per_pixel);
						float bottom = rect.top + rect.height;
						int n_channels = textures->peak_texture[WF_RIGHT].main ? 2 : 1;
						peak_shader->set_uniforms(peaks_per_pixel, rect.top, bottom, 0xffffffff, n_channels);
					}

					glEnable(GL_TEXTURE_1D);
					int c = 0;
					texture_unit_use_texture(a->canvas->texture_unit[0], textures->peak_texture[c].main[b]);
					texture_unit_use_texture(a->canvas->texture_unit[1], textures->peak_texture[c].neg[b]);
					if(a->waveform->priv->peak.buf[WF_RIGHT]){
						c = 1;
						texture_unit_use_texture(a->canvas->texture_unit[2], textures->peak_texture[c].main[b]);
						texture_unit_use_texture(a->canvas->texture_unit[3], textures->peak_texture[c].neg[b]);
					}

					double top = 0;
					double bot = fbo->height;
					double x1 = 0;
					double x2 = fbo->width;
											double tex_start = 0, tex_pct = 1.0; //TODO
					glBegin(GL_QUADS);
					glMultiTexCoord2f(WF_TEXTURE0, tex_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + 0.0,     0.0); glVertex2d(x1, top);
					glMultiTexCoord2f(WF_TEXTURE0, tex_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + tex_pct, 0.0); glVertex2d(x2, top);
					glMultiTexCoord2f(WF_TEXTURE0, tex_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + tex_pct, 1.0); glVertex2d(x2, bot);
					glMultiTexCoord2f(WF_TEXTURE0, tex_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + 0.0,     1.0); glVertex2d(x1, bot);
					glEnd();
				} end_draw_to_fbo;
			} else gwarn("fbo not allocated");
		}
		gl_warn("fbo");
	}
}
#endif


static void
wf_actor_allocate_block_hi(WaveformActor* a, int b)
{
	PF;
	WfTextureHi* texture = g_hash_table_lookup(a->waveform->textures_hi->textures, &b);

	int c = WF_LEFT;

	if(glIsTexture(texture->t[c].main)){
		gwarn("already assigned");
		return;
	}

#warning TODO texture_cache not suitable for hires textures.
	int n_ch = waveform_get_n_channels(a->waveform);
	if(a->canvas->use_1d_textures){
		for(c=0;c<n_ch;c++){
			texture->t[c].main = texture_cache_assign_new(wf->texture_cache, (WaveformBlock){a->waveform, b});
			texture->t[c].neg  = texture_cache_assign_new(wf->texture_cache, (WaveformBlock){a->waveform, b});
		}

		wf_actor_load_texture1d(a->waveform, MODE_HI, (WfGlBlock*)NULL, b);
	}else{
		gwarn("TODO non-shader");
	}
}


static void
wf_actor_allocate_block_med(WaveformActor* a, int b)
{
	// load resources (textures) required for display of the block.

	g_return_if_fail(b >= 0);

	Waveform* w = a->waveform;
	WfGlBlock* blocks = w->textures;

	int c = WF_LEFT;
	if(blocks->peak_texture[c].main[b]){
		if(glIsTexture(blocks->peak_texture[c].main[b])){
			//gwarn("waveform texture already assigned for block %i: %i", b, blocks->peak_texture[c].main[b]);
			return;
		}else{
			gwarn("removing invalid texture...");
			texture_cache_remove(w, b);
			//TODO gldelete texture? mostly likely wont help.
			int c; for(c=0;c<WF_RIGHT;c++) blocks->peak_texture[c].main[b] = blocks->peak_texture[c].neg[b] = 0;
		}
	}

	int texture_id = blocks->peak_texture[c].main[b] = texture_cache_assign_new(wf->texture_cache, (WaveformBlock){a->waveform, b});

	if(a->canvas->use_1d_textures){
		guint* peak_texture[4] = {
			&blocks->peak_texture[WF_LEFT ].main[b],
			&blocks->peak_texture[WF_LEFT ].neg[b],
			&blocks->peak_texture[WF_RIGHT].main[b],
			&blocks->peak_texture[WF_RIGHT].neg[b]
		};
		blocks->peak_texture[c].neg[b] = texture_cache_assign_new (wf->texture_cache, (WaveformBlock){a->waveform, b});

		if(a->waveform->priv->peak.buf[WF_RIGHT]){
			int i; for(i=2;i<4;i++){
				*peak_texture[i] = texture_cache_assign_new (wf->texture_cache, (WaveformBlock){a->waveform, b});
			}
			dbg(1, "rhs: %i: texture=%i %i %i %i", b, *peak_texture[0], *peak_texture[1], *peak_texture[2], *peak_texture[3]);
		}else{
			dbg(1, "* %i: texture=%i %i (mono)", b, texture_id, *peak_texture[1]);
		}

		wf_actor_load_texture1d(w, MODE_MED, w->textures, b);

#if defined (USE_FBO) && defined (multipass)
		if(agl_get_instance()->use_shaders)
			block_to_fbo(a, b, blocks, 256);
#endif
	}else{
		dbg(1, "* %i: texture=%i", b, texture_id);
		AlphaBuf* alphabuf = wf_alphabuf_new(w, b, 1, false, TEX_BORDER);
		wf_canvas_load_texture_from_alphabuf(a->canvas, texture_id, alphabuf);
		wf_alphabuf_free(alphabuf);
	}

	gl_warn("(end)");
}


static void
wf_actor_allocate_block_low(WaveformActor* a, int b)
{
	g_return_if_fail(b >= 0);

	Waveform* w = a->waveform;
	WfGlBlock* blocks = w->textures_lo;

	int c = WF_LEFT;
	if(blocks->peak_texture[c].main[b] && glIsTexture(blocks->peak_texture[c].main[b])){
		//gwarn("waveform low-res texture already assigned for block %i: %i", b, blocks->peak_texture[c].main[b]);
		return;
	}

	int texture_id = blocks->peak_texture[c].main[b] = texture_cache_assign_new(wf->texture_cache, (WaveformBlock){a->waveform, b | WF_TEXTURE_CACHE_LORES_MASK});

	if(a->canvas->use_1d_textures){
		guint* peak_texture[4] = {
			&blocks->peak_texture[WF_LEFT ].main[b],
			&blocks->peak_texture[WF_LEFT ].neg[b],
			&blocks->peak_texture[WF_RIGHT].main[b],
			&blocks->peak_texture[WF_RIGHT].neg[b]
		};
		blocks->peak_texture[c].neg[b] = texture_cache_assign_new (wf->texture_cache, (WaveformBlock){a->waveform, b | WF_TEXTURE_CACHE_LORES_MASK});

		if(a->waveform->priv->peak.buf[WF_RIGHT]){
			if(peak_texture[2]){
				int i; for(i=2;i<4;i++){
					*peak_texture[i] = texture_cache_assign_new (wf->texture_cache, (WaveformBlock){a->waveform, b | WF_TEXTURE_CACHE_LORES_MASK});
				}
				dbg(1, "rhs: %i: texture=%i %i %i %i", b, *peak_texture[0], *peak_texture[1], *peak_texture[2], *peak_texture[3]);
			}
		}else{
			dbg(1, "* %i: textures=%i,%i (rhs peak.buf not loaded)", b, texture_id, *peak_texture[1]);
		}

		wf_actor_load_texture1d(w, MODE_LOW, w->textures_lo, b);
#if defined (USE_FBO) && defined (multipass)
		block_to_fbo(a, b, blocks, 1024);
#endif
	}else{
		dbg(1, "* %i: texture=%i", b, texture_id);
		AlphaBuf* alphabuf = wf_alphabuf_new(w, b, WF_PEAK_STD_TO_LO, false, TEX_BORDER);
		wf_canvas_load_texture_from_alphabuf(a->canvas, texture_id, alphabuf);
		wf_alphabuf_free(alphabuf);
	}
}


static void
wf_actor_get_viewport(WaveformActor* a, WfViewPort* viewport)
{
	WaveformCanvas* canvas = a->canvas;

	if(canvas->viewport) *viewport = *canvas->viewport;
	else {
		viewport->left   = a->priv->animatable.rect_left.val.f;
		viewport->top    = a->rect.top;
		viewport->right  = viewport->left + a->priv->animatable.rect_len.val.f;
		viewport->bottom = a->rect.top + a->rect.height;
	}
}


static void
wf_actor_on_animation_finished(WaveformActor* actor, WfAnimation* animation)
{
	g_return_if_fail(actor);
	g_return_if_fail(animation);
	WfActorPriv* _a = actor->priv;

	int l = g_list_length(_a->transitions);
	_a->transitions = g_list_remove(_a->transitions, animation);
	if(g_list_length(_a->transitions) != l - 1) gwarn("animation not removed. len=%i-->%i", l, g_list_length(_a->transitions));
}


#warning TODO duplicate fn
//duplicates wf_actor_get_first_visible_block
	static int
	get_first_visible_block(WfSampleRegion* region, double zoom, WfRectangle* rect, WfViewPort* viewport_px, int block_size)
	{
		double part_inset_px = wf_actor_samples2gl(zoom, region->start);
		double file_start_px = rect->left - part_inset_px;
		double block_wid = wf_actor_samples2gl(zoom, block_size);

		int part_start_block = region->start / block_size;
		int part_end_block = (region->start + region->len) / block_size;
		int b; for(b=part_start_block;b<=part_end_block;b++){
			int block_pos_px = file_start_px + b * block_wid;
			dbg(3, "block_pos_px=%i", block_pos_px);
			double block_end_px = block_pos_px + block_wid;
			if(block_end_px >= viewport_px->left) return b;
		}

		dbg(1, "region outside viewport? vp_left=%.2f", viewport_px->left);
		return 10000;
	}

	int get_last_visible_block(WaveformActor* a, double zoom, WfViewPort* viewport_px, int block_size)
	{
		//TODO this is the same as for the texture blocks - refactor with block_size argument. need to remove MIN

		Waveform* waveform = a->waveform;
		WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
		WfRectangle rect = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};

		g_return_val_if_fail(waveform, -1);
		g_return_val_if_fail(waveform->textures, -1);
		g_return_val_if_fail(viewport_px->right - viewport_px->left > 0.01, -1);

		//dbg(1, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

		double part_inset_px = wf_actor_samples2gl(zoom, region.start);
		double file_start_px = rect.left - part_inset_px;
		double block_wid = wf_actor_samples2gl(zoom, block_size);
		//dbg(1, "vp->right=%.2f", viewport_px->right);
		int region_start_block = region.start / block_size;
		float _end_block = ((float)(region.start + region.len)) / block_size;
		dbg(2, "region_start=%i region_end=%i start_block=%i end_block=%.2f(%.i) n_peak_frames=%i gl->size=%i", region.start, region.len, region_start_block, _end_block, (int)ceil(_end_block), waveform->textures->size * 256, waveform->textures->size);

		//we round _down_ as the block corresponds to the _start_ of a section.
		//int region_end_block = MIN(ceil(_end_block), waveform->textures->size - 1);
// FIXME
		//int region_end_block = MIN(_end_block, waveform->textures->size - 1);
int region_end_block = MIN(_end_block, waveform_get_n_audio_blocks(waveform) - 1);

//		if(region_end_block >= waveform->textures->size) gwarn("!!");

		//crop to viewport:
		int b; for(b=region_start_block;b<=region_end_block-1;b++){ //note we dont check the last block which can be partially outside the viewport
			float block_end_px = file_start_px + (b + 1) * block_wid;
			//dbg(1, " %i: block_px: %.1f --> %.1f", b, block_end_px - (int)block_wid, block_end_px);
			if(block_end_px > viewport_px->right) dbg(2, "end %i clipped by viewport at block %i. vp.right=%.2f block_end=%.1f", region_end_block, MAX(0, b/* - 1*/), viewport_px->right, block_end_px);
			if(block_end_px > viewport_px->right) return MAX(0, b/* - 1*/);
		}

		dbg(2, "end not outside viewport. vp_right=%.2f last=%i", viewport_px->right, region_end_block);
		return region_end_block;

		//int n_blocks = waveform_get_n_audio_blocks(w);
		//return MIN(4, n_blocks - 1); //TODO
	}

	int get_min_n_tiers()
	{
		//int n_tiers_needed = WF_PEAK_RATIO / arr_samples_per_pix(arrange);
		return 3; //3 gives resolution of 1:16
	}

#if 0
	static int get_n_audio_blocks_loaded(Waveform* w)
	{
		WfAudioData* audio = w->audio_data;

		int n_loaded = 0;
		int n_blocks = waveform_get_n_audio_blocks(w);
		int b; for(b=0;b<n_blocks;b++){
			if(audio->buf16[b] && audio->buf16[b]->buf[0]) n_loaded++;
		}
		dbg(1, "n_loaded=%i", n_loaded);
		return n_loaded;
	}
#endif

static void
_wf_actor_get_viewport_max(WaveformActor* a, WfViewPort* viewport)
{
	//special version of get_viewport that gets the outer viewport for duration of the current the animation.

	WaveformCanvas* canvas = a->canvas;

	if(canvas->viewport) *viewport = *canvas->viewport;
	else {
		float left_max = MAX(a->rect.left, a->priv->animatable.rect_left.val.f);
		float left_min = MIN(a->rect.left, a->priv->animatable.rect_left.val.f);

		viewport->left   = left_min;
		viewport->top    = a->rect.top;
		viewport->right  = left_max + a->rect.len;
		viewport->bottom = a->rect.top + a->rect.height;
	}
}


#define ZOOM_HI  (1.0/  16)
#define ZOOM_MED (1.0/ 128) // px_per_sample - transition point from std to hi-res mode.
#define ZOOM_LO  (1.0/4096) // px_per_sample - transition point from low-res to std mode.

//deprectated - use get_mode instead
static inline int
get_resolution(double zoom)
{
	return (zoom > ZOOM_HI)
		? 1
		: (zoom > ZOOM_MED)
			? 16
			: (zoom > ZOOM_LO)
				? 256
				: 1024;      //TODO
}


static inline int
get_mode(double zoom)
{
	return (zoom > ZOOM_HI)
		? MODE_V_HI
		: (zoom > ZOOM_MED)
			? MODE_HI
			: (zoom > ZOOM_LO)
				? MODE_MED
				: MODE_LOW;      //TODO
}


static void
_wf_actor_allocate_hi(WaveformActor* a)
{
	/*

	How many textures do we need?
	16 * 60 * 60 / hour    => 5760
		-because this is relatively high, a hashtable is used instead of an array.

	caching options:
		- say that is inherently uncachable
		- have per-waveform texture cache
		- add to normal texture cache (will cause other stuff to be purged prematurely?)
		- add low-priority flag to regular texture cache
		- have separate low-priority texture cache       ****

	*/
	Waveform* w = a->waveform;
	g_return_if_fail(!w->offline);
	WfRectangle* rect = &a->rect;
	WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
	double zoom = rect->len / a->region.len;

	WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
	int first_block = get_first_visible_block(&region, zoom, rect, &viewport, WF_PEAK_BLOCK_SIZE);
	int last_block = get_last_visible_block(a, zoom, &viewport, WF_PEAK_BLOCK_SIZE);
	dbg(1, "%i--->%i", first_block, last_block);

	int b;for(b=first_block;b<=last_block;b++){
		int n_tiers_needed = get_min_n_tiers();
		if(waveform_load_audio_async(a->waveform, b, n_tiers_needed)){
			_wf_actor_load_texture_hi(a, b);
		}
	}
}


static void
_wf_actor_load_missing_blocks(WaveformActor* a)
{
	PF2;
	WfRectangle* rect = &a->rect;
	double _zoom = rect->len / a->region.len;

	double zoom_start = a->priv->animatable.rect_len.val.f / a->priv->animatable.len.val.i;
	if(zoom_start == 0.0) zoom_start = _zoom;
	dbg(2, "zoom=%.4f-->%.4f (%.4f)", zoom_start, _zoom, ZOOM_MED);
	int resolution1 = get_resolution(zoom_start);
	int resolution2 = get_resolution(_zoom);

	double zoom = MIN(_zoom, zoom_start); //use the zoom which uses the most blocks
	double zoom_max = MAX(_zoom, zoom_start);

	if(zoom_max >= ZOOM_MED){
dbg(1, "HI-RES");
		if(a->waveform->offline){ resolution1 = MIN(resolution1, RES_MED); resolution1 = MIN(resolution2, RES_MED); } //fallback to lower res
		else _wf_actor_allocate_hi(a);
	}

	if(resolution1 == RES_MED || resolution2 == RES_MED){
		WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
		double zoom_ = MAX(zoom, ZOOM_LO + 0.00000001);
		//dbg(0, "STD %.4f %.4f", zoom, zoom_);
		//dbg(0, "STD end=%i", ((int)a->region.start) + a->region.len);

		uint64_t start_max = MAX(a->region.start, a->priv->animatable.start.val.i);
		uint64_t start_min = MIN(a->region.start, a->priv->animatable.start.val.i);
		int len_max = start_max - start_min + MAX(a->region.len, a->priv->animatable.len.val.i);
		WfSampleRegion region = {start_min, len_max};

		WfRectangle rect_ = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};

		WfViewPort clippingport = viewport;
		float dl = a->region.start < a->priv->animatable.start.val.i ?-(a->region.start - a->priv->animatable.start.val.i) : 0.0; //panning left. make adjustment to prevent clipping
		float dr = a->region.start > a->priv->animatable.start.val.i ? (a->region.start - a->priv->animatable.start.val.i) : 0.0; //panning right. make adjustment to prevent clipping
		clippingport.left  = viewport.left  - dl;
		clippingport.right = viewport.right + dr;

		int viewport_start_block = wf_actor_get_first_visible_block(&a->region, zoom_, rect, &clippingport);
		int viewport_end_block   = wf_actor_get_last_visible_block (&region, &rect_, zoom_, &clippingport, a->waveform->textures);
		dbg(1, "STD block range: %i --> %i", viewport_start_block, viewport_end_block);

		int b; for(b=viewport_start_block;b<=viewport_end_block;b++){
			wf_actor_allocate_block_med(a, b);
		}

#ifdef USE_FBO
		if(agl_get_instance()->use_shaders && !fbo_test) fbo_test = fbo_new_test();
#endif
	}

	if(resolution1 == 1024 || resolution2 == 1024){
		// low res
		// TODO this resolution doesnt have the same adjustments to region and viewport like STD mode does.
		// -this doesnt seem to be causing any problems though
		dbg(1, "LOW");
		WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
		double zoom_ = MIN(zoom, ZOOM_LO - 0.0001);

		Waveform* w = a->waveform;
		if(!w->textures_lo){
			w->textures_lo = wf_texture_array_new(w->num_peaks / (WF_PEAK_STD_TO_LO * WF_PEAK_TEXTURE_SIZE) + ((w->num_peaks % (WF_PEAK_STD_TO_LO * WF_PEAK_TEXTURE_SIZE)) ? 1 : 0), w->n_channels);
		}

		WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
		WfRectangle rect_ = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};
		int viewport_start_block = wf_actor_get_first_visible_block(&a->region, zoom_, rect, &viewport);
		int viewport_end_block   = wf_actor_get_last_visible_block (&region, &rect_, zoom_, &viewport, a->waveform->textures_lo);
		dbg(1, "L block range: %i --> %i", viewport_start_block, viewport_end_block);

		int b; for(b=viewport_start_block;b<=viewport_end_block;b++){
			wf_actor_allocate_block_low(a, b);
		}
	}
	gl_warn("");
}


void
wf_actor_allocate(WaveformActor* a, WfRectangle* rect)
{
	g_return_if_fail(a);
	g_return_if_fail(rect);
	WfActorPriv* _a = a->priv;
	gl_warn("pre");

	if(rect->len == a->rect.len && rect->left == a->rect.left && rect->height == a->rect.height && rect->top == a->rect.top) return;

	gboolean is_new = a->rect.len == 0.0;

	a->priv->animatable.start.start_val.i = a->region.start;
	a->priv->animatable.len.start_val.i = MAX(1, a->region.len);
	a->priv->animatable.rect_left.start_val.f = a->rect.left;
	a->priv->animatable.rect_len.start_val.f = MAX(1, a->rect.len);

	if(is_new){
		//dont use a transition for a new actor
		a->priv->animatable.rect_len.val.f = rect->len;
		a->priv->animatable.rect_len.start_val.f = rect->len;
	}

	a->rect = *rect;

	dbg(1, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

	if(a->waveform->offline) return; //TODO try and load from existing peakfile.

	_wf_actor_load_missing_blocks(a);

	GList* animatables = NULL;
	if(_a->animatable.rect_left.start_val.f != *_a->animatable.rect_left.model_val.f) animatables = g_list_prepend(animatables, &_a->animatable.rect_left);
	if(_a->animatable.rect_len.start_val.f != *_a->animatable.rect_len.model_val.f) animatables = g_list_prepend(animatables, &_a->animatable.rect_len);
#if 0
	animatable->val.i = a->canvas->draw
		? animatable->start_val.i
		: *animatable->model_val.i;
#else
	GList* l = animatables; //ownership is transferred to the WfAnimation.
	for(;l;l=l->next){
		wf_actor_start_transition(a, (WfAnimatable*)l->data); //TODO this fn needs refactoring - doesnt do much now
	}
#endif
	//				dbg(1, "%.2f --> %.2f", animatable->start_val.f, animatable->val.f);

	if(a->canvas->draw){ //we can initiate painting so can animate.

		GList* l = _a->transitions;
		for(;l;l=l->next){
			//only remove animatables we are replacing. others need to finish.
			//****** except we start a new animation!
			GList* k = animatables;
			for(;k;k=k->next){
				wf_animation_remove_animatable((WfAnimation*)l->data, (WfAnimatable*)k->data);
			}
		}

		if(animatables){
			WfAnimation* animation = wf_animation_add_new(wf_actor_on_animation_finished);
			_a->transitions = g_list_append(_a->transitions, animation);
			wf_transition_add_member(animation, a, animatables);
			wf_animation_start(animation);
		}
	}
	gl_warn("gl error");
}


static inline void
_set_gl_state_for_block(WaveformCanvas* wfc, Waveform* w, WfGlBlock* textures, int b, WfColourFloat fg, float alpha)
{
	g_return_if_fail(b < textures->size);

	if(wfc->use_1d_textures){
		int c = 0;
		//dbg(0, "tex=%i", textures->peak_texture[c].main[b]);
		//dbg(0, "%i %i %i", textures->peak_texture[c].main[0], textures->peak_texture[c].main[1], textures->peak_texture[c].main[2]);
		texture_unit_use_texture(wfc->texture_unit[0], textures->peak_texture[c].main[b]);
		texture_unit_use_texture(wfc->texture_unit[1], textures->peak_texture[c].neg[b]);

		if(w->textures->peak_texture[WF_RIGHT].main){
			texture_unit_use_texture(wfc->texture_unit[2], textures->peak_texture[WF_RIGHT].main[b]);
			texture_unit_use_texture(wfc->texture_unit[3], textures->peak_texture[WF_RIGHT].neg[b]);
		}

		glActiveTexture(WF_TEXTURE0);
	}else
		use_texture(textures->peak_texture[0].main[b]);

	gl_warn("cannot bind texture: block=%i: %i", b, textures->peak_texture[0].main[b]);

	glColor4f(fg.r, fg.g, fg.b, alpha); //seems we have to set colour _after_ binding... ?

	gl_warn("gl error");
}


static inline float
get_peaks_per_pixel(WaveformCanvas* wfc, WfSampleRegion* region, WfRectangle* rect, int mode)
{
	//eg: for 51200 frame sample 256pixels wide: n_peaks=51200/256=200, ppp=200/256=0.8

	float region_width_px = wf_canvas_gl_to_px(wfc, rect->len);
	if(mode == MODE_HI) region_width_px /= 16; //this gives the correct result but dont know why.
	float peaks_per_pixel = ceil(((float)region->len / WF_PEAK_TEXTURE_SIZE) / region_width_px);
	dbg(2, "region_width_px=%.2f peaks_per_pixel=%.2f (%.2f)", region_width_px, peaks_per_pixel, ((float)region->len / WF_PEAK_TEXTURE_SIZE) / region_width_px);
	if(mode == MODE_LOW) peaks_per_pixel /= 16;
	return peaks_per_pixel;
}


static inline void
block_hires_shader(WaveformActor* actor, int b, double _block_wid, gboolean is_first, gboolean is_last, int first_offset, int samples_per_texture, WfRectangle rect, WfSampleRegion region, int region_end_block, double zoom, double x, double first_offset_px)
{
	//1d textures?
	//if we dont subdivide the blocks, size will be 256 x 16 = 4096 - should be ok.

	gl_warn("pre");

	WaveformCanvas* wfc = actor->canvas;
	Waveform* w = actor->waveform; 

	WfTextureHi* texture = g_hash_table_lookup(actor->waveform->textures_hi->textures, &b);
	if(!texture){
		dbg(1, "texture not available");
		return;
	}
	glEnable(GL_TEXTURE_1D);
	int c;for(c=0;c<waveform_get_n_channels(w);c++){
		texture_unit_use_texture(wfc->texture_unit[0 + 2 * c], texture->t[c].main);
		texture_unit_use_texture(wfc->texture_unit[1 + 2 * c], texture->t[c].neg);
	}
	dbg(1, "%i: textures: %u %u ok=%i,%i", b, texture->t[WF_LEFT].main, texture->t[WF_LEFT].neg, glIsTexture(texture->t[WF_LEFT].main), glIsTexture(texture->t[WF_LEFT].neg));
	gl_warn("texture assign");
	glActiveTexture(GL_TEXTURE0);

	HiResShader* hires_shader = wfc->priv->shaders.hires;
	hires_shader->uniform.fg_colour = actor->fg_colour;
	hires_shader->uniform.peaks_per_pixel = get_peaks_per_pixel(wfc, &region, &rect, MODE_HI);
	//dbg(0, "peaks_per_pixel=%.2f", hires_shader->uniform.peaks_per_pixel);
	hires_shader->uniform.top = rect.top;
	hires_shader->uniform.bottom = rect.top + rect.height;
	hires_shader->uniform.n_channels = waveform_get_n_channels(w);

	wf_canvas_use_program_(wfc, &hires_shader->shader);

	WfColourFloat fg;
	wf_colour_rgba_to_float(&fg, actor->fg_colour);
	float alpha = ((float)(actor->fg_colour & 0xff)) / 256.0;
	glColor4f(fg.r, fg.g, fg.b, alpha); //seems we have to set colour _after_ binding... ?

					//duplicate from _paint - temporary only!

					double block_wid = _block_wid;
					double tex_pct = 1.0; //use the whole texture
					double tex_start = 0.0;
					if (is_first){
						if(first_offset) tex_pct = 1.0 - ((double)first_offset) / samples_per_texture;
						block_wid = _block_wid * tex_pct;
						tex_start = 1 - tex_pct;
						dbg(2, "rect.left=%.2f region->start=%i first_offset=%i", rect.left, region.start, first_offset);
					}
					if (is_last){
						//if(x + _block_wid < x0 + rect->len){
						if(b < region_end_block){
							//end is offscreen. last block is not smaller.
						}else{
							//end is trimmed
							double part_inset_px = wf_actor_samples2gl(zoom, region.start);
							//double file_start_px = rect.left - part_inset_px;
							double distance_from_file_start_to_region_end = part_inset_px + rect.len;
							block_wid = distance_from_file_start_to_region_end - b * _block_wid;
							dbg(2, " %i: inset=%.2f s->e=%.2f i*b=%.2f", b, part_inset_px, distance_from_file_start_to_region_end, b * _block_wid);
							if(b * _block_wid > distance_from_file_start_to_region_end){ gwarn("!!"); return; }
						}

						if(b == w->textures->size - 1) dbg(1, "last sample block. fraction=%.2f", w->textures->last_fraction);
						//TODO check what happens here if we allow non-square textures
						tex_pct = block_wid / _block_wid;
					}

					dbg (2, "%i: is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex_start=%.2f", b, is_last, x, block_wid, _block_wid, tex_pct, tex_start);
					if(tex_pct > 1.0 || tex_pct < 0.0) gwarn("tex_pct! %.2f", tex_pct);
					double tex_x = x + ((is_first && first_offset) ? first_offset_px : 0);

	glBegin(GL_QUADS);
#if defined (USE_FBO) && defined (multipass)
//	if(false){
	if(true){    //fbo not yet implemented for hi-res mode.
#else
	if(wfc->use_1d_textures){
#endif
//dbg(0, "  x: %.2f --> %.2f", tex_x, tex_x + block_wid);
		glMultiTexCoord2f(WF_TEXTURE0, tex_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       rect.top);
		glMultiTexCoord2f(WF_TEXTURE0, tex_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, rect.top);
		glMultiTexCoord2f(WF_TEXTURE0, tex_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, rect.top + rect.height);
		glMultiTexCoord2f(WF_TEXTURE0, tex_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       rect.top + rect.height);
	}else{
		gerr("TODO 2d textures in MODE_HI");
		glTexCoord2d(tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       rect.top);
		glTexCoord2d(tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, rect.top);
		glTexCoord2d(tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, rect.top + rect.height);
		glTexCoord2d(tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       rect.top + rect.height);
	}
	glEnd();
}


void
wf_actor_paint(WaveformActor* actor)
{
	//-must have called gdk_gl_drawable_gl_begin() first.
	//-it is assumed that all textures have been loaded.
					gl_warn("pre");

	//note: there is some benefit in quantising the x positions (eg for subpixel consistency),
	//but to preserve relative actor positions it must be done at the canvas level.

	AGl* agl = agl_get_instance();
	g_return_if_fail(actor);
	WaveformCanvas* wfc = actor->canvas;
	g_return_if_fail(wfc);
	g_return_if_fail(actor);
	Waveform* w = actor->waveform; 
	if(w->offline) return;
	WfRectangle rect = {actor->rect.left, actor->rect.top, actor->priv->animatable.rect_len.val.f, actor->rect.height};
	g_return_if_fail(rect.len);

	WfSampleRegion region = {actor->priv->animatable.start.val.i, actor->priv->animatable.len.val.i};
	static gboolean region_len_warning_done = false;
	if(!region_len_warning_done && !region.len){ region_len_warning_done = true; gwarn("zero region length"); }

	WfViewPort viewport; wf_actor_get_viewport(actor, &viewport);

	double zoom = rect.len / region.len;

	int mode = get_mode(zoom);
//dbg(1, "mode=%s", modes[mode].name);

	WfColourFloat fg;
	wf_colour_rgba_to_float(&fg, actor->fg_colour);
	WfColourFloat bg;
	wf_colour_rgba_to_float(&bg, actor->bg_colour);
//#if defined (USE_FBO) && defined (multipass)
//#else
	float alpha = ((float)(actor->fg_colour & 0xff)) / 256.0;
//#endif

	if(w->num_peaks){
		WfGlBlock* textures = mode == MODE_LOW ? w->textures_lo : w->textures;
		if(!textures) return; //in hi-res mode, textures are loaded asynchronously and may not be ready yet

		int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (mode == MODE_LOW ? WF_PEAK_STD_TO_LO : 1);

		int region_start_block   = region.start / samples_per_texture;
		int region_end_block     = (region.start + region.len) / samples_per_texture;
		int viewport_start_block = wf_actor_get_first_visible_block(&region, zoom, &rect, &viewport);
		int viewport_end_block   = wf_actor_get_last_visible_block (&region, &rect, zoom, &viewport, textures);
		if(region_end_block > textures->size -1){ gwarn("region too long? region_end_block=%i n_blocks=%i region.len=%i", region_end_block, textures->size, region.len); region_end_block = w->textures->size -1; }
		dbg(2, "block range: region=%i-->%i viewport=%i-->%i", region_start_block, region_end_block, viewport_start_block, viewport_end_block);
		dbg(2, "rect=%.2f %.2f viewport=%.2f %.2f", rect.left, rect.len, viewport.left, viewport.right);
//dbg(0, "%.4f %i block range: region=%i-->%i viewport=%i-->%i", zoom, resolution, region_start_block, region_end_block, viewport_start_block, viewport_end_block);

		int first_offset = region.start % samples_per_texture;
		double first_offset_px = wf_actor_samples2gl(zoom, first_offset);

		double _block_wid = wf_actor_samples2gl(zoom, samples_per_texture);
#if defined (USE_FBO) && defined (multipass)
		if(agl->use_shaders){
			//set gl state

			wfc->priv->shaders.vertical->uniform.fg_colour = actor->fg_colour;
			wfc->priv->shaders.vertical->uniform.peaks_per_pixel = get_peaks_per_pixel(wfc, &region, &rect, mode);
			//TODO the vertical shader needs to check _all_ the available texture values to get true peak.
			wf_canvas_use_program_(wfc, &vertical.shader);

					glDisable(GL_TEXTURE_1D);
					glEnable(GL_TEXTURE_2D);
					glActiveTexture(GL_TEXTURE0);
		}
#else
		if(wfc->use_1d_textures){
			wf_canvas_use_program(wfc, ((WfShader*)wfc->priv->shaders.peak)->program);

			//uniforms: (must be done on each paint because some vars are actor-specific)
			float peaks_per_pixel = get_peaks_per_pixel(wfc, &region, &rect, mode);
			dbg(2, "vpwidth=%.2f region_len=%i region_n_peaks=%.2f peaks_per_pixel=%.2f", (viewport.right - viewport.left), region.len, ((float)region.len / WF_PEAK_TEXTURE_SIZE), peaks_per_pixel);
			float bottom = rect.top + rect.height;
			int n_channels = textures->peak_texture[WF_RIGHT].main ? 2 : 1;
			wfc->priv->shaders.peak->set_uniforms(peaks_per_pixel, rect.top, bottom, actor->fg_colour, n_channels);
		}
#endif

		if(mode <= MODE_MED){
			//check textures are loaded
			if(wf_debug > 1){ //textures may initially not be loaded, so don't show this warning too much
				int n = 0;
				int b; for(b=viewport_start_block;b<=viewport_end_block;b++){
					if(!textures->peak_texture[WF_LEFT].main[b]){ n++; gwarn("texture not loaded: b=%i", b); }
				}
				if(n) gwarn("%i textures not loaded", n);
			}
		glEnable(wfc->use_1d_textures ? GL_TEXTURE_1D : GL_TEXTURE_2D);
		}

		//for hi-res mode:
		//block_region specifies the a sample range within the current block
		WfSampleRegion block_region = {region.start % WF_PEAK_BLOCK_SIZE, WF_PEAK_BLOCK_SIZE - region.start % WF_PEAK_BLOCK_SIZE};
		WfSampleRegion block_region_v_hi = {region.start, WF_PEAK_BLOCK_SIZE - region.start % WF_PEAK_BLOCK_SIZE};

		double x = rect.left + (viewport_start_block - region_start_block) * _block_wid - first_offset_px; // x is now the start of the first block (can be before part start when inset is present)
		g_return_if_fail(WF_PEAK_BLOCK_SIZE == (WF_PEAK_RATIO * WF_PEAK_TEXTURE_SIZE)); // temp check. we use a simplified loop which requires the two block sizes are the same
		gboolean is_first = true;
		int b; for(b=viewport_start_block;b<=viewport_end_block;b++){
			//dbg(0, "b=%i x=%.2f", b, x);
			gboolean is_last = (b == viewport_end_block) || (b == textures->size - 1); //2nd test is unneccesary?

			//try hi res painting first, fallback to lower resolutions.

			gboolean block_done = false;

			switch(mode){
				case MODE_V_HI:
					//dbg(1, "----- super hi mode !!");
					;WfAudioData* audio = w->priv->audio_data;
					if(audio->n_blocks){
						WfBuf16* buf = audio->buf16[b];
						if(buf){
							//TODO might these prevent further blocks at different res? difficult to notice as they are usually the same.
							wf_canvas_use_program(wfc, 0);
							glDisable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							glDisable(GL_TEXTURE_2D);
							glDisable(GL_TEXTURE_1D);

							int c; for(c=0;c<w->n_channels;c++){
								if(buf->buf[c]){
									//dbg(1, "peakbuf: %i:%i: %i", b, c, ((short*)peakbuf->buf[c])[0]);
									dbg(2, "b=%i", b);
									float block_rect_start = is_first ? 0 : x;//block_region.start * zoom;
									WfRectangle block_rect = {block_rect_start, rect.top + c * rect.height/2, block_region_v_hi.len * zoom, rect.height/w->n_channels};
									draw_wave_buffer_v_hi(w, block_region_v_hi, &block_rect, &viewport, buf, c, wfc->v_gain, actor->fg_colour);
								}
							}
							block_done = true; //super hi res was succussful. no more painting needed for this block.
							break;
						}
					}

				case MODE_HI:
					;Peakbuf* peakbuf = waveform_get_peakbuf_n(w, b);
					if(peakbuf){
						if(agl->use_shaders){
							block_hires_shader(actor, b, _block_wid, is_first, is_last, first_offset, samples_per_texture, rect, region, region_end_block, zoom, x, first_offset_px);
							block_done = true; //TODO should really use the fall-through code to apply the textures
						}else{

						dbg(1, "  b=%i x=%.2f", b, x);

						//TODO might these prevent further blocks at different res? difficult to notice as they are usually the same.
						wf_canvas_use_program(wfc, 0);
						glDisable(GL_BLEND);
						//glEnable(GL_BLEND);
						//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						glDisable(GL_TEXTURE_2D);
						glDisable(GL_TEXTURE_1D);

						int c; for(c=0;c<w->n_channels;c++){
							if(peakbuf->buf[c]){
								//dbg(1, "peakbuf: %i:%i: %i", b, c, ((short*)peakbuf->buf[c])[0]);
								//WfRectangle block_rect = {x, rect.top + c * rect.height/2, _block_wid, rect.height / w->n_channels};
								WfRectangle block_rect = {is_first
										? x + (block_region.start - region.start % WF_PEAK_BLOCK_SIZE) * zoom
										: x,
									rect.top + c * rect.height/2, _block_wid, rect.height / w->n_channels};
								if(is_first){
									float first_fraction =((float)block_region.len) / WF_PEAK_BLOCK_SIZE;
									block_rect.left += (WF_PEAK_BLOCK_SIZE - WF_PEAK_BLOCK_SIZE * first_fraction) * zoom;
								}
								//WfRectangle block_rect = {x + (region.start % WF_PEAK_BLOCK_SIZE) * zoom, rect.top + c * rect.height/2, _block_wid, rect.height / w->n_channels};
								dbg(1, "  HI: %i: rect=%.2f-->%.2f", b, block_rect.left, block_rect.left + block_rect.len);

								if(is_last){
									if(b < region_end_block){
										// end is offscreen. last block is not smaller.
										// reducing the block size here would be an optimisation rather than essential
									}else{
										// last block of region (not truncated by viewport).
										//block_region.len = region.len - b * WF_PEAK_BLOCK_SIZE;
										block_region.len = region.len % WF_PEAK_BLOCK_SIZE;
										//block_rect.len = _block_wid * ((float)block_region.len) / WF_PEAK_BLOCK_SIZE;
										dbg(1, "REGIONLAST: %i/%i region.len=%i ratio=%.2f rect=%.2f %.2f", b, region_end_block, block_region.len, ((float)block_region.len) / WF_PEAK_BLOCK_SIZE, block_rect.left, block_rect.len);
									}
								}
								block_rect.len = block_region.len * zoom; //always!
								draw_wave_buffer_hi(w, block_region, &block_rect, peakbuf, c, wfc->v_gain, actor->fg_colour);
							}
							else dbg(1, "buf not ready: %i", c);
						}
						block_done = true; //hi res was succussful. no more painting needed for this block.
						} //end if use_shaders
					}
					if(block_done) break;

				// standard res and low res
				case MODE_LOW ... MODE_MED:
					;double block_wid = _block_wid;
					double tex_pct = 1.0; //use the whole texture
					double tex_start = 0.0;
					if (is_first){
						if(first_offset) tex_pct = 1.0 - ((double)first_offset) / samples_per_texture;
						block_wid = _block_wid * tex_pct;
						tex_start = 1 - tex_pct;
						dbg(2, "rect.left=%.2f region->start=%i first_offset=%i", rect.left, region.start, first_offset);
					}
					if (is_last){
						//if(x + _block_wid < x0 + rect->len){
						if(b < region_end_block){
							//end is offscreen. last block is not smaller.
						}else{
							//end is trimmed
#if 0
							block_wid = rect.len - x0 - _block_wid * (i - viewport_start_block); //last block is smaller
#else
							double part_inset_px = wf_actor_samples2gl(zoom, region.start);
							//double file_start_px = rect.left - part_inset_px;
							double distance_from_file_start_to_region_end = part_inset_px + rect.len;
							block_wid = distance_from_file_start_to_region_end - b * _block_wid;
							dbg(2, " %i: inset=%.2f s->e=%.2f i*b=%.2f", b, part_inset_px, distance_from_file_start_to_region_end, b * _block_wid);
							if(b * _block_wid > distance_from_file_start_to_region_end){ gwarn("!!"); continue; }
#endif
						}

						if(b == w->textures->size - 1) dbg(2, "last sample block. fraction=%.2f", w->textures->last_fraction);
						//TODO check what happens here if we allow non-square textures
#if 0 // !! doesnt matter if its the last block or not, we must still take the region into account.
						tex_pct = (i == w->textures->size - 1)
							? (block_wid / _block_wid) / w->textures->last_fraction    // block is at end of sample
															  // -cannot use block_wid / _block_wid for last block because the last texture is smaller.
							: block_wid / _block_wid;
#else
						tex_pct = block_wid / _block_wid;
#endif
					}

					dbg (2, "%i: is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex_start=%.2f", b, is_last, x, block_wid, _block_wid, tex_pct, tex_start);
					if(tex_pct > 1.0 || tex_pct < 0.0) gwarn("tex_pct! %.2f", tex_pct);
					double tex_x = x + ((is_first && first_offset) ? first_offset_px : 0);

#if defined (USE_FBO) && defined (multipass)
					if(agl->use_shaders){
						//rendering from 2d texture not 1d
						WfFBO* fbo = true ? textures->fbo[b] : fbo_test;
						if(fbo){ //seems that the fbo may not be created initially...
							use_texture(fbo->texture);
						}
					}else{
						_set_gl_state_for_block(wfc, w, textures, b, fg, alpha);
					}
#else
					_set_gl_state_for_block(wfc, w, textures, b, fg, alpha);
#endif

					glPushMatrix();
					glTranslatef(0, 0, actor->z);
					glBegin(GL_QUADS);
#if defined (USE_FBO) && defined (multipass)
					if(false){
#else
					if(wfc->use_1d_textures){
#endif
						glMultiTexCoord2f(WF_TEXTURE0, tex_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       rect.top);
						glMultiTexCoord2f(WF_TEXTURE0, tex_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, rect.top);
						glMultiTexCoord2f(WF_TEXTURE0, tex_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, rect.top + rect.height);
						glMultiTexCoord2f(WF_TEXTURE0, tex_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       rect.top + rect.height);
					}else{
						glTexCoord2d(tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       rect.top);
						glTexCoord2d(tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, rect.top);
						glTexCoord2d(tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, rect.top + rect.height);
						glTexCoord2d(tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       rect.top + rect.height);
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
					break;

				default:
					break;
			} // end resolution switch

#if 0
#undef DEBUG_BLOCKS
#define DEBUG_BLOCKS
#ifdef DEBUG_BLOCKS
			double bot = rect.top + rect.height;
			int pr = wfc->_program;
			wf_canvas_use_program(wfc, 0);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_1D);
//					glDisable(GL_BLEND);
			glColor4f(1.0, 0.0, 1.0, 1.0);
			glLineWidth(2);
			glBegin(GL_LINES);
				glVertex3f(x,              rect.top + 1, 0);    glVertex3f(x + _block_wid, rect.top + 1, 0);
				glVertex3f(x + _block_wid, rect.top + 1, 0);    glVertex3f(x + _block_wid, bot - 1,      0);
				glVertex3f(x,              bot - 1,      0);    glVertex3f(x + _block_wid, bot - 1,      0);
				glVertex3f(x ,             rect.top + 1, 0);    glVertex3f(x,              bot - 1,      0);
			glEnd();
			glColor3f(1.0, 1.0, 1.0);
			glEnable(wfc->use_1d_textures ? GL_TEXTURE_1D : GL_TEXTURE_2D);
			wf_canvas_use_program(wfc, pr); //TODO move ?
#endif
#endif
			x += _block_wid;
			block_region.start = 0; //all blocks except first start at 0
			block_region.len = WF_PEAK_BLOCK_SIZE;
			block_region_v_hi.start = (block_region_v_hi.start / WF_PEAK_BLOCK_SIZE + 1) * WF_PEAK_BLOCK_SIZE;
			block_region_v_hi.len   = WF_PEAK_BLOCK_SIZE - block_region_v_hi.start % WF_PEAK_BLOCK_SIZE;
			is_first = false;
		}
	}
	wf_canvas_use_program_(wfc, NULL);

	gl_warn("(@ paint end)");
}


/*
 *  Load all textures for the given block.
 *  There will be between 1 and 4 textures depending on shader/alphabuf mono/stero.
 */
static void
wf_actor_load_texture1d(Waveform* w, Mode mode, WfGlBlock* blocks, int blocknum)
{

	if(blocks) dbg(2, "%i: %i %i %i %i", blocknum,
		blocks->peak_texture[0].main[blocknum],
		blocks->peak_texture[0].neg[blocknum],
		blocks->peak_texture[1].main ? blocks->peak_texture[WF_RIGHT].main[blocknum] : 0,
		blocks->peak_texture[1].neg ? blocks->peak_texture[WF_RIGHT].neg[blocknum] : 0
	);

	//TODO not used for MODE_HI (wrong size)
	struct _buf {
		guchar positive[WF_PEAK_TEXTURE_SIZE];
		guchar negative[WF_PEAK_TEXTURE_SIZE];
	} buf;

	void make_texture_data(Waveform* w, int ch, struct _buf* buf)
	{
		//copy peak data into a temporary buffer, translating from 16 bit to 8 bit

		WfPeakBuf* peak = &w->priv->peak;
		int f; for(f=0;f<WF_PEAK_TEXTURE_SIZE;f++){
			int i = (WF_PEAK_TEXTURE_SIZE * blocknum + f) * WF_PEAK_VALUES_PER_SAMPLE;
			if(i >= peak->size){
#if 0
				int j; for(j=0;j<5;j++){
					printf("  %i=%i %i\n", j, peak->buf[ch][2*j], peak->buf[ch][2*j +1]);
				}
				for(j=0;j<5;j++){
					printf("  %i %i=%i %i\n", i, i -2*j -2, peak->buf[ch][i -2*j -2 ], peak->buf[ch][i -2*j -1]);
				}
#endif
				dbg(1, "end of peak: %i b=%i n_sec=%.3f", peak->size, blocknum, ((float)((WF_PEAK_TEXTURE_SIZE * blocknum + f) * WF_PEAK_RATIO))/44100); break;
			}

			buf->positive[f] =  peak->buf[ch][i  ] >> 8;
			buf->negative[f] = -peak->buf[ch][i+1] >> 8;
		}
	}

	void make_texture_data_low(Waveform* w, int ch, struct _buf* buf)
	{
		dbg(2, "b=%i", blocknum);

		WfPeakBuf* peak = &w->priv->peak;
		int f; for(f=0;f<WF_PEAK_TEXTURE_SIZE;f++){
			int i = (WF_PEAK_TEXTURE_SIZE * blocknum + f) * WF_PEAK_VALUES_PER_SAMPLE * WF_PEAK_STD_TO_LO;
			if(i >= peak->size) break;

			WfPeakSample p = {0, 0};
			int j; for(j=0;j<WF_PEAK_STD_TO_LO;j++){
				int ii = i + WF_PEAK_VALUES_PER_SAMPLE * j;
				if(ii >= peak->size) break; // last item
				p.positive = MAX(p.positive, peak->buf[ch][ii    ]);
				p.negative = MIN(p.negative, peak->buf[ch][ii + 1]);
			}

			buf->positive[f] =  p.positive >> 8;
			buf->negative[f] = -p.negative >> 8;
		}
	}

	static IntBufHi buf_hi; //TODO shouldnt be static
	void make_data(Waveform* w, int c, struct _buf* buf)
	{
		if(mode == MODE_HI){
			modes[MODE_HI].make_texture_data(w, c, &buf_hi, blocknum);
		}
		else if(blocks == w->textures)
			make_texture_data(w, c, buf);
		else
			make_texture_data_low(w, c, buf);
	}

	struct _d {
		int          tex_unit;
		int          tex_id;
		guchar*      buf;
	};

	void _load_texture(struct _d* d)
	{
		//copy the data to the hardware texture

		glActiveTexture(d->tex_unit);
glEnable(GL_TEXTURE_1D);
		glBindTexture(GL_TEXTURE_1D, d->tex_id);
		dbg (2, "loading texture1D... texture_id=%u", d->tex_id);
		gl_warn("gl error: bind failed: unit=%i buf=%p tid=%i", d->tex_unit, d->buf, d->tex_id);
		if(!glIsTexture(d->tex_id)) gwarn ("invalid texture: texture_id=%u", d->tex_id);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//
		//using MAG LINEAR smooths nicely but can reduce the peak.
		//
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage1D(GL_TEXTURE_1D, 0, GL_ALPHA8, modes[mode].texture_size, 0, GL_ALPHA, GL_UNSIGNED_BYTE, d->buf);

		gl_warn("unit=%i buf=%p tid=%i", d->tex_unit, d->buf, d->tex_id);
	}

	WfTextureHi* texture_hi = (mode == MODE_HI) ? g_hash_table_lookup(w->textures_hi->textures, &blocknum) : NULL;

	int c;for(c=0;c<waveform_get_n_channels(w);c++){
		make_data(w, c, &buf);

		struct _d d[2] = {
			{WF_TEXTURE0 + 2 * c, texture_hi ? texture_hi->t[c].main : blocks->peak_texture[c].main[blocknum], buf.positive},
			{WF_TEXTURE1 + 2 * c, texture_hi ? texture_hi->t[c].neg  : blocks->peak_texture[c].neg [blocknum], buf.negative},
		};

		if(mode == MODE_HI){
			d[0].buf = buf_hi.positive;
			d[1].buf = buf_hi.negative;
		}

		int v; for(v=0;v<2;v++){
			_load_texture(&d[v]);
		}
		gl_warn("loading textures failed (lhs)");
	}

	glActiveTexture(WF_TEXTURE0);

	gl_warn("done");
}


static void
make_texture_data_hi(Waveform* w, int ch, IntBufHi* buf, int blocknum)
{
	dbg(1, "b=%i", blocknum);
	int texture_size = modes[MODE_HI].texture_size;
	Peakbuf* peakbuf = waveform_get_peakbuf_n(w, blocknum);
	int f; for(f=0;f<texture_size;f++){
		int i = f * WF_PEAK_VALUES_PER_SAMPLE;
		if(i >= peakbuf->size){
			dbg(2, "end of peak: %i b=%i n_sec=%.3f", peakbuf->size, blocknum, ((float)((texture_size * blocknum + f) * WF_PEAK_RATIO))/44100); break;
		}

		short* p = peakbuf->buf[ch];
		buf->positive[f] =  p[i  ] >> 8;
		buf->negative[f] = -p[i+1] >> 8;
	}
#if 0
	int j; for(j=0;j<20;j++){
		printf("  %2i: %5i %5i %5u %5u\n", j, ((short*)peakbuf->buf[ch])[2*j], ((short*)peakbuf->buf[ch])[2*j +1], (guint)(buf->positive[j] * 0x100), (guint)(buf->negative[j] * 0x100));
	}
#endif
}


static void
_wf_actor_load_texture_hi(WaveformActor* a, int block)
{
	// audio data for this block _must_ already be loaded

	/*
	WfViewPort viewport; wf_actor_get_viewport(actor, &viewport);
	*/
	WfRectangle* rect = &a->rect;
	double zoom_end = rect->len / a->region.len;
	double zoom_start = a->priv->animatable.rect_len.val.f / a->priv->animatable.len.val.i;
	if(zoom_start == 0.0) zoom_start = zoom_end;
	//dbg(1, "zoom=%.4f-->%.4f (%.4f)", zoom_start, zoom_end, ZOOM_MED);
	int mode1 = get_mode(zoom_start);
	int mode2 = get_mode(zoom_end);

	if(mode1 == MODE_HI || mode2 == MODE_HI){
		//TODO check this block is within current viewport

		WfTextureHi* texture = g_hash_table_lookup(a->waveform->textures_hi->textures, &block);
		if(!texture){
			texture = waveform_texture_hi_new();
			dbg(1, "b=%i: inserting...", block);
			uint32_t* key = (uint32_t*)g_malloc(sizeof(uint32_t));
			*key = block;
			g_hash_table_insert(a->waveform->textures_hi->textures, key, texture);
			wf_actor_allocate_block_hi(a, block);
		}
		else dbg(1, "b=%i: already have texture. t=%i", block, texture->t[WF_LEFT].main);
		if(wf_debug > 1) _wf_actor_print_hires_textures(a);

		if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
	}
}


static void
wf_actor_start_transition(WaveformActor* a, WfAnimatable* animatable)
{
	g_return_if_fail(a);

	animatable->val.i = a->canvas->draw
		? animatable->start_val.i
		: *animatable->model_val.i;
}


static void
_wf_actor_print_hires_textures(WaveformActor* a)
{
	dbg(0, "");
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init (&iter, a->waveform->textures_hi->textures);
	while (g_hash_table_iter_next (&iter, &key, &value)){
		int b = *((int*)key);
		//WfTextureHi* th = value;
		printf("  b=%i\n", b);
	}
}

