/*
  copyright (C) 2012-2013 Tim Orford <tim@orford.org>

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

  The rendering process depends on magnification and hardwave capabilities,
  but for at STD resolution with shaders available, and both USE_FBO and
  'multipass' are defined, the method for each block is as follows:

  1- when the waveform is loaded, the peakdata is copied to 1d textures.
  2- on a horizontal zoom change, an fbo is created that maps the 1d peakdata
     to a 2d map using the 'peak_shader'. This texture is independent of
     colour and vertical-zoom.
  3- on each expose, a single rectangle is drawn. The Vertical shader is used
     to apply vertical convolution to the 2d texture.

  animated transitions:

  WaveformActor has internal support for animating the following properties:
  region start and end, and start and end position onscreen
  (zoom is derived from these)

  Where an external animation framework is available, it should be used
  in preference, eg Clutter (TODO).

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
#include "agl/ext.h"
#include "transition/animator.h"
#include "waveform/typedefs.h"
#include "waveform/utils.h"
#include "waveform/gl_utils.h"
#include "waveform/peak.h"
#include "waveform/audio.h"
#include "waveform/texture_cache.h"
#include "waveform/hi_res.h"
#include "waveform/alphabuf.h"
#include "waveform/fbo.h"
#include "waveform/actor.h"
#ifdef USE_FBO
#define multipass
#endif
																												extern int n_loads[4096];

#define HIRES_NONSHADER_TEXTURES // work in progress
                                 // because of the issue with missing peaks with reduced size textures without shaders, this option is possibly unwanted.
#undef HIRES_NONSHADER_TEXTURES

/*
	TODO Mipmapping

	For both the shader and non-shader case, it would be useful to have
	mipmapping to have proper visibility of short term peaks and/or to reduce
	the number of (oversampled) texture lookups. How to fix the problem
	that we need to preserve the height of the texture to not lose vertical detail?

 */
#define USE_MIPMAPPING
#undef USE_MIPMAPPING

struct _actor_priv
{
	float           opacity;     // derived from background colour

	struct {
		WfAnimatable  start;     // (int) region
		WfAnimatable  len;       // (int) region
		WfAnimatable  rect_left; // (float)
		WfAnimatable  rect_len;  // (float)
		WfAnimatable  z;         // (float)
		WfAnimatable  opacity;   // (float)
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

//#warning texture size 4096 will be too high for Intel
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
typedef struct { int lower; int upper; } ModeRange;

static void   wf_actor_get_viewport          (WaveformActor*, WfViewPort*);
static void   wf_actor_init_transition       (WaveformActor*, WfAnimatable*);
static void   wf_actor_start_transition      (WaveformActor*, GList* /* WfAnimatable* */, AnimationFn, gpointer);
static void   wf_actor_on_animation_finished (WfAnimation*, gpointer);
static void   wf_actor_allocate_block_hi     (WaveformActor*, int b);
static void   wf_actor_allocate_block_med    (WaveformActor*, int b);
static void   wf_actor_allocate_block_low    (WaveformActor*, int b);
static void  _wf_actor_load_missing_blocks   (WaveformActor*);
static void   wf_actor_load_texture1d        (Waveform*, Mode, WfGlBlock*, int b);
static void   wf_actor_load_texture2d        (WaveformActor*, Mode, int t, int b);
static void  _wf_actor_load_texture_hi       (WaveformActor*, int b);
static void  _wf_actor_print_hires_textures  (WaveformActor*);
static inline int   get_resolution           (double zoom);
static inline float get_peaks_per_pixel_i    (WaveformCanvas*, WfSampleRegion*, WfRectangle*, int mode);
static inline float get_peaks_per_pixel      (WaveformCanvas*, WfSampleRegion*, WfRectangle*, int mode);
static inline void _draw_block               (float tex_start, float tex_pct, float x, float y, float width, float height, float gain);
static inline void _draw_block_from_1d       (float tex_start, float tex_pct, float x, float y, float width, float height, int tsize);
#if defined (USE_FBO) && defined (multipass)
static void   block_to_fbo                   (WaveformActor*, int b, WfGlBlock*, int resolution);
static void   wf_actor_canvas_finalize_notify         (gpointer, GObject*);
#endif
static inline int  get_mode                  (double zoom);
static ModeRange   mode_range                (WaveformActor*);

#ifdef USE_FBO
static AglFBO* fbo_test = NULL;
#endif

void
wf_actor_init()
{
	get_gl_extensions();

	modes[MODE_HI].make_texture_data = make_texture_data_hi;
}


WaveformActor*
wf_actor_new(Waveform* w, WaveformCanvas* wfc)
{
	dbg(2, "%s-------------------------%s", "\x1b[1;33m", "\x1b[0;39m");

	g_return_val_if_fail(wfc, NULL);

	uint32_t get_frame(int time)
	{
		return 0;
	}

	WaveformActor* a = g_new0(WaveformActor, 1);
	a->canvas = wfc;
	a->priv = g_new0(WfActorPriv, 1);
	WfActorPriv* _a = a->priv;

	a->vzoom = 1.0;
	wf_actor_set_colour(a, 0xffffffff, 0x000000ff);
	a->waveform = g_object_ref(w);

	WfAnimatable* animatable = &_a->animatable.start;
	animatable->model_val.i = (uint32_t*)&a->region.start; //TODO add uint64_t to union
	animatable->start_val.i = a->region.start;
	animatable->val.i       = a->region.start;
	animatable->type        = WF_INT;

	animatable = &a->priv->animatable.len;
	animatable->model_val.i = &a->region.len;
	animatable->start_val.i = a->region.len;
	animatable->val.i       = a->region.len;
	animatable->type        = WF_INT;

	animatable = &a->priv->animatable.rect_left;
	animatable->model_val.f = &a->rect.left;
	animatable->start_val.f = a->rect.left;
	animatable->val.f       = a->rect.left;
	animatable->type        = WF_FLOAT;

	animatable = &a->priv->animatable.rect_len;
	animatable->model_val.f = &a->rect.len;
	animatable->start_val.f = a->rect.len;
	animatable->val.f       = a->rect.len;
	animatable->type        = WF_FLOAT;

	animatable = &a->priv->animatable.z;
	animatable->model_val.f = &a->z;
	animatable->start_val.f = a->z;
	animatable->val.f       = a->z;
	animatable->type        = WF_FLOAT;

	animatable = &a->priv->animatable.opacity;
	animatable->model_val.f = &_a->opacity;
	animatable->start_val.f = _a->opacity;
	animatable->val.f       = _a->opacity;
	animatable->type        = WF_FLOAT;

#ifdef WF_DEBUG
	g_strlcpy(_a->animatable.start.name,     "start",     16);
	g_strlcpy(_a->animatable.len.name,       "len",       16);
	g_strlcpy(_a->animatable.rect_left.name, "rect_left", 16);
	g_strlcpy(_a->animatable.rect_len.name,  "rect_len",  16);
#endif

	void _wf_actor_on_peakdata_available(Waveform* waveform, int block, gpointer _actor)
	{
		// because there can be many actors showing the same waveform
		// this can be called multiple times, but the texture must only
		// be updated once.
		// if the waveform has changed, the existing data must be cleared first.

		WaveformActor* a = _actor;
		dbg(1, "block=%i", block);

		ModeRange mode = mode_range(a);
		if(mode.lower == MODE_HI || mode.upper == MODE_HI)
			_wf_actor_load_texture_hi(a, block);

		else if(mode.lower == MODE_V_HI || mode.upper == MODE_V_HI)
			if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
	}
	_a->peakdata_ready_handler = g_signal_connect (w, "peakdata-ready", (GCallback)_wf_actor_on_peakdata_available, a);

	g_object_weak_ref((GObject*)a->canvas, wf_actor_canvas_finalize_notify, a);
	return a;
}


void
wf_actor_free(WaveformActor* a)
{
	PF;
	g_return_if_fail(a);

	if(a->waveform){
		g_signal_handler_disconnect((gpointer)a->waveform, a->priv->peakdata_ready_handler);
		a->priv->peakdata_ready_handler = 0;

		g_object_weak_unref((GObject*)a->canvas, wf_actor_canvas_finalize_notify, a);

		waveform_unref0(a->waveform);
	}
	g_free(a->priv);
	g_free(a);
}


static void wf_actor_canvas_finalize_notify(gpointer _actor, GObject* was)
{
	//should not get here. the weak_ref is removed in wf_actor_free.

	WaveformActor* a = (WaveformActor*)_actor;
	gwarn("actor should have been freed before canvas finalize. actor=%p", a);
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

	WfAnimatable* a1 = &a->priv->animatable.start;
	WfAnimatable* a2 = &a->priv->animatable.len;

	// set the transition start point to the _old_ model value: NO, this is supposed to be managed by the animator
															//dbg(0, "%i start=%i model=%i", a1->val.i, a1->start_val.i, *a1->model_val.i);
	//a1->start_val.i = *a1->model_val.i;
	//a2->start_val.i = MAX(1, *a2->model_val.i);

	a->region = *region;

	if(!start && !end) return;

	if(a->rect.len > 0.00001) _wf_actor_load_missing_blocks(a);

	if(!a->canvas->draw || !a->canvas->enable_animations){
		a1->val.i = MAX(1, a->region.start);
		a2->val.i = MAX(1, a->region.len);

		if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
		return; //no animations
	}

	GList* animatables = start ? g_list_append(NULL, a1) : NULL;
	       animatables = end ? g_list_append(animatables, a2) : animatables;

	wf_actor_start_transition(a, animatables, NULL, NULL);
}


void
wf_actor_set_colour(WaveformActor* a, uint32_t fg_colour, uint32_t bg_colour)
{
	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	dbg(2, "0x%08x", fg_colour);
	a->fg_colour = fg_colour;
	a->bg_colour = bg_colour;

	_a->opacity = ((float)(fg_colour & 0xff)) / 0x100;
	_a->animatable.opacity.val.f = _a->opacity;
}


static double wf_actor_samples2gl(double zoom, uint32_t n_samples)
{
	//zoom is pixels per sample
	return n_samples * zoom;
}


static int
wf_actor_get_first_visible_block(WfSampleRegion* region, double zoom, WfRectangle* rect, WfViewPort* viewport_px)
{
	// return the block number of the first block of the actor that is within the given viewport.

	int resolution = get_resolution(zoom);
	int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (resolution == 1024 ? WF_PEAK_STD_TO_LO : 1);

	double region_inset_px = wf_actor_samples2gl(zoom, region->start);
	double file_start_px = rect->left - region_inset_px;
	double block_wid = wf_actor_samples2gl(zoom, samples_per_texture);

	int region_start_block = region->start / samples_per_texture;
	int region_end_block = (region->start + region->len) / samples_per_texture;
	int b; for(b=region_start_block;b<=region_end_block;b++){
		int block_start_px = file_start_px + b * block_wid;
		double block_end_px = block_start_px + block_wid;
		dbg(3, "block_pos_px=%i", block_start_px);
		if(block_end_px >= viewport_px->left) return b;
	}

	dbg(1, "region outside viewport? vp_left=%.2f region_end=%.2f", viewport_px->left, file_start_px + region_inset_px + wf_actor_samples2gl(zoom, region->len));
	//														dbg(0, "region outside viewport? vp_left=%.2f region_end=%.2f", viewport_px->left, file_start_px + region_inset_px + wf_actor_samples2gl(zoom, region->len));
	return 10000;
}
	//duplicates wf_actor_get_first_visible_block
	/*
	static int
	get_first_visible_block(WfSampleRegion* region, double zoom, WfRectangle* rect, WfViewPort* viewport_px)
	{
		int block_size = WF_PEAK_BLOCK_SIZE;

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
	*/



static int
wf_actor_get_last_visible_block(WfSampleRegion* region, WfRectangle* rect, double zoom, WfViewPort* viewport_px, WfGlBlock* textures)
{
	//the region, rect and viewport are passed explictly because different users require slightly different values during transitions.

	int resolution = get_resolution(zoom);
	int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (resolution == 1024 ? WF_PEAK_STD_TO_LO : 1);

	g_return_val_if_fail(textures, -1);
	g_return_val_if_fail(viewport_px->right - viewport_px->left > 0.01, -1);

	//dbg(1, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

	double region_inset_px = wf_actor_samples2gl(zoom, region->start);
	double file_start_px = rect->left - region_inset_px;
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

	//duplicates wf_actor_get_last_visible_block above
#if 0
	static int
	get_last_visible_block(WaveformActor* a, double zoom, WfViewPort* viewport_px)
	{
		//TODO this is the same as for the texture blocks - refactor with block_size argument. need to remove MIN

		int block_size = WF_PEAK_BLOCK_SIZE;

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
	}
#endif



#if defined (USE_FBO) && defined (multipass)
static void
block_to_fbo(WaveformActor* a, int b, WfGlBlock* blocks, int resolution)
{
	// create a new fbo for the given block and render to it using the raw 1d peak data.

	g_return_if_fail(!blocks->fbo[b]);
	blocks->fbo[b] = agl_fbo_new(256, 256, 0);
	{
		WaveformCanvas* wfc = a->canvas;
		WaveformActor* actor = a;
		WfGlBlock* textures = blocks;

		if(a->canvas->use_1d_textures){
			AglFBO* fbo = blocks->fbo[b];
			if(fbo){
				agl_draw_to_fbo(fbo) {
					WfColourFloat fg; wf_colour_rgba_to_float(&fg, actor->fg_colour);
					glClearColor(fg.r, fg.g, fg.b, 0.0); //background colour must be same as foreground for correct antialiasing
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

					{
						//set shader program
						PeakShader* peak_shader = wfc->priv->shaders.peak_nonscaling;
						peak_shader->uniform.n_channels = textures->peak_texture[WF_RIGHT].main ? 2 : 1;
						agl_use_program(&peak_shader->shader);
					}

					glEnable(GL_TEXTURE_1D);
					int c = 0;
					texture_unit_use_texture(wfc->texture_unit[0], textures->peak_texture[c].main[b]);
					texture_unit_use_texture(wfc->texture_unit[1], textures->peak_texture[c].neg[b]);
					if(a->waveform->priv->peak.buf[WF_RIGHT]){
						c = 1;
						texture_unit_use_texture(a->canvas->texture_unit[2], textures->peak_texture[c].main[b]);
						texture_unit_use_texture(a->canvas->texture_unit[3], textures->peak_texture[c].neg[b]);
					}

					//must introduce the overlap as early as possible in the pipeline. It is introduced during the copy from peakbuf to 1d texture.
					//-here only a 1:1 copy is needed.

					double top = 0;
					double bot = fbo->height;
					double x1 = 0;// + TEX_BORDER;
					double x2 = fbo->width;// - TEX_BORDER;

					double src_start = 0;//TEX_BORDER / 256.0;
					double tex_pct = 1.0;//(256.0 - 2.0 * TEX_BORDER) / 256.0;

					glBegin(GL_QUADS);
					glMultiTexCoord2f(WF_TEXTURE0, src_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE1, src_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE2, src_start + 0.0,     0.0); glMultiTexCoord2f(WF_TEXTURE3, src_start + 0.0,     0.0); glVertex2d(x1, top);
					glMultiTexCoord2f(WF_TEXTURE0, src_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE1, src_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE2, src_start + tex_pct, 0.0); glMultiTexCoord2f(WF_TEXTURE3, src_start + tex_pct, 0.0); glVertex2d(x2, top);
					glMultiTexCoord2f(WF_TEXTURE0, src_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE1, src_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE2, src_start + tex_pct, 1.0); glMultiTexCoord2f(WF_TEXTURE3, src_start + tex_pct, 1.0); glVertex2d(x2, bot);
					glMultiTexCoord2f(WF_TEXTURE0, src_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE1, src_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE2, src_start + 0.0,     1.0); glMultiTexCoord2f(WF_TEXTURE3, src_start + 0.0,     1.0); glVertex2d(x1, bot);
					glEnd();
				} agl_end_draw_to_fbo;
#if USE_FX
				//now process the first fbo onto the fx_fbo

				if(blocks->fx_fbo[b]) gwarn("expected empty");
				AglFBO* fx_fbo = blocks->fx_fbo[b] = agl_fbo_new(256, 256, 0);
				if(fx_fbo){
					dbg(1, "%i: rendering to fx fbo. from: id=%i texture=%u - to: texture=%u", b, fbo->id, fbo->texture, fx_fbo->texture);
					agl_draw_to_fbo(fx_fbo) {
						glClearColor(1.0, 1.0, 1.0, 0.0); //background colour must be same as foreground for correct antialiasing
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

						agl_use_texture(fbo->texture);

						BloomShader* shader = wfc->priv->shaders.vertical;
						shader->uniform.fg_colour = 0xffffffff;
						shader->uniform.peaks_per_pixel = 256; // peaks_per_pixel not used by this shader
						agl_use_program(&shader->shader);

						double top = 0;
						double bot = fbo->height;
						double x1 = 0;
						double x2 = fbo->width;
						double tex_pct = 1.0; //TODO check
						glBegin(GL_QUADS);
						glTexCoord2d(0.0,     0.0); glVertex2d(x1, top);
						glTexCoord2d(tex_pct, 0.0); glVertex2d(x2, top);
						glTexCoord2d(tex_pct, 1.0); glVertex2d(x2, bot);
						glTexCoord2d(0.0,     1.0); glVertex2d(x1, bot);
						glEnd();
					} agl_end_draw_to_fbo;
				}
#endif
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

	// TODO is the texture_cache suitable for hires textures?
	// it uses an array, so all items must be of the same type, but hires textures can still use Texture object in cache?
	// (they would be both in the per-actor list (as WfTextureHi) and the global cache as Texture)
	int n_ch = waveform_get_n_channels(a->waveform);
	if(a->canvas->use_1d_textures){
		for(c=0;c<n_ch;c++){
			texture->t[c].main = texture_cache_assign_new(wf->texture_cache, (WaveformBlock){a->waveform, b});
			texture->t[c].neg  = texture_cache_assign_new(wf->texture_cache, (WaveformBlock){a->waveform, b});
		}

		wf_actor_load_texture1d(a->waveform, MODE_HI, (WfGlBlock*)NULL, b);
	}else{
#ifdef HIRES_NONSHADER_TEXTURES
		texture->t[WF_LEFT].main = texture_cache_assign_new(wf->texture_cache, (WaveformBlock){a->waveform, b | WF_TEXTURE_CACHE_HIRES_MASK});
		dbg(0, "assigned texture=%u", texture->t[WF_LEFT].main);

		wf_actor_load_texture2d(a, MODE_HI, texture->t[c].main, b);
#else
		// non-shader hi-res not using textures, so nothing to do here.
#endif
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
			//if we get here something has gone badly wrong. Most likely unrecoverable.
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
				//g_return_if_fail(peak_texture[i]);
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
		wf_actor_load_texture2d(a, MODE_MED, texture_id, b);
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
wf_actor_on_animation_finished(WfAnimation* animation, gpointer user_data)
{
	WaveformActor* actor = user_data;
	g_return_if_fail(actor);
	g_return_if_fail(animation);
	WfActorPriv* _a = actor->priv;

	int l = g_list_length(_a->transitions);
	_a->transitions = g_list_remove(_a->transitions, animation);
	if(g_list_length(_a->transitions) != l - 1) gwarn("animation not removed. len=%i-->%i", l, g_list_length(_a->transitions));
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
	//special version of get_viewport that gets the outer viewport for duration of the current animation.

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
#define ZOOM_MED (1.0/ 256) // px_per_sample - transition point from std to hi-res mode.
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


static ModeRange
mode_range(WaveformActor* a)
{
	WfRectangle* rect = &a->rect;

	double zoom_end = rect->len / a->region.len;
	double zoom_start = a->priv->animatable.rect_len.val.f / a->priv->animatable.len.val.i;
	if(zoom_start == 0.0) zoom_start = zoom_end;

	return (ModeRange){get_mode(zoom_start), get_mode(zoom_end)};
}


static void
_wf_actor_allocate_hi(WaveformActor* a)
{
	/*

	How many textures do we need?
	16 * 60 * 60 / hour    => 5760 / hour
		-because this is relatively high, a hashtable is used instead of an array.

	caching options:
		- say that is inherently uncachable
		- have per-waveform texture cache
		- add to normal texture cache (will cause other stuff to be purged prematurely?)
		- add low-priority flag to regular texture cache
		- have separate low-priority texture cache       ****

	*/
	WfActorPriv* _a = a->priv;
	Waveform* w = a->waveform;
	g_return_if_fail(!w->offline);
	WfRectangle* rect = &a->rect;
	WfViewPort viewport; _wf_actor_get_viewport_max(a, &viewport);
	double zoom = rect->len / a->region.len;

#if 0
	// load _all_ blocks for the transition range
	// -works well at low-zoom and for smallish transitions
	// but can uneccesarily load too many blocks.

	WfAnimatable* start = &_a->animatable.start;
	WfSampleRegion region = {MIN(start->val.i, *start->model_val.i), _a->animatable.len.val.i};
	int first_block = wf_actor_get_first_visible_block(&region, zoom, rect, &viewport);
	//int last_block = get_last_visible_block(a, zoom, &viewport); -- looks like we might be able to do away with this now...?
	region.start = MAX(start->val.i, *_a->animatable.start.model_val.i);
	int last_block = wf_actor_get_last_visible_block(&region, rect, zoom, &viewport, a->waveform->textures);
	dbg(1, "%i--->%i", first_block, last_block);
	if(last_block - first_block > wf_audio_cache_get_size()) gwarn("too many blocks requested. increase cache size");

	int b;for(b=first_block;b<=last_block;b++){
		int n_tiers_needed = get_min_n_tiers();
		if(waveform_load_audio_async(a->waveform, b, n_tiers_needed)){
			_wf_actor_load_texture_hi(a, b);
		}
	}
#else
	// load only the needed blocks - unfortunately is difficult to predict.

	void add_block(WaveformActor* a, int b)
	{
		// dont worry about dupes, the queue will remove them

		int n_tiers_needed = get_min_n_tiers();
		if(waveform_load_audio_async(a->waveform, b, n_tiers_needed)){
			_wf_actor_load_texture_hi(a, b);
		}
	}

	WfAnimatable* start = &_a->animatable.start;
	WfSampleRegion region = {*start->model_val.i, *_a->animatable.len.model_val.i};
	int first_block = wf_actor_get_first_visible_block(&region, zoom, rect, &viewport);
	int last_block = wf_actor_get_last_visible_block(&region, rect, zoom, &viewport, a->waveform->textures);

	int b;for(b=first_block;b<=last_block;b++){
		add_block(a, b);
	}

	gboolean is_new = a->rect.len == 0.0;
	gboolean animate = a->canvas->draw && a->canvas->enable_animations && !is_new;
	if(animate){
		// add blocks for transition
		// note that the animation doesnt run exactly on time so the position when redrawing cannot be precisely predicted.

		//FIXME duplicated from animator.c
		uint32_t transition_linear(WfAnimation* Xanimation, WfAnimatable* animatable, int time)
		{
			uint64_t len = 300;//animation->end - animation->start;
			uint64_t t = time - 0;//animation->start;

			float time_fraction = MIN(1.0, ((float)t) / len);
			float orig_val   = animatable->type == WF_INT ? animatable->start_val.i : animatable->start_val.f;
			float target_val = animatable->type == WF_INT ? *animatable->model_val.i : *animatable->model_val.f;
			dbg(2, "%.2f orig=%.2f target=%.2f", time_fraction, orig_val, target_val);
			return  (1.0 - time_fraction) * orig_val + time_fraction * target_val;
		}

		int t; for(t=0;t<300;t+=WF_FRAME_INTERVAL){ //TODO 300
			uint32_t s = transition_linear(NULL, start, t);
			region.start = s;
			int b = wf_actor_get_first_visible_block(&region, zoom, rect, &viewport);
			dbg(0, "transition: %u %i", s, b);

			add_block(a, b);
		}
	}
#endif
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
		dbg(2, "HI-RES");
		if(!a->waveform->offline) _wf_actor_allocate_hi(a);
		else { resolution1 = MIN(resolution1, RES_MED); resolution1 = MIN(resolution2, RES_MED); } //fallback to lower res
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
		dbg(2, "STD block range: %i --> %i", viewport_start_block, viewport_end_block);

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
#warning check TEX_BORDER effect not multiplied in WF_PEAK_STD_TO_LO transformation
			w->textures_lo = wf_texture_array_new(w->num_peaks / (WF_PEAK_STD_TO_LO * WF_TEXTURE_VISIBLE_SIZE) + ((w->num_peaks % (WF_PEAK_STD_TO_LO * WF_TEXTURE_VISIBLE_SIZE)) ? 1 : 0), w->n_channels);
		}

		WfSampleRegion region = {a->priv->animatable.start.val.i, a->priv->animatable.len.val.i};
		WfRectangle rect_ = {a->rect.left, a->rect.top, a->priv->animatable.rect_len.val.f, a->rect.height};
		int viewport_start_block = wf_actor_get_first_visible_block(&a->region, zoom_, rect, &viewport);
		int viewport_end_block   = wf_actor_get_last_visible_block (&region, &rect_, zoom_, &viewport, a->waveform->textures_lo);
		dbg(2, "L block range: %i --> %i", viewport_start_block, viewport_end_block);

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

	//_a->animatable.start.start_val.i = a->region.start;
	//_a->animatable.len.start_val.i = MAX(1, a->region.len);
	_a->animatable.rect_left.start_val.f = a->rect.left;
	_a->animatable.rect_len.start_val.f = MAX(1, a->rect.len);

	gboolean animate = a->canvas->draw && a->canvas->enable_animations && !is_new;

	a->rect = *rect;

	dbg(2, "rect: %.2f --> %.2f", rect->left, rect->left + rect->len);

	if(!a->waveform->offline) _wf_actor_load_missing_blocks(a);
	//TODO if offline, try and load from existing peakfile.

	GList* animatables = NULL;
	if(_a->animatable.rect_left.start_val.f != *_a->animatable.rect_left.model_val.f) animatables = g_list_prepend(animatables, &_a->animatable.rect_left);
	if(_a->animatable.rect_len.start_val.f != *_a->animatable.rect_len.model_val.f) animatables = g_list_prepend(animatables, &_a->animatable.rect_len);

	if(animate){
	#if 0
		animatable->val.i = a->canvas->draw
			? animatable->start_val.i
			: *animatable->model_val.i;
	#else
		GList* l = animatables; //ownership is transferred to the WfAnimation.
		for(;l;l=l->next){
			wf_actor_init_transition(a, (WfAnimatable*)l->data); //TODO this fn needs refactoring - doesnt do much now
		}
	#endif
		//				dbg(1, "%.2f --> %.2f", animatable->start_val.f, animatable->val.f);

#if 1
		l = _a->transitions;
		for(;l;l=l->next){
dbg(2, " animation=%p: will remove %i animatables", l->data, g_list_length(animatables));
			//only remove animatables we are replacing. others need to finish.
			//****** except we start a new animation!
			GList* k = animatables;
			for(;k;k=k->next){
				//dbg(0, "    l=%p l->data=%p", l, l->data);
				if(wf_animation_remove_animatable((WfAnimation*)l->data, (WfAnimatable*)k->data)) break;
				//dbg(0, "    a->transitions=%p", _a->transitions);
				//if(!g_list_length(animation->members))
				if(l->data != _a->transitions){
					//break;
				}
			}
		}
#else
		dbg(0, ".... n_animatables=%i", g_list_length(animatables));
		int ki = 0;
		GList* k = animatables;
		for(;k;k=k->next){
			int n = 0;
			GList* tr = g_list_copy(_a->transitions); //this is modified by wf_animation_remove_animatable()
			GList* l = tr;
			for(;l;l=l->next){
				//WfAnimation* animation = _a->transitions->data;
				WfAnimation* animation = l->data;
				GList* m = animation->members;
				dbg(0, "    %i: l=%p l->data=%p members=%i", ki, l, l->data, g_list_length(m));
				wf_animation_remove_animatable(animation, (WfAnimatable*)k->data);
				//dbg(0, "    a->transitions=%p members=%i", _a->transitions, g_list_length(animation->members));
				//print_animation(animation);

				//l = l->next;
				//if(n++ > 10){
				//	dbg(0, "max exceeded!");
				//	break;
				//}
			}
			g_list_free(tr);
			ki++;
			dbg(0, "animatable done %i", ki);
		}
		dbg(0, "....E");

		//did they all get cleared?
		{
			dbg(0, "remaining transitions: %i", g_list_length(_a->transitions));
			GList* l = _a->transitions;
			for(;l;l=l->next){
				WfAnimation* animation = l->data;
				GList* m = animation->members;
				dbg(0, "  members=%i", g_list_length(m));
			}
		}
#endif
		void on_frame(WfAnimation* animation, int time)
		{
			wf_canvas_queue_redraw(((WaveformActor*)animation->user_data)->canvas);
		}

		if(animatables){
			WfAnimation* animation = wf_animation_add_new(wf_actor_on_animation_finished, a);
			animation->on_frame = on_frame;
#if 0
			C* c = g_new0(C, 1);
			c->animation = animation;
			c->id = animation->id;
			animation->timeout = g_timeout_add(1000, animation_timeout, c);
#endif
			_a->transitions = g_list_append(_a->transitions, animation);
			wf_transition_add_member(animation, animatables);
			wf_animation_start(animation);
		}
	}else{
		//TODO this duplicates the function of wf_actor_init_transition.
		_a->animatable.rect_len.val.f = rect->len;
		_a->animatable.rect_len.start_val.f = rect->len;
		_a->animatable.rect_left.val.f = rect->left;
		_a->animatable.rect_left.start_val.f = rect->left;

		if(animatables) g_list_free(animatables);
		if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
	}
}


void
wf_actor_set_z(WaveformActor* a, float z)
{
	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	if(z == a->z) return;

	//check if already animating
#if 0 // not needed, as old transitions are removed in wf_actor_start_transition
	{

		GList* l = _a->transitions;
		for(;l;l=l->next){
			WfAnimation* animation = l->data;
			GList* m = animation->members;
			for(;m;m=m->next){
				WfAnimActor* aa = m->data;
				GList* t = aa->transitions;
				for(;t;t=t->next){
					WfAnimatable* animatable = t->data;
					if(aa->actor == a && animatable == &_a->animatable.z) gwarn("already animating z. actor=%p", a);
				}
			}
		}
	}
#endif

	// the start value is taken from the current value, not the model value, to support overriding of previous transitions.
	WfAnimatable* animatable = &_a->animatable.z;
	if(animatable->val.f != *animatable->model_val.f) dbg(0, "*** transient value in effect. is currently animating?");
	animatable->start_val.f = animatable->val.f;
	a->z = z;

	if(animatable->start_val.f != *animatable->model_val.f){
		GList* animatables = g_list_prepend(NULL, animatable);
		wf_actor_start_transition(a, animatables, NULL, NULL);
	}
}


void
wf_actor_fade_out(WaveformActor* a, WaveformActorFn callback, gpointer user_data)
{
	// not yet sure if this fn will remain (see sig of _fade_in)

	//TODO for consistency we need to change the background colour, though also need to preserve it to fade in.
	//     -can we get away with just setting _a->opacity?

	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	if(_a->opacity == 0.0f) return;

	WfAnimatable* animatable = &_a->animatable.opacity;
	animatable->start_val.f = animatable->val.f;
	_a->opacity = 0.0f;

	GList* animatables = TRUE || (animatable->start_val.f != *animatable->model_val.f)
		? g_list_prepend(NULL, animatable)
		: NULL;

	typedef struct {
		WaveformActor*  actor;
		WaveformActorFn callback;
		gpointer        user_data;
	} C;
	C* c = g_new0(C, 1);
	c->actor = a;
	c->callback = callback;
	c->user_data = user_data;

	void _on_fadeout_finished(WfAnimation* animation, gpointer user_data)
	{
		PF;
		g_return_if_fail(user_data);
		g_return_if_fail(animation);
		C* c = user_data;
#if 0
		WfActorPriv* _a = actor->priv;
		if(animation->members){
			GList* m = animation->members;
			if(m){
				GList* t = ((WfAnimActor*)m->data)->transitions;
				if(!t) gwarn("animation has no transitions");
				for(;t;t=t->next){
					WfAnimatable* animatable = t->data;
					if(animatable == &_a->animatable.opacity) dbg(0, "opacity transition finished");
				}
			}
		}
#endif
		if(c->callback) c->callback(c->actor, c->user_data);
		g_free(c);
	}
	wf_actor_start_transition(a, animatables, _on_fadeout_finished, c);
}


// FIXME temporary signature
void
wf_actor_fade_in(WaveformActor* a, void* /* WfAnimatable* */ _animatable, float target, WaveformActorFn callback, gpointer user_data)
{
	PF;
	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	if(_a->opacity == 1.0f) return; //TODO interaction with fg_colour?

	WfAnimatable* animatable = &_a->animatable.opacity;
	animatable->start_val.f = animatable->val.f;
	_a->opacity = ((float)(a->fg_colour & 0xff)) / 256.0;

	GList* animatables = TRUE || (animatable->start_val.f != *animatable->model_val.f)
		? g_list_prepend(NULL, animatable)
		: NULL;

	typedef struct {
		WaveformActor*  actor;
		WaveformActorFn callback;
		gpointer        user_data;
	} C;
	C* c = g_new0(C, 1);
	c->actor = a;
	c->callback = callback;
	c->user_data = user_data;

	void _on_fadein_finished(WfAnimation* animation, gpointer user_data)
	{
		PF;
		g_return_if_fail(user_data);
		g_return_if_fail(animation);
		C* c = user_data;
		if(c->callback) c->callback(c->actor, c->user_data);
		g_free(c);
	}
	wf_actor_start_transition(a, animatables, _on_fadein_finished, c);
}


void
wf_actor_set_vzoom(WaveformActor* a, float vzoom)
{
	dbg(0, "vzoom=%.2f", vzoom);
	#define MAX_VZOOM 100.0
	g_return_if_fail(!(vzoom < 1.0 || vzoom > MAX_VZOOM));
	a->vzoom = vzoom;

	//TODO perhaps better to just the canvas gain instead? why would one need individual actor gain?
	wf_canvas_set_gain(a->canvas, vzoom);

	if(a->canvas->draw) wf_canvas_queue_redraw(a->canvas);
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
		agl_use_texture(textures->peak_texture[0].main[b]);

	gl_warn("cannot bind texture: block=%i: %i", b, textures->peak_texture[0].main[b]);

	glColor4f(fg.r, fg.g, fg.b, alpha); //seems we have to set colour _after_ binding... ?

	gl_warn("gl error");
}


static inline float
get_peaks_per_pixel_i(WaveformCanvas* wfc, WfSampleRegion* region, WfRectangle* rect, int mode)
{
	//eg: for 51200 frame sample 256pixels wide: n_peaks=51200/256=200, ppp=200/256=0.8

	float region_width_px = wf_canvas_gl_to_px(wfc, rect->len);
	if(mode == MODE_HI) region_width_px /= 16; //this gives the correct result but dont know why.
	float peaks_per_pixel = ceil(((float)region->len / WF_PEAK_TEXTURE_SIZE) / region_width_px);
	dbg(2, "region_width_px=%.2f peaks_per_pixel=%.2f (%.2f)", region_width_px, peaks_per_pixel, ((float)region->len / WF_PEAK_TEXTURE_SIZE) / region_width_px);
	if(mode == MODE_LOW) peaks_per_pixel /= 16;
	return peaks_per_pixel;
}


static inline float
get_peaks_per_pixel(WaveformCanvas* wfc, WfSampleRegion* region, WfRectangle* rect, int mode)
{
	//as above but not rounded to nearest integer value

	float region_width_px = wf_canvas_gl_to_px(wfc, rect->len);
	if(mode == MODE_HI) region_width_px /= 16; //this gives the correct result but dont know why.
	float peaks_per_pixel = ((float)region->len / WF_PEAK_TEXTURE_SIZE) / region_width_px;
	dbg(2, "region_width_px=%.2f peaks_per_pixel=%.2f (%.2f)", region_width_px, peaks_per_pixel, ((float)region->len / WF_PEAK_TEXTURE_SIZE) / region_width_px);
	if(mode == MODE_LOW) peaks_per_pixel /= 16;
	return peaks_per_pixel;
}


static inline void
block_hires_shader(WaveformActor* actor, int b, double _block_wid, gboolean is_first, gboolean is_last, int first_offset, int samples_per_texture, WfRectangle rect, WfSampleRegion region, int region_end_block, double zoom, double x, double first_offset_px)
{
	//render the 1d peak textures onto a 2d block.

	//if we dont subdivide the blocks, size will be 256 x 16 = 4096 - should be ok.
	//-it works but is the large texture size causing performance issues?

	//TODO merge with non-shader version below.

	gl_warn("pre");

	WaveformCanvas* wfc = actor->canvas;
	Waveform* w = actor->waveform; 

	WfTextureHi* texture = g_hash_table_lookup(actor->waveform->textures_hi->textures, &b);
	if(!texture){
		dbg(1, "texture not available. b=%i", b);
		return;
	}
	glEnable(GL_TEXTURE_1D);
	int c;for(c=0;c<waveform_get_n_channels(w);c++){
		texture_unit_use_texture(wfc->texture_unit[0 + 2 * c], texture->t[c].main);
		texture_unit_use_texture(wfc->texture_unit[1 + 2 * c], texture->t[c].neg);
		dbg(2, "%i: textures: %u %u ok=%i,%i", b, texture->t[c].main, texture->t[c].neg, glIsTexture(texture->t[c].main), glIsTexture(texture->t[c].neg));
	}
	gl_warn("texture assign");
	glActiveTexture(GL_TEXTURE0);

	HiResShader* hires_shader = wfc->priv->shaders.hires;
	hires_shader->uniform.fg_colour = actor->fg_colour;
	hires_shader->uniform.peaks_per_pixel = get_peaks_per_pixel_i(wfc, &region, &rect, MODE_HI);
	//dbg(0, "peaks_per_pixel=%.2f", hires_shader->uniform.peaks_per_pixel);
	hires_shader->uniform.top = rect.top;
	hires_shader->uniform.bottom = rect.top + rect.height;
	hires_shader->uniform.n_channels = waveform_get_n_channels(w);

	agl_use_program(&hires_shader->shader); //TODO ideally we want to do each block without resetting the program.

	WfColourFloat fg;
	wf_colour_rgba_to_float(&fg, actor->fg_colour);
	float alpha = ((float)(actor->fg_colour & 0xff)) / 256.0;
	glColor4f(fg.r, fg.g, fg.b, alpha); //seems we have to set colour _after_ binding... ?

					//duplicate from _paint
					// *** now contains BORDER offsetting which should be duplicated for other modes.
					double usable_pct = (modes[MODE_HI].texture_size - 2.0 * TEX_BORDER_HI) / modes[MODE_HI].texture_size;
					double border_pct = (1.0 - usable_pct)/2;

					double block_wid = _block_wid;
					double tex_pct = 1.0 * usable_pct; //use the whole texture
					double tex_start = TEX_BORDER_HI / modes[MODE_HI].texture_size;
					if (is_first){
						double _tex_pct = 1.0;
						if(first_offset){
							_tex_pct = 1.0 - ((double)first_offset) / samples_per_texture;
							tex_pct = tex_pct - (usable_pct) * ((double)first_offset) / samples_per_texture;
						}
						block_wid = _block_wid * _tex_pct;
						//tex_start = 1.0 - (_tex_pct + tex_pct)/2;
						tex_start = 1.0 - border_pct - tex_pct;
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

						if(b == w->textures->size - 1) dbg(2, "last sample block. fraction=%.2f", w->textures->last_fraction);
						//TODO when non-square textures enabled, tex_pct can be wrong because the last texture is likely to be smaller
						//     (currently this only applies in non-shader mode)
						//tex_pct = block_wid / _block_wid;
						tex_pct = (block_wid / _block_wid) * usable_pct;
					}

					dbg (2, "%i: is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex_start=%.2f", b, is_last, x, block_wid, _block_wid, tex_pct, tex_start);
if(tex_pct > usable_pct || tex_pct < 0.0){
	dbg (0, "%i: is_first=%i is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex_start=%.2f", b, is_first, is_last, x, block_wid, _block_wid, tex_pct, tex_start);
}
					if(tex_pct > usable_pct || tex_pct < 0.0) gwarn("tex_pct! %.2f (b=%i)", tex_pct, b);
					double tex_x = x + ((is_first && first_offset) ? first_offset_px : 0);

	glBegin(GL_QUADS);
#if defined (USE_FBO) && defined (multipass)
//	if(false){
	if(true){    //fbo not yet implemented for hi-res mode.
#else
	if(wfc->use_1d_textures){
#endif
		_draw_block_from_1d(tex_start, tex_pct, tex_x, rect.top, block_wid, rect.height, modes[MODE_HI].texture_size);
	}else{
		gerr("TODO 2d textures in MODE_HI");
		glTexCoord2d(tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       rect.top);
		glTexCoord2d(tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, rect.top);
		glTexCoord2d(tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, rect.top + rect.height);
		glTexCoord2d(tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       rect.top + rect.height);
	}
	glEnd();
}


static inline void
block_hires_nonshader(WaveformActor* actor, int b, double _block_wid, gboolean is_first, gboolean is_last, int first_offset, int samples_per_texture, WfRectangle rect, WfSampleRegion region, int region_end_block, double zoom, double x, double first_offset_px)
{
	//TODO temporary fn - should share code

	//render the 2d peak texture onto a block.

	//if we dont subdivide the blocks, size will be 256 x 16 = 4096. TODO intel 945 has max size of 2048
	//-it works but is the large texture size causing performance issues?

	int texture_size = modes[MODE_HI].texture_size / 2;
samples_per_texture /= 2; //due to half size texture

	gl_warn("pre");

	WaveformCanvas* wfc = actor->canvas;
	Waveform* w = actor->waveform; 

									//int b_ = b | WF_TEXTURE_CACHE_HIRES_MASK;
	WfTextureHi* texture = g_hash_table_lookup(actor->waveform->textures_hi->textures, &b);
	if(!texture){
		dbg(1, "texture not available. b=%i", b);
		return;
	}
	glEnable(GL_TEXTURE_2D);
	agl_use_texture(texture->t[WF_LEFT].main);
	gl_warn("texture assign");

	float texels_per_px = ((float)texture_size) /_block_wid;
	#define EXTRA_PASSES 4 // empirically determined for visual effect.
	int texels_per_px_i = ((int)texels_per_px) + EXTRA_PASSES;

	WfColourFloat fg;
	wf_colour_rgba_to_float(&fg, actor->fg_colour);
	float alpha = ((float)(actor->fg_colour & 0xff)) / 256.0;
	alpha /= (texels_per_px_i * 0.5); //reduce the opacity depending on how many texture passes we do, but not be the full amount (which looks too much).
	glColor4f(fg.r, fg.g, fg.b, alpha); //seems we have to set colour _after_ binding... ?

	//gboolean no_more = false;
	//#define RATIO 2
	//int r; for(r=0;r<RATIO;r++){ // no, this is horrible, we need to move this into the main block loop.

					//duplicate from _paint - temporary only!

					double block_wid = _block_wid / 2.0;
					double tex_pct = 1.0; //use the whole texture
					double tex_start = 0.0;
					if (is_first){
						if(first_offset >= samples_per_texture) return;
						if(first_offset) tex_pct = 1.0 - ((double)first_offset) / samples_per_texture;
						block_wid = (_block_wid / 2.0) * tex_pct;
						tex_start = 1 - tex_pct;
						dbg(2, "rect.left=%.2f region->start=%Lu first_offset=%i", rect.left, region.start, first_offset);
					}
					if (is_last){
						//if(x + _block_wid < x0 + rect->len){
						if(b < region_end_block){
							//end is offscreen. last block is not smaller.
						}else{
//TODO for last block, the texture width is likely to be smaller
//how to get texture size???
//peakbuf->size is always 8192
//Peakbuf* peakbuf = waveform_get_peakbuf_n(actor->waveform, b);

//dbg(0, "last_fraction=%.2f", actor->waveform->textures->last_fraction);
							//end is trimmed
							double part_inset_px = wf_actor_samples2gl(zoom, region.start);
							//double file_start_px = rect.left - part_inset_px;
							double distance_from_file_start_to_region_end = part_inset_px + rect.len;
							block_wid = distance_from_file_start_to_region_end - b * _block_wid;
							dbg(2, " %i: inset=%.2f s->e=%.2f i*b=%.2f", b, part_inset_px, distance_from_file_start_to_region_end, b * _block_wid);
							if(b * _block_wid > distance_from_file_start_to_region_end){ gwarn("!!"); return; }
						}
						block_wid = MIN(block_wid, _block_wid/2.0);

						if(b == w->textures->size - 1) dbg(2, "last sample block. fraction=%.2f", w->textures->last_fraction);
						//TODO check what happens here if we allow non-square textures
						tex_pct = (block_wid / _block_wid) * 2.0;

//						if(tex_pct < 0.5) no_more = true;
					}

					dbg (2, "%i: is_last=%i x=%.2f wid=%.2f/%.2f tex_pct=%.3f tex_start=%.2f", b, is_last, x, block_wid, _block_wid, tex_pct, tex_start);
					if(tex_pct > 1.0 || tex_pct < 0.0) gwarn("tex_pct! %.2f", tex_pct);
					double tex_x = x + ((is_first && first_offset) ? first_offset_px : 0);

		glBegin(GL_QUADS);
	//#if defined (USE_FBO) && defined (multipass)
	//	if(false){
	//	if(true){    //fbo not yet implemented for hi-res mode.
	//#else
		if(wfc->use_1d_textures){
	//#endif
			_draw_block_from_1d(tex_start, tex_pct, tex_x, rect.top, block_wid, rect.height, modes[MODE_HI].texture_size);
		}else{
			dbg(0, "x=%.2f wid=%.2f tex_pct=%.2f", tex_x, block_wid, tex_pct);

			/*
			 * render the texture multiple times so that all peaks are shown
			 * -this looks quite nice, but without saturation, the peaks can be very faint.
			 *  (not possible to have saturation while blending over background)
			 */
			float texel_offset = 1.0 / ((float)texture_size);
			int i; for(i=0;i<texels_per_px_i;i++){
				dbg(0, "texels_per_px=%.2f %i texel_offset=%.3f tex_start=%.4f", texels_per_px, texels_per_px_i, (texels_per_px / 2.0) / ((float)texture_size), tex_start);
				glTexCoord2d(tex_start + 0.0,     0.0); glVertex2d(tex_x + 0.0,       rect.top);
				glTexCoord2d(tex_start + tex_pct, 0.0); glVertex2d(tex_x + block_wid, rect.top);
				glTexCoord2d(tex_start + tex_pct, 1.0); glVertex2d(tex_x + block_wid, rect.top + rect.height);
				glTexCoord2d(tex_start + 0.0,     1.0); glVertex2d(tex_x + 0.0,       rect.top + rect.height);

				tex_start += texel_offset;
			}
		}
		glEnd();

	//	if(no_more) break;
	//	x += _block_wid * 0.5;
	//}
}


static inline void
_draw_block(float tex_start, float tex_pct, float tex_x, float top, float width, float height, float gain)
{
	// the purpose of this fn is to hide the effect of the texture border.
	// (******* no longer the case. TODO remove all text border stuff OUT of here - its too complicated - see _draw_block_from_1d in MODE_HI)
	// - the start point is offset by TEX_BORDER
	// - the tex_pct given is the one that would be used if there was no border (ie it has max value of 1.0), so the scaling needs to be modified also.

	//TODO it might be better to not have tex_pct as an arg, but use eg n_pix instead?

//												dbg(0, "width=%.2f tex_pct=%.3f %.3f start=%.3f", width, tex_pct, tex_pct * (256.0 - 2.0 * TEX_BORDER) / 256.0, tex_start + TEX_BORDER / 256.0);

	tex_start += TEX_BORDER / 256.0;
	tex_pct *= (256.0 - 2.0 * TEX_BORDER) / 256.0;

	glTexCoord2d(tex_start + 0.0,     0.5 - 0.5/gain); glVertex2d(tex_x + 0.0,   top);
	glTexCoord2d(tex_start + tex_pct, 0.5 - 0.5/gain); glVertex2d(tex_x + width, top);
	glTexCoord2d(tex_start + tex_pct, 0.5 + 0.5/gain); glVertex2d(tex_x + width, top + height);
	glTexCoord2d(tex_start + 0.0,     0.5 + 0.5/gain); glVertex2d(tex_x + 0.0,   top + height);
}


static inline void
_draw_block_from_1d(float tex_start, float tex_pct, float x, float y, float width, float height, int t_size)
{
	if(t_size > 256){
		// MODE_HI
		// no longer do any offsetting here - do the same for non MODE_HI also.
	}else{
		float offset = TEX_BORDER;

		tex_start += offset / t_size;
		tex_pct *= (t_size - 2.0 * offset) / t_size;
	}

	float tex_end = tex_start + tex_pct;
	glMultiTexCoord2f(WF_TEXTURE0, tex_start, 1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start, 1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start, 1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start, 1.0); glVertex2d(x + 0.0,   y);
	glMultiTexCoord2f(WF_TEXTURE0, tex_end,   1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_end,   1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_end,   1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_end,   1.0); glVertex2d(x + width, y);
	glMultiTexCoord2f(WF_TEXTURE0, tex_end,   0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_end,   0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_end,   0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_end,   0.0); glVertex2d(x + width, y + height);
	glMultiTexCoord2f(WF_TEXTURE0, tex_start, 0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start, 0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start, 0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start, 0.0); glVertex2d(x + 0.0,   y + height);
}


// temporary - performance testing
static void
use_texture_no_blend(GLuint texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glDisable(GL_BLEND);
}


void
wf_actor_paint(WaveformActor* actor)
{
	//-must have called gdk_gl_drawable_gl_begin() first.
	//-it is assumed that all textures have been loaded.

	//note: there is some benefit in quantising the x positions (eg for subpixel consistency),
	//but to preserve relative actor positions it must be done at the canvas level.

	AGl* agl = agl_get_instance();
	g_return_if_fail(actor);
	WaveformCanvas* wfc = actor->canvas;
	g_return_if_fail(wfc);
	WfCanvasPriv* _c = wfc->priv;
	WfActorPriv* _a = actor->priv;
	Waveform* w = actor->waveform; 
	if(w->offline) return;
	WfRectangle rect = {_a->animatable.rect_left.val.f, actor->rect.top, _a->animatable.rect_len.val.f, actor->rect.height};
	g_return_if_fail(rect.len);

	WfSampleRegion region = {_a->animatable.start.val.i, _a->animatable.len.val.i};
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

	float alpha = _a->animatable.opacity.val.f;

	if(w->num_peaks){
		WfGlBlock* textures = mode == MODE_LOW ? w->textures_lo : w->textures;
		if(!textures) return; //in hi-res mode, textures are loaded asynchronously and may not be ready yet

		int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (mode == MODE_LOW ? WF_PEAK_STD_TO_LO : 1);

		int region_start_block   = region.start / samples_per_texture;
		int region_end_block     = (region.start + region.len) / samples_per_texture - (!((region.start + region.len) % samples_per_texture) ? 1 : 0);
		int viewport_start_block = wf_actor_get_first_visible_block(&region, zoom, &rect, &viewport);
		int viewport_end_block   = wf_actor_get_last_visible_block (&region, &rect, zoom, &viewport, textures);
		g_return_if_fail(viewport_end_block >= viewport_start_block);

		if(rect.left > viewport.right || rect.left + rect.len < viewport.left){
			dbg(2, "actor is outside viewport. not drawing");
			return;
		}

		if(region_end_block > textures->size -1){ gwarn("region too long? region_end_block=%i n_blocks=%i region.len=%i", region_end_block, textures->size, region.len); region_end_block = w->textures->size -1; }
		dbg(2, "block range: region=%i-->%i viewport=%i-->%i", region_start_block, region_end_block, viewport_start_block, viewport_end_block);
		dbg(2, "rect=%.2f %.2f viewport=%.2f %.2f", rect.left, rect.len, viewport.left, viewport.right);

		int first_offset = region.start % samples_per_texture;
		double first_offset_px = wf_actor_samples2gl(zoom, first_offset);

		double _block_wid = wf_actor_samples2gl(zoom, samples_per_texture);
#if defined (USE_FBO) && defined (multipass)
		if(agl->use_shaders){
			//set gl state
#ifdef USE_FX
			BloomShader* shader = _c->shaders.horizontal;
			shader->uniform.fg_colour = (actor->fg_colour & 0xffffff00) + MIN(0xff, 0x100 * _a->animatable.opacity.val.f);
			shader->uniform.peaks_per_pixel = get_peaks_per_pixel(wfc, &region, &rect, mode) / 1.0;
			agl_use_program(&shader->shader);
#else
			BloomShader* shader = wfc->priv->shaders.vertical;
			wfc->priv->shaders.vertical->uniform.fg_colour = (actor->fg_colour & 0xffffff00) + MIN(0xff, 0x100 * _a->animatable.opacity.val.f);
			wfc->priv->shaders.vertical->uniform.peaks_per_pixel = get_peaks_per_pixel_i(wfc, &region, &rect, mode);
			//TODO the vertical shader needs to check _all_ the available texture values to get true peak.
			agl_use_program(&shader->shader);
#endif

					glDisable(GL_TEXTURE_1D);
					glEnable(GL_TEXTURE_2D);
					glActiveTexture(GL_TEXTURE0);
		}
#else
		if(wfc->use_1d_textures){
			agl_use_program((AGlShader*)wfc->priv->shaders.peak);

			//uniforms: (must be done on each paint because some vars are actor-specific)
			float peaks_per_pixel = get_peaks_per_pixel_i(wfc, &region, &rect, mode);
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
#ifndef HIRES_NONSHADER_TEXTURES
		//block_region specifies the sample range for that part of the waveform region that is within the current block
		//-note that the block region can exceed the range of the waveform region.
		WfSampleRegion block_region = {region.start % WF_SAMPLES_PER_TEXTURE, WF_SAMPLES_PER_TEXTURE - region.start % WF_SAMPLES_PER_TEXTURE};
#endif
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
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#define ANTIALIASED_LINES
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
							if(is_last){
								block_region_v_hi.len = (region.start + region.len) % WF_SAMPLES_PER_TEXTURE;
							}

							//alternative calculation of block_region_v_hi - does it give same results? NO
							uint64_t st = MAX((uint64_t)(region.start),              (uint64_t)((b)     * samples_per_texture));
							uint64_t e  = MIN((uint64_t)(region.start + region.len), (uint64_t)((b + 1) * samples_per_texture));
							WfSampleRegion block_region_v_hi2 = {st, e - st};
							//dbg(0, "block_region_v_hi=%Lu(%Lu)-->%Lu len=%Lu (buf->size=%Lu region=%Lu-->%Lu)", st, (uint64_t)block_region_v_hi.start, e, (uint64_t)block_region_v_hi2.len, ((uint64_t)buf->size), ((uint64_t)region.start), ((uint64_t)region.start) + ((uint64_t)region.len));

							if(rect.left + rect.len < viewport.left){
								gerr("rect is outside viewport");
							}

							draw_wave_buffer_v_hi(actor, region, block_region_v_hi2, &rect, &viewport, buf, wfc->v_gain, actor->fg_colour, is_first, x);

							block_done = true; //super hi res was succussful. no more painting needed for this block.
							break;
						}
					}
					// if we fall through here, the MODE_HI texture is very likely not to be loaded.

				case MODE_HI:
					;Peakbuf* peakbuf = waveform_get_peakbuf_n(w, b);
					if(peakbuf){
						if(agl->use_shaders){
							block_hires_shader(actor, b, _block_wid, is_first, is_last, first_offset, samples_per_texture, rect, region, region_end_block, zoom, x, first_offset_px);
							block_done = true; //TODO should really use the fall-through code to apply the textures
						}else{
#ifdef HIRES_NONSHADER_TEXTURES
							block_hires_nonshader(actor, b, _block_wid, is_first, is_last, first_offset, samples_per_texture, rect, region, region_end_block, zoom, x, first_offset_px);
							block_done = true; //TODO should really use the fall-through code to apply the textures
#else
							// without shaders, each sample line is drawn directly without using textures, so performance will be relatively poor.

							dbg(2, "  b=%i x=%.2f", b, x);

							//TODO might these prevent further blocks at different res? difficult to notice as they are usually the same.
							agl_use_program(NULL);
							//TODO blending is needed to support opacity, however the actual opacity currently varies depending on zoom.
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							glDisable(GL_TEXTURE_2D);
							glDisable(GL_TEXTURE_1D);

							int c; for(c=0;c<w->n_channels;c++){
								if(peakbuf->buf[c]){
									//dbg(1, "peakbuf: %i:%i: %i", b, c, ((short*)peakbuf->buf[c])[0]);
									//WfRectangle block_rect = {x, rect.top + c * rect.height/2, _block_wid, rect.height / w->n_channels};
									WfRectangle block_rect = {is_first
											? x + (block_region.start - region.start % WF_SAMPLES_PER_TEXTURE) * zoom
											: x,
										rect.top + c * rect.height/2, _block_wid, rect.height / w->n_channels};
									if(is_first){
										float first_fraction =((float)block_region.len) / WF_SAMPLES_PER_TEXTURE;
										block_rect.left += (WF_SAMPLES_PER_TEXTURE - WF_SAMPLES_PER_TEXTURE * first_fraction) * zoom;
									}
									//WfRectangle block_rect = {x + (region.start % WF_PEAK_BLOCK_SIZE) * zoom, rect.top + c * rect.height/2, _block_wid, rect.height / w->n_channels};
									dbg(2, "  HI: %i: rect=%.2f-->%.2f", b, block_rect.left, block_rect.left + block_rect.len);

									if(is_last){
										if(b < region_end_block){
											// end is offscreen. last block is not smaller.
											// reducing the block size here would be an optimisation rather than essential
										}else{
											// last block of region (not merely truncated by viewport).
											//block_region.len = region.len - b * WF_PEAK_BLOCK_SIZE;
											//block_region.len = (region.start - block_region.start + region.len) % WF_PEAK_BLOCK_SIZE;// - block_region.start;
											if(is_first){
												block_region.len = region.len % WF_SAMPLES_PER_TEXTURE;
											}else{
												block_region.len = (region.start + region.len) % WF_SAMPLES_PER_TEXTURE;
											}
											dbg(2, "REGIONLAST: %i/%i region.len=%i ratio=%.2f rect=%.2f %.2f", b, region_end_block, block_region.len, ((float)block_region.len) / WF_PEAK_BLOCK_SIZE, block_rect.left, block_rect.len);
										}
									}
									block_rect.len = block_region.len * zoom; // always
									draw_wave_buffer_hi(w, block_region, &block_rect, peakbuf, c, wfc->v_gain, actor->fg_colour);
								}
								else dbg(1, "buf not ready: %i", c);
							}
							block_done = true; //hi res was successful. no more painting needed for this block.
#endif
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
						AglFBO* fbo = false
							? fbo_test
#ifdef USE_FX
							: textures->fx_fbo[b]
								? textures->fx_fbo[b]
#endif
								: textures->fbo[b];
						if(fbo){ //seems that the fbo may not be created initially...
							if(wfc->blend)
								agl_use_texture(fbo->texture);
							else
								use_texture_no_blend(fbo->texture);
						}
					}else{
						_set_gl_state_for_block(wfc, w, textures, b, fg, alpha);
					}
#else
					_set_gl_state_for_block(wfc, w, textures, b, fg, alpha);
#endif

					glPushMatrix();
					glTranslatef(0, 0, _a->animatable.z.val.f);
					glBegin(GL_QUADS);
#if defined (USE_FBO) && defined (multipass)
					if(false){
#else
					if(wfc->use_1d_textures){
#endif
						_draw_block_from_1d(tex_start, tex_pct, tex_x, rect.top, block_wid, rect.height, modes[mode].texture_size);
					}else{
						_draw_block(tex_start, tex_pct, tex_x, rect.top, block_wid, rect.height, wfc->v_gain);
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
			if(wfc->use_1d_textures){
				agl->shaders.plain->uniform.colour = 0x6677ffaa;
				agl_use_program((AGlShader*)agl->shaders.plain);
				glDisable(GL_TEXTURE_1D);
			}else{
				//agl_use_program(NULL); // it should now no longer be neccesary to ever set the program if shaders are not used.
			}
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(1.0, 0.0, 1.0, 0.75);
			glLineWidth(1);
			float x_ = x + 0.1; //TODO this was added for intel 945 - test on other hardware.
			glBegin(GL_LINES);
				glVertex3f(x_,              rect.top + 1, 0);    glVertex3f(x_ + _block_wid, rect.top + 1, 0);
				glVertex3f(x_ + _block_wid, rect.top + 1, 0);    glVertex3f(x_ + _block_wid, bot - 1,      0);
				glVertex3f(x_,              bot - 1,      0);    glVertex3f(x_ + _block_wid, bot - 1,      0);

				glVertex3f(x_,              rect.top + 1, 0);    glVertex3f(x_,              rect.top + 10,0);
				glVertex3f(x_,              bot - 1,      0);    glVertex3f(x_,              bot - 10,     0);
			glEnd();
			glColor3f(1.0, 1.0, 1.0);
			glEnable(wfc->use_1d_textures ? GL_TEXTURE_1D : GL_TEXTURE_2D);
			if(wfc->use_1d_textures){
#ifdef USE_FBO
#ifdef USE_FX
				//TODO can we push and pop shader?
				BloomShader* shader = wfc->priv->shaders.horizontal;
				agl_use_program(&shader->shader);
#else
				BloomShader* shader = wfc->priv->shaders.vertical;
				agl_use_program(&shader->shader);
#endif
#else
				agl_use_program((AGlShader*)wfc->priv->shaders.peak);
#endif
			}
#endif
#endif
			x += _block_wid;
#ifndef HIRES_NONSHADER_TEXTURES
			block_region.start = 0; //all blocks except first start at 0
			block_region.len = WF_PEAK_BLOCK_SIZE;
#endif
			block_region_v_hi.start = (block_region_v_hi.start / WF_PEAK_BLOCK_SIZE + 1) * WF_PEAK_BLOCK_SIZE;
			block_region_v_hi.len   = WF_PEAK_BLOCK_SIZE - block_region_v_hi.start % WF_PEAK_BLOCK_SIZE;
			is_first = false;
		}
	}

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

	void make_texture_data_med(Waveform* w, int ch, struct _buf* buf)
	{
		//copy peak data into a temporary buffer, translating from 16 bit to 8 bit
		// -src: peak buffer covering whole file (16bit)
		// -dest: temp buffer for single block (8bit) - will be loaded directly into a gl texture.

		WfPeakBuf* peak = &w->priv->peak;
		int f; for(f=0;f<WF_PEAK_TEXTURE_SIZE;f++){
			int i = ((WF_PEAK_TEXTURE_SIZE - 2 * TEX_BORDER) * blocknum + f - TEX_BORDER) * WF_PEAK_VALUES_PER_SAMPLE;
			if(i >= peak->size){
#if 0
				int j; for(j=0;j<5;j++){
					printf("  %i=%i %i\n", j, peak->buf[ch][2*j], peak->buf[ch][2*j +1]);
				}
				for(j=0;j<5;j++){
					printf("  %i %i=%i %i\n", i, i -2*j -2, peak->buf[ch][i -2*j -2 ], peak->buf[ch][i -2*j -1]);
				}
#endif
				buf->positive[f] = 0;
				buf->negative[f] = 0;
				if(i == peak->size) dbg(1, "end of peak: %i b=%i n_sec=%.3f", peak->size, blocknum, ((float)((WF_PEAK_TEXTURE_SIZE * blocknum + f) * WF_PEAK_RATIO))/44100);// break;

			}else if(i >= 0){ //TODO change the loop start point instead (TEX_BORDER)
				buf->positive[f] =  peak->buf[ch][i  ] >> 8;
				buf->negative[f] = -peak->buf[ch][i+1] >> 8;
			}else{
				buf->positive[f] = 0;
				buf->negative[f] = 0;
			}
		}
	}

	void make_texture_data_low(Waveform* w, int ch, struct _buf* buf)
	{
		dbg(2, "b=%i", blocknum);

		WfPeakBuf* peak = &w->priv->peak;
		int f; for(f=0;f<WF_PEAK_TEXTURE_SIZE;f++){
			int i = ((WF_PEAK_TEXTURE_SIZE  - 2 * TEX_BORDER) * blocknum + f  - TEX_BORDER) * WF_PEAK_VALUES_PER_SAMPLE * WF_PEAK_STD_TO_LO;
			if(i >= peak->size) break;
			if(i < 0) continue;        //TODO change the loop start point instead (TEX_BORDER)

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
			make_texture_data_med(w, c, buf);
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
		//using MAG LINEAR smooths nicely but can reduce the opacity of peaks too much.
		//
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
wf_actor_load_texture2d(WaveformActor* a, Mode mode, int texture_id, int blocknum)
{
	Waveform* w = a->waveform;
	dbg(1, "* %i: texture=%i", blocknum, texture_id);
	if(mode == MODE_MED){
		AlphaBuf* alphabuf = wf_alphabuf_new(w, blocknum, 1, false, TEX_BORDER);
		wf_canvas_load_texture_from_alphabuf(a->canvas, texture_id, alphabuf);
		wf_alphabuf_free(alphabuf);
	}
	else if(mode == MODE_HI){
		AlphaBuf* alphabuf = wf_alphabuf_new_hi(w, blocknum, 1, false, TEX_BORDER);
		wf_canvas_load_texture_from_alphabuf(a->canvas, texture_id, alphabuf);
		wf_alphabuf_free(alphabuf);
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
				/*
				//int k; for(k=0;k<20;k++){
				//	buf->positive[k] = 100;
				//}
				int k; for(k=0;k<100;k++){
					buf->positive[texture_size - 1 - k] = 80;
				}
				*/
}


static void
_wf_actor_load_texture_hi(WaveformActor* a, int block)
{
	// audio data for this block _must_ already be loaded

	/*
	WfViewPort viewport; wf_actor_get_viewport(actor, &viewport);
	*/

	ModeRange mode = mode_range(a);
	if(mode.lower == MODE_HI || mode.upper == MODE_HI){
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
wf_actor_init_transition(WaveformActor* a, WfAnimatable* animatable)
{
	g_return_if_fail(a);

	animatable->val.i = a->canvas->draw
		? animatable->start_val.i
		: *animatable->model_val.i;
}


/*
 *   Set initial values of animatables and start the transition.
 *   The start_val of each animatable must be set by the caller. <--- no, it should be set when the animatable is created, after which it is maintained by the animator.
 *   If the actor has any other transitions using the same animatable, these animatables
 *   are removed from that transition.
 *
 *   @param animatables - ownership of this list is transferred to the WfAnimation.
 */
static void
wf_actor_start_transition(WaveformActor* a, GList* animatables, AnimationFn done, gpointer user_data)
{
	g_return_if_fail(a);
	WfActorPriv* _a = a->priv;

	// set initial value
#if 0
	GList* l = animatables;
	for(;l;l=l->next){
		WfAnimatable* animatable = l->data;
		animatable->val.i = a->canvas->draw && a->canvas->enable_animations
			? animatable->start_val.i
			: *animatable->model_val.i;
	}
#endif

	if(!a->canvas->enable_animations || !a->canvas->draw){ //if we cannot initiate painting we cannot animate.
		g_list_free(animatables);
		return;
	}

	typedef struct {
		WaveformActor* actor;
		AnimationFn    done;
		gpointer       user_data;
	} C;
	C* c = g_new0(C, 1);
	c->actor = a;
	c->done = done;
	c->user_data = user_data;

	void on_frame(WfAnimation* animation, int time)
	{
		C* c = animation->user_data;
		wf_canvas_queue_redraw(c->actor->canvas);
	}

	void _on_animation_finished(WfAnimation* animation, gpointer user_data)
	{
		g_return_if_fail(user_data);
		g_return_if_fail(animation);
		C* c = user_data;

		if(c->done) c->done(animation, c->user_data);

		wf_actor_on_animation_finished(animation, c->actor);
		g_free(c);
	}

	GList* l = _a->transitions;
	for(;l;l=l->next){
		//only remove animatables we are replacing. others need to finish.
		GList* k = animatables;
		for(;k;k=k->next){
			if(wf_animation_remove_animatable((WfAnimation*)l->data, (WfAnimatable*)k->data)) break;
		}
	}

	if(animatables){
		WfAnimation* animation = wf_animation_add_new(_on_animation_finished, c);
		animation->on_frame = on_frame;
		_a->transitions = g_list_append(_a->transitions, animation);
		wf_transition_add_member(animation, animatables);
		wf_animation_start(animation);
	}
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
		WfTextureHi* th = value;
		printf("  b=%i t=%i\n", b, th->t[WF_LEFT].main);
	}
}

#ifdef USE_TEST
GList*
wf_actor_get_transitions(WaveformActor* a)
{
	return a->priv->transitions;
}
#endif
