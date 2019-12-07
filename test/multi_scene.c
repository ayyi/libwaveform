/*
  Demonstration of the libwaveform WaveformActor interface

  Several waveforms are drawn onto a single canvas with
  different colours and zoom levels. The canvas can be zoomed
  and panned.

  This example demonstrates the use of two separate agl scenes
  rendered together on the same display.

  ---------------------------------------------------------------

  Copyright (C) 2012-2019 Tim Orford <tim@orford.org>

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
#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "agl/utils.h"
#include "waveform/waveform.h"
#include "test/common.h"

#define GL_WIDTH 256.0
#define GL_HEIGHT 256.0
#define VBORDER 8

GdkGLConfig*     glconfig  = NULL;
GtkWidget*       canvas    = NULL;
AGlRootActor*    scene1    = NULL;
AGlRootActor*    scene2    = NULL;
WaveformContext* wfc       = NULL;
Waveform*        w1        = NULL;
Waveform*        w2        = NULL;
WaveformActor*   a[]       = {NULL, NULL, NULL, NULL};
float            zoom      = 1.0;
float            vzoom     = 1.0;
gpointer         tests[]   = {};

static void setup_projection   (GtkWidget*);
static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
static bool on_expose          (GtkWidget*, GdkEventExpose*, gpointer);
static void start_zoom         (float target_zoom);
uint64_t    get_time           ();

KeyHandler
	zoom_in,
	zoom_out,
	vzoom_up,
	vzoom_down,
	scroll_left,
	scroll_right,
	toggle_animate,
	quit;

Key keys[] = {
	{KEY_Left,      scroll_left},
	{KEY_KP_Left,   scroll_left},
	{KEY_Right,     scroll_right},
	{KEY_KP_Right,  scroll_right},
	{61,            zoom_in},
	{45,            zoom_out},
	{(char)'w',     vzoom_up},
	{(char)'s',     vzoom_down},
	{GDK_KP_Enter,  NULL},
	{(char)'<',     NULL},
	{(char)'>',     NULL},
	{(char)'a',     toggle_animate},
	{GDK_Delete,    NULL},
	{113,           quit},
	{0},
};

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";


int
main (int argc, char *argv[])
{
	set_log_handlers();

	wf_debug = 0;

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(3000, (gpointer)exit, NULL);
				break;
		}
	}

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

	scene1 = (AGlRootActor*)agl_actor__new_root(canvas);
	scene2 = (AGlRootActor*)agl_actor__new_root(canvas);

	char* filename = find_wav("mono_0:10.wav");
	w1 = waveform_load_new(filename);
	g_free(filename);

	agl_get_instance()->pref_use_shaders = USE_SHADERS;

	wfc = wf_context_new((AGlActor*)scene1);

	int n_frames = waveform_get_n_frames(w1);

	WfSampleRegion region[] = {
		{0,            n_frames     - 1},
		{0,            n_frames / 2 - 1},
		{n_frames / 4, n_frames / 4 - 1},
		{n_frames / 2, n_frames / 2 - 1},
	};

	uint32_t colours[4][2] = {
		{0xffffff77, 0x0000ffff},
		{0x66eeffff, 0x0000ffff},
		{0xffdd66ff, 0x0000ffff},
		{0x66ff66ff, 0x0000ffff},
	};

	int i; for(i=0;i<G_N_ELEMENTS(a);i++){
		a[i] = wf_canvas_add_new_actor(wfc, w1);
		agl_actor__add_child((AGlActor*)(i < 2 ? scene1 : scene2), (AGlActor*)a[i]);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(on_expose), NULL);

	gtk_widget_show_all(window);

	add_key_handlers_gtk((GtkWindow*)window, NULL, (Key*)&keys);

	bool window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
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
on_canvas_realise(GtkWidget* _canvas, gpointer user_data)
{
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	on_allocate(canvas, &canvas->allocation, user_data);
}


static void
on_allocate(GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	if(!wfc) return;

	setup_projection(widget);

	((AGlActor*)scene1)->region = (AGlfRegion){0, 0, GL_WIDTH, GL_HEIGHT};
	((AGlActor*)scene2)->region = (AGlfRegion){0, 0, GL_WIDTH, GL_HEIGHT};

	start_zoom(zoom);
}


static gboolean
on_expose(GtkWidget* widget, GdkEventExpose* event, gpointer user_data)
{
	if(!GTK_WIDGET_REALIZED(widget)) return true;
	if(!wfc) return true;

	AGL_ACTOR_START_DRAW(scene1) {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		agl_actor__paint((AGlActor*)scene1);
		agl_actor__paint((AGlActor*)scene2);

#ifdef RENDER_SAME_SCENE_MULTIPLE_TIMES
		glPushMatrix();
		glTranslatef(10.0, 10.0, 0.0);
		agl_actor__paint((AGlActor*)scene1);
		glPopMatrix();
#endif

#undef SHOW_BOUNDING_BOX
#ifdef SHOW_BOUNDING_BOX
		glPushMatrix();
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

#if USE_SYSTEM_GTKGLEXT
		gdk_gl_drawable_swap_buffers(scene1->gl.gdk.drawable);
#else
		gdk_gl_window_swap_buffers(scene1->gl.gdk.drawable);
#endif
	} AGL_ACTOR_END_DRAW(scene1)

	return TRUE;
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


void
zoom_in(gpointer _)
{
	start_zoom(zoom * 1.5);
}


void
zoom_out(gpointer _)
{
	start_zoom(zoom / 1.5);
}


void
vzoom_up(gpointer _)
{
	vzoom *= 1.1;
	zoom = MIN(vzoom, 100.0);
	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
vzoom_down(gpointer _)
{
	vzoom /= 1.1;
	zoom = MAX(vzoom, 1.0);
	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
scroll_left(gpointer _)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame - n_visible_frames / 10);
}


void
scroll_right(gpointer _)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame + n_visible_frames / 10);
}


	bool on_idle(gpointer _)
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
		return G_SOURCE_CONTINUE;
	}

void
toggle_animate(gpointer _)
{
	PF0;
	g_timeout_add(50, on_idle, NULL);
}


void
quit(gpointer _)
{
	exit(EXIT_SUCCESS);
}


uint64_t
get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


