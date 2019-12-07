/*
  Demonstration of the WaveformActor object

  A single waveform is cut into several regions that should be
  seamlessly displayed to look like a single region.

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
#define __wf_private__
#include "config.h"
#include <getopt.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "agl/utils.h"
#include "waveform/waveform.h"
#include "test/common2.h"

struct _app
{
	int timeout;
} app;

#define WAV "mono_0:10.wav"

#define GL_WIDTH 480.0
#define GL_HEIGHT 160.0
#define VBORDER 8

GdkGLConfig*     glconfig       = NULL;
GtkWidget*       canvas         = NULL;
AGlScene*        scene          = NULL;
WaveformContext* wfc            = NULL;
Waveform*        w1             = NULL;
Waveform*        w2             = NULL;
WaveformActor*   a[4]           = {NULL,};
float            zoom           = 1.0;
gpointer         tests[]        = {};

static void setup_projection   (GtkWidget*);
static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
static void start_zoom         (float target_zoom);
static void toggle_animate     ();
uint64_t    get_time           ();

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

	gtk_widget_set_can_focus     (canvas, true);
	gtk_widget_set_size_request  (canvas, 480, 128);
	gtk_widget_set_gl_capability (canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events        (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);

	scene = (AGlRootActor*)agl_actor__new_root(canvas);

	wfc = wf_context_new((AGlActor*)scene);
	//wfc->enable_animations = false;

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);

	gtk_widget_show_all(window);

	bool key_press (GtkWidget* widget, GdkEventKey* event, gpointer user_data)
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
setup_projection (GtkWidget* widget)
{
	int vw = widget->allocation.width;
	int vh = widget->allocation.height;
	glViewport(0, 0, vw, vh);
	dbg (1, "viewport: %i %i", vw, vh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double hborder = GL_WIDTH / 32;

	double left = -hborder;
	double right = GL_WIDTH + hborder;
	double bottom = GL_HEIGHT + VBORDER;
	double top = -VBORDER;
	glOrtho (left, right, bottom, top, 10.0, -100.0);

	((AGlActor*)scene)->region.x2 = vw;
	((AGlActor*)scene)->region.y2 = vh;
}


static void
on_canvas_realise (GtkWidget* _canvas, gpointer user_data)
{
	PF;
	if(w1) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	char* filename = find_wav(WAV);
	w1 = waveform_load_new(filename);
	g_free(filename);

	int n_frames = waveform_get_n_frames(w1);

	WfSampleRegion region[] = {
		{0,                  n_frames / 4},
		{(1 * n_frames) / 4, n_frames / 4},
		{(2 * n_frames) / 4, n_frames / 4},
		{(3 * n_frames) / 4, n_frames / 4},
	};

	uint32_t colours[4][2] = {
		{0xffffff77, 0x0000ffff},
		{0x66eeffff, 0x0000ffff},
		{0xffdd66ff, 0x0000ffff},
		{0x66ff66ff, 0x0000ffff},
	};

	int i; for(i=0;i<G_N_ELEMENTS(a);i++){
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)(a[i] = wf_canvas_add_new_actor(wfc, w1)));

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	on_allocate(canvas, &canvas->allocation, user_data);
}


static void
on_allocate (GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	setup_projection(widget);

	start_zoom(zoom);
}


static void
start_zoom (float target_zoom)
{
	// When zooming in, the Region is preserved so the box gets bigger.

	// This example illustrates zooming by setting the object sizes directly.
	// Normally you would use wf_context_set_zoom() instead

	PF0;
	zoom = MAX(0.1, target_zoom);

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			GL_WIDTH * target_zoom * i / 4,
			0.0,
			GL_WIDTH * target_zoom / 4,
			GL_HEIGHT / 4
		});
}


static void
toggle_animate ()
{
	PF0;
	bool on_idle(gpointer _)
	{
		static uint64_t frame = 0;
		static uint64_t t0    = 0;
		if(!frame)
			t0 = get_time();
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
	g_timeout_add(50, on_idle, NULL);
}


uint64_t
get_time ()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


