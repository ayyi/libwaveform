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
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/audio.h"
#include "waveform/texture_cache.h"
#include "waveform/animator.h"
#include "waveform/shaderutil.h"
#include "waveform/hi_res.h"
//#include "gl_utils.h" //FIXME
extern void use_texture(int texture);
#include "waveform/actor.h"
#include "waveform/gl_ext.h"

/*
	Mipmapping is need because without it, peaks can be missed at small zoom, and can be too blurry.

	However, it is problematic:
	- doesnt work with non-square textures (is supposed to; possible driver issues)
	- small x size reduces y resolution - doesnt look good.
 */
#define USE_MIPMAPPING
#undef USE_MIPMAPPING

Shader sh_main = {"peak.vert", "peak.frag", 0};

struct _actor_priv
{
	struct {
		WfAnimatable  start;     // region
		WfAnimatable  len;       // region
		WfAnimatable  rect_len;
	}               animatable;
	GList*          transitions; // list of WfAnimation*

	gulong          peakdata_ready_handler;
};

static void   wf_actor_get_viewport          (WaveformActor*, WfViewPort*);
static void   wf_actor_start_transition      (WaveformActor*, WfAnimatable*);
static void   wf_actor_on_animation_finished (WaveformActor*, WfAnimation*);
//static void   wf_actor_paint_hi2             (WaveformActor*);
static void   wf_actor_allocate_block        (WaveformActor*, int b);
static void  _wf_actor_load_missing_blocks   (WaveformActor*);
static void   rgba_to_float                  (uint32_t rgba, float* r, float* g, float* b);


void
wf_actor_init()
{
	get_gl_extensions();
}


WaveformActor*
wf_actor_new(Waveform* w)
{
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

	animatable = &a->priv->animatable.rect_len;
	animatable->model_val.f = &a->rect.len;
	animatable->start_val.f = a->rect.len;
	animatable->val.i       = a->rect.len;
	animatable->type        = WF_FLOAT;

	void _wf_actor_on_peakdata_available(Waveform* waveform, int block, gpointer _actor)
	{
		dbg(2, ">>> block=%i", block);

//the idle is now done before emitting the signal
//		gboolean __redraw(gpointer _actor)
//		{
			WaveformActor* a = _actor;
			if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
//			return IDLE_STOP;
//		}
//		g_idle_add(__redraw, _actor);
	}
	a->priv->peakdata_ready_handler = g_signal_connect (w, "peakdata-ready", (GCallback)_wf_actor_on_peakdata_available, a);
	return a;
}


void
wf_actor_free(WaveformActor* a)
{
	g_return_if_fail(a);

	g_signal_handler_disconnect((gpointer)a->waveform, a->priv->peakdata_ready_handler);
	g_free(a->priv);
	a->waveform = waveform_unref0(a->waveform);
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

//dbg(1, "---- len=%u %u", a->priv->animatable.len.start_val.i, MAX(1, a->region.len));
	if(!start && !end) return;

	_wf_actor_load_missing_blocks(a);

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
			//Blah* blah = m->data;
			if(start){
				wf_animation_remove_animatable(animation, &a->priv->animatable.start);
#if 0
				//start already animated?
				WfAnimatable* animatable = &a->priv->animatable.start;
				//dbg(1, "    %p %p", blah->transitions, animatable);
				if(g_list_find(blah->transitions, animatable)){
					//remove the animatable from the old animation
					dbg(1, "       >>>>>>> already animating: 'start'");
					animatable->start_val.i = animatable->val.i;

					if(!(blah->transitions = g_list_remove(blah->transitions, animatable))) wf_animation_remove(animation);
					break;
				}
#endif
			}
			if(end){
				wf_animation_remove_animatable(animation, &a->priv->animatable.len);
#if 0
				WfAnimatable* animatable = &a->priv->animatable.len;
				//dbg(1, "    %p %p", blah->transitions, animatable);
				if(g_list_find(blah->transitions, animatable)){
					dbg(2, "       >>>>>>> already animating: 'end'. new_transition=%i-->%i", animatable->val.i, *animatable->model_val.i);
					animatable->start_val.i = animatable->val.i;

					int l = g_list_length(blah->transitions);
					blah->transitions = g_list_remove(blah->transitions, animatable);
					if(g_list_length(blah->transitions) != l - 1) gwarn("transition not removed");
					//dbg(1, "      len=%i", g_list_length(blah->transitions));

					if(!blah->transitions) wf_animation_remove(animation);
					break;
				}
#endif
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
	double part_inset_px = wf_actor_samples2gl(zoom, region->start);
	double file_start_px = rect->left - part_inset_px;
	double block_wid = wf_actor_samples2gl(zoom, WF_SAMPLES_PER_TEXTURE);

	int part_start_block = region->start / WF_SAMPLES_PER_TEXTURE;
	int part_end_block = (region->start + region->len) / WF_SAMPLES_PER_TEXTURE;
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
wf_actor_get_last_visible_block(WaveformActor* a, double zoom, WfViewPort* viewport_px)
{
	Waveform* waveform = a->waveform;
	WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
	WfRectangle rect = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};

	g_return_val_if_fail(waveform, -1);
	g_return_val_if_fail(waveform->gl_blocks, -1);
	g_return_val_if_fail(viewport_px->right - viewport_px->left > 0.01, -1);

	//dbg(1, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

	double part_inset_px = wf_actor_samples2gl(zoom, region.start);
	double file_start_px = rect.left - part_inset_px;
	double block_wid = wf_actor_samples2gl(zoom, WF_SAMPLES_PER_TEXTURE);
	//dbg(1, "vp->right=%.2f", viewport_px->right);
	int region_start_block = region.start / WF_SAMPLES_PER_TEXTURE;
	float _end_block = ((float)(region.start + region.len)) / WF_SAMPLES_PER_TEXTURE;
	dbg(2, "region_start=%i region_end=%i start_block=%i end_block=%.2f(%.i) n_peak_frames=%i gl->size=%i", region.start, region.len, region_start_block, _end_block, (int)ceil(_end_block), waveform->gl_blocks->size * 256, waveform->gl_blocks->size);

	//we round _down_ as the block corresponds to the _start_ of a section.
	//int region_end_block = MIN(ceil(_end_block), waveform->gl_blocks->size - 1);
	int region_end_block = MIN(_end_block, waveform->gl_blocks->size - 1);

	if(region_end_block >= waveform->gl_blocks->size) gwarn("!!");

	//crop to viewport:
	int b; for(b=region_start_block;b<=region_end_block-1;b++){ //note we dont check the last block which can be partially outside the viewport
		float block_end_px = file_start_px + (b + 1) * block_wid;
		//dbg(1, " %i: block_px: %.1f --> %.1f", b, block_end_px - (int)block_wid, block_end_px);
		if(block_end_px > viewport_px->right) dbg(1, "end %i clipped by viewport at block %i. vp.right=%.2f block_end=%.1f", region_end_block, MAX(0, b/* - 1*/), viewport_px->right, block_end_px);
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


static void
wf_actor_allocate_block(WaveformActor* a, int b)
{
	//g_return_if_fail(WAVEFORM_IS_DRAWING(a->canvas));
	g_return_if_fail(b >= 0);

	Waveform* w = a->waveform;
	WfGlBlocks* blocks = w->gl_blocks;

	int c = WF_LEFT;
	if(blocks->peak_texture[c].main[b] && glIsTexture(blocks->peak_texture[c].main[b])){
		//gwarn("waveform texture already assigned for block %i: %i", b, blocks->peak_texture[c].main[b]);
		return;
	}

	int texture_id = blocks->peak_texture[c].main[b] = texture_cache_assign_new((WaveformBlock){a->waveform, b});

	if(a->canvas->use_shaders){
		guint* peak_texture[4] = {
			&blocks->peak_texture[WF_LEFT ].main[b],
			&blocks->peak_texture[WF_LEFT ].neg[b],
			&blocks->peak_texture[WF_RIGHT].main[b],
			&blocks->peak_texture[WF_RIGHT].neg[b]
		};
		blocks->peak_texture[c].neg[b] = texture_cache_assign_new ((WaveformBlock){a->waveform, b});

		if(a->waveform->priv->peak.buf[WF_RIGHT]){
			int i; for(i=2;i<4;i++){
				*peak_texture[i] = texture_cache_assign_new ((WaveformBlock){a->waveform, b});
			}
			dbg(1, "rhs: %i: texture=%i %i %i %i", b, *peak_texture[0], *peak_texture[1], *peak_texture[2], *peak_texture[3]);
		}else{
			dbg(1, "* %i: texture=%i %i (rhs peak.buf not loaded)", b, texture_id, *peak_texture[1]);
		}

		wf_actor_load_texture1d(w, b);
	}else{
		dbg(1, "* %i: texture=%i", b, texture_id);
		AlphaBuf* alphabuf = wf_alphabuf_new(w, b, false); //TODO free
		wf_actor_load_texture_from_alphabuf(a->canvas, texture_id, alphabuf);
	}
}


static void
wf_actor_get_viewport(WaveformActor* a, WfViewPort* viewport)
{
	WaveformCanvas* canvas = a->canvas;

	if(canvas->viewport) *viewport = *canvas->viewport;
	else {
		viewport->left   = a->rect.left;
		viewport->top    = a->rect.top;
		viewport->right  = a->rect.left + a->priv->animatable.rect_len.val.f;
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

		//int viewport_end_block   = wf_actor_get_last_visible_block (a, zoom, &viewport);
		Waveform* waveform = a->waveform;
		WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
		WfRectangle rect = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};

		g_return_val_if_fail(waveform, -1);
		g_return_val_if_fail(waveform->gl_blocks, -1);
		g_return_val_if_fail(viewport_px->right - viewport_px->left > 0.01, -1);

		//WfAudioData* audio = w->audio_data;

		//dbg(1, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

		double part_inset_px = wf_actor_samples2gl(zoom, region.start);
		double file_start_px = rect.left - part_inset_px;
		double block_wid = wf_actor_samples2gl(zoom, block_size);
		//dbg(1, "vp->right=%.2f", viewport_px->right);
		int region_start_block = region.start / block_size;
		float _end_block = ((float)(region.start + region.len)) / block_size;
		dbg(2, "region_start=%i region_end=%i start_block=%i end_block=%.2f(%.i) n_peak_frames=%i gl->size=%i", region.start, region.len, region_start_block, _end_block, (int)ceil(_end_block), waveform->gl_blocks->size * 256, waveform->gl_blocks->size);

		//we round _down_ as the block corresponds to the _start_ of a section.
		//int region_end_block = MIN(ceil(_end_block), waveform->gl_blocks->size - 1);
// FIXME
		//int region_end_block = MIN(_end_block, waveform->gl_blocks->size - 1);
int region_end_block = MIN(_end_block, waveform_get_n_audio_blocks(waveform) - 1);

//		if(region_end_block >= waveform->gl_blocks->size) gwarn("!!");

		//crop to viewport:
		int b; for(b=region_start_block;b<=region_end_block-1;b++){ //note we dont check the last block which can be partially outside the viewport
			float block_end_px = file_start_px + (b + 1) * block_wid;
			//dbg(1, " %i: block_px: %.1f --> %.1f", b, block_end_px - (int)block_wid, block_end_px);
			if(block_end_px > viewport_px->right) dbg(1, "end %i clipped by viewport at block %i. vp.right=%.2f block_end=%.1f", region_end_block, MAX(0, b/* - 1*/), viewport_px->right, block_end_px);
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
__wf_actor_get_viewport(WaveformActor* a, WfViewPort* viewport)
{
	//special version of get_viewport that ignores the animation so we get all the blocks.

	WaveformCanvas* canvas = a->canvas;

	if(canvas->viewport) *viewport = *canvas->viewport;
	else {
		viewport->left   = a->rect.left;
		viewport->top    = a->rect.top;
		viewport->right  = a->rect.left + a->rect.len;
		viewport->bottom = a->rect.top + a->rect.height;
	}
}


static void
_wf_actor_allocate_hi(WaveformActor* a)
{
	PF;
	Waveform* w = a->waveform;
	g_return_if_fail(!w->offline);
	WfRectangle* rect = &a->rect;
	WfViewPort viewport; __wf_actor_get_viewport(a, &viewport);
	double zoom = rect->len / a->region.len;

	WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
	int first_block = get_first_visible_block(&region, zoom, rect, &viewport, WF_PEAK_BLOCK_SIZE);
	int last_block = get_last_visible_block(a, zoom, &viewport, WF_PEAK_BLOCK_SIZE);

	int b;for(b=first_block;b<=last_block;b++){
#if 0
		gboolean need_data = false;
		if(w->audio_data && w->audio_data->buf16){
			g_return_if_fail(b < w->audio_data->n_blocks);
			//TODO use api
			WfBuf16* buf = w->audio_data->buf16[b];
			if(!buf || !buf->buf[0]) need_data = true;
			else dbg(2, "have data");
		} else need_data = true;

		if(need_data){
			dbg(1, "requesting data... b=%i", b);
			int n_tiers_needed = get_min_n_tiers();
			waveform_load_audio_async(a->waveform, b, n_tiers_needed);
		}
#else
		int n_tiers_needed = get_min_n_tiers();
		waveform_load_audio_async(a->waveform, b, n_tiers_needed);
#endif
	}
}


static void
_wf_actor_load_missing_blocks(WaveformActor* a)
{
	PF;
	WfRectangle* rect = &a->rect;
	double zoom = rect->len / a->region.len;

	if(zoom > 0.0125 && !a->waveform->offline){
		_wf_actor_allocate_hi(a);
	}else{
		WfViewPort viewport; __wf_actor_get_viewport(a, &viewport);

		int viewport_start_block = wf_actor_get_first_visible_block(&a->region, zoom, rect, &viewport);
		int viewport_end_block   = wf_actor_get_last_visible_block (a, zoom, &viewport);
		dbg(1, "block range: %i --> %i", viewport_start_block, viewport_end_block);

		int b; for(b=viewport_start_block;b<=viewport_end_block;b++){
			wf_actor_allocate_block(a, b);
		}
	}
}


void
wf_actor_allocate(WaveformActor* a, WfRectangle* rect)
{
	g_return_if_fail(a);
	g_return_if_fail(rect);
	WfActorPriv* _a = a->priv;
	PF;

	if(rect->len == a->rect.len && rect->left == a->rect.left) return;

	a->priv->animatable.start.start_val.i = a->region.start;
	a->priv->animatable.len.start_val.i = MAX(1, a->region.len);
	a->priv->animatable.rect_len.start_val.f = MAX(1, a->rect.len);

	a->rect = *rect;

#if 0
	double zoom = rect->len / a->region.len;

	int viewport_start_block = wf_actor_get_first_visible_block(&a->region, zoom, rect, &viewport);
	int viewport_end_block   = wf_actor_get_last_visible_block (a, zoom, &viewport);
#endif
	dbg(1, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

	_wf_actor_load_missing_blocks(a);

	WfAnimatable* animatable = &_a->animatable.rect_len;
#if 0
	animatable->val.i = a->canvas->draw
		? animatable->start_val.i
		: *animatable->model_val.i;
#else
	wf_actor_start_transition(a, animatable); //TODO this fn needs refactoring - doesnt do much now
#endif
	//				dbg(1, "%.2f --> %.2f", animatable->start_val.f, animatable->val.f);

	if(a->canvas->draw){ //we control painting so can animate.

		GList* l = _a->transitions;
		for(;l;l=l->next){
			wf_animation_remove_animatable((WfAnimation*)l->data, animatable);
		}

		WfAnimation* animation = wf_animation_add_new(wf_actor_on_animation_finished);
		a->priv->transitions = g_list_append(a->priv->transitions, animation);
		GList* animatables = g_list_append(NULL, &a->priv->animatable.rect_len);
		wf_transition_add_member(animation, a, animatables);

		wf_animation_start(animation);
	}
}


typedef struct
{
	float r;
	float g;
	float b;
} ColourFloat;

//TODO move (is dupe)
static void
colour_rgba_to_float(ColourFloat* colour, uint32_t rgba)
{
	//returned values are in the range 0.0 to 1.0;

	g_return_if_fail(colour);

	colour->r = (float)((rgba & 0xff000000) >> 24) / 0xff;
	colour->g = (float)((rgba & 0x00ff0000) >> 16) / 0xff;
	colour->b = (float)((rgba & 0x0000ff00) >>  8) / 0xff;
}


static void
_wf_actor_set_uniforms(float peaks_per_pixel, float top, float bottom, uint32_t _fg_colour, int n_channels)
{
#if 0
	dbg(1, "peaks_per_pixel=%.2f top=%.2f bottom=%.2f n_channels=%i", peaks_per_pixel, top, bottom, n_channels);
#endif

	GLuint offsetLoc = glGetUniformLocation(sh_main.program, "peaks_per_pixel");
	glUniform1f(offsetLoc, peaks_per_pixel);

	offsetLoc = glGetUniformLocation(sh_main.program, "bottom");
	glUniform1f(offsetLoc, bottom);

	offsetLoc = glGetUniformLocation(sh_main.program, "top");
	glUniform1f(offsetLoc, top);

	glUniform1i(glGetUniformLocation(sh_main.program, "n_channels"), n_channels);

	float fg_colour[4] = {0.0, 0.0, 0.0, ((float)(_fg_colour & 0xff)) / 0x100};
	rgba_to_float(_fg_colour, &fg_colour[0], &fg_colour[1], &fg_colour[2]);
	glUniform4fv(glGetUniformLocation(sh_main.program, "fg_colour"), 1, fg_colour);
}


#define gl_error (glGetError() != GL_NO_ERROR)

void
wf_actor_paint(WaveformActor* actor)
{
	//-must have called gdk_gl_drawable_gl_begin() first.
	//-it is assumed that all textures have been loaded.

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
//printf("----\n");
//dbg(1, "zoom=%.3f", zoom);

	int resolution = (zoom > 1.0/16)
		? 1
		: (zoom > 0.0125) ? 16 : 256;

	ColourFloat fg;
	colour_rgba_to_float(&fg, actor->fg_colour);
	ColourFloat bg;
	colour_rgba_to_float(&bg, actor->bg_colour);
	float alpha = ((float)(actor->fg_colour & 0xff)) / 256.0;

	if(w->num_peaks){
		int region_start_block   = region.start / WF_SAMPLES_PER_TEXTURE;
		int region_end_block     = (region.start + region.len) / WF_SAMPLES_PER_TEXTURE;
		int viewport_start_block = wf_actor_get_first_visible_block(&region, zoom, &rect, &viewport);
		int viewport_end_block   = wf_actor_get_last_visible_block (actor, zoom, &viewport);
		if(region_end_block > w->gl_blocks->size -1){ gwarn("region too long? region_end_block=%i n_blocks=%i region.len=%i", region_end_block, w->gl_blocks->size, region.len); region_end_block = w->gl_blocks->size -1; }
		dbg(2, "block range: region=%i-->%i viewport=%i-->%i", region_start_block, region_end_block, viewport_start_block, viewport_end_block);
		dbg(2, "rect=%.2f %.2f viewport=%.2f %.2f", rect.left, rect.len, viewport.left, viewport.right);

		int first_offset = region.start % WF_SAMPLES_PER_TEXTURE;
		double first_offset_px = wf_actor_samples2gl(zoom, first_offset);

		double _block_wid = wf_actor_samples2gl(zoom, WF_SAMPLES_PER_TEXTURE);
		if(wfc->use_shaders){
			wf_canvas_use_program(wfc, sh_main.program);

			//for 51200 frame sample 256pixels wide: n_peaks=51200/256=200, ppp=200/256=0.8
			float region_width_px = wf_canvas_gl_to_px(wfc, rect.len);
			dbg(2, "region_width_px=%.2f", region_width_px);
			float peaks_per_pixel = ceil(((float)region.len / WF_PEAK_TEXTURE_SIZE) / region_width_px);
			dbg(2, "vpwidth=%.2f region_len=%i region_n_peaks=%.2f peaks_per_pixel=%.2f", (viewport.right - viewport.left), region.len, ((float)region.len / WF_PEAK_TEXTURE_SIZE), peaks_per_pixel);
			float bottom = rect.top + rect.height;
			int n_channels = w->gl_blocks->peak_texture[WF_RIGHT].main ? 2 : 1;
			_wf_actor_set_uniforms(peaks_per_pixel, rect.top, bottom, actor->fg_colour, n_channels);
		}
#undef INTEGER_BLOCK_WIDTH //no, we cannot do this as it changes the zoom, destroying alignment with other items. Instead the user should ensure that the zoom tracks WF_SAMPLES_PER_TEXTURE
#ifdef INTEGER_BLOCK_WIDTH
		_block_wid = ceil(_block_wid);
#endif

		{
			//check textures are loaded
			int n = 0;
			int b; for(b=viewport_start_block;b<=viewport_end_block;b++){
				if(!w->gl_blocks->peak_texture[WF_LEFT].main[b]){ n++; if(wf_debug) gwarn("texture not loaded: b=%i", b); }
			}
			if(n) gwarn("%i textures not loaded", n);
		}

		glEnable(wfc->use_shaders ? GL_TEXTURE_1D : GL_TEXTURE_2D);

		//for hi-res mode:
		//block_region specifies the a sample range within the current block
		WfSampleRegion block_region = {region.start, WF_PEAK_BLOCK_SIZE};
		WfSampleRegion block_region_v_hi = {region.start, WF_PEAK_BLOCK_SIZE - region.start % WF_PEAK_BLOCK_SIZE};

		double x = rect.left + (viewport_start_block - region_start_block) * _block_wid - first_offset_px; // x is now the start of the first block (can be before part start when inset is present)
		g_return_if_fail(WF_PEAK_BLOCK_SIZE == (WF_PEAK_RATIO * WF_PEAK_TEXTURE_SIZE)); // temp check. we use a simplified loop which requires the two block sizes are the same
		gboolean is_first = true;
		int b; for(b=viewport_start_block;b<=viewport_end_block;b++){
			//dbg(1, "b=%i", b);
			gboolean is_last = (b == viewport_end_block) || (b == w->gl_blocks->size - 1); //2nd test is unneccesary?

			//try hi res painting first, fallback to lower resolutions.

			gboolean block_done = false;

			switch(resolution){
				// v high res
				case 1 ... 12:
					//dbg(1, "----- super hi mode !!");
					;WfAudioData* audio = w->priv->audio_data;
					if(audio->n_blocks){
						WfBuf16* buf = audio->buf16[b];
						if(buf){
#if 0
							{
								//find the sample corresponding to viewport left.
								double part_inset_px = wf_actor_samples2gl(zoom, region.start);
								double file_start_px = rect.left - part_inset_px;
								//double block_wid = wf_actor_samples2gl(zoom, WF_SAMPLES_PER_TEXTURE);
								double block_start_px = file_start_px + _block_wid * b;
								double block_end_px = block_start_px + _block_wid;
								uint64_t block_region_end = block_region.start + block_region.len;

								//float s2p = (viewport.right - viewport.left) / block_region.len; //wrong
								//dbg(1, "zoom=%f s2p=%.4f", zoom, s2p);
								if(block_start_px <= viewport.left && block_end_px >= viewport.right){
									//dbg(1, "viewport contains single block");
								}
												WfRectangle block_rect = {0.0, rect.top, _block_wid, rect.height/w->n_channels};

int64_t left_samples = block_rect.left / zoom;
dbg(1, "left=%f samples=%f %Li", block_rect.left, block_rect.left / zoom, left_samples);
								dbg(1, "* block_px=%.2f-->%.2f block_region=%Lu-->%Lu viewport_region=%.2f-->%.2f(samples)",
									block_start_px, block_end_px,
									block_region.start, block_region_end,
									(float)block_region.start - ((float)block_rect.left / zoom), ((float)block_region_end) * viewport.right / block_end_px);
							}
#endif
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

				//hi res
				case 13 ... 255:
					;Peakbuf* peakbuf = wf_get_peakbuf_n(w, b);
					if(peakbuf){
						//dbg(1, "  b=%i x=%.2f", b, x);

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
								WfRectangle block_rect = {x, rect.top + c * rect.height/2, _block_wid, rect.height/w->n_channels};
								draw_wave_buffer_hi(w, block_region, &block_rect, peakbuf, c, wfc->v_gain, actor->fg_colour);
							}
						}
						block_done = true; //hi res was succussful. no more painting needed for this block.
					}
					if(block_done) break;

				// low res
				case 256 ... 1024:
					if(wfc->use_shaders){
						int c = 0;
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_1D, w->gl_blocks->peak_texture[c].main[b]);

						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_1D, w->gl_blocks->peak_texture[c].neg[b]);
						glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

						if(w->gl_blocks->peak_texture[WF_RIGHT].main){
							glActiveTexture(GL_TEXTURE2);
							glBindTexture(GL_TEXTURE_1D, w->gl_blocks->peak_texture[WF_RIGHT].main[b]);
							glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

							glActiveTexture(GL_TEXTURE3);
							glBindTexture(GL_TEXTURE_1D, w->gl_blocks->peak_texture[WF_RIGHT].neg[b]);
							glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						}

						glActiveTexture(GL_TEXTURE0);
					}else
						use_texture(w->gl_blocks->peak_texture[0].main[b]);
					if(gl_error) gwarn("cannot bind texture: block=%i", b);
					glColor4f(fg.r, fg.g, fg.b, alpha); //seems we have to set colour _after_ binding... ?

					double block_wid = _block_wid;
					double tex_pct = 1.0; //use the whole texture
					double tex_start = 0.0;
					if (is_first){
						if(first_offset) tex_pct = 1.0 - ((double)first_offset) / WF_SAMPLES_PER_TEXTURE;
						block_wid = _block_wid * tex_pct;
						tex_start = 1 - tex_pct;
						dbg(2, "rect.left=%.2f region->start=%i first_offset=%i", rect.left, region.start, first_offset);
					}
					if (is_last){
						if(true){
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
							if(b * _block_wid > distance_from_file_start_to_region_end){ gwarn("!!"); continue;}
#endif
						}
					}

					if(b == w->gl_blocks->size - 1) dbg(1, "last sample block. fraction=%.2f", w->gl_blocks->last_fraction);
					//TODO check what happens here if we allow non-square textures
#if 0 // !! doesnt matter if its the last block or not, we must still take the region into account.
					tex_pct = (i == w->gl_blocks->size - 1)
						? (block_wid / _block_wid) / w->gl_blocks->last_fraction    // block is at end of sample
														  // -cannot use block_wid / _block_wid for last block because the last texture is smaller.
						: block_wid / _block_wid;
#else
					tex_pct = block_wid / _block_wid;
#endif
				}

				dbg (2, "%i: is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex_start=%.2f", b, is_last, x, block_wid, _block_wid, tex_pct, tex_start);
				if(tex_pct > 1.0 || tex_pct < 0.0) gwarn("tex_pct! %.2f", tex_pct);
				double tex_x = x + ((is_first && first_offset) ? first_offset_px : 0);
				glPushMatrix();
//				glTranslatef(0, 0, part->track->track_num * TRACK_SPACING_Z);
				glBegin(GL_QUADS);
				if(wfc->use_shaders){
					glMultiTexCoord2f(GL_TEXTURE0, tex_start + 0.0,     0.0); glMultiTexCoord2f(GL_TEXTURE1, tex_start + 0.0,     0.0); glMultiTexCoord2f(GL_TEXTURE2, tex_start + 0.0,     0.0); glMultiTexCoord2f(GL_TEXTURE3, tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       rect.top);
					glMultiTexCoord2f(GL_TEXTURE0, tex_start + tex_pct, 0.0); glMultiTexCoord2f(GL_TEXTURE1, tex_start + tex_pct, 0.0); glMultiTexCoord2f(GL_TEXTURE2, tex_start + tex_pct, 0.0); glMultiTexCoord2f(GL_TEXTURE3, tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, rect.top);
					glMultiTexCoord2f(GL_TEXTURE0, tex_start + tex_pct, 1.0); glMultiTexCoord2f(GL_TEXTURE1, tex_start + tex_pct, 1.0); glMultiTexCoord2f(GL_TEXTURE2, tex_start + tex_pct, 1.0); glMultiTexCoord2f(GL_TEXTURE3, tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, rect.top + rect.height);
					glMultiTexCoord2f(GL_TEXTURE0, tex_start + 0.0,     1.0); glMultiTexCoord2f(GL_TEXTURE1, tex_start + 0.0,     1.0); glMultiTexCoord2f(GL_TEXTURE2, tex_start + 0.0,     1.0); glMultiTexCoord2f(GL_TEXTURE3, tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       rect.top + rect.height);
				}else{
					glTexCoord2d(tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       rect.top);
					glTexCoord2d(tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, rect.top);
					glTexCoord2d(tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, rect.top + rect.height);
					glTexCoord2d(tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       rect.top + rect.height);
				}
				glEnd();
				glPopMatrix();
				if(gl_error) gwarn("gl error! block=%i", b);

#ifdef WF_SHOW_RMS
				double bot = rect.top + rect.height;
				double top = rect->top;
				if(wfc->show_rms && w->gl_blocks->rms_texture){
					glBindTexture(GL_TEXTURE_2D, w->gl_blocks->rms_texture[b]);
#if 0
					if(!glIsTexture(w->gl_blocks->rms_texture[i])) gwarn ("texture not loaded. block=%i", i);
#endif
					//note seems we have to do this after binding...
					glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
					glColor4f(bg.r, bg.g, bg.b, 0.5);

					dbg (2, "rms: %i: is_last=%i x=%.2f wid=%.2f tex_pct=%.2f", i, is_last, x, block_wid, tex_pct);
					glPushMatrix();
	//TODO use wfc->rotation
	//				glTranslatef(0, 0, part->track->track_num * TRACK_SPACING_Z);
					glBegin(GL_QUADS);
					glTexCoord2d(tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       top);
					glTexCoord2d(tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, top);
					glTexCoord2d(tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, bot);
					glTexCoord2d(tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       bot);
					glEnd();
					glPopMatrix();
				}
#endif
					break;
				default:
					break;
			} // end switch
			//--------------------------------------

#undef DEBUG_BLOCKS
//#define DEBUG_BLOCKS
#ifdef DEBUG_BLOCKS
			double bot = rect.top + rect.height;
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
			glEnable(wfc->use_shaders ? GL_TEXTURE_1D : GL_TEXTURE_2D);
			wf_canvas_use_program(wfc, sh_main.program); //TODO move
#endif
			x += _block_wid;
//uint64_t o = block_region.start;
			block_region.start = (block_region.start / WF_PEAK_BLOCK_SIZE + 1) * WF_PEAK_BLOCK_SIZE;
			block_region_v_hi.start = (block_region_v_hi.start / WF_PEAK_BLOCK_SIZE + 1) * WF_PEAK_BLOCK_SIZE;
			block_region_v_hi.len   = WF_PEAK_BLOCK_SIZE - block_region_v_hi.start % WF_PEAK_BLOCK_SIZE;
//dbg(1, "block_region_start: %Lu --> %Lu", o, block_region.start);
			is_first = false;
		}
	}
	wf_canvas_use_program(wfc, 0);

	if(gl_error) gwarn("gl error!");
}


#ifdef DEPRECATED
void
wf_actor_paint_hi(WaveformActor* actor)
{
	WaveformCanvas* wfc = actor->canvas;
	g_return_if_fail(wfc);
	g_return_if_fail(actor);
	Waveform* w = actor->waveform; 

	if(waveform_get_n_frames(w)){
		WfRectangle rect = {actor->rect.left, actor->rect.top, actor->priv->animatable.rect_len.val.f, actor->rect.height};
		WfSampleRegion region = {actor->priv->animatable.start.val.i, actor->priv->animatable.len.val.i};
		double zoom = rect.len / region.len;

		int n_channels = w->gl_blocks->peak_texture[WF_RIGHT].main ? 2 : 1;
		if(!w->cache) w->cache = wf_wav_cache_new(n_channels);

		WfViewPort viewport; wf_actor_get_viewport(actor, &viewport);
		float n_points = (viewport.right - viewport.left) / zoom;
		int offx = rect.left;
		int mode = 4; //doesnt appear to make much difference. mode 0 does not draw any lines, only points

		int width = viewport.right - viewport.left;
		//int height = viewport.bottom - viewport.top;
		//dbg(1, "height=%i", height);

		glPushMatrix();
		draw_waveform(actor, (WfSampleRegion){region.start, n_points}, width, rect.height, offx, actor->rect.top, mode, actor->fg_colour);
		glPopMatrix();
	}
}
#endif


#if 0
static void
wf_actor_paint_hi2(WaveformActor* a)
{
	//has been merged with wf_actor_paint_hi. fallback to lower res for each block if neccesary

	WaveformCanvas* wfc = a->canvas;
	g_return_if_fail(wfc);
	g_return_if_fail(a);
	Waveform* w = a->waveform; 
	if(!w->audio_data) return;
	float v_gain = 1.0; //TODO

	if(waveform_get_n_frames(w)){
		wf_canvas_use_program(a->canvas, 0);
		glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_1D);

		WfViewPort viewport; wf_actor_get_viewport(a, &viewport);
		WfRectangle rect = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};
		WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
		//int width = viewport.right - viewport.left;
		double zoom = rect.len / region.len;

		int viewport_start_block = get_first_visible_block(&region, zoom, &rect, &viewport, WF_PEAK_BLOCK_SIZE);
		int viewport_end_block = get_last_visible_block(a, zoom, &viewport, WF_PEAK_BLOCK_SIZE);
		g_return_if_fail(viewport_end_block < w->audio_data->n_blocks);
		dbg(1, "block range: %i --> %i", viewport_start_block, viewport_end_block);

		double _block_wid = wf_actor_samples2gl(zoom, WF_PEAK_BLOCK_SIZE);
		//dbg(1, "block_wid=%.2f", _block_wid);

		int first_offset = region.start % WF_PEAK_BLOCK_SIZE;
		double first_offset_px = wf_actor_samples2gl(zoom, first_offset);

		//double px = wf_actor_samples2gl(zoom, region->start);
		int region_start_block = region.start / WF_PEAK_BLOCK_SIZE;
		double x = rect.left + (viewport_start_block - region_start_block) * _block_wid - first_offset_px; // x is now the start of the first block (can be before part start when inset is present)
		//dbg(1, "region.start=%i region_start_block=%i x=%.1f", region.start, region_start_block, x);
		dbg(1, "left=%.1f", rect.left);

		WfSampleRegion block_region = {region.start, WF_PEAK_BLOCK_SIZE};
		int b; for(b=viewport_start_block;b<=viewport_end_block;b++){ // iterating over audio blocks, not texture blocks.
#if 0
			if(w->audio_data->buf){
//if(b == 2){
				WfBuf* buf = w->audio_data->buf[b];
				if(buf){
					dbg(1, "  b=%i x=%.2f", b, x);
					int offy = 0;
					int c; for(c=0;c<w->n_channels;c++){
						if(buf->buf[c]){
							draw_wave_buffer2(w, block_region, v_gain, _block_wid, rect.height/w->n_channels, x, offy + c * rect.height/2, buf->buf[c], a->fg_colour);
						}
					}
				}
//}
			}
#else
			Peakbuf* peakbuf = wf_get_peakbuf_n(w, b);
			if(peakbuf){
				//dbg(1, "  b=%i x=%.2f", b, x);
				int c; for(c=0;c<w->n_channels;c++){
					if(peakbuf->buf[c]){
						//dbg(1, "peakbuf: %i:%i: %p %p %i", b, c, peakbuf, peakbuf->buf[0], ((short*)peakbuf->buf[c])[0]);
						WfRectangle block_rect = {x, rect.top + c * rect.height/2, _block_wid, rect.height/w->n_channels};
						draw_wave_buffer3(w, block_region, &block_rect, peakbuf, c, v_gain, a->fg_colour);
					}
				}
			}
#endif
			x += _block_wid;
			block_region.start += WF_PEAK_BLOCK_SIZE;
		}
	}
}
#endif


/*
 *  Load all textures for the given block.
 *  There will be between 1 and 4 textures depending on shader/alphabuf mono/stero.
 */
void
wf_actor_load_texture1d(Waveform* w, int blocknum)
{
	WfGlBlocks* blocks = w->gl_blocks;

	dbg(2, "%i: %i %i %i %i", blocknum,
		blocks->peak_texture[0].main[blocknum],
		blocks->peak_texture[0].neg[blocknum],
		blocks->peak_texture[1].main ? blocks->peak_texture[WF_RIGHT].main[blocknum] : 0,
		blocks->peak_texture[1].neg ? blocks->peak_texture[WF_RIGHT].neg[blocknum] : 0
	);

	struct _buf {
		guchar positive[WF_PEAK_TEXTURE_SIZE];
		guchar negative[WF_PEAK_TEXTURE_SIZE];
	} buf;

	void make_texture_data(Waveform* w, int ch, struct _buf* buf)
	{
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
				dbg(1, "end of peak: %i b=%i n_sec=%.3f", peak->size, blocknum, ((float)((WF_PEAK_TEXTURE_SIZE * blocknum + f) * WF_PEAK_RATIO))/44100); break; }

			buf->positive[f] =  peak->buf[ch][i  ] >> 8;
			buf->negative[f] = -peak->buf[ch][i+1] >> 8;
		}
	}
	make_texture_data(w, WF_LEFT, &buf);

	struct _d {
		int          tex_unit;
		int          tex_id;
		guchar*      buf;
	} d[2] = {
		{GL_TEXTURE0, blocks->peak_texture[WF_LEFT].main[blocknum], buf.positive},
		{GL_TEXTURE1, blocks->peak_texture[WF_LEFT].neg [blocknum], buf.negative},
	};

	void _load_texture(struct _d* d)
	{
		glActiveTexture(d->tex_unit);
glEnable(GL_TEXTURE_1D);
		glBindTexture(GL_TEXTURE_1D, d->tex_id);
		dbg (2, "loading texture1D... texture_id=%u", d->tex_id);
		if(glGetError() != GL_NO_ERROR) gwarn("gl error: bind failed: unit=%i buf=%p tid=%i", d->tex_unit, d->buf, d->tex_id);
if(!glIsTexture(d->tex_id)) gwarn ("not is texture: texture_id=%u", d->tex_id);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//using MAG LINEAR smooths nicely but can reduce the peak.
//		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_ALPHA8, WF_PEAK_TEXTURE_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, d->buf);

		if(glGetError() != GL_NO_ERROR) gwarn("gl error: unit=%i buf=%p tid=%i", d->tex_unit, d->buf, d->tex_id);
	}

	int v; for(v=0;v<2;v++){
		_load_texture(&d[v]);
	}
	if(glGetError() != GL_NO_ERROR) gwarn("lhs. gl error!");

	if(blocks->peak_texture[WF_RIGHT].main && blocks->peak_texture[WF_RIGHT].main[blocknum]){                //stereo
		make_texture_data(w, WF_RIGHT, &buf);

		struct _d stuff[WF_MAX_CH] = {
			{GL_TEXTURE2, blocks->peak_texture[WF_RIGHT].main[blocknum], buf.positive},
			{GL_TEXTURE3, blocks->peak_texture[WF_RIGHT].neg [blocknum], buf.negative},
		};
		int u; for(u=0;u<2;u++){
			_load_texture(&stuff[u]);
		}
		if(glGetError() != GL_NO_ERROR) gwarn("rhs. gl error!");
	}

	glActiveTexture(GL_TEXTURE0);

	if(glGetError() != GL_NO_ERROR) gwarn("gl error!");
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
rgba_to_float(uint32_t rgba, float* r, float* g, float* b)
{
	double _r = (rgba & 0xff000000) >> 24;
	double _g = (rgba & 0x00ff0000) >> 16;
	double _b = (rgba & 0x0000ff00) >>  8;

	*r = _r / 0xff;
	*g = _g / 0xff;
	*b = _b / 0xff;
	dbg (2, "%08x --> %.2f %.2f %.2f", rgba, *r, *g, *b);
}


