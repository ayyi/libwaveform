/*
  Tests for opengl textures using large files

  Automated but opens a gui window.

  1-test basic hires
  2-test textures are loaded when scrolling
      Shows many different views at using all modes to check that the cache
      works when full and that the correct data for the view is available.
  3-test texture cache is emptied at lowres when the Waveform is free'd.

  ---------------------------------------------------------------

  copyright (C) 2013-2014 Tim Orford <tim@orford.org>

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
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#include "agl/utils.h"
#include "waveform/waveform.h"
#include "waveform/actor.h"
#include "waveform/audio.h"
#include "waveform/gl_utils.h"
#include "waveform/texture_cache.h"
#include "test/common.h"

extern void texture_cache_print ();
extern void hi_ng_cache_print   ();

#define GL_WIDTH 512.0
#define GL_HEIGHT 128.0
#define VBORDER 8
#define bool gboolean
#define WAV1 "test/data/large1.wav"
#define WAV2 "test/data/large2.wav"

GdkGLConfig*    glconfig       = NULL;
static bool     gl_initialised = false;
GtkWidget*      canvas         = NULL;
WaveformCanvas* wfc            = NULL;
Waveform*       w[2]           = {NULL,};
WaveformActor*  a[2]           = {NULL,};
bool            files_created  = false;

static void setup_projection   (GtkWidget*);
static void draw               (GtkWidget*);
static bool on_expose          (GtkWidget*, GdkEventExpose*, gpointer);
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


int
main (int argc, char *argv[])
{
	wf_debug = 0;
	test_init(tests, G_N_ELEMENTS(tests));

	gtk_init(&argc, &argv);
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		gerr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
	}

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	canvas = gtk_drawing_area_new();
#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus     (canvas, true);
#endif
	gtk_widget_set_size_request  (canvas, GL_WIDTH, GL_HEIGHT);
	gtk_widget_set_gl_capability (canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events        (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_container_add            ((GtkContainer*)window, (GtkWidget*)canvas);
	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose_event",  G_CALLBACK(on_expose), NULL);

	gtk_widget_show_all(window);

	gboolean window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


void
create_files()
{
	START_TEST;
	reset_timeout(60000);

	if(!g_file_test(WAV1, G_FILE_TEST_EXISTS)) create_large_file(WAV1);
	files_created = true;

	FINISH_TEST;
}


void
test_shown()
{
	START_TEST;

	assert(GTK_WIDGET_REALIZED(canvas), "widget not realised");
	assert(wfc, "canvas not created");
	assert(a[0], "actor not created");

	bool _test_shown(gpointer _)
	{
#ifdef AGL_ACTOR_RENDER_CACHE
		AGlActor* actor = (AGlActor*)a[0];
		if(actor->cache.valid){
		}else{
			extern bool wf_actor_test_is_not_blank(WaveformActor*);
			assert_and_stop(wf_actor_test_is_not_blank(a[0]), "output is blank");
		}
#endif
		FINISH_TEST_TIMER_STOP;
	}

	g_timeout_add(500, _test_shown, NULL);
}


#define REGION_LEN 2048

void
test_hires()
{
	START_TEST;

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

	gboolean check_zoom(gpointer data)
	{
		WfSampleRegion* region = &a[0]->region;

		assert_and_stop((region->len == REGION_LEN), "region length");

		FINISH_TEST_TIMER_STOP;
	}

	g_timeout_add(1000, check_zoom, NULL);
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

void
test_scroll()
{
	START_LONG_TEST;

	static const int n = 256;
	static int iter = 0;
	static uint64_t start = 0;
	static void (*next)();
	static int wait_count = 0;

	{
		// test the number of blocks is being calculated correctly.

		int r = w[0]->n_frames - REGION_LEN - 1;
		WfSampleRegion region = {r, REGION_LEN};
		double zoom = a[0]->rect.len / a[0]->region.len;
		BlockRange range = wf_actor_get_visible_block_range(&region, &a[0]->rect, zoom, a[0]->canvas->viewport, a[0]->waveform->priv->n_blocks);
		assert((range.last == waveform_get_n_audio_blocks(w[0]) - 1), "bad block_num %i / %i", range.last, waveform_get_n_audio_blocks(w[0]));
	}

	void next_scroll()
	{
		wait_count = 0;

		dbg(0, "------------------------- %i", iter);
		WfSampleRegion region = get_random_region(a[0], MODE_V_HI, UINT_MAX);
		start = region.start;

		int i; for(i=0;i<G_N_ELEMENTS(a)&&a[i];i++)
			wf_actor_set_region(a[i], &region);

		gboolean _check_scroll(gpointer data)
		{
			/* private!
			WfAnimatable* animatable = &a[0]->priv->animatable.start;
			assert_and_stop(animatable->val.i == *animatable->model_val.i, "animation not finished");
			*/

			//GList* transitions = wf_actor_get_transitions(a[0]);
			//dbg(0, "n_transitions=%i", g_list_length(transitions));
			if(g_list_length(wf_actor_get_transitions(a[0]))) return TIMER_CONTINUE; // not yet ready

			wait_count++;

			WfSampleRegion* region = &a[0]->region;

			assert_and_stop((region->start == start), "region");

			const int samples_per_texture = WF_SAMPLES_PER_TEXTURE;// * (resolution == 1024 ? WF_PEAK_STD_TO_LO : 1);
			int region_start_block = region->start / samples_per_texture;
			//WfTextureHi* texture = g_hash_table_lookup(w[0]->textures_hi->textures, &region_start_block);
			//assert_and_stop(texture, "texture loaded %i", region_start_block);

			WfAudioData* audio = w[0]->priv->audio_data;
			assert_and_stop(audio->n_blocks, "n_blocks not set");

			WfBuf16* buf = audio->buf16[region_start_block];
			int n_jobs = g_list_length(wf->jobs);
			if(n_jobs || !buf){
				if(wait_count < 30) return TIMER_CONTINUE;
			}

			//assert_and_stop(wait_count < 30, "timeout loading blocks");
			assert_and_stop(buf, "buf is empty %i", region_start_block);
			assert_and_stop(!n_jobs, "jobs still pending: %i", n_jobs);

			if(++iter < n){
				next();
			}else{
				FINISH_TEST_TIMER_STOP;
			}

			return TIMER_STOP;
		}

		g_timeout_add(200, _check_scroll, NULL);
	}
	next = next_scroll;

	next_scroll();
}


void
test_hi_double()
{
	START_LONG_TEST;

	void add_actor(int i)
	{
		char* filename = g_build_filename(g_get_current_dir(), WAV2, NULL);
		// TODO this is be very slow if peakfile not present
		w[i] = waveform_load_new(filename);
		g_free(filename);

		a[i] = wf_canvas_add_new_actor(wfc, w[i]);
		assert(a[i], "failed to create actor");

		wf_actor_set_region(a[i], &(WfSampleRegion){0, 4096 * 256});
		wf_actor_set_colour(a[i], 0x66eeffff);
		on_allocate(canvas, NULL, NULL);
	}
	add_actor(1);

	static const int n = 256;
	static int iter = 0;
	static WfSampleRegion region[2];
	static void (*next)();
	static int wait_count = 0;
	static int timeouts[] =  {1000,   700,   600,    500,    400,      300,     100};
	static uint64_t move[] = {4096, 16384, 65536, 262144, 1048576, 4194304, INT_MAX};

	void next_scroll()
	{
		wait_count = 0;
		int stage = MIN(G_N_ELEMENTS(move) - 1, iter / 8);
		dbg(0, "------------------------- %i %i", iter, stage);

		int i; for(i=0;i<G_N_ELEMENTS(a);i++){
			region[i] = get_random_region(a[i], MODE_HI, move[stage]);
			wf_actor_set_region(a[i], &region[i]);
		}

		gboolean _check_scroll(gpointer data)
		{
			/* private!
			WfAnimatable* animatable = &a[0]->priv->animatable.start;
			assert_and_stop(animatable->val.i == *animatable->model_val.i, "animation not finished");
			*/

			//GList* transitions = wf_actor_get_transitions(a[0]);
			//dbg(0, "n_transitions=%i", g_list_length(transitions));
			if(g_list_length(wf_actor_get_transitions(a[0]))) return TIMER_CONTINUE; // not yet ready

			wait_count++;

			WfSampleRegion* _region = &a[0]->region;

			assert_and_stop((_region->start == region[0].start), "region");

			const int samples_per_texture = WF_SAMPLES_PER_TEXTURE;// * (resolution == 1024 ? WF_PEAK_STD_TO_LO : 1);
			int region_start_block = _region->start / samples_per_texture;
			//WfTextureHi* texture = g_hash_table_lookup(w[0]->textures_hi->textures, &region_start_block);
			//assert_and_stop(texture, "texture loaded %i", region_start_block);

			WfAudioData* audio = w[0]->priv->audio_data;
			assert_and_stop(audio->n_blocks, "n_blocks not set");

			WfBuf16* buf = audio->buf16[region_start_block];
			int n_jobs = g_list_length(wf->jobs);
			if(n_jobs || !buf){
				if(wait_count < 30) return TIMER_CONTINUE;
			}

			//assert_and_stop(wait_count < 30, "timeout loading blocks");
			assert_and_stop(buf, "buf is empty %i", region_start_block);
			assert_and_stop(!n_jobs, "jobs still pending: %i", n_jobs);

			if(++iter < n){
				next();
			}else{
				/*
				hi_ng_cache_print();
				texture_cache_print();
				*/
				FINISH_TEST_TIMER_STOP;
			}

			return TIMER_STOP;
		}

		g_timeout_add(timeouts[stage], _check_scroll, NULL);
	}

	next = next_scroll;

	next_scroll();
}


void
test_add_remove()
{
	// check the texture is cleared of dead Waveform's in LOW res.

	START_TEST;

	wf_cancel_jobs(a[0]->waveform);

	void set_low_res()
	{
		int i; for(i=0;i<G_N_ELEMENTS(a)&&a[i];i++)
			wf_actor_set_region(a[i], &(WfSampleRegion){
				0,
				a[i]->waveform->n_frames - 1             //TODO this won't neccesarily be LOW_RES for all wav's
			});
	}
	set_low_res();

	static WaveformBlock wb; wb = (WaveformBlock){a[0]->waveform, 0 | WF_TEXTURE_CACHE_LORES_MASK};

	gboolean check_not_in_cache(gpointer user_data)
	{
		int t = texture_cache_lookup(GL_TEXTURE_1D, wb);
		assert_and_stop((t == -1), "cache not cleared: lookup got texture: %i", t);

		texture_cache_print();

		FINISH_TEST_TIMER_STOP;
	}

	gboolean check_in_cache(gpointer user_data)
	{
		WaveformPriv* _w = w[0]->priv;

		WfSampleRegion* region = &a[0]->region;
		assert_and_stop((region->start == 0), "region start");

		assert_and_stop(_w->render_data[MODE_LOW], "low res mode not initialised");

		// In fact the 1d textures are no longer kept if rendering from fbo
#ifdef USE_FBO
		assert_and_stop(((WfGlBlock*)_w->render_data[MODE_LOW])->fbo[0] && ((WfGlBlock*)_w->render_data[MODE_LOW])->fbo[0]->texture, "fbo texture");
#else
		// This will fail if the cache size is too small to fit all low_res blocks
		// In fact with multiple v long wavs, is almost guaranteed to fail.

		int t = texture_cache_lookup(GL_TEXTURE_1D, wb);
		assert_and_stop((t != -1), "block 0 not found in cache");
#endif

		int i; for(i=0;i<G_N_ELEMENTS(a);i++){
			wf_canvas_remove_actor(wfc, a[i]);
			a[i] = 0;
		}
		for(i=0;i<G_N_ELEMENTS(w);i++){
			if(w[i]){
				g_object_unref(w[i]);
				w[i] = NULL;
			}
		}

		g_timeout_add(2000, check_not_in_cache, NULL);
		return TIMER_STOP;
	}

	g_timeout_add(5000, check_in_cache, NULL);
}


void
finish()
{
	START_TEST;

	gboolean _finish(gpointer data)
	{
		//assert_and_stop(!g_unlink(WAV1), "file delete failed");

		FINISH_TEST_TIMER_STOP;
	}
	g_timeout_add(5000, _finish, NULL);
}


static void
gl_init()
{
	if(gl_initialised) return;

	gl_initialised = true;
}


static void
setup_projection(GtkWidget* widget)
{
	int vx = 0;
	int vy = 0;
	int vw = widget->allocation.width;
	int vh = widget->allocation.height;
	glViewport(vx, vy, vw, vh);
	dbg (0, "viewport: %i %i %i %i", vx, vy, vw, vh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double hborder = GL_WIDTH / 32;

	double left = -hborder;
	double right = GL_WIDTH + hborder;
	double bottom = GL_HEIGHT + VBORDER;
	double top = -VBORDER;
	glOrtho (left, right, bottom, top, 10.0, -100.0);
}


static void
draw(GtkWidget* widget)
{
	glEnable(GL_BLEND); glEnable(GL_DEPTH_TEST); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix(); /* modelview matrix */
		int i; for(i=0;i<G_N_ELEMENTS(a);i++) if(a[i]) ((AGlActor*)a[i])->paint((AGlActor*)a[i]);
	glPopMatrix();

#undef SHOW_BOUNDING_BOX
#ifdef SHOW_BOUNDING_BOX
	glPushMatrix(); /* modelview matrix */
		glTranslatef(0.0, 0.0, 0.0);
		glNormal3f(0, 0, 1);
		glDisable(GL_TEXTURE_2D);
		glLineWidth(1);

		int w = GL_WIDTH;
		int h = GL_HEIGHT/2;
		glBegin(GL_QUADS);
		glVertex3f(-0.2, -0.2, 1); glVertex3f(w, -0.2, 1);
		glVertex3f(w, h, 1);       glVertex3f(-0.2, h, 1);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	glPopMatrix();
#endif
}


static gboolean
on_expose(GtkWidget* widget, GdkEventExpose* event, gpointer user_data)
{
	if(!GTK_WIDGET_REALIZED(widget)) return TRUE;
	if(!gl_initialised) return TRUE;

	AGL_ACTOR_START_DRAW(wfc->root) {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw(widget);

		gdk_gl_drawable_swap_buffers(wfc->root->gl.gdk.drawable);
	} AGL_ACTOR_END_DRAW(wfc->root)
	return TRUE;
}


static void
on_canvas_realise(GtkWidget* _canvas, gpointer user_data)
{
	if(wfc) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	bool _on_canvas_realise(gpointer user_data)
	{
		if(!files_created){
			return TIMER_CONTINUE;
		}

		//agl_get_instance()->pref_use_shaders = false;

		gl_init();

		wfc = wf_canvas_new((AGlRootActor*)agl_actor__new_root(canvas));

		char* filename = g_build_filename(g_get_current_dir(), WAV1, NULL);
		w[0] = waveform_load_new(filename);
		g_free(filename);

		WfSampleRegion region[] = {
			{0,            REGION_LEN    },
			{0,            w[0]->n_frames - 1    },
		};

		uint32_t colours[4][2] = {
			{0xffffff77, 0x0000ffff},
			{0x66eeffff, 0x0000ffff},
		};

		int i; for(i=0;i<1;i++){ // initially only create 1 actor
			a[i] = wf_canvas_add_new_actor(wfc, w[0]);

			wf_actor_set_region(a[i], &region[i]);
			wf_actor_set_colour(a[i], colours[i][0]);
		}

		on_allocate(canvas, &canvas->allocation, user_data);

		//allow the WaveformCanvas to initiate redraws
		void _on_wf_canvas_requests_redraw(WaveformCanvas* wfc, gpointer _)
		{
			gdk_window_invalidate_rect(canvas->window, NULL, false);
		}
		wfc->draw = _on_wf_canvas_requests_redraw;

		return TIMER_STOP;
	}
	g_timeout_add(200, _on_canvas_realise, user_data);
}


static void
on_allocate(GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	if(!gl_initialised) return;

	setup_projection(widget);

	//optimise drawing by telling the canvas which area is visible
	wf_canvas_set_viewport(wfc, &(WfViewPort){0, 0, GL_WIDTH, GL_HEIGHT});

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			0.0,
			i * GL_HEIGHT / G_N_ELEMENTS(a),
			GL_WIDTH,
			GL_HEIGHT / G_N_ELEMENTS(a) * 0.95
		});
}


uint64_t
get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


