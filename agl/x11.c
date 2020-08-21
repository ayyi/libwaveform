/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define XLIB_ILLEGAL_ACCESS // needed to access Display internals
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "agl/debug.h"
#include "agl/ext.h"
#include "agl/x11.h"

#ifdef USE_XRANDR
#include "X11/extensions/Xrandr.h"
#endif

#define NUL '\0'
#undef SWAP_INTERVAL

#ifdef SWAP_INTERVAL
#ifndef GLX_MESA_swap_control
typedef GLint (*PFNGLXSWAPINTERVALMESAPROC)    (unsigned interval);
typedef GLint (*PFNGLXGETSWAPINTERVALMESAPROC) (void);
#endif
static GLboolean has_SGI_swap_control = false;
static GLboolean has_MESA_swap_control = false;
#endif

Display* dpy = NULL;

static GList* windows = NULL;
static GLXContext ctx = {0,};
static GMainLoop* _mainloop = NULL;
static long unsigned current = 0;

static char** extension_table = NULL;
static unsigned num_extensions;

static Atom wm_protocol;
static Atom wm_close;

PFNGLXGETFRAMEUSAGEMESAPROC get_frame_usage = NULL;

static GLboolean has_OML_sync_control = false;
static GLboolean has_MESA_swap_frame_usage = false;

#define make_current(dpy, W, context) ({bool ok = true; if(W != current) ok = glXMakeCurrent(dpy, W, context); current = W; ok;})

static void set_fullscreen              (Window);
static void no_border                   (Display*, Window);
static void draw                        (AGlScene*, gpointer);
static int  find_window_instance_number (AGlWindow*);
static void on_window_resize            (AGlWindow*, int width, int height);
static void show_refresh_rate           ();
static void make_extension_table        (const char*);
static bool is_extension_supported      (const char*);


static void
glx_init (Display* dpy)
{
	PFNGLXGETSWAPINTERVALMESAPROC get_swap_interval = NULL;

	make_extension_table((char*)glXQueryExtensionsString(dpy,DefaultScreen(dpy)));
	has_OML_sync_control = is_extension_supported("GLX_OML_sync_control");
	has_MESA_swap_frame_usage = is_extension_supported("GLX_MESA_swap_frame_usage");

	if (has_MESA_swap_frame_usage) {
		get_frame_usage = (PFNGLXGETFRAMEUSAGEMESAPROC)  glXGetProcAddressARB((const GLubyte*) "glXGetFrameUsageMESA");
	}

#ifdef DEBUG
	if(wf_debug){
		printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
		printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
		printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
		printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
		if(has_OML_sync_control){
			show_refresh_rate();
		}
		if(get_swap_interval){
			printf("Default swap interval = %d\n", (*get_swap_interval)());
		}
	}
#endif

#ifdef SWAP_INTERVAL
	PFNGLXSWAPINTERVALMESAPROC set_swap_interval = NULL;

	has_SGI_swap_control = is_extension_supported("GLX_SGI_swap_control");
	has_MESA_swap_control = is_extension_supported("GLX_MESA_swap_control");

	int swap_interval = 1;

	if (has_MESA_swap_control) {
		set_swap_interval = (PFNGLXSWAPINTERVALMESAPROC) glXGetProcAddressARB((const GLubyte*) "glXSwapIntervalMESA");
		get_swap_interval = (PFNGLXGETSWAPINTERVALMESAPROC) glXGetProcAddressARB((const GLubyte*) "glXGetSwapIntervalMESA");
	}
	else if (has_SGI_swap_control) {
		set_swap_interval = (PFNGLXSWAPINTERVALMESAPROC) glXGetProcAddressARB((const GLubyte*) "glXSwapIntervalSGI");
	}

	if(set_swap_interval != NULL){
		if(((swap_interval == 0) && !has_MESA_swap_control) || (swap_interval < 0)){
			printf( "Swap interval must be non-negative or greater than zero "
				"if GLX_MESA_swap_control is not supported.\n" );
		}
		else{
			(*set_swap_interval)(swap_interval);
		}

		if(wf_debug && (get_swap_interval != NULL)){
			printf("Current swap interval = %d\n", (*get_swap_interval)());
		}
	}
	else {
		printf("Unable to set swap-interval. Neither GLX_SGI_swap_control "
			"nor GLX_MESA_swap_control are supported.\n" );
	}
#endif
}


/**
 * Remove window border/decorations.
 */
static void
no_border (Display* dpy, Window w)
{
	static const unsigned MWM_HINTS_DECORATIONS = (1 << 1);
	static const int PROP_MOTIF_WM_HINTS_ELEMENTS = 5;

	typedef struct
	{
		unsigned long    flags;
		unsigned long    functions;
		unsigned long    decorations;
		long             inputMode;
		unsigned long    status;
	} PropMotifWmHints;

	unsigned long flags = 0;

	// setup the property
	PropMotifWmHints motif_hints = {
		.flags       = MWM_HINTS_DECORATIONS,
		.decorations = flags
	};

	// get the atom for the property
	Atom prop = XInternAtom(dpy, "_MOTIF_WM_HINTS", True);
	if (!prop) {
		return;
	}

	// not sure this is correct, seems to work, XA_WM_HINTS didn't work
	Atom proptype = prop;

	XChangeProperty(dpy, w,
		prop, proptype,                 /* property, type */
		32,                             /* format: 32-bit datums */
		PropModeReplace,                /* mode */
		(unsigned char *) &motif_hints, /* data */
		PROP_MOTIF_WM_HINTS_ELEMENTS    /* n_elements */
	);
}


static AGlWindow*
window_lookup (Window window)
{
	for(GList* l = windows;l;l=l->next){
		if(((AGlWindow*)l->data)->window == window)
			return l->data;
	}
	return NULL;
}


/*
 * Create an RGB, double-buffered window.
 * Return the window and context handles.
 */
AGlWindow*
agl_window (const char* name, int x, int y, int width, int height, bool fullscreen)
{
	if(!dpy && !(dpy = XOpenDisplay(NULL))){
		printf("Error: couldn't open display %s\n", XDisplayName(NULL));
		return NULL;
	}

	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		GLX_STENCIL_SIZE, 8,
		None
	};

	int scrnum = DefaultScreen(dpy);
	Window root = RootWindow(dpy, scrnum);

#ifdef USE_XRANDR
	if (fullscreen) {
		x = y = 0;

		int n_monitors = 0;
		XRRMonitorInfo* monitors = XRRGetMonitors(dpy, root, true, &n_monitors);
		for(int i=0;i<n_monitors;i++){
			XRRMonitorInfo m = monitors[i];
			if(!i || m.primary){
				width = m.width;
				height = m.height;
			}
		}
	}
#endif

	XVisualInfo* visinfo = glXChooseVisual(dpy, scrnum, attrib);
	if (!visinfo) {
		printf("Error: couldn't get an RGB, Double-buffered visual\n");
		exit(1);
	}

	XSetWindowAttributes attr = {
		.background_pixel = 0,
		.border_pixel = 0,
		.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone),
		.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask
	};
	unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	Window win = XCreateWindow(dpy, root, 0, 0, width, height, 0, visinfo->depth, InputOutput, visinfo->visual, mask, &attr);

	/* set hints and properties */
	{
		XSizeHints sizehints = {
			.x = x,
			.y = y,
			.width  = width,
			.height = height,
			.flags = USSize | USPosition
		};
		XSetNormalHints(dpy, win, &sizehints);
		XSetStandardProperties(dpy, win, name, name, None, (char**)NULL, 0, &sizehints);
	}

	wm_protocol = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wm_close = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	// Receive the 'close' event from the WM
	XSetWMProtocols(dpy, win, &wm_close, 1);

	XMoveWindow(dpy, win, x, y);

	if (fullscreen){
		no_border(dpy, win);
		set_fullscreen (win);
	}

	if(!windows){
		GLXContext sharelist = NULL;
		ctx = glXCreateContext(dpy, visinfo, sharelist, True);
		if (!ctx) {
			printf("Error: glXCreateContext failed\n");
			exit(1);
		}
	}

	XFree(visinfo);

	XMapWindow(dpy, win);

#if 0
	{
		Atom wmState = XInternAtom(dpy, "_NET_WM_STATE", true);
		Atom type;
		int format;
		unsigned long nItems, bytesAfter;
		unsigned char* properties = NULL;
		XGetWindowProperty(dpy, win, wmState, 0, (~0L), False, AnyPropertyType, &type, &format, &nItems, &bytesAfter, &properties);
		printf("n _NET_WM_STATE properties: %ld\n", nItems);
		for (int i = 0; i < nItems; i++)
			printf("  property=%ld\n", ((long*)properties)[i]);
	}
#endif

	if(!windows){
		make_current(dpy, win, ctx);
		glx_init(dpy);
		agl_gl_init();
	}

	AGlScene* scene = (AGlScene*)agl_actor__new_root_(CONTEXT_TYPE_GLX);

	scene->draw = draw;
	scene->gl.glx.window = win;
	scene->gl.glx.context = ctx;

	AGlWindow* agl_window = AGL_NEW(AGlWindow,
		.window = win,
		.scene = scene,
	);

	windows = g_list_append(windows, agl_window);

	return agl_window;
}


void
agl_window_destroy (AGlWindow** window)
{
	dbg(1, "%i/%i", find_window_instance_number(*window), g_list_length(windows));

	windows = g_list_remove(windows, *window);

	if(!windows){
		glXDestroyContext(dpy, (*window)->scene->gl.glx.context);
		(*window)->scene->gl.glx.context = NULL;
	}
	XDestroyWindow(dpy, (*window)->window);

	agl_actor__free((AGlActor*)(*window)->scene);

	g_clear_pointer(window, g_free);

	// temporary fix for glviewport/glortho getting changed for remaining windows
	for(GList* l=windows;l;l=l->next){
		AGlWindow* w = l->data;
		agl_scene_queue_draw(w->scene);
	}
}


/**
 * agl_window_set_icons:
 * @window: The Window for which to set the icon.
 * @pixbufs: (transfer full) (element-type GdkPixbuf):
 *     A list of pixbufs, of different sizes.
 *
 * Sets a list of icons for the window. One of these will be used
 * to represent the window when it has been iconified. The icon is
 * usually shown in an icon box or some sort of task bar. Which icon
 * size is shown depends on the window manager. The window manager
 * can scale the icon  but setting several size icons can give better
 * image quality since the window manager may only need to scale the
 * icon by a small amount or not at all.
 *
 **/
void
agl_window_set_icons (Window window, GList* pixbufs)
{
	GList* l = pixbufs;
	int size = 0;
	int n = 0;
	while (l) {
		GdkPixbuf* pixbuf = l->data;
		g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

		int width = gdk_pixbuf_get_width (pixbuf);
		int height = gdk_pixbuf_get_height (pixbuf);

		/* silently ignore overlarge icons */
#if 0
		if (size + 2 + width * height > GDK_SELECTION_MAX_SIZE(display)) {
			g_warning ("gdk_window_set_icon_list: icons too large");
			break;
		}
#endif

		n++;
		size += 2 + width * height;

		l = g_list_next (l);
	}

	gulong* data = g_malloc (size * sizeof (gulong));

	l = pixbufs;
	gulong* p = data;
	while (l && n > 0) {
		GdkPixbuf* pixbuf = l->data;

		int width = gdk_pixbuf_get_width (pixbuf);
		int height = gdk_pixbuf_get_height (pixbuf);
		int stride = gdk_pixbuf_get_rowstride (pixbuf);
		int n_channels = gdk_pixbuf_get_n_channels (pixbuf);

		*p++ = width;
		*p++ = height;

		guchar* pixels= gdk_pixbuf_get_pixels (pixbuf);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				guchar r = pixels[y*stride + x*n_channels + 0];
				guchar g = pixels[y*stride + x*n_channels + 1];
				guchar b = pixels[y*stride + x*n_channels + 2];
				guchar a = (n_channels >= 4)
					? pixels[y*stride + x*n_channels + 3]
					: 255;

				*p++ = a << 24 | r << 16 | g << 8 | b ;
			}
		}

		g_object_unref(pixbuf);

		l = g_list_next (l);
		n--;
	}

	if (size > 0) {
		XChangeProperty (dpy, window, XInternAtom(dpy, "_NET_WM_ICON", False), XA_CARDINAL, 32, PropModeReplace, (guchar*) data, size);
	} else {
		XDeleteProperty (dpy, window, XInternAtom(dpy, "_NET_WM_ICON", False));
	}

	g_free (data);
	g_list_free(pixbufs);
}

static void
on_window_resize (AGlWindow* window, int width, int height)
{
	make_current(dpy, window->window, window->scene->gl.glx.context);

	dbg (1, "%i x %i", width, height);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double left   = 0;
	double right  = width;
	double bottom = height;
	double top    = 0;
	glOrtho (left, right, bottom, top, 1.0, -1.0);

	if(width != agl_actor__width((AGlActor*)window->scene) || height != agl_actor__height((AGlActor*)window->scene)){
		((AGlActor*)window->scene)->region = (AGlfRegion){
			.x2 = width,
			.y2 = height,
		};
		agl_actor__set_size((AGlActor*)window->scene);
	}
}


bool
agl_is_fullscreen (Window win)
{
	bool fs = false;

	Atom netwmstate = XInternAtom(dpy, "_NET_WM_STATE", True);
	Atom netwmstatefullscreen = XInternAtom (dpy, "_NET_WM_STATE_FULLSCREEN", True);
	Atom actualType;
	int format;
	unsigned long n_items, bytesAfter;
	unsigned char* data = NULL;

	int result = XGetWindowProperty(dpy,
		win,
		netwmstate,
		0L,
		(~0L),
		False,
		AnyPropertyType,
		&actualType,
		&format,
		&n_items,
		&bytesAfter,
		&data
	);

	if (result == Success && data){
		for(int i=0;i<n_items;i++){
			if ((fs = (((Atom*)data)[i] == netwmstatefullscreen))){
				break;
			}
		}
		XFree(data);
	}

	return fs;
}


/*
 *  Set fullscreen property. Must be run before window is mapped to have any effect.
 */
static void
set_fullscreen (Window win)
{
	Atom atoms[1] = {
		XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false),
	};

	XChangeProperty(
		dpy,
		win,
		XInternAtom(dpy, "_NET_WM_STATE", false),
		XA_ATOM,
		32,
		PropModeReplace,
		(guchar*)atoms,
		1
	);
}


void
agl_toggle_fullscreen (Window win)
{
	#define _NET_WM_STATE_ADD    1
	#define _NET_WM_STATE_TOGGLE 2

	XClientMessageEvent xclient = {
		.type = ClientMessage,
		.message_type = XInternAtom(dpy, "_NET_WM_STATE", 1),
		.format = 32,
		.window = win,
		.data.l[0] = _NET_WM_STATE_TOGGLE,
		.data.l[1] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false)
	};

	XSendEvent(dpy,
		DefaultRootWindow(dpy),
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		(XEvent *)&xclient
	);
}


static int
find_window_instance_number (AGlWindow* window)
{
	GList* l = windows;
	for(int i=0;l;l=l->next){
		AGlWindow* w = l->data;
		if(w->window == window->window) return i + 1;
		i++;
	}
	return -1;
}


#if 0
static int
find_window_instance_number_by_scene (AGlScene* scene)
{
	GList* l = windows;
	for(int i=0;l;l=l->next){
		AGlWindow* w = l->data;
		if(w->scene == scene) return i + 1;
		i++;
	}
	return -1;
}
#endif


static void
draw (AGlScene* scene, gpointer _)
{
	AGlActor* actor = (AGlActor*)scene;

	if(actor->region.x2 < 0.1 || actor->region.y2 < 0.1){
		return;
	}

	if(scene->gl.glx.window != current){
		glXMakeCurrent(dpy, scene->gl.glx.window, scene->gl.glx.context);

		int width = actor->region.x2;
		int height = actor->region.y2;
		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		double left   = 0;
		double right  = width;
		double bottom = height;
		double top    = 0;
		glOrtho (left, right, bottom, top, 1.0, -1.0);
	}
	current = scene->gl.glx.window;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	agl_actor__paint(actor);
	glXSwapBuffers(dpy, current);

#ifdef DEBUG
	static float frame_usage = 0.f;

	if (get_frame_usage) {
		GLfloat temp;

		(*get_frame_usage)(dpy, ((AGlWindow*)windows->data)->window, & temp);
		frame_usage += temp;
	}

	if(wf_debug > 1){
		// calc framerate
		static int t0 = -1;
		static int frames = 0;
		int t = g_get_monotonic_time() / 1000000;

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
#endif
}


static gboolean
stop (gpointer _)
{
	g_main_loop_quit(_mainloop);

	return G_SOURCE_REMOVE;
}


/*
 *  Called before the x11 file descriptor is polled. If the source can determine
 *  that it is ready here (without waiting for the results of the poll() call) it
 *  should return TRUE.
 *
 *  "For file descriptor sources, the prepare function typically returns FALSE,
 *  since it must wait until poll() has been called before it knows whether any
 *  events need to be processed. It sets the returned timeout to -1 to indicate
 *  that it doesn't mind how long the poll() call blocks. In the check function, it
 *  tests the results of the poll() call to see if the required condition has been
 *  met, and returns TRUE if so."
 */
static gboolean
x11_fd_prepare (GSource* source, gint* timeout)
{
	*timeout = -1;

	// if event ready, go straight to dispatch, otherwise poll.
	return XPending(dpy);
}


/*
 *  Called after the file descriptor is polled. The source should return TRUE
 *  if it is ready to be dispatched. Note that some time may have passed since the
 *  previous prepare function was called, so the source should be checked again here. 
 */
static gboolean
x11_fd_check (GSource* source)
{
	return XPending(dpy);
}


/*
 *  Called to dispatch the event source, after it has returned TRUE in either its prepare or its check function.
 *
 *  The dispatch function is passed in a callback function and data. The callback function
 *  may be NULL if the source was never connected to a callback using g_source_set_callback().
 *  The dispatch function should call the callback function with user_data and whatever additional
 *  parameters are needed for this type of event source.
 */
static gboolean
x11_fd_dispatch (GSource* source, GSourceFunc callback, gpointer user_data)
{
	while (XPending(dpy) > 0) {
		XEvent event;
		XNextEvent(dpy, &event);
		AGlWindow* window = window_lookup(((XAnyEvent*)&event)->window);
		if(window){
			switch (event.type) {
				case ClientMessage:
					dbg(1, "client message");
					if (
						(event.xclient.message_type == wm_protocol) // coming from the WM
						&& ((Atom)event.xclient.data.l[0] == wm_close)
					) {
						if(g_list_length(windows) > 1 && window != ((AGlWindow*)windows->data)){
							agl_window_destroy(&window);
						}else{
							g_idle_add(stop, NULL);
						}
					}
					break;
				case Expose:
					agl_scene_queue_draw(window->scene);
					break;
				case ConfigureNotify:
					// There was a change to size, position, border, or stacking order.
					dbg(1, "Configure %i: %ix%i (scene=%.0fx%.0f)", g_list_index(windows, window), event.xconfigure.width, event.xconfigure.height, agl_actor__width((AGlActor*)window->scene), agl_actor__height((AGlActor*)window->scene));
					on_window_resize(window, event.xconfigure.width, event.xconfigure.height);
					break;
				case MotionNotify:
					agl_actor__xevent(window->scene, &event);
					break;
				case KeyPress: {
					char buffer[10];

#if 0
					bool shift = ((XKeyEvent*)&event)->state & ShiftMask;
					bool control = ((XKeyEvent*)&event)->state & ControlMask;
#endif
					XLookupString(&event.xkey, buffer, sizeof(buffer), NULL, NULL);
					switch(buffer[0]){
						case 27: // ESC
							if(g_list_length(windows) > 1 && window != ((AGlWindow*)windows->data)){
								agl_window_destroy(&window);
								break;
							}
							// falling through ...
						case 'q':
							g_idle_add(stop, NULL);
							return G_SOURCE_REMOVE;
						default:
							agl_actor__xevent(window->scene, &event);
					}

					} break;
				case KeyRelease: {
						agl_actor__xevent(window->scene, &event);
					} break;
				case ButtonPress:
				case ButtonRelease:
					if(event.xbutton.button == 1){
						int x = event.xbutton.x;
						int y = event.xbutton.y;
						dbg(1, "button: %i %i\n", x, y);
					}
					agl_actor__xevent(window->scene, &event);
					break;
			}
		}
	}

	return G_SOURCE_CONTINUE;
}


GMainLoop*
agl_main_loop_new ()
{
	g_return_val_if_fail(dpy, NULL);
	g_return_val_if_fail(!_mainloop, NULL);

	AGlWindow* window = windows->data;
	// maybe this can be moved to agl_window()
	XSelectInput(dpy, window->window, StructureNotifyMask|ExposureMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|PropertyChangeMask|KeyPressMask|KeyReleaseMask);

	_mainloop = g_main_loop_new(NULL, FALSE);

	static GPollFD pollfd;
	pollfd = (GPollFD){dpy->fd, G_IO_IN | G_IO_HUP | G_IO_ERR, 0};

	static GSourceFuncs x11_source_funcs = {
		x11_fd_prepare,
		x11_fd_check,
		x11_fd_dispatch,
		NULL
	};

	GSource* x11_source = g_source_new(&x11_source_funcs, sizeof(GSource));
	g_source_set_priority (x11_source, GDK_PRIORITY_EVENTS);
	g_source_set_can_recurse (x11_source, TRUE);
	g_source_add_poll(x11_source, &pollfd);
	g_source_attach(x11_source, NULL);

	return _mainloop;
}


/**
 * Display the refresh rate of the display using the GLX_OML_sync_control
 * extension.
 */
#ifdef DEBUG
static void
show_refresh_rate ()
{
#if defined(GLX_OML_sync_control) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
	int32_t n;
	int32_t d;

	PFNGLXGETMSCRATEOMLPROC get_msc_rate = (PFNGLXGETMSCRATEOMLPROC)glXGetProcAddressARB((const GLubyte*) "glXGetMscRateOML");
	if (get_msc_rate != NULL) {
		(*get_msc_rate)(dpy, glXGetCurrentDrawable(), &n, &d);
		printf( "refresh rate: %.1fHz\n", (float) n / d);
		return;
	}
#endif
	printf("glXGetMscRateOML not supported.\n");
}
#endif


/**
 * Determine if an extension is supported. The extension string table
 * must have already be initialized by calling \c make_extension_table.
 *
 * \param ext  Extension to be tested.
 * \return GL_TRUE of the extension is supported, GL_FALSE otherwise.
 * \sa make_extension_table
 */
static bool
is_extension_supported (const char* ext)
{
	for(unsigned i=0;i<num_extensions;i++) {
		if(strcmp(ext, extension_table[i]) == 0){
			return GL_TRUE;
		}
	}

	return GL_FALSE;
}


/**
 * Fill in the table of extension strings from a supplied extensions string
 * (as returned by glXQueryExtensionsString).
 *
 * \param string   String of GLX extensions.
 * \sa is_extension_supported
 */
static void
make_extension_table (const char* string)
{
	/* Count the number of spaces in the string.  That gives a base-line
	 * figure for the number of extension in the string.
	 */

	unsigned num_strings = 1;
	for (unsigned i = 0; string[i] != NUL; i++) {
		if (string[i] == ' ') {
			num_strings++;
		}
	}

	char** string_tab;
	if(!(string_tab = (char**) malloc(sizeof(char*) * num_strings))){
		return;
	}

	unsigned base = 0;
	unsigned idx = 0;

	while (string[base] != NUL) {
		// Determine the length of the next extension string.
		unsigned i;

		for (i = 0
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

			string_tab[idx] = malloc(sizeof(char) * (i + 1));
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

