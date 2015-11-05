/*
  Demonstration of the libwaveform WaveformActor interface
  showing a 3d presentation of a list of waveforms.

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
#define __wf_private__
#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <GL/gl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "agl/utils.h"
#include "waveform/waveform.h"
#include "waveform/gl_utils.h"
#include "test/ayyi_utils.h"
#include "test/common2.h"

struct
{
	int timeout;
} app;

#define WAV "test/data/mono_0:10.wav"

#define GL_WIDTH 256.0
#define GL_HEIGHT 256.0
#define VBORDER 8

//float rotate[3] = {45.0, 45.0, 45.0};
float rotate[3] = {30.0, 30.0, 30.0};
float isometric_rotation[3] = {35.264f, 45.0f, 0.0f};

GdkGLConfig*    glconfig       = NULL;
static bool     gl_initialised = false;
GtkWidget*      canvas         = NULL;
WaveformCanvas* wfc            = NULL;
AGlScene*       scene          = NULL;
Waveform*       w1             = NULL;
Waveform*       w2             = NULL;
WaveformActor*  a[]            = {NULL, NULL, NULL, NULL};
int             a_front        = 3;
float           zoom           = 1.0;
float           dz             = 20.0;
gpointer        tests[]        = {};

static AGlActor* rotator            (WaveformActor*);
static void      on_canvas_realise  (GtkWidget*, gpointer);
static void      on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
static void      start_zoom         (float target_zoom);
static void      forward            ();
static void      backward           ();
static void      toggle_animate     ();
static uint64_t  get_time           ();


int
main (int argc, char *argv[])
{
	set_log_handlers();

	wf_debug = 0;

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

	scene = (AGlScene*)agl_actor__new_root(canvas);

	wfc = wf_canvas_new(scene);

	char* filename = g_build_filename(g_get_current_dir(), WAV, NULL);
	w1 = waveform_load_new(filename);
	g_free(filename);

	agl_actor__add_child((AGlActor*)scene, rotator(NULL));

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose-event",  G_CALLBACK(agl_actor__on_expose), scene);

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
				break;
			case KEY_Right:
			case KEY_KP_Right:
				dbg(0, "right");
				break;
			case KEY_Up:
			case KEY_KP_Up:
				dbg(0, "up");
				forward();
				break;
			case KEY_Down:
			case KEY_KP_Down:
				dbg(0, "down");
				backward();
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


static gboolean canvas_init_done = false;
static void
on_canvas_realise(GtkWidget* _canvas, gpointer user_data)
{
	PF;
	if(canvas_init_done) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	gl_initialised = true;

	canvas_init_done = true;

	int n_frames = waveform_get_n_frames(w1);

	WfSampleRegion region[] = {
		{0,            n_frames    },
		{0,            n_frames / 2},
		{n_frames / 4, n_frames / 16},
		{n_frames / 2, n_frames / 2},
	};

	uint32_t colours[4][2] = {
		{0x66eeffff, 0x0000ffff}, // blue
		{0xffffff77, 0x0000ffff}, // grey
		{0xffdd66ff, 0x0000ffff}, // orange
		{0x66ff66ff, 0x0000ffff}, // green
	};

	int i; for(i=0;i<G_N_ELEMENTS(a);i++){
		agl_actor__add_child((AGlActor*)scene, (AGlActor*)(a[i] = wf_canvas_add_new_actor(wfc, w1)));

		wf_actor_set_region (a[i], &region[i]);
		wf_actor_set_colour (a[i], colours[i][0]);
		wf_actor_set_z      (a[i], i * dz);
	}

	on_allocate(canvas, &canvas->allocation, user_data);
}


static void
on_allocate(GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	if(!gl_initialised) return;

	((AGlActor*)scene)->region.x2 = allocation->width;
	((AGlActor*)scene)->region.y2 = allocation->height;

	//optimise drawing by telling the canvas which area is visible
	wf_canvas_set_viewport(wfc, &(WfViewPort){0, 0, GL_WIDTH, GL_HEIGHT});

	start_zoom(zoom);
}


AGlActor*
rotator(WaveformActor* wf_actor)
{
	void rotator_set_state(AGlActor* actor)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(rotate[0], 1.0f, 0.0f, 0.0f);
		glRotatef(rotate[1], 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.0f, -1.0f);
	}

	AGlActor* actor = agl_actor__new();
#ifdef AGL_DEBUG_ACTOR
	actor->name = "Rotator";
#endif
	actor->set_state = rotator_set_state;

	return actor;
}


static void
set_position(int i, int j)
{
	#define Y_FACTOR 0.0f //0.5f //currently set to zero to simplify changing stack order

	if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
		40.0,
		((float)j) * GL_HEIGHT * Y_FACTOR / 4 + 10.0f,
		GL_WIDTH * zoom,
		GL_HEIGHT / 4 * 0.95
	});
}


static void
start_zoom(float target_zoom)
{
	//when zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	PF0;
	zoom = MAX(0.1, target_zoom);

	int i; for(i=0;i<G_N_ELEMENTS(a);i++) set_position(i, i);
}


static void
forward()
{
	void fade_out_done(WaveformActor* actor, gpointer user_data)
	{
		gboolean fade_out_really_done(WaveformActor* actor)
		{
			wfc->enable_animations = false;
			wf_actor_set_z(actor, 0);
			wfc->enable_animations = true;
			wf_actor_fade_in(actor, NULL, 1.0f, NULL, NULL);
			set_position(a_front, 0); //move to back

			if(--a_front < 0) a_front = G_N_ELEMENTS(a) - 1;
			return G_SOURCE_REMOVE;
		}
		g_idle_add((GSourceFunc)fade_out_really_done, actor); //TODO fix issues with overlapping transitions.
	}

	int i; for(i=0;i<G_N_ELEMENTS(a);i++) wf_actor_set_z(a[i], a[i]->z + dz);
	dbg(1, "a_front=%i", a_front);
	wf_actor_fade_out(a[a_front], fade_out_done, NULL);
}


static void
backward()
{
	void fade_out_done(WaveformActor* actor, gpointer user_data)
	{
		gboolean fade_out_really_done(WaveformActor* actor)
		{
			wfc->enable_animations = false;
			wf_actor_set_z(actor, dz * (G_N_ELEMENTS(a) - 1));
			wfc->enable_animations = true;
			wf_actor_fade_in(actor, NULL, 1.0f, NULL, NULL);
			set_position(a_front, G_N_ELEMENTS(a) - 1); //move to front

			if(++a_front > G_N_ELEMENTS(a) - 1) a_front = 0;
			return G_SOURCE_REMOVE;
		}
		g_idle_add((GSourceFunc)fade_out_really_done, actor); //TODO fix issues with overlapping transitions.
	}

	int i; for(i=0;i<G_N_ELEMENTS(a);i++) wf_actor_set_z(a[i], a[i]->z - dz);
	dbg(1, "a_front=%i", a_front);
	wf_actor_fade_out(a[(a_front + 1) % G_N_ELEMENTS(a)], fade_out_done, NULL);
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
				forward();
			}
		}
		frame++;
		return IDLE_CONTINUE;
	}
	g_timeout_add(50, on_idle, NULL);
}


static uint64_t
get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


