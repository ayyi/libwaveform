/*
  Demonstration of the libwaveform WaveformActor interface

  Several waveforms are drawn onto a single canvas with
  different colours and zoom levels. The canvas can be zoomed
  and panned.

  In this example, drawing is managed by the AGl scene graph.
  See actor_no_scene.c for a version that doesnt use the scene graph.

  ---------------------------------------------------------------

  copyright (C) 2012-2019 Tim Orford <tim@orford.org>

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
#include <getopt.h>
#include <sys/time.h>
#include <gdk/gdkkeysyms.h>
#include "agl/utils.h"
#include "waveform/waveform.h"
#include "test/common.h"

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";

#define WAV "mono_0:10.wav"

#define GL_WIDTH 256.0
#define VBORDER 8

GtkWidget*       canvas    = NULL;
AGlScene*        scene     = NULL;
WaveformContext* wfc[4]    = {NULL,}; // This test has 4 separate contexts. Normally you would use a single context.
Waveform*        w1        = NULL;
WaveformActor*   a[4]      = {NULL,};
float            zoom      = 1.0;
float            vzoom     = 1.0;
gpointer         tests[]   = {};

KeyHandler
	zoom_in,
	zoom_out,
	vzoom_up,
	vzoom_down,
	scroll_left,
	scroll_right,
	toggle_animate,
	delete,
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
	{GDK_Delete,    delete},
	{113,           quit},
	{0},
};

static void on_canvas_realise (GtkWidget*, gpointer);
static void on_allocate       (GtkWidget*, GtkAllocation*, gpointer);
static void start_zoom        (float target_zoom);
static bool test_delete       ();


static void
window_content (GtkWindow* window, GdkGLConfig* glconfig)
{
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

	char* filename = find_wav(WAV);
	w1 = waveform_load_new(filename);
	g_free(filename);

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
		wfc[i] = wf_context_new((AGlActor*)scene); // each waveform has its own context so as to have a different zoom

		a[i] = wf_canvas_add_new_actor(wfc[i], w1);
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)a[i]);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	g_object_unref(w1); // this effectively transfers ownership of the waveform to the Scene

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);
}


static bool
automated ()
{
	if(!test_delete())
		exit(EXIT_FAILURE);

	gtk_main_quit();

	return G_SOURCE_REMOVE;
}


int
main (int argc, char *argv[])
{
	set_log_handlers();

	wf_debug = 0;

	gtk_init(&argc, &argv);

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(3000, automated, NULL);
				break;
		}
	}

	return gtk_window((Key*)&keys, window_content);
}


static void
on_canvas_realise (GtkWidget* _canvas, gpointer user_data)
{
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	on_allocate(canvas, &canvas->allocation, user_data);
}


static void
on_allocate (GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	((AGlActor*)scene)->region.x2 = allocation->width;
	((AGlActor*)scene)->region.y2 = allocation->height;

	int i; for(i=0;i<G_N_ELEMENTS(a);i++){
		wfc[i]->samples_per_pixel = a[i]->region.len / allocation->width;
		if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			0.0,
			i * allocation->height / 4,
			GL_WIDTH * zoom,
			allocation->height / 4 * 0.95
		});
	}

	start_zoom(zoom);
}


static void
start_zoom (float target_zoom)
{
	//when zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	PF0;
	zoom = MAX(0.1, target_zoom);

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		wf_context_set_zoom(wfc[i], zoom);
}


void
zoom_in (gpointer _)
{
	start_zoom(zoom * 1.5);
}


void
zoom_out (gpointer _)
{
	start_zoom(zoom / 1.5);
}


void
vzoom_up (gpointer _)
{
	vzoom *= 1.1;
	zoom = MIN(vzoom, 100.0);
	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
vzoom_down (gpointer _)
{
	vzoom /= 1.1;
	zoom = MAX(vzoom, 1.0);
	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
scroll_left (gpointer _)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame - n_visible_frames / 10);
}


void
scroll_right (gpointer _)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame + n_visible_frames / 10);
}


	bool on_idle (gpointer _)
	{
		static uint64_t frame = 0;
		static uint64_t t0    = 0;
		if(!frame) t0 = g_get_monotonic_time();
		else{
			uint64_t time = g_get_monotonic_time();
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
toggle_animate (gpointer _)
{
	PF0;
	g_timeout_add(50, on_idle, NULL);
}


static int finalize_done = false;

static void
finalize_notify (gpointer data, GObject* was)
{
	PF;

	w1 = NULL;
	finalize_done = true;
}


static bool
test_delete ()
{
	if(!a[0]) return false;

	g_object_weak_ref((GObject*)w1, finalize_notify, NULL);

	a[0] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[0]), NULL);

	if(finalize_done){
		gwarn("waveform should not be free'd");
		return false;
	}

	a[1] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[1]), NULL);

	a[2] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[2]), NULL);

	a[3] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[3]), NULL);

	if(!finalize_done){
		gwarn("waveform was not free'd");
		return false;
	}

	return true;
}


void
delete (gpointer _)
{
	test_delete();
}


void
quit (gpointer _)
{
	exit(EXIT_SUCCESS);
}

