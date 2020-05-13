/*
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

  ---------------------------------------------------------------

  WaveformView is a Gtk widget based on GtkDrawingArea.
  It displays an audio waveform represented by a Waveform object.

*/
#define __wf_private__
#define __waveform_view_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include "agl/utils.h"
#include "waveform/waveform.h"
#include "waveform/view.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define DIRECT 1
#define DEFAULT_HEIGHT 64
#define DEFAULT_WIDTH 256
#define GL_HEIGHT 256.0 // same as the height of the waveform texture

static GdkGLConfig*  glconfig = NULL;
static GdkGLContext* gl_context = NULL;

#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _WaveformViewPrivate {
	AGlActor*       root;
	WaveformContext* context;
	WaveformActor*  actor;
	AMPromise*      ready;
};

enum {
    PROMISE_DISP_READY,
    PROMISE_WAVE_READY,
    PROMISE_MAX
};
#define promise(A) ((AMPromise*)g_list_nth_data(view->priv->ready->children, A))

static gpointer waveform_view_parent_class = NULL;

#define WAVEFORM_VIEW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_WAVEFORM_VIEW, WaveformViewPrivate))
enum  {
	WAVEFORM_VIEW_DUMMY_PROPERTY
};
static gboolean waveform_view_on_expose            (GtkWidget*, GdkEventExpose*);
static gboolean waveform_view_button_press_event   (GtkWidget*, GdkEventButton*);
static gboolean waveform_view_button_release_event (GtkWidget*, GdkEventButton*);
static gboolean waveform_view_motion_notify_event  (GtkWidget*, GdkEventMotion*);
static void     waveform_view_realize              (GtkWidget*);
static void     waveform_view_unrealize            (GtkWidget*);
static void     waveform_view_allocate             (GtkWidget*, GdkRectangle*);
static void     waveform_view_finalize             (GObject*);
static void     waveform_view_set_projection       (GtkWidget*);
static int      waveform_view_get_width            (WaveformView*);


static gboolean
__init ()
{
	dbg(2, "...");

	if(!gl_context){
		gtk_gl_init(NULL, NULL);
		if(wf_debug){
			gint major, minor;
#ifdef USE_SYSTEM_GTKGLEXT
			gdk_gl_query_version (&major, &minor);
#else
			glXQueryVersion (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), &major, &minor);
#endif
			g_print ("GLX version %d.%d\n", major, minor);
		}
	}

	glconfig = gdk_gl_config_new_by_mode( GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE );
	if (!glconfig) { gerr ("Cannot initialise gtkglext."); return false; }

	return true;
}


/*
 *  For use where the widget needs to share an opengl context with other items.
 *  Should be called before any widgets are instantiated.
 */
void
waveform_view_set_gl (GdkGLContext* _gl_context)
{
	gl_context = _gl_context;
}


WaveformView*
construct ()
{
	WaveformView* self = (WaveformView*) g_object_new (TYPE_WAVEFORM_VIEW, NULL);

	gtk_widget_add_events ((GtkWidget*) self, (gint) ((GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK) | GDK_POINTER_MOTION_MASK));
	if(!gtk_widget_set_gl_capability((GtkWidget*)self, glconfig, gl_context ? gl_context : agl_get_gl_context(), DIRECT, GDK_GL_RGBA_TYPE)){
		gwarn("failed to set gl capability");
	}

	return self;
}


	static void display_ready(AGlActor* a)
	{
		WaveformView* view = (WaveformView*)((AGlRootActor*)a)->gl.gdk.widget;
		WaveformViewPrivate* v = view->priv;
		v->context = wf_context_new(v->root);

		waveform_view_set_projection(((AGlRootActor*)a)->gl.gdk.widget);

		am_promise_resolve(g_list_nth_data(v->ready->children, PROMISE_DISP_READY), NULL);
	}

WaveformView*
waveform_view_new (Waveform* waveform)
{
	PF;

	g_return_val_if_fail(glconfig || __init(), NULL);

	WaveformView* view = construct ();
	GtkWidget* widget = (GtkWidget*)view;
	WaveformViewPrivate* v = view->priv;

	view->waveform = waveform ? g_object_ref(waveform) : NULL;

#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus(widget, TRUE);
#endif
	gtk_widget_set_size_request(widget, DEFAULT_WIDTH, DEFAULT_HEIGHT);

	view->priv->ready = am_promise_new(view);
	am_promise_when(view->priv->ready, am_promise_new(view), am_promise_new(view), NULL);

	v->root = agl_actor__new_root(widget);
	v->root->init = display_ready;

	((AGlScene*)v->root)->user_data = view;

	return view;
}


	static void _waveform_view_on_draw(AGlScene* scene, gpointer _view)
	{
		gtk_widget_queue_draw((GtkWidget*)_view);
	}

static void
_waveform_view_set_actor (WaveformView* view)
{
	WaveformActor* actor = view->priv->actor;
	WaveformViewPrivate* v = view->priv;

	int width = waveform_view_get_width(view);
	wf_actor_set_rect(actor, &(WfRectangle){0, 0, width, GL_HEIGHT});

	((AGlScene*)v->root)->draw = _waveform_view_on_draw;
}


static void
_show_waveform(gpointer _view, gpointer _c)
{
	// this must NOT be called until the canvas is ready

	WaveformView* view = _view;
	g_return_if_fail(view);
	WaveformViewPrivate* v = view->priv;
	g_return_if_fail(v->context);

	if(v->actor) return; // nothing to do

	if(view->waveform){ // it is valid for the widget to not have a waveform set.
		v->actor = wf_canvas_add_new_actor(v->context, view->waveform);

		agl_actor__add_child(v->root, (AGlActor*)v->actor);

		uint64_t n_frames = waveform_get_n_frames(view->waveform);
		if(n_frames){
			wf_actor_set_region(v->actor, &(WfSampleRegion){0, n_frames});

			// the waveform may be shared so may already be loaded.
			if(!view->waveform->priv->peak.size) waveform_load_sync(view->waveform);

			am_promise_resolve(promise(PROMISE_WAVE_READY), NULL);
		}
		_waveform_view_set_actor(view);

		gtk_widget_queue_draw((GtkWidget*)view);
	}
}


void
waveform_view_load_file (WaveformView* view, const char* filename)
{
	// if filename is NULL the display will be cleared.

	WaveformViewPrivate* v = view->priv;

	if(v->actor){
		v->actor = (agl_actor__remove_child(v->root, (AGlActor*)v->actor), NULL);
	}
	else dbg(2, " ... no actor");
	if(view->waveform){
		_g_object_unref0(view->waveform);
	}

	if(!filename){
		gtk_widget_queue_draw((GtkWidget*)view);
		return;
	}

	view->waveform = waveform_new(filename);

	am_promise_add_callback(promise(PROMISE_DISP_READY), _show_waveform, NULL);
}


void
waveform_view_set_waveform (WaveformView* view, Waveform* waveform)
{
	PF;
	WaveformViewPrivate* v = view->priv;

	if(v->actor && v->context){
		v->actor = (agl_actor__remove_child(v->root, (AGlActor*)v->actor), NULL);
	}
	if(view->waveform){
		g_object_unref(view->waveform);
	}

	view->waveform = g_object_ref(waveform); //TODO a reference is added in wf_canvas_add_new_actor so this is not really neccesary.
	view->zoom = 1.0;

	am_promise_add_callback(promise(PROMISE_DISP_READY), _show_waveform, NULL);
}


void
waveform_view_set_zoom (WaveformView* view, float zoom)
{
	WaveformViewPrivate* v = view->priv;

	#define MAX_ZOOM 51200.0 //TODO
	g_return_if_fail(view);
	dbg(1, "zoom=%.2f", zoom);
	view->zoom = CLAMP(zoom, 1.0, MAX_ZOOM);

	WfSampleRegion region = {view->start_frame, (waveform_get_n_frames(view->waveform) - view->start_frame) / view->zoom};
	wf_actor_set_region(v->actor, &region);

	if(!((AGlScene*)v->root)->draw) gtk_widget_queue_draw((GtkWidget*)view);
}


void
waveform_view_set_start (WaveformView* view, int64_t start_frame)
{
	WaveformViewPrivate* v = view->priv;

	// the number of visible frames is reduced as the zoom increases.
	int64_t n_frames_visible = waveform_get_n_frames(view->waveform) / view->zoom;

	view->start_frame = CLAMP(
		start_frame,
		0,
		(int64_t)(waveform_get_n_frames(view->waveform) - MAX(10, n_frames_visible))
	);
	dbg(1, "start=%Lu", view->start_frame);
	wf_actor_set_region(view->priv->actor, &(WfSampleRegion){
		view->start_frame,
		n_frames_visible
	});
	if(!((AGlActor*)v->actor)->root->draw) gtk_widget_queue_draw((GtkWidget*)view);
}


void
waveform_view_set_region (WaveformView* view, int64_t start_frame, int64_t end_frame)
{
	WaveformViewPrivate* v = view->priv;

	uint32_t max_len = waveform_get_n_frames(view->waveform) - start_frame;
	uint32_t length = MIN(max_len, end_frame - start_frame);

	view->start_frame = CLAMP(start_frame, 0, (int64_t)waveform_get_n_frames(view->waveform) - 10);
	view->zoom = waveform_view_get_width(view) / length;
	dbg(1, "start=%Lu", view->start_frame);

	if(view->priv->actor){
		wf_actor_set_region(v->actor, &(WfSampleRegion){
			view->start_frame,
			length
		});
		if(!((AGlScene*)v->root)->draw) gtk_widget_queue_draw((GtkWidget*)view);
	}
}


	static void resolved(gpointer _view, gpointer fg)
	{
		wf_actor_set_colour(((WaveformView*)_view)->priv->actor, GPOINTER_TO_UINT(fg));
	}

void
waveform_view_set_colour(WaveformView* view, uint32_t fg, uint32_t bg)
{
	((AGlScene*)view->priv->root)->bg_colour = bg;

	am_promise_add_callback(view->priv->ready, resolved, GUINT_TO_POINTER(fg));
}


	static gboolean show;

	static gboolean _on_idle(gpointer _view)
	{
		WaveformView* view = _view;

													if(!view->priv->context) return G_SOURCE_CONTINUE;
		view->priv->context->show_rms = show;
		gtk_widget_queue_draw((GtkWidget*)view);

		return G_SOURCE_REMOVE;
	}

void
waveform_view_set_show_rms (WaveformView* view, gboolean _show)
{
	//FIXME this idle hack is because wa is not created until realise.

	show = _show;

	g_idle_add(_on_idle, view);
}


static void
waveform_view_realize (GtkWidget* base)
{
	PF2;
	WaveformView* view = (WaveformView*)base;
	GdkWindowAttr attrs = {0};
	GTK_WIDGET_SET_FLAGS (base, GTK_REALIZED);
	memset (&attrs, 0, sizeof (GdkWindowAttr));
	attrs.window_type = GDK_WINDOW_CHILD;
	attrs.width = base->allocation.width;
	attrs.wclass = GDK_INPUT_OUTPUT;
	attrs.event_mask = gtk_widget_get_events(base) | GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK;
	_g_object_unref0(base->window);
	base->window = gdk_window_new (gtk_widget_get_parent_window(base), &attrs, 0);
	gdk_window_set_user_data (base->window, view);
	gtk_widget_set_style (base, gtk_style_attach(gtk_widget_get_style(base), base->window));
	gtk_style_set_background (gtk_widget_get_style (base), base->window, GTK_STATE_NORMAL);
	gdk_window_move_resize (base->window, base->allocation.x, base->allocation.y, base->allocation.width, base->allocation.height);

	// note that we do not initialize other display aspects yet. Must wait until scene is ready.
}


static void
waveform_view_unrealize (GtkWidget* widget)
{
	PF;
	WaveformView* view = (WaveformView*)widget;
	WaveformViewPrivate* v = view->priv;
	gdk_window_set_user_data (widget->window, NULL);

	am_promise_unref(promise(PROMISE_DISP_READY));

	// create a new promise that will be resolved if and when the canvas is available again.
	GList* nth = g_list_nth(v->ready->children, PROMISE_DISP_READY);
	nth->data = am_promise_new(view);
	am_promise_add_callback(nth->data, _show_waveform, NULL);
}


static gboolean
waveform_view_on_expose (GtkWidget* widget, GdkEventExpose* event)
{
	WaveformView* view = (WaveformView*)widget;
	WaveformViewPrivate* v = (WaveformViewPrivate*)view->priv;
	g_return_val_if_fail (event, false);

	if(!GTK_WIDGET_REALIZED(widget)) return true;
	if(!promise(PROMISE_DISP_READY)->is_resolved) return true;

	AGL_ACTOR_START_DRAW(v->root) {
		// needed for the case of shared contexts, where one of the other contexts modifies the projection.
		waveform_view_set_projection(widget);

		agl_bg_colour_rbga(((AGlScene*)v->root)->bg_colour);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0 // white border
		glPushMatrix(); /* modelview matrix */
			glNormal3f(0, 0, 1); glDisable(GL_TEXTURE_2D);
			glLineWidth(1);
			glColor3f(1.0, 1.0, 1.0);

			int wid = GL_WIDTH;
			int h   = GL_HEIGHT;
			glBegin(GL_LINES);
			glVertex3f(0.0, 0.0, 1); glVertex3f(wid, 0.0, 1);
			glVertex3f(wid, h,   1); glVertex3f(0.0,   h, 1);
			glEnd();
		glPopMatrix();
#endif

		agl_actor__paint(v->root);

#if USE_SYSTEM_GTKGLEXT
		gdk_gl_drawable_swap_buffers(((AGlRootActor*)v->root)->gl.gdk.drawable);
#else
		gdk_gl_window_swap_buffers(((AGlRootActor*)v->root)->gl.gdk.drawable);
#endif
	} AGL_ACTOR_END_DRAW(v->root)

	return true;
}


static gboolean
waveform_view_button_press_event (GtkWidget* base, GdkEventButton* event)
{
	g_return_val_if_fail (event != NULL, false);
	gboolean result = false;
	return result;
}


static gboolean
waveform_view_button_release_event (GtkWidget* widget, GdkEventButton* event)
{
	g_return_val_if_fail(event, false);
	gboolean result = false;
	return result;
}


static gboolean
waveform_view_motion_notify_event (GtkWidget* widget, GdkEventMotion* event)
{
	g_return_val_if_fail(event, false);
	gboolean result = false;
	return result;
}


static void
waveform_view_allocate (GtkWidget* widget, GdkRectangle* allocation)
{
	PF;
	g_return_if_fail (allocation);

	WaveformView* view = (WaveformView*)widget;
	WaveformViewPrivate* v = view->priv;

	widget->allocation = (GtkAllocation)(*allocation);

	v->root->region = (AGlfRegion){
		.x1 = 0,
		.y1 = 0,
		.x2 = allocation->width,
		.y2 = allocation->height,
	};

	if ((GTK_WIDGET_FLAGS (widget) & GTK_REALIZED) == 0) return;
	gdk_window_move_resize(widget->window, widget->allocation.x, widget->allocation.y, widget->allocation.width, widget->allocation.height);

	if(!promise(PROMISE_DISP_READY)->is_resolved) return;

	if(v->actor){
		int width = waveform_view_get_width(view);
		wf_actor_set_rect(v->actor, &(WfRectangle){0, 0, width, GL_HEIGHT});
	}

	waveform_view_set_projection(widget);
}


static void
waveform_view_class_init (WaveformViewClass* klass)
{
	waveform_view_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (WaveformViewPrivate));
	GTK_WIDGET_CLASS (klass)->expose_event = waveform_view_on_expose;
	GTK_WIDGET_CLASS (klass)->button_press_event = waveform_view_button_press_event;
	GTK_WIDGET_CLASS (klass)->button_release_event = waveform_view_button_release_event;
	GTK_WIDGET_CLASS (klass)->motion_notify_event = waveform_view_motion_notify_event;
	GTK_WIDGET_CLASS (klass)->realize = waveform_view_realize;
	GTK_WIDGET_CLASS (klass)->unrealize = waveform_view_unrealize;
	GTK_WIDGET_CLASS (klass)->size_allocate = waveform_view_allocate;
	G_OBJECT_CLASS (klass)->finalize = waveform_view_finalize;
}


static void
waveform_view_instance_init (WaveformView* self)
{
	self->zoom = 1.0;
	self->start_frame = 0;
	self->priv = WAVEFORM_VIEW_GET_PRIVATE (self);
	self->priv->context = NULL;
	self->priv->actor = NULL;
}


static void
waveform_view_finalize (GObject* obj)
{
	// note that actor freeing is now done in unrealise. finalise is too late because the gl_drawable changes during an unrealise/realise cycle.

	WaveformView* view = WAVEFORM_VIEW(obj);
	WaveformViewPrivate* v = view->priv;
	AGlScene* scene = (AGlScene*)v->root;

	// these should really be done in dispose
	if(v->actor){
		wf_actor_clear(v->actor);
		v->actor = (agl_actor__remove_child(v->root, (AGlActor*)v->actor), NULL);
	}

	if(v->context) wf_context_free0(v->context);
	scene->draw = NULL;

	G_OBJECT_CLASS (waveform_view_parent_class)->finalize(obj);
}


GType
waveform_view_get_type ()
{
	static volatile gsize waveform_view_type_id__volatile = 0;
	if (g_once_init_enter (&waveform_view_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (WaveformViewClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) waveform_view_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (WaveformView), 0, (GInstanceInitFunc) waveform_view_instance_init, NULL };
		GType waveform_view_type_id;
		waveform_view_type_id = g_type_register_static (GTK_TYPE_DRAWING_AREA, "WaveformView", &g_define_type_info, 0);
		g_once_init_leave (&waveform_view_type_id__volatile, waveform_view_type_id);
	}
	return waveform_view_type_id__volatile;
}


static void
waveform_view_set_projection(GtkWidget* widget)
{
	int vx = 0;
	int vy = 0;
	int vw = widget->allocation.width;
	int vh = widget->allocation.height;
	glViewport(vx, vy, vw, vh);
	dbg (1, "viewport: %i %i %i %i", vx, vy, vw, vh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// establish clipping volume (left, right, bottom, top, near, far) :
	// This gives us an addressable area of GL_WIDTH x GL_HEIGHT (256)
	int width = waveform_view_get_width((WaveformView*)widget);
	double hborder = 0 * width / 32;

	double left = -hborder;
	double right = width + hborder;
	double top   = GL_HEIGHT;
	double bottom = 0.0;
	glOrtho (left, right, bottom, top, 10.0, -100.0);
}


/*
 *  Returns the context that the WaveformView is using.
 */
WaveformContext*
waveform_view_get_canvas(WaveformView* view)
{
	g_return_val_if_fail(view, NULL);
	return view->priv->context;
}


static int
waveform_view_get_width(WaveformView* view)
{
	GtkWidget* widget = (GtkWidget*)view;

	return GTK_WIDGET_REALIZED(widget) ? widget->allocation.width : 256;
}


