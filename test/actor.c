/*
  Demonstration of the libwaveform WaveformActor interface

  Several waveforms are drawn onto a single canvas with
  different colours and zoom levels. The canvas can be zoomed
  and panned.

  In this example, drawing is managed by the AGl scene graph.
  See actor_no_scene.c for a version that doesnt use the scene graph.

  ---------------------------------------------------------------

  copyright (C) 2012-2026 Tim Orford <tim@orford.org>

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

#include "config.h"
#include <getopt.h>
#include <gdk/gdkkeysyms-compat.h>
#include "glib/gstdio.h"
#include "agl/gtk.h"
#include "waveform/actor.h"
#include "test/common.h"
#ifdef DEBUG
#include "ui/debug_helper.h"
#endif

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
	{},
};

static const char* const short_options = "n";

#define WAV "mono_0:10.wav"

#define GL_WIDTH 400.0
#define VBORDER 8

AGlScene*        scene     = NULL;
WaveformContext* wfc[4]    = {NULL,}; // This test has 4 separate contexts. Normally you would use a single context.
Waveform*        w1        = NULL;
WaveformActor*   a[4]      = {NULL,};
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
	{GDK_Left,      scroll_left},
	{GDK_KP_Left,   scroll_left},
	{GDK_Right,     scroll_right},
	{GDK_KP_Right,  scroll_right},
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
	GtkWidget* canvas = gtk_drawing_area_new();

#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus     (canvas, true);
#endif
	gtk_widget_set_size_request  (canvas, 400, 128);
	gtk_widget_set_gl_capability (canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events        (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	gtk_container_add((GtkContainer*)window, canvas);

	scene = (AGlScene*)agl_new_scene_gtk(canvas);

	g_autofree char* filename = find_wav(WAV);
	w1 = waveform_new(filename);
    waveform_load(w1, NULL, NULL);

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

	for (int i=0;i<G_N_ELEMENTS(a);i++) {
		wfc[i] = wf_context_new((AGlActor*)scene); // each waveform has its own context so as to have a different zoom

		a[i] = wf_context_add_new_actor(wfc[i], w1);
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)a[i]);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);

		g_object_unref(wfc[i]);
	}

	g_object_unref(w1); // transfer ownership of the waveform to the Scene

#ifdef DEBUG
	agl_actor__add_behaviour((AGlActor*)a[0], debug_helper());
#endif

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
#if GTK_MAJOR_VERSION < 3
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);
#else
	g_signal_connect((gpointer)canvas, "draw",  G_CALLBACK(agl_actor__draw), scene);
#endif
}


static gboolean
automated (void* _)
{
	static bool done = false;
	if (!done) {
		done = true;

		if (!test_delete())
			exit(EXIT_FAILURE);

		gtk_main_quit();
	}
	return G_SOURCE_REMOVE;
}


int
main (int argc, char* argv[])
{
	set_log_handlers();

#ifdef TEMP_CACHE
	// Set up a temporary XDG cache directory to isolate the test
	char temp_cache_dir[128];
	g_snprintf(temp_cache_dir, sizeof(temp_cache_dir), "/tmp/libwaveform_test");
	if (g_mkdir_with_parents(temp_cache_dir, 0755) != 0) {
		printf("Error creating temporary cache directory: %s\n", temp_cache_dir);
		return 1;
	}

	// Set the XDG_CACHE_HOME environment variable to use our temp directory
	if (setenv("XDG_CACHE_HOME", temp_cache_dir, 1) != 0) {
		printf("Error setting XDG_CACHE_HOME environment variable\n");
		g_rmdir(temp_cache_dir);
		return 1;
	}
#endif

	wf_debug = 0;

	gtk_init(&argc, &argv);

	int opt;
	while ((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(3000, automated, NULL);
				break;
		}
	}

	if(g_getenv("NON_INTERACTIVE")){
		g_timeout_add(3000, automated, NULL);
	}

	return gtk_window((Key*)&keys, window_content);
}


static void
on_canvas_realise (GtkWidget* canvas, gpointer user_data)
{
	if (!gtk_widget_get_realized(canvas)) return;

#if GTK_MAJOR_VERSION < 3
	on_allocate(canvas, &canvas->allocation, user_data);
#else
	GtkAllocation allocation;
	gtk_widget_get_allocation(canvas, &allocation);
	on_allocate(canvas, &allocation, user_data);
#endif
}


static void
on_allocate (GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	((AGlActor*)scene)->region.x2 = allocation->width;
	((AGlActor*)scene)->region.y2 = allocation->height;

	for (int i=0;i<G_N_ELEMENTS(a);i++) {
		wfc[i]->samples_per_pixel = a[i]->region.len / allocation->width;
		if (a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			0.0,
			i * allocation->height / 4,
			GL_WIDTH * wfc[0]->zoom->value.f,
			allocation->height / 4 * 0.95
		});
	}

	start_zoom(wfc[0]->zoom->value.f);
}


static void
start_zoom (float target_zoom)
{
	// When zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	for (int i=0;i<G_N_ELEMENTS(a);i++)
		wf_context_set_zoom(wfc[i], target_zoom);
}


void
zoom_in (gpointer _)
{
	start_zoom(wfc[0]->zoom->value.f * 1.5);
}


void
zoom_out (gpointer _)
{
	start_zoom(wfc[0]->zoom->value.f / 1.5);
}


void
vzoom_up (gpointer _)
{
	vzoom *= 1.1;
	vzoom = MIN(vzoom, 100.0);
	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_vzoom(a[i], vzoom);
}


void
vzoom_down (gpointer _)
{
	vzoom /= 1.1;
	vzoom = MAX(vzoom, 1.0);
	for (int i=0;i<G_N_ELEMENTS(a);i++)
		if (a[i]) wf_actor_set_vzoom(a[i], vzoom);
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


static gboolean
on_idle (gpointer _)
{
	static uint64_t frame = 0;
#ifdef DEBUG
	static uint64_t t0    = 0;
#endif

	if(!frame)
#ifdef DEBUG
		t0 = g_get_monotonic_time();
#else
		;
#endif
	else{
#ifdef DEBUG
		uint64_t time = g_get_monotonic_time();
		if (!(frame % 1000))
			dbg(0, "rate=%.2f fps", ((float)frame) / ((float)(time - t0)) / 1000.0);
#endif

		if (!(frame % 8)) {
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

	if (finalize_done) {
		pwarn("waveform should not be free'd");
		return false;
	}

	a[1] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[1]), NULL);

	a[2] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[2]), NULL);

	a[3] = (agl_actor__remove_child((AGlActor*)scene, (AGlActor*)a[3]), NULL);

	if (!finalize_done) {
		pwarn("waveform was not free'd");
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
	agl_actor__free((AGlActor*)scene);

	exit(EXIT_SUCCESS);
}

