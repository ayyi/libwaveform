/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform https://github.com/ayyi/libwaveform |
* | copyright (C) 2013-2018 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <sys/time.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include "SDL2/SDL.h"
#define USE_SDL_GFX // measures well but is it any smoother subjectively ?
#ifdef USE_SDL_GFX
#include "sdl/SDL2_framerate.h"
#endif
#include "agl/ext.h"
#include "waveform/waveform.h"
#include "common.h"

#include "transition/frameclockidle.h"

#define WAV "mono_0:10.wav"

#define FPS 60
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
   AGlScene*       scene;
   WaveformContext* wfc;
   Waveform*       w1;
   WaveformActor*  a[4];
   WfSampleRegion  region[2];
   float           zoom;
   bool            dirty;
} window = {
	640, 240, true,
	.zoom = 1.0,
	.dirty = true
};

gpointer tests[] = {};

static void setup_projection ();
static void on_event         (SDL_Event*);

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

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";


		static void _on_scene_requests_redraw(AGlScene* wfc, gpointer _)
		{
			window.dirty = true;
		}
int
main (int argc, char **argv)
{
	wf_debug = 0;

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(3000, (gpointer)exit, NULL);
				break;
		}
	}

	agl = agl_get_instance();

	g_main_loop_new(NULL, true);

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		dbg(0, "Unable to Init SDL: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	uint32_t flags = SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL;

	window.mainWindow = SDL_CreateWindow("Waveform SDL Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window.width, window.height, flags);
	window.gl_context = SDL_GL_CreateContext(window.mainWindow);
	//SDL_GL_SetSwapInterval(1); // supposed to enable sync to vblank but appears to have no effect

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

	agl_get_extensions(); // TODO what is the SDL way?

#ifdef USE_SDL_GFX
	FPSmanager fpsManager;
	SDL_initFramerate(&fpsManager);
	SDL_setFramerate(&fpsManager, FPS);
#endif
	{
		window.wfc = wf_context_new_sdl(window.gl_context);
		window.scene = window.wfc->root->root;

		char* filename = find_wav(WAV);
		window.w1 = waveform_load_new(filename);
		g_free(filename);

		int n_frames = 10000;
		window.region[0] = (WfSampleRegion){0,      n_frames};
		window.region[1] = (WfSampleRegion){100000, n_frames / 2};

		int i = 0; for(;i<2;i++){
			agl_actor__add_child((AGlActor*)window.scene, (AGlActor*)(window.a[i] = wf_canvas_add_new_actor(window.wfc, window.w1)));

			wf_actor_set_rect(window.a[i], &(WfRectangle){
				0.0,
				i * window.height / 2,
				window.width,
				window.height / 2 * 0.95
			});

			wf_actor_set_region(window.a[i], &window.region[i]);
		}

		window.scene->draw = _on_scene_requests_redraw;
	}

	void loop() {
		g_main_context_iteration(NULL, false); // update animations

		// TODO tell libwaveform to do preloading and calculations here.

#ifdef MEASURE_FRAMERATE
		#define N 120
		static uint32_t c = 0;
		static int i = 0;

		uint32_t ticks = SDL_GetTicks(); // milliseconds

		if(++i >= N){
			uint32_t t = (ticks - c) / N;
			dbg(0, "%.2f fps", 1000.0 / t);
			i = 0;
			c = ticks;
		}
#endif
	}

	void render() {
		glClear(GL_COLOR_BUFFER_BIT);

		agl->shaders.plain->uniform.colour = 0xaaaaffff;
		agl_use_program((AGlShader*)agl->shaders.plain);

		glRectf(0.0, window.height/2.0, window.width, window.height);

		agl_actor__paint((AGlActor*)window.scene);

		SDL_GL_SwapWindow(window.mainWindow); // does not wait for vblank
	}

	SDL_Event event;
#if 1
	while(window.running) {
		while(SDL_PollEvent(&event)) {
			on_event(&event);
		}
#else
	while(window.running && (SDL_WaitEvent(&event))) {
		on_event(&event);
#endif

		loop();

		if(window.dirty){
			render();
			window.dirty = false;
		}

#ifdef USE_SDL_GFX
		SDL_framerateDelay(&fpsManager);
#else
		//usleep(1000 * (1000 - (SDL_GetTicks() - ticks)) / FPS);
		SDL_Delay(1000 / FPS);
#endif
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
		case SDL_KEYUP:
		case SDL_TEXTINPUT:
		case SDL_WINDOWEVENT:
		case SDL_MOUSEMOTION ... SDL_MOUSEWHEEL:
			break;
		case SDL_QUIT:
			window.running = false;
			break;
		default:
			dbg(0, "event.other 0x%x", event->type);
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
		if(window.a[i])
			wf_actor_set_region(window.a[i], &(WfSampleRegion){
				window.region[i].start,
				window.region[i].len / window.zoom
			});
}


void
zoom_in(gpointer waveform)
{
	start_zoom(window.zoom * 1.3);
}


void
zoom_out(gpointer waveform)
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
scroll_left(gpointer waveform)
{
	scroll(waveform, -1000.0f / window.zoom);
}


void
scroll_right(gpointer waveform)
{
	scroll(waveform, 1000.0f / window.zoom);
}


void
quit(gpointer waveform)
{
	window.running = false;
}


