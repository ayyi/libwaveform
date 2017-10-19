/*
  Demonstration of the libwaveform WaveformActor interface

  Similar to actor.c but with additional features, eg background, ruler.

  ---------------------------------------------------------------

  copyright (C) 2012-2017 Tim Orford <tim@orford.org>

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
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "agl/utils.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "test/common.h"

#define WAV "test/data/mono_0:10.wav"

#define GL_WIDTH 300.0
#define GL_HEIGHT 256.0
#define HBORDER 16
#define VBORDER 8

AGl*            agl            = NULL;
static bool     gl_initialised = false;
GtkWidget*      canvas         = NULL;
AGlRootActor*   scene          = NULL;
AGlActor*       group          = NULL;
WaveformContext* wfc            = NULL;
Waveform*       w1             = NULL;
WaveformActor*  a[]            = {NULL};
gpointer        tests[]        = {};

KeyHandler
	zoom_in,
	zoom_out,
	scroll_left,
	scroll_right,
	toggle_animate,
	toggle_shaders,
	quit;

Key keys[] = {
	{KEY_Left,      scroll_left},
	{KEY_KP_Left,   scroll_left},
	{KEY_Right,     scroll_right},
	{KEY_KP_Right,  scroll_right},
	{61,            zoom_in},
	{45,            zoom_out},
	{GDK_KP_Enter,  NULL},
	{(char)'<',     NULL},
	{(char)'>',     NULL},
	{(char)'a',     toggle_animate},
	{(char)'s',     toggle_shaders},
	{GDK_Delete,    NULL},
	{113,           quit},
	{0},
};

static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
static void start_zoom         (float target_zoom);
uint64_t    get_time           ();


int
main (int argc, char *argv[])
{
	set_log_handlers();

	wf_debug = 1;

	gtk_init(&argc, &argv);
	GdkGLConfig* glconfig;
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		gerr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
	}

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	canvas = gtk_drawing_area_new();
	gtk_widget_set_can_focus     (canvas, true);
	gtk_widget_set_size_request  (canvas, GL_WIDTH + 2 * HBORDER, 128);
	gtk_widget_set_gl_capability (canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events        (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);

	agl = agl_get_instance();

	scene = (AGlRootActor*)agl_actor__new_root(canvas);
	//scene->enable_animations = false;

	wfc = wf_context_new((AGlRootActor*)scene);
	wf_context_set_zoom(wfc, 1.0);

	char* filename = find_wav(WAV);
	w1 = waveform_load_new(filename);
	g_free(filename);

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);

	gtk_widget_show_all(window);

	add_key_handlers((GtkWindow*)window, NULL, (Key*)&keys);

	bool window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		return gtk_main_quit(), G_SOURCE_REMOVE;
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
on_canvas_realise(GtkWidget* _canvas, gpointer user_data)
{
	PF;
	static gboolean canvas_init_done = false;
	if(canvas_init_done) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	gl_init();

	canvas_init_done = true;

	agl_actor__add_child((AGlActor*)scene, group = group_actor(a[0]));

	void group__set_size(AGlActor* actor)
	{
		actor->region = (AGliRegion){
			.x1 = HBORDER,
			.y1 = VBORDER,
			.x2 = actor->parent->region.x2 - HBORDER,
			.y2 = actor->parent->region.y2 - VBORDER,
		};
	}
	group->set_size = group__set_size;

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
		agl_actor__add_child(group, (AGlActor*)(a[i] = wf_canvas_add_new_actor(wfc, w1)));

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	agl_actor__add_child(group, background_actor(a[0]));

	AGlActor* ruler = agl_actor__add_child(group, ruler_actor(a[0]));
	ruler->region = (AGliRegion){0, 0, 0, 20};

	on_allocate(canvas, &canvas->allocation, user_data);

	void _scene_needs_redraw(AGlScene* scene, gpointer _)
	{
		gdk_window_invalidate_rect(canvas->window, NULL, false);
	}
	scene->draw = _scene_needs_redraw;
}


static void
on_allocate(GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	double width = canvas->allocation.width - 2 * ((int)HBORDER);
	wfc->samples_per_pixel = waveform_get_n_frames(w1) / width;

	((AGlActor*)scene)->region = (AGliRegion){0, 0, allocation->width, allocation->height};
	agl_actor__set_size((AGlActor*)scene);

	#define ruler_height 20.0

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			0.0,
			ruler_height + 5 + i * ((AGlActor*)scene)->region.y2 / 2,
			width,
			agl_actor__height(group) / 2 * 0.95
		});

	start_zoom(wf_context_get_zoom(wfc));
}


static void
start_zoom(float target_zoom)
{
	wf_context_set_zoom(wfc, target_zoom);
}


void
toggle_animate(gpointer _)
{
	PF0;
	gboolean on_idle(gpointer _)
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
		return IDLE_CONTINUE;
	}
	g_timeout_add(50, on_idle, NULL);
}


void
toggle_shaders(gpointer _)
{
	PF0;
	agl_actor__set_use_shaders(scene, !agl->use_shaders);
}


void
zoom_in(gpointer _)
{
	start_zoom(wf_context_get_zoom(wfc) * 1.5);
}


void
zoom_out(gpointer _)
{
	start_zoom(wf_context_get_zoom(wfc) / 1.5);
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


