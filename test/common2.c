/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform https://github.com/ayyi/libwaveform |
* | copyright (C) 2013-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __common2_c__
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#define XLIB_ILLEGAL_ACCESS // needed to access Display internals
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include "agl/actor.h"
#include "waveform/utils.h"
#include "test/common2.h"
#include "waveform/wf_private.h"

#define BENCHMARK
#define NUL '\0'

static GList* windows = NULL;
static GLXContext ctx = {0,};

PFNGLXGETFRAMEUSAGEMESAPROC get_frame_usage = NULL;

static GLboolean has_OML_sync_control = GL_FALSE;
static GLboolean has_SGI_swap_control = GL_FALSE;
static GLboolean has_MESA_swap_control = GL_FALSE;
static GLboolean has_MESA_swap_frame_usage = GL_FALSE;

static char** extension_table = NULL;
static unsigned num_extensions;

static int  current_time           ();


					// TODO can be called from test_init
	static void log_handler(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data)
	{
	  switch(log_level){
		case G_LOG_LEVEL_CRITICAL:
		  printf("%s %s\n", ayyi_err, message);
		  break;
		case G_LOG_LEVEL_WARNING:
		  printf("%s %s\n", ayyi_warn, message);
		  break;
		default:
		  printf("log_handler(): level=%i %s\n", log_level, message);
		  break;
	  }
	}

void
set_log_handlers()
{
	g_log_set_handler (NULL, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);

	char* domain[] = {NULL, "Waveform", "GLib-GObject", "GLib", "Gdk", "Gtk", "AGl"};
	int i; for(i=0;i<G_N_ELEMENTS(domain);i++){
		g_log_set_handler (domain[i], G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	}
}


char*
find_wav (const char* wav)
{
	if(wav[0] == '/'){
		return g_strdup(wav);
	}
	char* filename = g_build_filename(g_get_current_dir(), wav, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	wf_free(filename);

	filename = g_build_filename(g_get_current_dir(), "test", wav, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	wf_free(filename);

	filename = g_build_filename(g_get_current_dir(), "test/data", wav, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	wf_free(filename);

	filename = g_build_filename(g_get_current_dir(), "data", wav, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	wf_free(filename);

	return NULL;
}


const char*
find_data_dir ()
{
	static char* dirs[] = {"test/data", "data"};

	int i; for(i=0;i<G_N_ELEMENTS(dirs);i++){
		if(g_file_test(dirs[i], G_FILE_TEST_EXISTS)){
			return dirs[i];
		}
	}

	return NULL;
}


/**
 * Fill in the table of extension strings from a supplied extensions string
 * (as returned by glXQueryExtensionsString).
 *
 * \param string   String of GLX extensions.
 * \sa is_extension_supported
 */
static void
make_extension_table(const char* string)
{
	char** string_tab;
	unsigned  num_strings;
	unsigned  base;
	unsigned  idx;
	unsigned  i;

	/* Count the number of spaces in the string.  That gives a base-line
	 * figure for the number of extension in the string.
	 */

	num_strings = 1;
	for (i = 0 ; string[i] != NUL ; i++) {
		if (string[i] == ' ') {
			num_strings++;
		}
	}

	string_tab = (char**) malloc(sizeof( char * ) * num_strings);
	if (string_tab == NULL) {
		return;
	}

	base = 0;
	idx = 0;

	while (string[ base ] != NUL) {
	// Determine the length of the next extension string.

		for ( i = 0
		; (string[base + i] != NUL) && (string[base + i] != ' ')
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
 * Determine if an extension is supported. The extension string table
 * must have already be initialized by calling \c make_extension_table.
 *
 * \param ext  Extension to be tested.
 * \return GL_TRUE of the extension is supported, GL_FALSE otherwise.
 * \sa make_extension_table
 */
GLboolean
is_extension_supported( const char * ext )
{
	unsigned i;

	for(i=0;i<num_extensions;i++) {
		if(strcmp(ext, extension_table[i]) == 0){
			return GL_TRUE;
		}
	}

	return GL_FALSE;
}


static void
glx_init (Display* dpy)
{
	PFNGLXSWAPINTERVALMESAPROC set_swap_interval = NULL;
	PFNGLXGETSWAPINTERVALMESAPROC get_swap_interval = NULL;
	GLboolean do_swap_interval = GL_FALSE;
	GLboolean force_get_rate = GL_FALSE;
	int swap_interval = 1;

	// TODO is make_extension_table needed as well as agl_get_extensions ?
	make_extension_table((char*)glXQueryExtensionsString(dpy,DefaultScreen(dpy)));
	has_OML_sync_control = is_extension_supported("GLX_OML_sync_control");
	has_SGI_swap_control = is_extension_supported("GLX_SGI_swap_control");
	has_MESA_swap_control = is_extension_supported("GLX_MESA_swap_control");
	has_MESA_swap_frame_usage = is_extension_supported("GLX_MESA_swap_frame_usage");

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

#define _debug_ false
	if(_debug_){
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
			else{
				(*set_swap_interval)(swap_interval);
			}

			if(_debug_ && (get_swap_interval != NULL)){
				printf("Current swap interval = %d\n", (*get_swap_interval)());
			}
		}
		else {
			printf("Unable to set swap-interval. Neither GLX_SGI_swap_control "
				"nor GLX_MESA_swap_control are supported.\n" );
		}
	}
}


static int
find_window_instance_number(AGlWindow* window)
{
	GList* l = windows;
	int i = 0;
	for(;l;l=l->next){
		AGlWindow* w = l->data;
		if(w->window == window->window) return i + 1;
		i++;
	}
	return -1;
}


static AGlWindow*
window_lookup (Window window)
{
	GList* l = windows;
	for(;l;l=l->next){
		if(((AGlWindow*)l->data)->window == window) return l->data;
	}
	return NULL;
}


static void
scene_needs_redraw (AGlScene* scene, gpointer _){
	scene->gl.glx.needs_draw = true;
}


/*
 * Create an RGB, double-buffered window.
 * Return the window and context handles.
 */
AGlWindow*
agl_make_window (Display* dpy, const char* name, int width, int height, AGlScene* scene)
{
	AGl* agl = agl_get_instance();
	agl->xdisplay = dpy;

	scene->draw = scene_needs_redraw;

	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		None
	};

	int scrnum = DefaultScreen(dpy);
	Window root = RootWindow(dpy, scrnum);

	if(!agl->xvinfo){
		if(!(agl->xvinfo = glXChooseVisual(dpy, scrnum, attrib))){
			printf("Error: couldn't get an RGB, Double-buffered visual\n");
			exit(1);
		}
	}

	/* window attributes */
	XSetWindowAttributes attr = {
		.background_pixel = 0,
		.border_pixel = 0,
		.colormap = XCreateColormap(dpy, root, agl->xvinfo->visual, AllocNone),
		.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask
	};
	unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	Window win = XCreateWindow(dpy, root, 0, 0, width, height, 0, agl->xvinfo->depth, InputOutput, agl->xvinfo->visual, mask, &attr);

	/* set hints and properties */
	{
		XSizeHints sizehints = {
			.x = 0,
			.y = 0,
			.width = width,
			.height = height,
			.flags = USSize | USPosition
		};
		XSetNormalHints(dpy, win, &sizehints);
		XSetStandardProperties(dpy, win, name, name, None, (char **)NULL, 0, &sizehints);
	}

	XMoveWindow(dpy, win, (XDisplayWidth(dpy, scrnum) - width) / 2, (XDisplayHeight(dpy, scrnum) - height) / 2); // centre the window

	if(!windows){
		GLXContext sharelist = NULL;
		ctx = glXCreateContext(dpy, agl->xvinfo, sharelist, True);
		if (!ctx) {
			printf("Error: glXCreateContext failed\n");
			exit(1);
		}
	}

	XMapWindow(dpy, win);

	scene->gl.glx.window = win;
	scene->gl.glx.context = ctx;

	if(!windows){
		glXMakeCurrent(dpy, win, ctx);
		glx_init(dpy);
		agl_gl_init();
	}

	AGlWindow* agl_window = AGL_NEW(AGlWindow,
		.window = win,
		.scene = scene
	);
	windows = g_list_append(windows, agl_window);

	return agl_window;
}


void
agl_window_destroy (Display* dpy, AGlWindow** window)
{
	dbg(1, "%i/%i", find_window_instance_number(*window), g_list_length(windows));

	windows = g_list_remove(windows, *window);

	if(!windows) glXDestroyContext(dpy, (*window)->scene->gl.glx.context);
	XDestroyWindow(dpy, (*window)->window);

	agl_actor__free((AGlActor*)(*window)->scene);

	g_free0(*window);

	// temporary fix for glviewport/glortho getting changed for remaining windows
	GList* l = windows;
	for(;l;l=l->next){
		AGlWindow* w = l->data;
		w->scene->gl.glx.needs_draw = true;
	}
}


/* new window size or exposure */
void
on_window_resize (Display* dpy, AGlWindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double left   = 0;
	double right  = width;
	double bottom = height;
	double top    = 0;
	glOrtho (left, right, bottom, top, 1.0, -1.0);

	((AGlActor*)window->scene)->region = (AGlfRegion){
		.x2 = width,
		.y2 = height,
	};
	agl_actor__set_size((AGlActor*)window->scene);
}


static void
draw (Display* dpy, AGlWindow* window)
{
	AGlActor* scene = (AGlActor*)window->scene;

	static long unsigned current = 0;

	if(window->window != current){
		glXMakeCurrent(dpy, window->window, window->scene->gl.glx.context);

		int width = scene->region.x2;
		int height = scene->region.y2;
		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		double left   = 0;
		double right  = width;
		double bottom = height;
		double top    = 0;
		glOrtho (left, right, bottom, top, 1.0, -1.0);
	}
	current = window->window;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	agl_actor__paint(scene);
}


void
event_loop (Display* dpy)
{
	float frame_usage = 0.0;
	fd_set rfds;

	while (1) {
		FD_ZERO(&rfds);
		FD_SET(dpy->fd, &rfds);
		select(dpy->fd + 1, &rfds, NULL, NULL, &(struct timeval){.tv_usec = 50000});

		while (XPending(dpy) > 0) {
			XEvent event;
			XNextEvent(dpy, &event);
			AGlWindow* window = window_lookup(((XAnyEvent*)&event)->window);
			if(window){
				switch (event.type) {
					case Expose:
						window->scene->gl.glx.needs_draw = True;
						break;
					case ConfigureNotify:
						on_window_resize(dpy, window, event.xconfigure.width, event.xconfigure.height);
						window->scene->gl.glx.needs_draw = True;
						break;
					case KeyPress: {
						int code = XLookupKeysym(&event.xkey, 0);

						extern KeyHandler* key_lookup  (int keycode);
						KeyHandler* handler = key_lookup(code);
						if(handler){
							handler(NULL);
						}else{
							char buffer[10];
							XLookupString(&event.xkey, buffer, sizeof(buffer), NULL, NULL);
							if (buffer[0] == 27 || buffer[0] == 'q') {
								/* escape */
								return;
							}
						}
					}
				}
			}

			if (get_frame_usage) {
				GLfloat temp;

				(*get_frame_usage)(dpy, window->window, & temp);
				frame_usage += temp;
			}
		}

		GList* l = windows;
		for(;l;l=l->next){
			AGlWindow* window = l->data;
			if(window->scene->gl.glx.needs_draw){
				draw(dpy, window);
				glXSwapBuffers(dpy, window->window);
				window->scene->gl.glx.needs_draw = false;
			}
		}

		/* calc framerate */
#if 0
		if(print_info){
#else
		if(false){
#endif
			static int t0 = -1;
			static int frames = 0;
			int t = current_time();

			if (t0 < 0) t0 = t;

			frames++;

			if (t - t0 >= 5.0) {
				GLfloat seconds = t - t0;
				GLfloat fps = frames / seconds;
				if (get_frame_usage) {
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

		int i = 0;
		while(g_main_context_iteration(NULL, false) && i++ < 32); // update animations, idle callbacks etc
	}
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
   (void) gettimeofday(&tv, NULL);
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


/**
 * Display the refresh rate of the display using the GLX_OML_sync_control
 * extension.
 */
void
show_refresh_rate(Display* dpy)
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
	printf("glXGetMscRateOML not supported.\n");
}


static bool
on_window_delete (GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	gtk_main_quit();

	return false;
}


int
gtk_window (Key keys[], WindowFn content)
{
	GdkGLConfig* glconfig;
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		gerr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
	}

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	content((GtkWindow*)window, glconfig);

	gtk_widget_show_all(window);

	add_key_handlers_gtk((GtkWindow*)window, NULL, keys);

	g_signal_connect(window, "delete-event", G_CALLBACK(on_window_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


	static KeyHold key_hold = {0, NULL};
	static bool key_down = false;
	static GHashTable* key_handlers = NULL;

	static gboolean key_hold_on_timeout(gpointer user_data)
	{
		WaveformView* waveform = user_data;
		if(key_hold.handler) key_hold.handler(waveform);
		return TIMER_CONTINUE;
	}

	static gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		if(key_down){
			// key repeat
			return true;
		}

		KeyHandler* handler = g_hash_table_lookup(key_handlers, &event->keyval);
		if(handler){
			key_down = true;
			if(key_hold.timer) gwarn("timer already started");
			key_hold.timer = g_timeout_add(100, key_hold_on_timeout, user_data);
			key_hold.handler = handler;

			handler(user_data);
		}
		else dbg(1, "%i", event->keyval);

		return key_down;
	}

	static gboolean key_release(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		PF;
		if(!key_down) return AGL_NOT_HANDLED; // sometimes happens at startup

		key_down = false;
		g_source_remove0(key_hold.timer);

		return true;
	}

void
add_key_handlers_gtk (GtkWindow* window, WaveformView* waveform, Key keys[])
{
	//list of keys must be terminated with a key of value zero.

	key_handlers = g_hash_table_new(g_int_hash, g_int_equal);
	int i = 0; while(true){
		Key* key = &keys[i];
		if(i > 100 || !key->key) break;
		g_hash_table_insert(key_handlers, &key->key, key->handler);
		i++;
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), waveform);
	g_signal_connect(window, "key-release-event", G_CALLBACK(key_release), waveform);
}


void
add_key_handlers (Key keys[])
{
	if(!key_handlers){
		key_handlers = g_hash_table_new(g_int_hash, g_int_equal);

		int i = 0; while(true) {
			Key* key = &keys[i];
			if(i > 100 || !key->key) break;
			g_hash_table_insert(key_handlers, &key->key, key->handler);
			i++;
		}
	}
}


KeyHandler*
key_lookup (int keycode)
{
	return g_hash_table_lookup(key_handlers, &keycode);
}

