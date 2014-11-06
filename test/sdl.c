/*
  copyright (C) 2014 Tim Orford <tim@orford.org>

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
#define __wf_canvas_priv__
#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include "SDL2/SDL.h"
#include "waveform/waveform.h"
#include "waveform/actor.h"
#include "waveform/fbo.h"
#include "waveform/gl_utils.h"
#include "common.h"

#include "transition/gdkframeclockidle.h"

#define WAV \
	"test/data/mono_1.wav"

#define HBORDER (16.0)
#define VBORDER 8

AGl* agl = NULL;

struct
{
   int             width;
   int             height;
   bool            running;
   SDL_GLContext   gl_context;
   SDL_Window*     mainWindow;
   WaveformCanvas* wfc;
   Waveform*       w1;
   WaveformActor*  a[4];
   float           zoom;
   bool            dirty;
} window = {
	640, 240, true,
	.zoom = 10.0,
	.dirty = true
};

gpointer tests[] = {};

static void setup_projection   ();
static void on_event           (SDL_Event*);

KeyHandler
	zoom_in,
	zoom_out,
	scroll_left,
	scroll_right,
	quit;

Key keys[] = {
	{1073741904,    scroll_left},
	{1073741903,    scroll_right},
	{46,            scroll_right},
	{61,            zoom_in},
	{45,            zoom_out},
	{113,           quit},
	{0},
};


int
main (int argc, char **argv)
{
	wf_debug = 1;

	window.zoom = 1.0;

	agl = agl_get_instance();

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		dbg(0, "Unable to Init SDL: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	uint32_t flags = SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL;

	window.mainWindow = SDL_CreateWindow("Waveform SDL2 Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window.width, window.height, flags);
	window.gl_context = SDL_GL_CreateContext(window.mainWindow);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,            8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,          8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,           8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,          8);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         16);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,        32);

	SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,      8);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,    8);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,     8);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,    8);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);

	setup_projection();

	{
		window.wfc = wf_canvas_new_sdl(window.gl_context);

		char* filename = find_wav(WAV);
		window.w1 = waveform_load_new(filename);
		g_free(filename);

		int i = 0;
		window.a[i] = wf_canvas_add_new_actor(window.wfc, window.w1);

		wf_actor_set_rect(window.a[i], &(WfRectangle){
			0.0,
			window.height / 2,
			window.width,
			window.height / 2 * 0.95
		});

		int n_frames = 10000;
		WfSampleRegion region[] = {
			{0, n_frames},
		};

		wf_actor_set_region(window.a[i], &region[i]);

		/*
		TODO animations currently depend on the glib main loop.

		void _on_wf_canvas_requests_redraw(WaveformCanvas* wfc, gpointer _)
		{
			window.dirty = true;
		}
		window.wfc->draw = _on_wf_canvas_requests_redraw;
		*/
	}

	void loop() {
	}

	void render() {
		glClear(GL_COLOR_BUFFER_BIT);

		agl->shaders.plain->uniform.colour = 0xaaaaffff;
		agl_use_program((AGlShader*)agl->shaders.plain);

		glRectf(0.0, window.height/2.0, window.width, window.height);

		wf_actor_paint(window.a[0]);

		SDL_GL_SwapWindow(window.mainWindow);
	}

	SDL_Event event;
	while(window.running) {
		while(SDL_PollEvent(&event)) {
			on_event(&event);
		}

		loop();

		if(window.dirty){
			render();
			window.dirty = false;
		}
	}

	void cleanup()
	{
		SDL_GL_DeleteContext(window.gl_context);
		SDL_DestroyWindow(window.mainWindow);
		SDL_Quit();
	}

	cleanup();

	return EXIT_SUCCESS;
}


static void
on_event(SDL_Event* event)
{
	switch(event->type) {
		case SDL_KEYDOWN:
			;SDL_KeyboardEvent* e = (SDL_KeyboardEvent*)event;
			int i; for(i=0;i<G_N_ELEMENTS(keys)-1;i++){
				if(keys[i].key == e->keysym.sym){
					keys[i].handler(NULL);
					break;
				}
			}
			break;
		case SDL_QUIT:
			window.running = false;
			break;
	}
}


static void
setup_projection()
{
	int vx = 0;
	int vy = 0;
	int vw = window.width;
	int vh = window.height;
	glViewport(vx, vy, vw, vh);
	dbg (2, "viewport: %i %i %i %i", vx, vy, vw, vh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double left   = -HBORDER;
	double right  = vw + HBORDER;
	double bottom = window.height + VBORDER;
	double top    = -VBORDER;
	glOrtho (left, right, bottom, top, 1.0, -1.0);
}


static void
start_zoom(float target_zoom)
{
	PF;
	window.zoom = MAX(0.1, target_zoom);
	window.dirty = true;

	int i; for(i=0;i<G_N_ELEMENTS(window.a);i++)
		if(window.a[i]) {
			int n_frames = 10000;
			WfSampleRegion region[] = {
				{0, ((float)n_frames) / window.zoom},
			};

			wf_actor_set_region(window.a[i], &region[i]);
		}
}


void
zoom_in(WaveformView* waveform)
{
	start_zoom(window.zoom * 1.3);
}


void
zoom_out(WaveformView* waveform)
{
	start_zoom(window.zoom / 1.3);
}


void
scroll(WaveformView* waveform, int dx)
{
	window.dirty = true;

	int i; for(i=0;i<G_N_ELEMENTS(window.a);i++)
		if(window.a[i]) {
			int64_t n_frames = waveform_get_n_frames(window.a[i]->waveform);

			wf_actor_set_region(window.a[i], &(WfSampleRegion){
				CLAMP(((int64_t)window.a[i]->region.start) + (int64_t)dx, 0, n_frames - (int64_t)window.a[i]->region.len),
				window.a[i]->region.len
			});
		}
}


void
scroll_left(WaveformView* waveform)
{
	scroll(waveform, -1000.0f / window.zoom);
}


void
scroll_right(WaveformView* waveform)
{
	scroll(waveform, 1000.0f / window.zoom);
}


void
quit(WaveformView* waveform)
{
	window.running = false;
}


