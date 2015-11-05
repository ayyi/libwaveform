/*
  Demonstration of the libwaveform WaveformActor interface

  Several waveforms are drawn onto a single canvas with
  different colours and zoom levels. The canvas can be zoomed
  and panned.

  In this example, drawing is managed by the AGl scene graph.
  See actor_no_scene.c for a version that doesnt use the scene graph.

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
#include "waveform/gl_utils.h"
#include "test/common.h"
#include "test/ayyi_utils.h"

#define GL_WIDTH 256.0
#define GL_HEIGHT 256.0
#define VBORDER 8

GdkGLConfig*    glconfig       = NULL;
GtkWidget*      canvas         = NULL;
AGlScene*       scene          = NULL;
WaveformCanvas* wfc            = NULL;
Waveform*       w1             = NULL;
Waveform*       w2             = NULL;
WaveformActor*  a[]            = {NULL, NULL, NULL, NULL};
float           zoom           = 1.0;
float           vzoom          = 1.0;
gpointer        tests[]        = {};

static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
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


int
main (int argc, char *argv[])
{
	set_log_handlers();

	wf_debug = 1;

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

	agl_get_instance()->pref_use_shaders = USE_SHADERS;

	scene = (AGlRootActor*)agl_actor__new_root(canvas);

	char* filename = g_build_filename(g_get_current_dir(), "test/data/mono_0:10.wav", NULL);
	w1 = waveform_load_new(filename);
	g_free(filename);

	wfc = wf_canvas_new(scene);

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
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)a[i]);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);

	gtk_widget_show_all(window);

	add_key_handlers((GtkWindow*)window, NULL, (Key*)&keys);

	bool window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data)
	{
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
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

	//optimise drawing by telling the canvas which area is visible
	wf_canvas_set_viewport(wfc, &(WfViewPort){0, 0, GL_WIDTH, GL_HEIGHT});

	((AGlActor*)scene)->region.x2 = allocation->width;
	((AGlActor*)scene)->region.y2 = allocation->height;

	wfc->samples_per_pixel = a[0]->region.len / allocation->width;

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			0.0,
			i * GL_HEIGHT / 4,
			GL_WIDTH * zoom,
			GL_HEIGHT / 4 * 0.95
		});

	start_zoom(zoom);
}


static void
start_zoom(float target_zoom)
{
	//when zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	PF0;
	zoom = MAX(0.1, target_zoom);

	wf_canvas_set_zoom(wfc, zoom);
}


void
zoom_in(WaveformView* waveform)
{
	start_zoom(zoom * 1.5);
}


void
zoom_out(WaveformView* waveform)
{
	start_zoom(zoom / 1.5);
}


void
vzoom_up(WaveformView* _)
{
	vzoom *= 1.1;
	zoom = MIN(vzoom, 100.0);
	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
vzoom_down(WaveformView* _)
{
	vzoom /= 1.1;
	zoom = MAX(vzoom, 1.0);
	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
scroll_left(WaveformView* waveform)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame - n_visible_frames / 10);
}


void
scroll_right(WaveformView* waveform)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame + n_visible_frames / 10);
}


void
toggle_animate(WaveformView* _)
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
quit(WaveformView* waveform)
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


