/*
  Tests for opengl textures at hi res using large files

  Automated but opens a gui window.

  Shows many different views at hi res mode to check that the cache
  works when full and that the correct data for the view is available.

  ---------------------------------------------------------------

  copyright (C) 2013 Tim Orford <tim@orford.org>

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
#include "test/common.h"

#define GL_WIDTH 512.0
#define GL_HEIGHT 128.0
#define VBORDER 8
#define bool gboolean
#define WAV1 "test/data/large1.wav"

GdkGLConfig*    glconfig       = NULL;
GdkGLDrawable*  gl_drawable    = NULL;
GdkGLContext*   gl_context     = NULL;
static bool     gl_initialised = false;
GtkWidget*      canvas         = NULL;
WaveformCanvas* wfc            = NULL;
Waveform*       w1             = NULL;
WaveformActor*  a[]            = {NULL};//, NULL, NULL, NULL};
bool            files_created  = false;

static void setup_projection   (GtkWidget*);
static void draw               (GtkWidget*);
static bool on_expose          (GtkWidget*, GdkEventExpose*, gpointer);
static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
uint64_t    get_time           ();

TestFn create_files, test_shown, test_hires, scroll, finish;

gpointer tests[] = {
	create_files,
	test_shown,
	test_hires,
	scroll,
	finish,
};


int
main (int argc, char *argv[])
{
	wf_debug = 1;
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

	assert(wfc, "canvas not created");

	FINISH_TEST;
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

		int i; for(i=0;i<G_N_ELEMENTS(a);i++)
			wf_actor_set_region(a[i], &(WfSampleRegion){
				0,
				REGION_LEN
			});
	}

	set_region();

	gboolean check_zoom(gpointer data)
	{
		//WfRectangle* rect = &a[0]->rect;
		WfSampleRegion* region = &a[0]->region;

		assert_and_stop((region->len == REGION_LEN), "region length");

		FINISH_TEST_TIMER_STOP;
	}

	g_timeout_add(1000, check_zoom, NULL);
}

				// duplicates of private stuff from actor.c

				#define ZOOM_HI  (1.0/  16)
				#define ZOOM_MED (1.0/ 256) // px_per_sample - transition point from std to hi-res mode.
				#define ZOOM_LO  (1.0/4096) // px_per_sample - transition point from low-res to std mode.

				typedef enum
				{
					MODE_LOW = 0,
					MODE_MED,
					MODE_HI,
					MODE_V_HI,
					N_MODES
				} Mode;

				static inline int
				get_resolution(double zoom)
				{
					return (zoom > ZOOM_HI)
						? 1
						: (zoom > ZOOM_MED)
							? 16
							: (zoom > ZOOM_LO)
								? 256
								: 1024;
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
								: MODE_LOW;
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

					//double region_inset_px = wf_actor_samples2gl(zoom, region->start);
					//double file_start_px = rect->left - region_inset_px;
					//double block_wid = wf_actor_samples2gl(zoom, samples_per_texture);
					//dbg(1, "vp->right=%.2f", viewport_px->right);
					int region_start_block = region->start / samples_per_texture;
					float _end_block = ((float)(region->start + region->len)) / samples_per_texture;
					dbg(2, "%s region_start=%Li region_end=%i start_block=%i end_block=%.2f(%.i) n_peak_frames=%i gl->size=%i", resolution == 1024 ? "LOW" : resolution == 256 ? "STD" : "HI", region->start, ((int)region->start) + region->len, region_start_block, _end_block, (int)ceil(_end_block), textures->size * 256, textures->size);

					//we round _down_ as the block corresponds to the _start_ of a section.
					int region_end_block = MIN(_end_block, textures->size - 1);

					dbg(2, "end not outside viewport. vp_right=%.2f last=%i", viewport_px->right, region_end_block);
					return region_end_block;
				}

void
scroll()
{
	START_TEST;

	static int n = 256;
	static int iter = 0;
	static uint64_t start = 0;
	static void (*next)();
	static int wait_count = 0;

	//long test. have to disable timeout.
	g_source_remove (app.timeout);
	app.timeout = 0;

	{
		// test the number of blocks is being calculated correctly.

		int r = w1->n_frames - REGION_LEN - 1;
		WfSampleRegion region = {r, REGION_LEN};
		double zoom = a[0]->rect.len / a[0]->region.len;
		int mode = get_mode(zoom);
		WfGlBlock* textures = mode == MODE_LOW ? w1->textures_lo : w1->textures;
		int b = wf_actor_get_last_visible_block(&region, &a[0]->rect, zoom, a[0]->canvas->viewport, textures);
		assert((b == waveform_get_n_audio_blocks(w1) - 1), "bad block_num %i / %i", b, waveform_get_n_audio_blocks(w1));
	}

	void next_scroll()
	{
		wait_count = 0;

		start = g_random_int_range(0, w1->n_frames - REGION_LEN + 1);
		int len = 128 + g_random_int_range(0, 8192 + 1);
		dbg(0, "-------------------------");
		dbg(0, "r=%Lu", start);


		int i; for(i=0;i<G_N_ELEMENTS(a);i++)
			wf_actor_set_region(a[i], &(WfSampleRegion){start, len});

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
			//WfTextureHi* texture = g_hash_table_lookup(w1->textures_hi->textures, &region_start_block);
			//assert_and_stop(texture, "texture loaded %i", region_start_block);

			WfAudioData* audio = w1->priv->audio_data;
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

	START_DRAW {

		if(!agl_shaders_supported()) gwarn("shaders not supported");

	} END_DRAW

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND); glEnable(GL_DEPTH_TEST); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix(); /* modelview matrix */
		int i; for(i=0;i<G_N_ELEMENTS(a);i++) if(a[i]) wf_actor_paint(a[i]);
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

	START_DRAW {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw(widget);

		gdk_gl_drawable_swap_buffers(gl_drawable);
	} END_DRAW
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

		gl_drawable = gtk_widget_get_gl_drawable(canvas);
		gl_context  = gtk_widget_get_gl_context(canvas);

		//agl_get_instance()->pref_use_shaders = false;

		gl_init();

		wfc = wf_canvas_new(gl_context, gl_drawable);

		char* filename = g_build_filename(g_get_current_dir(), WAV1, NULL);
		w1 = waveform_load_new(filename);
		g_free(filename);

		WfSampleRegion region[] = {
			{0,            REGION_LEN    },
			//{0,            n_frames / 2},
			//{n_frames / 4, n_frames / 4},
			//{n_frames / 2, n_frames / 2},
		};

		uint32_t colours[4][2] = {
			{0xffffff77, 0x0000ffff},
			//{0x66eeffff, 0x0000ffff},
			//{0xffdd66ff, 0x0000ffff},
			//{0x66ff66ff, 0x0000ffff},
		};

		int i; for(i=0;i<G_N_ELEMENTS(a);i++){
			a[i] = wf_canvas_add_new_actor(wfc, w1);

			wf_actor_set_region(a[i], &region[i]);
			wf_actor_set_colour(a[i], colours[i][0], colours[i][1]);
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
		if(a[i]) wf_actor_allocate(a[i], &(WfRectangle){
			0.0,
			i * GL_HEIGHT / 4,
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


