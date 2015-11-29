/*
  For debugging purposes, a fixed medium-res waveform display
  is shown together with all the individual textures in use.

  **** This is no longer being used as it appears to require
       a shader that was removed

  ---------------------------------------------------------------

  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

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
#define USE_SHADERS true

#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "agl/utils.h"
#include "waveform/waveform.h"
#include "waveform/texture_cache.h"
#include "test/ayyi_utils.h"

struct _app
{
	int timeout;
} app;

#define GL_WIDTH 256.0
#define GL_HEIGHT 256.0
#define VBORDER 8

AGl*            agl            = NULL;
GdkGLConfig*    glconfig       = NULL;
static bool     gl_initialised = false;
GtkWidget*      canvas         = NULL;
WaveformCanvas* wfc            = NULL;
Waveform*       w1             = NULL;
//Waveform*       w2             = NULL;
WaveformActor*  a[]            = {NULL};//, NULL, NULL, NULL};
float           zoom           = 1.0;
float           vzoom          = 1.0;
gpointer        tests[]        = {};

static void set_log_handlers   ();
static void setup_projection   (GtkWidget*);
static void draw               (GtkWidget*);
static bool on_expose          (GtkWidget*, GdkEventExpose*, gpointer);
static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
static void start_zoom         (float target_zoom);
static void vzoom_up           ();
static void vzoom_down         ();
static void toggle_animate     ();
uint64_t    get_time           ();


int
main (int argc, char *argv[])
{
	set_log_handlers();
	agl = agl_get_instance();

	wf_debug = 1;

	memset(&app, 0, sizeof(struct _app));

	gtk_init(&argc, &argv);
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		gerr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
	}

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	canvas = gtk_drawing_area_new();
#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus     (canvas, true);
#endif
	gtk_widget_set_size_request  (canvas, 320, 128);
	gtk_widget_set_gl_capability (canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events        (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);
	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose_event",  G_CALLBACK(on_expose), NULL);

	gtk_widget_show_all(window);

	gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		switch(event->keyval){
			case 61:
				start_zoom(zoom * 1.5);
				break;
			case 45:
				start_zoom(zoom / 1.5);
				break;
			case KEY_Left:
			case KEY_KP_Left:
				dbg(0, "left");
				//waveform_view_set_start(waveform, waveform->start_frame - 8192 / waveform->zoom);
				break;
			case KEY_Right:
			case KEY_KP_Right:
				dbg(0, "right");
				//waveform_view_set_start(waveform, waveform->start_frame + 8192 / waveform->zoom);
				break;
			case (char)'a':
				toggle_animate();
				break;
			case (char)'w':
				vzoom_up();
				break;
			case (char)'s':
				vzoom_down();
				break;
			case GDK_KP_Enter:
				break;
			case 113:
				exit(EXIT_SUCCESS);
				break;
			case GDK_Delete:
				break;
			default:
				dbg(0, "%i", event->keyval);
				break;
		}
		return TRUE;
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), NULL);

	gboolean window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
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
textured_rect_1d(WaveformActor* actor, guint texture, float x, float y, float w, float h)
{
	agl_use_program((AGlShader*)wfc->shaders.peak);

	int n_channels = 1;
	float ppp = 1.0;
	wfc->shaders.peak->set_uniforms(ppp, y, y + h, 0xffff00ff, n_channels);

	glActiveTexture(WF_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, texture);
	glActiveTexture(WF_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, texture + 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float tex_start = 0.0;
	float tex_end = 1.0;
	glBegin(GL_QUADS);
	glMultiTexCoord2f(WF_TEXTURE0, tex_start, 1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start, 1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start, 1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start, 1.0); glVertex2d(x,     y);
	glMultiTexCoord2f(WF_TEXTURE0, tex_end,   1.0); glMultiTexCoord2f(WF_TEXTURE1, tex_end,   1.0); glMultiTexCoord2f(WF_TEXTURE2, tex_end,   1.0); glMultiTexCoord2f(WF_TEXTURE3, tex_end,   1.0); glVertex2d(x + w, y);
	glMultiTexCoord2f(WF_TEXTURE0, tex_end,   0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_end,   0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_end,   0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_end,   0.0); glVertex2d(x + w, y + h);
	glMultiTexCoord2f(WF_TEXTURE0, tex_start, 0.0); glMultiTexCoord2f(WF_TEXTURE1, tex_start, 0.0); glMultiTexCoord2f(WF_TEXTURE2, tex_start, 0.0); glMultiTexCoord2f(WF_TEXTURE3, tex_start, 0.0); glVertex2d(x,     y + h);
	glEnd();
}


static void
draw(GtkWidget* widget)
{
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

	int tid;
	if((tid = texture_cache_lookup(GL_TEXTURE_1D, (WaveformBlock){w1, 0})) > -1){
		dbg(0, "tid=%i", tid);
		float x = 128.0;
		float y = 128.0;
		float w = 64.0;
		float h = 64.0;

		agl->shaders.plain->uniform.colour = 0xaaaaaaff;
		agl_use_program((AGlShader*)agl->shaders.plain);
		agl_rect(x - 1.0, y - 2.0,     1.0,     h + 4.0);
		agl_rect(x + w,   y - 2.0,     1.0,     h + 4.0);
		agl_rect(x - 1.0, y - 1.0,     w + 2.0, 2.0);
		agl_rect(x - 1.0, y + h + 1.0, w + 2.0, 2.0);

		textured_rect_1d(a[0], tid, x, y, w, h);
	}
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
	} AGL_ACTOR_END_DRAW(wfc->root);
	return TRUE;
}


static void
on_canvas_realise(GtkWidget* _canvas, gpointer user_data)
{
	if(wfc) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	agl_get_instance()->pref_use_shaders = USE_SHADERS;

	gl_init();

	wfc = wf_canvas_new((AGlRootActor*)agl_actor__new_root(canvas));

	char* filename = g_build_filename(g_get_current_dir(), "test/data/mono_1.wav", NULL);
	w1 = waveform_load_new(filename);
	g_free(filename);

	int n_frames = waveform_get_n_frames(w1);

	WfSampleRegion region[] = {
		{0,            n_frames    },
		{0,            n_frames / 2},
		{n_frames / 4, n_frames / 4},
		{n_frames / 2, n_frames / 2},
	};

	uint32_t colours[4][2] = {
		{0xffffff77, 0x0000ffff},
		{0x66eeffff, 0x0000ffff},
		{0xffdd66ff, 0x0000ffff},
		{0x66ff66ff, 0x0000ffff},
	};

	int i; for(i=0;i<G_N_ELEMENTS(a);i++){
		a[i] = wf_canvas_add_new_actor(wfc, w1);

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
}


static void
on_allocate(GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	if(!gl_initialised) return;

	setup_projection(widget);

	//optimise drawing by telling the canvas which area is visible
	wf_canvas_set_viewport(wfc, &(WfViewPort){0, 0, GL_WIDTH, GL_HEIGHT});

	start_zoom(zoom);
}


static void
start_zoom(float target_zoom)
{
	//when zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	PF0;
	zoom = MAX(0.1, target_zoom);

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			0.0,
			i * GL_HEIGHT / 4,
			GL_WIDTH * target_zoom,
			GL_HEIGHT / 4 * 0.95
		});
}


static void
vzoom_up()
{
	vzoom *= 1.1;
	zoom = MIN(vzoom, 100.0);
	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


static void
vzoom_down()
{
	vzoom /= 1.1;
	zoom = MAX(vzoom, 1.0);
	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


static void
toggle_animate()
{
	PF0;
	gboolean on_idle(gpointer _)
	{
		static uint64_t frame = 0;
		static uint64_t t0    = 0;
		if(!frame) t0 = get_time();
		else{
			uint64_t time = get_time();
			if(!(frame % 1000))
				dbg(0, "rate=%.2f fps", ((float)frame) / ((float)(time - t0)) / 1000.0);

			if(!(frame % 8)){
				float v = (frame % 16) ? 2.0 : 1.0/2.0;
				if(v > 16.0) v = 1.0;
				start_zoom(v);
			}
		}
		frame++;
		return IDLE_CONTINUE;
	}
	//g_idle_add(on_idle, NULL);
	//g_idle_add_full(G_PRIORITY_LOW, on_idle, NULL, NULL);
	g_timeout_add(50, on_idle, NULL);
}


void
set_log_handlers()
{
	void log_handler(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data)
	{
	  switch(log_level){
		case G_LOG_LEVEL_CRITICAL:
		  printf("%s %s\n", ayyi_err, message);
		  break;
		case G_LOG_LEVEL_WARNING:
		  printf("%s %s\n", ayyi_warn, message);
		  break;
		default:
		  printf("log_handler(): level=%i %s\n", log_level, message);
		  break;
	  }
	}

	g_log_set_handler (NULL, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);

	char* domain[] = {NULL, "Waveform", "GLib-GObject", "GLib", "Gdk", "Gtk"};
	int i; for(i=0;i<G_N_ELEMENTS(domain);i++){
		g_log_set_handler (domain[i], G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	}
}


uint64_t
get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


