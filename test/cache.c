/*
  Tests for opengl textures using large files

  Automated but opens a gui window.

  1-test basic hires
  2-test textures are loaded when scrolling
      Shows many different views at using all modes to check that the cache
      works when full and that the correct data for the view is available.
  3-test texture cache is emptied at lowres when the Waveform is free'd.

  ---------------------------------------------------------------

  copyright (C) 2013-2018 Tim Orford <tim@orford.org>

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
#define __wf_private__
#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#include "agl/utils.h"
#include "waveform/waveform.h"
#include "waveform/audio.h"
#include "waveform/texture_cache.h"
#include "waveform/worker.h"
#include "test/common.h"

extern void texture_cache_print ();
extern void hi_ng_cache_print   ();

#define GL_WIDTH 512.0
#define VBORDER 8
#define WAV1 "large1.wav"
#define WAV2 "large2.wav"

GdkGLConfig*    glconfig       = NULL;
static bool     gl_initialised = false;
GtkWidget*      canvas         = NULL;
AGlScene*       scene          = NULL;
WaveformContext*wfc            = NULL;
Waveform*       w[2]           = {NULL,};
WaveformActor*  a[2]           = {NULL,};
bool            files_created  = false;
AMPromise*      ready          = NULL;

static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
uint64_t    get_time           ();

TestFn create_files, test_shown, test_hires, test_scroll, test_hi_double, test_add_remove, finish;

gpointer tests[] = {
	create_files,
	test_shown,
	test_hires,
	test_scroll,
	test_hi_double,
	test_add_remove,
	finish,
};


	static bool window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}

int
main (int argc, char *argv[])
{
	wf_debug = 0;
	test_init(tests, G_N_ELEMENTS(tests));
 
	ready = am_promise_new(NULL);

	gtk_init(&argc, &argv);
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		gerr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
	}

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	canvas = gtk_drawing_area_new();
#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus     (canvas, true);
#endif
	gtk_widget_set_size_request  (canvas, GL_WIDTH, 128);
	gtk_widget_set_gl_capability (canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events        (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_container_add            ((GtkContainer*)window, (GtkWidget*)canvas);
	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);

	gtk_widget_show_all(window);

	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


void
create_files()
{
	START_TEST;
	reset_timeout(60000);

	char* wavs[] = {WAV1, WAV2};

	int i; for(i=0;i<G_N_ELEMENTS(wavs);i++){
		char* filename = find_wav(wavs[i]);
		if(filename){
			wf_free(filename);
		}else{
			const char* dir = find_data_dir();
			assert(dir, "data dir not found");

			char* filename = g_build_filename(dir, wavs[i], NULL);
			create_large_file(filename);
			wf_free(filename);
		}
	}
	files_created = true;

	FINISH_TEST;
}


	static bool _test_shown(gpointer _c)
	{
		WfTest* c = _c;
#ifdef AGL_ACTOR_RENDER_CACHE
		AGlActor* actor = (AGlActor*)a[0];
		if(actor->cache.valid){
		}else{
			extern bool wf_actor_test_is_not_blank(WaveformActor*);
			assert_and_stop(wf_actor_test_is_not_blank(a[0]), "output is blank");
		}
#endif
		WF_TEST_FINISH_TIMER_STOP;
	}

void
test_shown()
{
	WfTest* t = NEW_TEST();

	assert(GTK_WIDGET_REALIZED(canvas), "widget not realised");
	assert(wfc, "canvas not created");
	assert(a[0], "actor not created");

	g_timeout_add(500, _test_shown, t);
}


#define REGION_LEN 2048

	static bool check_zoom(gpointer _c)
	{
		WfTest* c = _c;

		WfSampleRegion* region = &a[0]->region;

		assert_and_stop((region->len == REGION_LEN), "region length");

		WF_TEST_FINISH_TIMER_STOP;
	}

void
test_hires()
{
	WfTest* t = NEW_TEST();

	assert(wfc, "canvas not created");

	void set_region()
	{
		PF0;

		int i; for(i=0;i<G_N_ELEMENTS(a)&&a[i];i++)
			wf_actor_set_region(a[i], &(WfSampleRegion){
				0,
				REGION_LEN
			});
	}

	set_region();

	g_timeout_add(1000, check_zoom, t);
}

				// duplicates of private stuff from actor.c

				typedef struct _RenderInfo RenderInfo;
				typedef struct _Renderer Renderer;

				typedef void    (*WaveformActorPreRenderFn) (Renderer*, WaveformActor*);
				typedef void    (*WaveformActorBlockFn)     (Renderer*, WaveformActor*, int b);
				typedef bool    (*WaveformActorRenderFn)    (Renderer*, WaveformActor*, int b, bool is_first, bool is_last, double x);
				typedef void    (*WaveformActorFreeFn)      (Renderer*, Waveform*);

				struct _Renderer
				{
					Mode                     mode;

					WaveformActorBlockFn     load_block;
					WaveformActorPreRenderFn pre_render;
					WaveformActorRenderFn    render_block;
					WaveformActorFreeFn      free;
				};

				struct _draw_mode
				{
					char             name[4];
					int              resolution;
					int              texture_size;      // mostly applies to 1d textures. 2d textures have non-square issues.
					void*            make_texture_data; // might not be needed after all
					Renderer*        renderer;
				};
				static struct _draw_mode modes[N_MODES] = {
					{"V_LO", 16384, WF_PEAK_TEXTURE_SIZE,      NULL},
					{"LOW",   1024, WF_PEAK_TEXTURE_SIZE,      NULL},
					{"MED",    256, WF_PEAK_TEXTURE_SIZE,      NULL},
					{"HI",      16, WF_PEAK_TEXTURE_SIZE * 16, NULL}, // texture size chosen so that blocks are the same as in medium res
					{"V_HI",     1, WF_PEAK_TEXTURE_SIZE,      NULL},
				};

				#define ZOOM_HI   (1.0/  16)
				#define ZOOM_MED  (1.0/ 256)  // px_per_sample - transition point from std to hi-res mode.
				#define ZOOM_LO   (1.0/4096)  // px_per_sample - transition point from low-res to std mode.
				#define ZOOM_V_LO (1.0/65536) // px_per_sample - transition point from v-low-res to low-res.

				static inline int
				get_resolution(double zoom)
				{
					return (zoom > ZOOM_HI)
						? 1
						: (zoom > ZOOM_MED)
							? 16
							: (zoom > ZOOM_LO)
								? 256
								: (zoom > ZOOM_V_LO)
									? 1024
									: 16384;
				}

				static inline Mode
				get_mode(double zoom)
				{
					return (zoom > ZOOM_HI)
						? MODE_V_HI
						: (zoom > ZOOM_MED)
							? MODE_HI
							: (zoom > ZOOM_LO)
								? MODE_MED
								: (zoom > ZOOM_V_LO)
									? MODE_LOW
									: MODE_V_LOW;
				}

				typedef struct {
				   int first;
				   int last;
				} BlockRange;

				#define FIRST_NOT_VISIBLE 10000
				#define LAST_NOT_VISIBLE (-1)

				#define WF_ACTOR_GET_RECT(A, RECT) \
					*RECT = (WfRectangle){ \
						.left = ((AGlActor*)A)->region.x1, \
						.top = ((AGlActor*)A)->region.y1, \
						.len = agl_actor__width(((AGlActor*)A)), \
						.height = agl_actor__height(((AGlActor*)A)), \
					}

#ifdef LATER
				static double
				wf_actor_samples2gl(double zoom, uint32_t n_samples)
				{
					//zoom is pixels per sample
					return n_samples * zoom;
				}

				static BlockRange
				wf_actor_get_visible_block_range(WfSampleRegion* region, WfRectangle* rect, double zoom, WfViewPort* viewport_px, int textures_size)
				{
					//the region, rect and viewport are passed explictly because different users require slightly different values during transitions.

					BlockRange range = {FIRST_NOT_VISIBLE, LAST_NOT_VISIBLE};

					Mode mode = get_mode(zoom);
					int samples_per_texture = WF_SAMPLES_PER_TEXTURE * (
						mode == MODE_V_LOW
							? WF_MED_TO_V_LOW
							: mode == MODE_LOW ? WF_PEAK_STD_TO_LO : 1);

					double region_inset_px = wf_actor_samples2gl(zoom, region->start);
					double file_start_px = rect->left - region_inset_px;
					double block_wid = wf_actor_samples2gl(zoom, samples_per_texture);
					int region_start_block = region->start / samples_per_texture;

					int region_end_block; {
						float _end_block = ((float)(region->start + region->len)) / samples_per_texture;
						dbg(2, "%s region_start=%Li region_end=%i start_block=%i end_block=%.2f(%.i) n_peak_frames=%i gl->size=%i", modes[mode].name, region->start, ((int)region->start) + region->len, region_start_block, _end_block, (int)ceil(_end_block), textures_size * 256, textures_size);

						// round _down_ as the block corresponds to the _start_ of a section.
						region_end_block = MIN(((int)_end_block), textures_size - 1);
					}

					// find first block
					if(rect->left <= viewport_px->right){

						int b; for(b=region_start_block;b<=region_end_block-1;b++){ // stop before the last block
							int block_start_px = file_start_px + b * block_wid;
							double block_end_px = block_start_px + block_wid;
							dbg(3, "block_pos_px=%i", block_start_px);
							if(block_end_px >= viewport_px->left){
								range.first = b;
								goto next;
							}
						}
						// check last block:
						double block_end_px = file_start_px + wf_actor_samples2gl(zoom, region->start + region->len);
						if(block_end_px >= viewport_px->left){
							range.first = b;
							goto next;
						}

						dbg(1, "region outside viewport? vp_left=%.2f region_end=%.2f", viewport_px->left, file_start_px + region_inset_px + wf_actor_samples2gl(zoom, region->len));
						range.first = FIRST_NOT_VISIBLE;
					}

					next:

					// find last block
					if(rect->left <= viewport_px->right){
						int last = range.last = MIN(range.first + WF_MAX_BLOCK_RANGE, region_end_block);

						g_return_val_if_fail(viewport_px->right - viewport_px->left > 0.01, range);

						//crop to viewport:
						int b; for(b=region_start_block;b<=last-1;b++){ //note we dont check the last block which can be partially outside the viewport
							float block_end_px = file_start_px + (b + 1) * block_wid;
							//dbg(1, " %i: block_px: %.1f --> %.1f", b, block_end_px - (int)block_wid, block_end_px);
							if(block_end_px > viewport_px->right) dbg(2, "end %i clipped by viewport at block %i. vp.right=%.2f block_end=%.1f", region_end_block, MAX(0, b/* - 1*/), viewport_px->right, block_end_px);
							if(block_end_px > viewport_px->right){
								range.last = MAX(0, b/* - 1*/);
								goto out;
							}
						}

						if(file_start_px + wf_actor_samples2gl(zoom, region->start + region->len) < viewport_px->left){
							range.last = LAST_NOT_VISIBLE;
							goto out;
						}

						dbg(2, "end not outside viewport. vp_right=%.2f last=%i", viewport_px->right, region_end_block);
					}

					out: return range;
				}
#endif


static WfSampleRegion
get_random_region(WaveformActor* a, Mode mode, uint32_t max_scroll)
{
	// currently this is very approximate re mode targetting

	int min = (mode == MODE_V_HI)        // sets max zoom
		? 128
		: 4096;

	int len_range = (mode == MODE_V_HI) // sets min zoom
		? 8192
		: 16384;
	uint32_t min_start = MAX(((int)a->region.start) - ((int)max_scroll), 0);
	uint64_t start = g_random_int_range(
		min_start,
						MAX(min_start + 8, // temp fix - range end must be after range start
		MIN(a->region.start + max_scroll, a->waveform->n_frames - len_range + 1)
						)
	);
																	//dbg(0, "start range: %i %i (max_scroll=%u)", min_start, MIN(a->region.start + max_scroll, a->waveform->n_frames - len_range + 1), max_scroll);
	int len = min + g_random_int_range(0, len_range + 1);
	dbg(1, "r=%Lu", start);

	if(start + len > a->waveform->n_frames){
		// the above calculation failed
		if(len < a->waveform->n_frames) start = a->waveform->n_frames - len;
	}

	return (WfSampleRegion){start, len};
}



	typedef struct {
		int test_idx;
		int n;
		int iter;
		int wait_count;
		void (*next)();
		uint64_t start;
	} C;

		bool _check_scroll(gpointer _c)
		{
			C* c = _c;

			/* private!
			WfAnimatable* animatable = &a[0]->priv->animatable.start;
			assert_and_stop(animatable->val.i == *animatable->model_val.i, "animation not finished");
			*/

			if(g_list_length(((AGlActor*)a[0])->transitions)) return TIMER_CONTINUE; // not yet ready

			c->wait_count++;

			WfSampleRegion* region = &a[0]->region;

			assert_and_stop((region->start == c->start), "region");

			const int samples_per_texture = WF_SAMPLES_PER_TEXTURE;// * (resolution == 1024 ? WF_PEAK_STD_TO_LO : 1);
			int region_start_block = region->start / samples_per_texture;
			//WfTextureHi* texture = g_hash_table_lookup(w[0]->textures_hi->textures, &region_start_block);
			//assert_and_stop(texture, "texture loaded %i", region_start_block);

			WfAudioData* audio = &w[0]->priv->audio;
			assert_and_stop(audio->n_blocks, "n_blocks not set");

			WfBuf16* buf = audio->buf16[region_start_block];
			WfWorker* worker = &wf->audio_worker;
			int n_jobs = g_list_length(worker->jobs);
			if(n_jobs || !buf){
				if(c->wait_count < 30) return TIMER_CONTINUE;
			}

			//assert_and_stop(c->wait_count < 30, "timeout loading blocks");
			assert_and_stop(buf, "buf is empty %i", region_start_block);
			assert_and_stop(!n_jobs, "jobs still pending: %i", n_jobs);

			if(++c->iter < c->n){
				c->next(c);
			}else{
				WF_TEST_FINISH_TIMER_STOP;
			}

			return TIMER_STOP;
		}

	void next_scroll(C* c)
	{
		c->wait_count = 0;

		dbg(0, "------------------------- %i", c->iter);
		WfSampleRegion region = get_random_region(a[0], MODE_V_HI, UINT_MAX);
		c->start = region.start;

		int i; for(i=0;i<G_N_ELEMENTS(a)&&a[i];i++)
			wf_actor_set_region(a[i], &region);

		g_timeout_add(200, _check_scroll, c);
	}

void
test_scroll()
{
	START_LONG_TEST;

	C* c = WF_NEW(C,
		.test_idx = __test_idx,
		.n = 256,
		.next = next_scroll,
	);

	{
		// test the number of blocks is being calculated correctly.

#if 0 // this test is failing. its not clear what the intention is.
		AGlActor* actor = (AGlActor*)a[0];

		int r = w[0]->n_frames - REGION_LEN - 1;
		WfSampleRegion region = {r, REGION_LEN};
		double zoom = agl_actor__width(actor) / a[0]->region.len;
		WfViewPort viewport = {
			.left   = actor->region.x1,
			.top    = actor->region.y1,
			.right  = actor->region.x2,
			.bottom = actor->region.y2
		};
		WfRectangle rect; WF_ACTOR_GET_RECT(a[0], &rect);
		BlockRange range = wf_actor_get_visible_block_range(&region, &rect, zoom, &viewport, a[0]->waveform->priv->n_blocks);
		assert((range.last == waveform_get_n_audio_blocks(w[0]) - 1), "bad block_num %i / %i", range.last, waveform_get_n_audio_blocks(w[0]));
#endif
	}

	next_scroll(c);
}


typedef struct {
	int            test_idx;
	int            n;
	int            iter;
	int            wait_count;
	WfSampleRegion region[2];
	void (*next)();
} C3;

		bool _hi_check_scroll(gpointer data)
		{
			C3* c = data;

			/* private!
			WfAnimatable* animatable = &a[0]->priv->animatable.start;
			assert_and_stop(animatable->val.i == *animatable->model_val.i, "animation not finished");
			*/

			if(g_list_length(((AGlActor*)a[0])->transitions)) return TIMER_CONTINUE; // not yet ready

			c->wait_count++;

			WfSampleRegion* _region = &a[0]->region;

			assert_and_stop((_region->start == c->region[0].start), "region");

			const int samples_per_texture = WF_SAMPLES_PER_TEXTURE;// * (resolution == 1024 ? WF_PEAK_STD_TO_LO : 1);
			int region_start_block = _region->start / samples_per_texture;
			//WfTextureHi* texture = g_hash_table_lookup(w[0]->textures_hi->textures, &region_start_block);
			//assert_and_stop(texture, "texture loaded %i", region_start_block);

			WfAudioData* audio = &w[0]->priv->audio;
			assert_and_stop(audio->n_blocks, "n_blocks not set");

			WfBuf16* buf = audio->buf16[region_start_block];
			int n_jobs = g_list_length(wf->audio_worker.jobs);
			if(n_jobs || !buf){
				if(c->wait_count < 30) return TIMER_CONTINUE;
			}

			//assert_and_stop(wait_count < 30, "timeout loading blocks");
			assert_and_stop(buf, "buf is empty %i", region_start_block);
			assert_and_stop(!n_jobs, "jobs still pending: %i", n_jobs);

			if(++c->iter < c->n){
				c->next(c);
			}else{
				/*
				hi_ng_cache_print();
				texture_cache_print();
				*/
				WF_TEST_FINISH_TIMER_STOP;
			}

			return G_SOURCE_REMOVE;
		}

	void hi_next_scroll(C3* c)
	{
		static int timeouts[] =  {1000,   700,   600,    500,    400,      300,     100};
		static uint64_t move[] = {4096, 16384, 65536, 262144, 1048576, 4194304, INT_MAX};

		c->wait_count = 0;
		int stage = MIN(G_N_ELEMENTS(move) - 1, c->iter / 8);
		dbg(0, "------------------------- %i %i", c->iter, stage);

		int i; for(i=0;i<G_N_ELEMENTS(a);i++){
			c->region[i] = get_random_region(a[i], MODE_HI, move[stage]);
			wf_actor_set_region(a[i], &c->region[i]);
		}

		g_timeout_add(timeouts[stage], _hi_check_scroll, c);
	}

void
test_hi_double()
{
	START_LONG_TEST;

	void add_actor(int i)
	{
		{
			char* filename = find_wav(WAV2);
			assert(filename, "failed to find wav: %s", WAV2)
			// TODO this is very slow if peakfile not present
			w[i] = waveform_load_new(filename);
			wf_free(filename);
		}

		a[i] = wf_canvas_add_new_actor(wfc, w[i]);
		assert(a[i], "failed to create actor");

		wf_actor_set_region(a[i], &(WfSampleRegion){0, 4096 * 256});
		wf_actor_set_colour(a[i], 0x66eeffff);
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)a[i]);
		on_allocate(canvas, &canvas->allocation, NULL);
	}
	add_actor(1);

	hi_next_scroll(WF_NEW(C3,
		.test_idx = __test_idx,
		.n = 256,
		.next = hi_next_scroll,
	));
}


	static Mode mode = MODE_LOW; // TODO support other render modes
	static int len;

		static WaveformBlock wb;

		gboolean check_not_in_cache(gpointer _c)
		{
			WfTest* c = _c;

			int t = texture_cache_lookup(GL_TEXTURE_1D, wb);
			assert_and_stop((t == -1), "cache not cleared: lookup got texture: %i", t);

			texture_cache_print();

			WF_TEST_FINISH_TIMER_STOP;
		}

		static bool check_in_cache(gpointer _c)
		{
			WfTest* c = _c;

			WaveformPrivate* _w = w[0]->priv;

			WfSampleRegion* region = &a[0]->region;
			assert_and_stop((region->start == 0), "region start");

			dbg(0, "%s %p", modes[MODE_V_LOW].name, _w->render_data[MODE_V_LOW]);
			dbg(0, "%s %p", modes[MODE_LOW].name, _w->render_data[MODE_LOW]);
			dbg(0, "%s %p", modes[MODE_MED].name, _w->render_data[MODE_MED]);

			assert_and_stop(_w->render_data[mode], "%s mode not initialised", modes[mode].name);
			assert_and_stop(_w->render_data[mode]->n_blocks, "%s n_blocks not set", modes[mode].name);

#ifdef USE_FBO
			int b = 0;

			WfGlBlock* blocks = (WfGlBlock*)_w->render_data[mode];
			guint tex = blocks->peak_texture[WF_LEFT ].main[b];
			// with shaders not available, the peak_texture will contain a 2D texture
			assert_and_stop(tex, "main texture not set");
			assert_and_stop(glIsTexture(tex), "texture is not texture");
			assert_and_stop((!blocks->peak_texture[WF_RIGHT].main[b]), "only one texture should be set?");

			dbg(0, "fbo=%p", ((WfGlBlock*)_w->render_data[mode])->fbo[b]);
#else
			// This will fail if the cache size is too small to fit all low_res blocks
			// In fact with multiple v long wavs, is almost guaranteed to fail.
			// [should be better now with addition of MODE_V_LOW]

			int t = texture_cache_lookup(GL_TEXTURE_1D, wb);
			assert_and_stop((t != -1), "block 0 not found in cache");
#endif

			int i; for(i=0;i<G_N_ELEMENTS(a);i++){
				if(a[i]){
					a[i] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[1]), NULL);
				}
			}
			for(i=0;i<G_N_ELEMENTS(w);i++){
				if(w[i]){
					g_object_unref(w[i]);
					w[i] = NULL;
				}
			}

			g_timeout_add(2000, check_not_in_cache, c);
			return TIMER_STOP;
		}

	static void on_ready (gpointer user_data, gpointer _)
	{
		WfTest* c = _;

		wf_worker_cancel_jobs(&wf->audio_worker, a[0]->waveform);

		// TODO support gl2 mode also
#if 0
		assert(!agl_get_instance()->use_shaders, "shaders must be disabled for this test");
#else
		if(agl_get_instance()->use_shaders){
			gwarn("test only suports GL 1");
			WF_TEST_FINISH;
		}
#endif
		wb = (WaveformBlock){a[0]->waveform, 0 | WF_TEXTURE_CACHE_LORES_MASK};

		void set_res()
		{
			int i; for(i=0;i<G_N_ELEMENTS(a)&&a[i];i++)
				wf_actor_set_region(a[i], &(WfSampleRegion){
					0,
					MIN(len, a[i]->waveform->n_frames - 1)
				});
		}
		set_res();

		g_timeout_add(5000, check_in_cache, c);
	}

void
test_add_remove()
{
	// check the texture is cleared of dead Waveform's in LOW res.

	WfTest* t = NEW_TEST();

	// calculate the region length needed to ensure that the view is of the above mode type.
	switch(mode){
		case MODE_LOW:
			len = 2 * GL_WIDTH * WF_PEAK_RATIO * WF_PEAK_STD_TO_LO;
			break;
		case MODE_V_LOW:
			len = 2 * GL_WIDTH * WF_PEAK_RATIO * WF_MED_TO_V_LOW;
			break;
		default:
			break;
	}

	files_created = true; // hack needed if earlier tests are disabled

	am_promise_add_callback(ready, on_ready, t);
}


	static bool _finish(gpointer _c)
	{
		WfTest* c = _c;
		//assert_and_stop(!g_unlink(WAV1), "file delete failed");

		WF_TEST_FINISH_TIMER_STOP;
	}

void
finish()
{
	g_timeout_add(5000, _finish, NEW_TEST());
}


static void
gl_init()
{
	if(gl_initialised) return;

	gl_initialised = true;
}



	static bool _on_canvas_realise(gpointer user_data)
	{
		if(!files_created){
			return TIMER_CONTINUE;
		}

		//agl_get_instance()->pref_use_shaders = false;

		gl_init();

		wfc = wf_context_new((AGlActor*)(scene = (AGlScene*)agl_actor__new_root(canvas)));

		g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);

		{
			char* filename = find_wav(WAV1);
			w[0] = waveform_load_new(filename);
			wf_free(filename);
		}

		WfSampleRegion region[] = {
			{0,    REGION_LEN        },
			{0,    w[0]->n_frames - 1},
		};

		uint32_t colours[4][2] = {
			{0xffffff77, 0x0000ffff},
			{0x66eeffff, 0x0000ffff},
		};

		int i; for(i=0;i<1;i++){ // initially only create 1 actor
			agl_actor__add_child((AGlActor*)scene, (AGlActor*)(a[i] = wf_canvas_add_new_actor(wfc, w[0])));

			wf_actor_set_region(a[i], &region[i]);
			wf_actor_set_colour(a[i], colours[i][0]);
		}

		on_allocate(canvas, &canvas->allocation, user_data);

		am_promise_resolve(ready, NULL);

		return TIMER_STOP;
	}

static void
on_canvas_realise(GtkWidget* _canvas, gpointer user_data)
{
	if(wfc) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	g_timeout_add(200, _on_canvas_realise, user_data);
}


static void
on_allocate(GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	if(!gl_initialised) return;

	g_return_if_fail(scene);
	g_return_if_fail(allocation);

	((AGlActor*)scene)->region.x2 = allocation->width;
	((AGlActor*)scene)->region.y2 = allocation->height;

	// Optimise drawing by telling the canvas which area is visible
	wf_context_set_viewport(wfc, &(WfViewPort){0, 0, GL_WIDTH, allocation->height});

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			0.0,
			i * allocation->height / G_N_ELEMENTS(a),
			GL_WIDTH,
			allocation->height / G_N_ELEMENTS(a) * 0.95
		});
}


uint64_t
get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


