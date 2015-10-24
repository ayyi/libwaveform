/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2015 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
* *********** TODO only redraw if something has changed (currently redrawing at 60fps) **********
*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#ifndef __VMS
/*# include <stdint.h>*/
#endif
# define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include "gtk/gtk.h" // TODO
#include "agl/ext.h"
#define __wf_private__ // TODO
#include "waveform/waveform.h"
#include "agl/actor.h"
#include "waveform/actors/background.h"

#ifndef GLX_MESA_swap_control
typedef GLint (*PFNGLXSWAPINTERVALMESAPROC)    (unsigned interval);
typedef GLint (*PFNGLXGETSWAPINTERVALMESAPROC) (void);
#endif

#if !defined( GLX_OML_sync_control ) && defined( _STDINT_H )
#define GLX_OML_sync_control 1
typedef Bool (*PFNGLXGETMSCRATEOMLPROC) (Display*, GLXDrawable, int32_t* numerator, int32_t* denominator);
#endif

#ifndef GLX_MESA_swap_frame_usage
#define GLX_MESA_swap_frame_usage 1
typedef int (*PFNGLXGETFRAMEUSAGEMESAPROC) (Display*, GLXDrawable, float* usage);
#endif

static int current_time();

#define BENCHMARK
#define NUL '\0'

PFNGLXGETFRAMEUSAGEMESAPROC get_frame_usage = NULL;


static GLboolean has_OML_sync_control = GL_FALSE;
static GLboolean has_SGI_swap_control = GL_FALSE;
static GLboolean has_MESA_swap_control = GL_FALSE;
static GLboolean has_MESA_swap_frame_usage = GL_FALSE;

static char** extension_table = NULL;
static unsigned num_extensions;

static AGlRootActor* scene = NULL;
static WaveformActor* wa = NULL;


static void
draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	agl_actor__paint((AGlActor*)scene);

	glPushMatrix();
	glTranslatef(15.0, 30.0, 0.0);
	agl_actor__paint((AGlActor*)scene);
	((AGlActor*)wa)->paint((AGlActor*)wa);
	glPopMatrix();
}


/* new window size or exposure */
static void
reshape(int width, int height)
{
	#define HBORDER 50
	#define VBORDER 50
	int vx = 0;
	int vy = 0;
	glViewport(vx, vy, width, height);
	dbg (2, "viewport: %i %i %i %i", vx, vy, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double left   = -HBORDER;
	double right  = width + HBORDER;
	double bottom = height + VBORDER;
	double top    = -VBORDER;
	glOrtho (left, right, bottom, top, 1.0, -1.0);

	((AGlActor*)scene)->region = (AGliRegion){
		.x2 = width,
		.y2 = height,
	};
}


static void
init(void)
{
}


/**
 * Remove window border/decorations.
 */
static void
no_border(Display* dpy, Window w)
{
   static const unsigned MWM_HINTS_DECORATIONS = (1 << 1);
   static const int PROP_MOTIF_WM_HINTS_ELEMENTS = 5;

   typedef struct
   {
      unsigned long       flags;
      unsigned long       functions;
      unsigned long       decorations;
      long                inputMode;
      unsigned long       status;
   } PropMotifWmHints;

   PropMotifWmHints motif_hints;
   Atom prop, proptype;
   unsigned long flags = 0;

   /* setup the property */
   motif_hints.flags = MWM_HINTS_DECORATIONS;
   motif_hints.decorations = flags;

   /* get the atom for the property */
   prop = XInternAtom( dpy, "_MOTIF_WM_HINTS", True );
   if (!prop) {
      /* something went wrong! */
      return;
   }

   /* not sure this is correct, seems to work, XA_WM_HINTS didn't work */
   proptype = prop;

   XChangeProperty( dpy, w,                         /* display, window */
                    prop, proptype,                 /* property, type */
                    32,                             /* format: 32-bit datums */
                    PropModeReplace,                /* mode */
                    (unsigned char *) &motif_hints, /* data */
                    PROP_MOTIF_WM_HINTS_ELEMENTS    /* nelements */
                  );
}


/*
 * Create an RGB, double-buffered window.
 * Return the window and context handles.
 */
static void
make_window(Display *dpy, const char *name, int x, int y, int width, int height, GLboolean fullscreen, Window *winRet, GLXContext *ctxRet)
{
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		None
	};
	XSetWindowAttributes attr;
	unsigned long mask;
	XVisualInfo *visinfo;

	int scrnum = DefaultScreen( dpy );
	Window root = RootWindow( dpy, scrnum );

	if (fullscreen) {
		x = y = 0;
		width = DisplayWidth( dpy, scrnum );
		height = DisplayHeight( dpy, scrnum );
	}

	visinfo = glXChooseVisual( dpy, scrnum, attrib );
	if (!visinfo) {
		printf("Error: couldn't get an RGB, Double-buffered visual\n");
		exit(1);
	}

	/* window attributes */
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap( dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	Window win = XCreateWindow( dpy, root, 0, 0, width, height, 0, visinfo->depth, InputOutput, visinfo->visual, mask, &attr);

	/* set hints and properties */
	{
		XSizeHints sizehints;
		sizehints.x = x;
		sizehints.y = y;
		sizehints.width  = width;
		sizehints.height = height;
		sizehints.flags = USSize | USPosition;
		XSetNormalHints(dpy, win, &sizehints);
		XSetStandardProperties(dpy, win, name, name, None, (char **)NULL, 0, &sizehints);
	}

	if (fullscreen)
		no_border(dpy, win);

	GLXContext ctx;
	ctx = glXCreateContext( dpy, visinfo, NULL, True );
	if (!ctx) {
		printf("Error: glXCreateContext failed\n");
		exit(1);
	}

	XFree(visinfo);

	*winRet = win;
	*ctxRet = ctx;
}


static void
event_loop(Display *dpy, Window win)
{
	float  frame_usage = 0.0;

	while (1) {
		while (XPending(dpy) > 0) {
			XEvent event;
			XNextEvent(dpy, &event);
			switch (event.type) {
				case Expose:
					/* we'll redraw below */
					break;
				case ConfigureNotify:
					reshape(event.xconfigure.width, event.xconfigure.height);
					break;
				case KeyPress: {
					char buffer[10];
					int code;
					code = XLookupKeysym(&event.xkey, 0);
					if (code == XK_Left) {
					}
					else if (code == XK_Right) {
					}
					else if (code == XK_Up) {
					}
					else if (code == XK_Down) {
					}
					else {
						XLookupString(&event.xkey, buffer, sizeof(buffer), NULL, NULL);
						if (buffer[0] == 27 || buffer[0] == 'q') {
							/* escape */
							return;
						}
					}
				}
			}
		}

		/* next frame */
		//angle += 2.0;

		draw();

		glXSwapBuffers(dpy, win);

		if ( get_frame_usage != NULL ) {
			GLfloat temp;

			(*get_frame_usage)( dpy, win, & temp );
			frame_usage += temp;
		}

		/* calc framerate */
		{
			static int t0 = -1;
			static int frames = 0;
			int t = current_time();

			if (t0 < 0) t0 = t;

			frames++;

			if (t - t0 >= 5.0) {
				GLfloat seconds = t - t0;
				GLfloat fps = frames / seconds;
				if ( get_frame_usage != NULL ) {
					printf("%d frames in %3.1f seconds = %6.3f FPS (%3.1f%% usage)\n", frames, seconds, fps, (frame_usage * 100.0) / (float) frames );
				}
				else {
					printf("%d frames in %3.1f seconds = %6.3f FPS\n", frames, seconds, fps);
				}
				fflush(stdout);

				t0 = t;
				frames = 0;
				frame_usage = 0.0;
			}
		}

		g_main_context_iteration(NULL, false); // update animations
	}
}


/**
 * Display the refresh rate of the display using the GLX_OML_sync_control
 * extension.
 */
static void
show_refresh_rate( Display * dpy )
{
#if defined(GLX_OML_sync_control) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
   int32_t  n;
   int32_t  d;

   PFNGLXGETMSCRATEOMLPROC get_msc_rate = (PFNGLXGETMSCRATEOMLPROC)glXGetProcAddressARB((const GLubyte*) "glXGetMscRateOML");
   if (get_msc_rate != NULL) {
      (*get_msc_rate)(dpy, glXGetCurrentDrawable(), &n, &d);
      printf( "refresh rate: %.1fHz\n", (float) n / d);
      return;
   }
#endif
   printf( "glXGetMscRateOML not supported.\n" );
}


/**
 * Fill in the table of extension strings from a supplied extensions string
 * (as returned by glXQueryExtensionsString).
 *
 * \param string   String of GLX extensions.
 * \sa is_extension_supported
 */
static void
make_extension_table( const char * string )
{
   char ** string_tab;
   unsigned  num_strings;
   unsigned  base;
   unsigned  idx;
   unsigned  i;
      
   /* Count the number of spaces in the string.  That gives a base-line
    * figure for the number of extension in the string.
    */
   
   num_strings = 1;
   for ( i = 0 ; string[i] != NUL ; i++ ) {
      if ( string[i] == ' ' ) {
		num_strings++;
      }
   }
   
   string_tab = (char **) malloc( sizeof( char * ) * num_strings );
   if ( string_tab == NULL ) {
      return;
   }

	base = 0;
	idx = 0;

	while ( string[ base ] != NUL ) {
	// Determine the length of the next extension string.

		for ( i = 0 
	    ; (string[ base + i ] != NUL) && (string[ base + i ] != ' ')
	    ; i++ ) {
			/* empty */ ;
		}

		if(i > 0){
			/* If the string was non-zero length, add it to the table.  We
			 * can get zero length strings if there is a space at the end of
			 * the string or if there are two (or more) spaces next to each
			 * other in the string.
			 */

			string_tab[ idx ] = malloc(sizeof(char) * (i + 1));
			if(string_tab[idx] == NULL){
				unsigned j = 0;

				for(j = 0; j < idx; j++){
					free(string_tab[j]);
				}

				free(string_tab);

				return;
			}

			(void) memcpy(string_tab[idx], & string[base], i);
			string_tab[idx][i] = NUL;
			idx++;
		}


		// Skip to the start of the next extension string.
		for (base += i; (string[ base ] == ' ') && (string[ base ] != NUL); base++ ) {
			/* empty */;
		}
	}

	extension_table = string_tab;
	num_extensions = idx;
}

    
/**
 * Determine of an extension is supported.  The extension string table
 * must have already be initialized by calling \c make_extension_table.
 * 
 * \praram ext  Extension to be tested.
 * \return GL_TRUE of the extension is supported, GL_FALSE otherwise.
 * \sa make_extension_table
 */
static GLboolean
is_extension_supported( const char * ext )
{
   unsigned   i;
   
   for ( i = 0 ; i < num_extensions ; i++ ) {
      if ( strcmp( ext, extension_table[i] ) == 0 ) {
	 return GL_TRUE;
      }
   }
   
   return GL_FALSE;
}


int
main(int argc, char *argv[])
{
	Window win;
	GLXContext ctx;
	char *dpyName = NULL;
	int swap_interval = 1;
	GLboolean do_swap_interval = GL_FALSE;
	GLboolean force_get_rate = GL_FALSE;
	GLboolean fullscreen = GL_FALSE;
	GLboolean printInfo = GL_FALSE;
	PFNGLXSWAPINTERVALMESAPROC set_swap_interval = NULL;
	PFNGLXGETSWAPINTERVALMESAPROC get_swap_interval = NULL;
	int width = 300, height = 300;

	int i; for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-display") == 0 && i + 1 < argc) {
			dpyName = argv[i+1];
			i++;
		}
		else if (strcmp(argv[i], "-info") == 0) {
			printInfo = GL_TRUE;
		}
		else if (strcmp(argv[i], "-swap") == 0 && i + 1 < argc) {
			swap_interval = atoi( argv[i+1] );
			do_swap_interval = GL_TRUE;
			i++;
		}
		else if (strcmp(argv[i], "-forcegetrate") == 0) {
			/* This option was put in because some DRI drivers don't support the
			 * full GLX_OML_sync_control extension, but they do support
			 * glXGetMscRateOML.
			 */
			force_get_rate = GL_TRUE;
		}
		else if (strcmp(argv[i], "-fullscreen") == 0) {
			fullscreen = GL_TRUE;
		}
		else if (strcmp(argv[i], "-help") == 0) {
			printf("Usage:\n");
			printf("  gears [options]\n");
			printf("Options:\n");
			printf("  -help                   Print this information\n");
			printf("  -display displayName    Specify X display\n");
			printf("  -info                   Display GL information\n");
			printf("  -swap N                 Swap no more than once per N vertical refreshes\n");
			printf("  -forcegetrate           Try to use glXGetMscRateOML function\n");
			printf("  -fullscreen             Full-screen window\n");
			return 0;
		}
	}

	Display* dpy = XOpenDisplay(dpyName);
	if (!dpy) {
		printf("Error: couldn't open display %s\n", XDisplayName(dpyName));
		return -1;
	}

	make_window(dpy, "waveformglxtest", 0, 0, width, height, fullscreen, &win, &ctx);
	XMapWindow(dpy, win);
	glXMakeCurrent(dpy, win, ctx);

	// TODO is make_extension_table needed as well as agl_get_extensions ?
	make_extension_table((char*)glXQueryExtensionsString(dpy,DefaultScreen(dpy)));
	has_OML_sync_control = is_extension_supported("GLX_OML_sync_control");
	has_SGI_swap_control = is_extension_supported("GLX_SGI_swap_control");
	has_MESA_swap_control = is_extension_supported("GLX_MESA_swap_control");
	has_MESA_swap_frame_usage = is_extension_supported("GLX_MESA_swap_frame_usage");

	agl_get_extensions();

	if (has_MESA_swap_control) {
		set_swap_interval = (PFNGLXSWAPINTERVALMESAPROC) glXGetProcAddressARB((const GLubyte*) "glXSwapIntervalMESA");
		get_swap_interval = (PFNGLXGETSWAPINTERVALMESAPROC) glXGetProcAddressARB((const GLubyte*) "glXGetSwapIntervalMESA");
	}
	else if (has_SGI_swap_control) {
		set_swap_interval = (PFNGLXSWAPINTERVALMESAPROC) glXGetProcAddressARB((const GLubyte*) "glXSwapIntervalSGI");
	}

	if (has_MESA_swap_frame_usage) {
		get_frame_usage = (PFNGLXGETFRAMEUSAGEMESAPROC)  glXGetProcAddressARB((const GLubyte*) "glXGetFrameUsageMESA");
	}

	if(printInfo){
		printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
		printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
		printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
		printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
		if(has_OML_sync_control || force_get_rate){
			show_refresh_rate(dpy);
		}

		if(get_swap_interval){
			printf("Default swap interval = %d\n", (*get_swap_interval)());
		}
	}

	if(do_swap_interval){
		if(set_swap_interval != NULL){
			if(((swap_interval == 0) && !has_MESA_swap_control) || (swap_interval < 0)){
				printf( "Swap interval must be non-negative or greater than zero "
					"if GLX_MESA_swap_control is not supported.\n" );
			}
			else {
				(*set_swap_interval)(swap_interval);
			}

			if (printInfo && (get_swap_interval != NULL)){
				printf("Current swap interval = %d\n", (*get_swap_interval)());
			}
		}
		else {
			printf("Unable to set swap-interval. Neither GLX_SGI_swap_control "
				"nor GLX_MESA_swap_control are supported.\n" );
		}
	}

	init();

	// -----------------------------------------------------------

	g_main_loop_new(NULL, true);

	scene = (AGlRootActor*)agl_actor__new_root_(CONTEXT_TYPE_GLX);

	AGlActor* bg = background_actor(NULL);

	agl_actor__add_child((AGlActor*)scene, bg);
	dbg(0, "actor=%p", scene);

	char* filename = g_build_filename(g_get_current_dir(), "test/data/mono_0:10.wav", NULL);
	Waveform* w = waveform_load_new(filename);
	g_free(filename);

	WaveformCanvas* wfc = NULL;
	wfc = wf_canvas_new(scene);

	agl_actor__add_child((AGlActor*)scene, (AGlActor*)(wa = wf_canvas_add_new_actor(wfc, w)));

	//wf_canvas_set_viewport(wfc, &(WfViewPort){0, 0, GL_WIDTH, GL_HEIGHT});

	wf_actor_set_region(wa, &(WfSampleRegion){0, 44100});

	wf_actor_set_rect(wa, &(WfRectangle){
		0.0,
		0.0,
		width,
		height
	});

	// -----------------------------------------------------------

	// Set initial projection/viewing transformation.
	reshape(width, height);

	event_loop(dpy, win);

	glXDestroyContext(dpy, ctx);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}


#ifdef BENCHMARK
#include <sys/time.h>
#include <unistd.h>

/* return current time (in seconds) */
static int
current_time(void)
{
   struct timeval tv;
#ifdef __VMS
   (void) gettimeofday(&tv, NULL );
#else
   struct timezone tz;
   (void) gettimeofday(&tv, &tz);
#endif
   return (int) tv.tv_sec;
}

#else /*BENCHMARK*/

/* dummy */
static int
current_time(void)
{
   return 0;
}

#endif /*BENCHMARK*/

