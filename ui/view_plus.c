/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | WaveformViewPlus is a Gtk widget based on GtkDrawingArea.            |
 | It displays an audio waveform represented by a Waveform object.      |
 | It also displays decorative title text and information text.         |
 |                                                                      |
 |  When the widget is focussed, the following keyboard shortcuts are   |
 |  active:                                                             |
 |    left         scroll left                                          |
 |    right        scroll right                                         |
 |    home         scroll to start                                      |
 |    -            zoom in                                              |
 |    +            zoom out                                             |
 |                                                                      |
 |  For a demonstration of usage, see the file test/view_plus.c         |
 |                                                                      |
 |  The openGL context can be obtained and used by calling              |
 |  `agl_get_gl_context()`.                                             |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#define __wf_private__
#define __waveform_view_private__
#define __wf_canvas_priv__

#include "config.h"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#include <gdk/gdkkeysyms.h>
#include "agl/debug.h"
#include "agl/utils.h"
#include "agl/behaviours/scrollable_h.h"
#include "agl/behaviours/fullsize.h"
#include "agl/behaviours/keytrap.h"
#include "waveform/ui-utils.h"
#include "waveform/actor.h"
#include "view_plus.h"

#define DIRECT 1
#define DEFAULT_HEIGHT 64
#define DEFAULT_WIDTH 256

static AGl* agl = NULL;

#define _g_source_remove0(S) {if(S) g_source_remove(S); S = 0;}

#define ROOT(view) ((AGlActor*)((GlArea*)view)->scene)

//-----------------------------------------

typedef void (KeyHandler)(WaveformViewPlus*);

typedef struct
{
	int         key;
	KeyHandler* handler;
} Key;

typedef struct
{
	guint          timer;
	KeyHandler*    handler;
} KeyHold;

static KeyHandler
	zoom_in,
	zoom_out,
	zoom_up,
	zoom_down,
	home;

static Key keys[] = {
	{61,        zoom_in},
	{45,        zoom_out},
	{'0',       zoom_up},
	{'9',       zoom_down},
	{KEY_Home,  home},
	{0},
};

//-----------------------------------------

struct _WaveformViewPlusPrivate {
	WaveformContext* context;
	WaveformActor*   actor;
	AMPromise*       ready;
};

enum {
    PROMISE_DISP_READY,
    PROMISE_WAVE_READY,
    PROMISE_MAX
};
#define promise(A) ((AMPromise*)g_list_nth_data(view->priv->ready->children, A))

static int      waveform_view_plus_get_width            (WaveformViewPlus*);

G_DEFINE_TYPE_WITH_PRIVATE (WaveformViewPlus, waveform_view_plus, TYPE_GL_AREA)

static gboolean waveform_view_plus_button_press_event   (GtkWidget*, GdkEventButton*);
static gboolean waveform_view_plus_button_release_event (GtkWidget*, GdkEventButton*);
static gboolean waveform_view_plus_motion_notify_event  (GtkWidget*, GdkEventMotion*);
static gboolean waveform_view_plus_focus_in_event       (GtkWidget*, GdkEventFocus*);
static gboolean waveform_view_plus_focus_out_event      (GtkWidget*, GdkEventFocus*);
static gboolean waveform_view_plus_enter_notify_event   (GtkWidget*, GdkEventCrossing*);
static gboolean waveform_view_plus_leave_notify_event   (GtkWidget*, GdkEventCrossing*);
static void     waveform_view_plus_realize              (GtkWidget*);
static void     waveform_view_plus_unrealize            (GtkWidget*);
static void     waveform_view_plus_allocate             (GtkWidget*, GdkRectangle*);
static void     waveform_view_plus_finalize             (GObject*);
static void    _waveform_view_plus_set_waveform         (WaveformViewPlus*, Waveform*);
static void    _waveform_view_plus_unset_waveform       (WaveformViewPlus*);

static void     waveform_view_plus_allocate_wave        (WaveformViewPlus*);

static void     add_key_handlers                        (GtkWindow*, WaveformViewPlus*, Key keys[]);
static void     remove_key_handlers                     (GtkWindow*, WaveformViewPlus*);

static AGlActor* waveform_actor                         (WaveformViewPlus*);
static void      waveform_actor_size                    (AGlActor*);


static WaveformViewPlus*
construct ()
{
	WaveformViewPlus* self = (WaveformViewPlus*) gl_area_construct (TYPE_WAVEFORM_VIEW_PLUS);

	gtk_widget_add_events ((GtkWidget*) self, (gint) (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_SCROLL_MASK));

	return self;
}


	static gboolean waveform_view_plus_load_new_on_idle (gpointer _view)
	{
		WaveformViewPlus* view = _view;
		g_return_val_if_fail(view, G_SOURCE_REMOVE);
		if (GTK_WIDGET_REALIZED(view)) {
			if (!promise(PROMISE_DISP_READY)->is_resolved) {
				am_promise_resolve(promise(PROMISE_DISP_READY), NULL);
				gtk_widget_queue_draw((GtkWidget*)view); //testing.
			}
		}
		return G_SOURCE_REMOVE;
	}

	gboolean try_canvas (gpointer _a)
	{
		AGlActor* a = _a;
		WaveformViewPlus* view = (WaveformViewPlus*)((AGlRootActor*)a)->gl.gdk.widget;

		if(agl->use_shaders && !agl->shaders.plain->program) return G_SOURCE_CONTINUE;

		am_promise_resolve(promise(PROMISE_DISP_READY), NULL);

		return G_SOURCE_REMOVE;
	}

WaveformViewPlus*
waveform_view_plus_new (Waveform* waveform)
{
	PF;

	WaveformViewPlus* view = construct ();
	GtkWidget* widget = (GtkWidget*)view;
	GlArea* area = (GlArea*)view;
	WaveformViewPlusPrivate* v = view->priv;

	if (waveform) _waveform_view_plus_set_waveform(view, waveform);

#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus(widget, TRUE);
#endif
	gtk_widget_set_size_request(widget, DEFAULT_WIDTH, DEFAULT_HEIGHT);

	// delay initialisation to allow for additional options to be set.
	g_idle_add(waveform_view_plus_load_new_on_idle, view);

	v->ready = am_promise_new(view);
	am_promise_when(v->ready, am_promise_new(view), am_promise_new(view), NULL);

	v->context = wf_context_new((AGlActor*)area->scene);

	v->actor = (WaveformActor*)waveform_actor(view);
	((AGlActor*)v->actor)->z = 2;
	area->scene->selected = (AGlActor*)v->actor;

	agl_actor__add_behaviour((AGlActor*)v->actor, ({
		AGlBehaviour* f = fullsize();
		((FullsizeBehaviour*)f)->border.y = 6;
		f;
	}));

	void view_plus_on_scroll (AGlObservable* o, AGlVal value, gpointer _view)
	{
		WaveformViewPlus* view = _view;

		wf_actor_scroll_to (view->priv->actor, value.i);
	}
	AGlBehaviour* scrollable = agl_actor__add_behaviour((AGlActor*)v->actor, scrollable_h());
	agl_observable_subscribe((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll, view_plus_on_scroll, view);

	typedef struct {
		WaveformViewPlus* view;
		float             prev;
	} C;

	void view_plus_on_zoom (AGlObservable* o, AGlVal zoom, gpointer _view)
	{
		C* c = _view;
		WaveformViewPlus* view = c->view;
		WaveformViewPlusPrivate* v = view->priv;
		AGlActor* actor = (AGlActor*)view->priv->actor;

		if (view->waveform) {
			int n_frames_in_viewport = wf_context_x_to_frame(v->context, agl_actor__width(actor));
			int max_scrolloffset = wf_context_frame_to_x(v->context, view->waveform->n_frames - n_frames_in_viewport);

			AGlBehaviour* scrollable = agl_actor__find_behaviour(actor, scrollable_h_get_class());
			if (((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll)->value.i > max_scrolloffset) {
				agl_observable_set_int ((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll, max_scrolloffset);
			}
		}
		c->prev = v->context->zoom->value.f;
	}

	agl_obserable_add_closure(v->context->zoom, G_OBJECT(view), view_plus_on_zoom, WF_NEW(C,
		view,
		v->context->zoom->value.f,
	));

	agl_actor__add_behaviour((AGlActor*)((GlArea*)view)->scene, keytrap_behaviour());

	return view;
}


				static void _waveform_view_plus__show_waveform_done (Waveform* w, gpointer _view)
				{
					WaveformViewPlus* view = _view;
					WaveformViewPlusPrivate* v = view->priv;

					if (w == view->waveform) { // it may have changed during load
						if (!g_list_find (((AGlActor*)((GlArea*)view)->scene)->children, v->actor)) {
							((AGlActor*)v->actor)->set_size((AGlActor*)v->actor);
						}

						am_promise_resolve(promise(PROMISE_WAVE_READY), NULL);
					}
				}

static void
_waveform_view_plus__show_waveform (gpointer _view, gpointer _c)
{
	// this must NOT be called until the canvas is ready

	WaveformViewPlus* view = _view;
	g_return_if_fail(view);
	WaveformViewPlusPrivate* v = view->priv;
	g_return_if_fail(v->context);
	AGlActor* actor = (AGlActor*)v->actor;

	ROOT(view)->scrollable = (AGliRegion){0, 0, ((GtkWidget*)view)->allocation.width, ((GtkWidget*)view)->allocation.height};

	if (!(actor->parent)) {
		if (view->waveform) { // it is valid for the widget to not have a waveform set.
			agl_actor__add_child((AGlActor*)((GlArea*)view)->scene, actor);

			if (promise(PROMISE_DISP_READY)->is_resolved) {
				agl_actor__set_size(actor);
			}

			uint64_t n_frames = waveform_get_n_frames(view->waveform);
			if (n_frames) {
				if (!v->actor->region.len) {
					wf_actor_set_region(v->actor, &(WfSampleRegion){0, n_frames});
				}

				g_signal_connect (view->waveform, "peakdata-ready", (GCallback)_waveform_view_plus__show_waveform_done, view);
			}
			waveform_view_plus_allocate_wave(view);
		}
	}
	gtk_widget_queue_draw((GtkWidget*)view);
}


static void
waveform_view_plus_load_file_done (WaveformActor* a, gpointer _c)
{
	WfClosure* c = (WfClosure*)_c;
	if (agl_actor__width(((AGlActor*)a))) {
		a->context->samples_per_pixel = waveform_get_n_frames(a->waveform) / agl_actor__width(((AGlActor*)a));

		if (((AGlActor*)a)->parent) agl_actor__invalidate(((AGlActor*)a)->parent); // we dont seem to track the layers, so have to invalidate everything.
	}
	call(c->callback, a->waveform, a->waveform->priv->peaks->error, c->user_data);
	g_free(c);
}


void
waveform_view_plus_load_file (WaveformViewPlus* view, const char* filename, WfCallback3 callback, gpointer user_data)
{
	dbg(1, "%s", filename);

	WaveformViewPlusPrivate* v = view->priv;

	_waveform_view_plus_unset_waveform(view);

	if (!filename) {
		gtk_widget_queue_draw((GtkWidget*)view);
		call(callback, view->waveform, NULL, user_data);
		return;
	}

	_waveform_view_plus_set_waveform(view, waveform_new(filename));

	if (v->actor) {
		wf_actor_set_waveform(v->actor, view->waveform, waveform_view_plus_load_file_done, AGL_NEW(WfClosure,
			.callback = callback,
			.user_data = user_data
		));
	}

	AGlBehaviour* scrollable = agl_actor__find_behaviour((AGlActor*)v->actor, scrollable_h_get_class());
	agl_observable_set_int ((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll, 0);

	am_promise_add_callback(promise(PROMISE_DISP_READY), _waveform_view_plus__show_waveform, NULL);
}


void
waveform_view_plus_set_waveform (WaveformViewPlus* view, Waveform* waveform)
{
	PF;
	WaveformViewPlusPrivate* v = view->priv;

	if (view->waveform) {
		_waveform_view_plus_unset_waveform(view);
		v->actor->region.len = 0; // force it to be set once the wav is loaded
	}

	_waveform_view_plus_set_waveform(view, waveform);

	if (v->actor) {
		wf_actor_set_waveform_sync(v->actor, waveform);
	}

#ifndef USE_CANVAS_SCALING
	view->zoom = 1.0;
#endif
	AGlBehaviour* scrollable = agl_actor__find_behaviour((AGlActor*)v->actor, scrollable_h_get_class());
	agl_observable_set_int ((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll, 0);

	am_promise_add_callback(promise(PROMISE_DISP_READY), _waveform_view_plus__show_waveform, NULL);
}


float
waveform_view_plus_get_zoom (WaveformViewPlus* view)
{
	return view->priv->context->zoom->value.f;
}


void
waveform_view_plus_set_zoom (WaveformViewPlus* view, float zoom)
{
	g_return_if_fail(view);
	WaveformViewPlusPrivate* v = view->priv;
	AGlActor* actor = (AGlActor*)v->actor;
	dbg(1, "zoom=%.2f", zoom);

#ifdef USE_CANVAS_SCALING
	if((zoom = CLAMP(zoom, 1.0, WF_CONTEXT_MAX_ZOOM)) == v->context->zoom->value.f) return;

	float ratio = zoom / v->context->zoom->value.f;
	wf_context_set_zoom(v->context, zoom);

	int max_scrolloffset = wf_context_frame_to_x(v->context, view->waveform->n_frames - wf_context_x_to_frame(v->context, agl_actor__width(actor)));

	AGlBehaviour* scrollable = agl_actor__find_behaviour(actor, scrollable_h_get_class());
	agl_observable_set_int ((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll, MIN(((AGlObservable*)((HScrollableBehaviour*)scrollable)->scroll)->value.i * ratio, max_scrolloffset));

	actor->scrollable.x2 = actor->scrollable.x1 + agl_actor__width(actor) * zoom;

	agl_actor__set_size(actor);


#else
	view->zoom = CLAMP(zoom, 1.0, WF_CONTEXT_MAX_ZOOM);

	wf_actor_set_region(v->actor, &(WfSampleRegion){
		view->start_frame,
		(waveform_get_n_frames(view->waveform) - view->start_frame) / view->zoom
	});

#endif
}


void
waveform_view_plus_set_start (WaveformViewPlus* view, int64_t start_frame)
{
	WaveformViewPlusPrivate* v = view->priv;

	// the number of visible frames reduces as the zoom increases.
#ifdef USE_CANVAS_SCALING
	int64_t n_frames_visible = agl_actor__width(((AGlActor*)v->actor)) * v->context->samples_per_pixel / v->context->zoom->value.f;
#else
	int64_t n_frames_visible = waveform_get_n_frames(view->waveform) / view->zoom;
#endif

	view->start_frame = CLAMP(
		start_frame,
		0,
		(int64_t)(waveform_get_n_frames(view->waveform) - MAX(10, n_frames_visible))
	);
	dbg(1, "start=%"PRIi64, view->start_frame);

	wf_context_set_start(v->context, view->start_frame);

	wf_actor_set_region(v->actor, &(WfSampleRegion){
		view->start_frame,
		n_frames_visible
	});
}


void
waveform_view_plus_set_region (WaveformViewPlus* view, int64_t start_frame, int64_t end_frame)
{
	g_return_if_fail(view->waveform);
	g_return_if_fail(end_frame > start_frame);
	WaveformViewPlusPrivate* v = view->priv;
	g_return_if_fail(v->actor);

	WfSampleRegion region = (WfSampleRegion){start_frame, end_frame - start_frame};

	region.len = MIN(region.len, waveform_get_n_frames(view->waveform) - region.start);

	view->start_frame = CLAMP(region.start, 0, (int64_t)waveform_get_n_frames(view->waveform) - 10);
#ifndef USE_CANVAS_SCALING
	view->zoom = view->waveform->n_frames / (float)region.len;
#endif
	dbg(1, "start=%"PRIu64, view->start_frame);

	wf_actor_set_region(v->actor, &region);

	if(!((AGlActor*)v->actor)->root->draw) gtk_widget_queue_draw((GtkWidget*)view);
}


void
waveform_view_plus_set_colour (WaveformViewPlus* view, uint32_t fg, uint32_t bg)
{
	WaveformViewPlusPrivate* v = view->priv;

	view->bg_colour = bg;
	if(view->priv->actor) wf_actor_set_colour(v->actor, fg);

	if(agl_get_instance()->use_shaders){
	}
}

	static bool show;

	static gboolean _on_idle(gpointer _view)
	{
		WaveformViewPlus* view = _view;
		if(!view->priv->context) return G_SOURCE_CONTINUE;

		view->priv->context->show_rms = show;
		gtk_widget_queue_draw((GtkWidget*)view);

		return G_SOURCE_REMOVE;
	}

void
waveform_view_plus_set_show_rms (WaveformViewPlus* view, bool _show)
{
	//FIXME this idle hack is because wa is not created until realise.
	//      (still true?)

	show = _show;

	g_idle_add(_on_idle, view);
}


AGlActor*
waveform_view_plus_add_layer (WaveformViewPlus* view, AGlActor* actor, int z)
{
	PF;

	actor->z = z;
	agl_actor__add_child(ROOT(view), actor);

	return actor;
}


AGlActor*
waveform_view_plus_get_layer (WaveformViewPlus* view, int z)
{
	return agl_actor__find_by_z(ROOT(view), z);
}


void
waveform_view_plus_remove_layer (WaveformViewPlus* view, AGlActor* actor)
{
	g_return_if_fail(actor);

	agl_actor__remove_child(ROOT(view), actor);
}


static void
waveform_view_plus_realize (GtkWidget* widget)
{
	PF2;
	WaveformViewPlus* view = (WaveformViewPlus*)widget;
	AGlActor* actor = (AGlActor*)view->priv->actor;

	GTK_WIDGET_CLASS (waveform_view_plus_parent_class)->realize(widget);

	if(!actor->colour){
		// currently the waveform background is always dark, so a light colour is needed for the foreground
		uint32_t base_colour = wf_get_gtk_base_color(widget, GTK_STATE_NORMAL, 0xaa);
		wf_actor_set_colour(view->priv->actor,
			wf_colour_is_dark_rgba(base_colour)
				? wf_get_gtk_fg_color(widget, GTK_STATE_NORMAL)
				: base_colour
		);
	}

	am_promise_resolve(promise(PROMISE_DISP_READY), NULL);
}


static void
waveform_view_plus_unrealize (GtkWidget* widget)
{
	// view->waveform can not be unreffed here as it needs to be re-used if the widget is realized again.
	// The gl context and actors are now preserved through an unrealize/realize cycle - the only thing that changes is the GlDrawable.

	PF;
	WaveformViewPlus* view = (WaveformViewPlus*)widget;
	WaveformViewPlusPrivate* v = view->priv;

	GTK_WIDGET_CLASS (waveform_view_plus_parent_class)->unrealize(widget);

	am_promise_unref(promise(PROMISE_DISP_READY));

	// create a new promise that will be resolved if and when the canvas is available again.
	GList* nth = g_list_nth(v->ready->children, PROMISE_DISP_READY);
	nth->data = am_promise_new(view);
	am_promise_add_callback(nth->data, _waveform_view_plus__show_waveform, NULL);
}


static gboolean
waveform_view_plus_button_press_event (GtkWidget* widget, GdkEventButton* event)
{
	g_return_val_if_fail (event, false);

	gtk_widget_grab_focus(widget);

	return AGL_NOT_HANDLED;
}


static gboolean
waveform_view_plus_button_release_event (GtkWidget* widget, GdkEventButton* event)
{
	g_return_val_if_fail(event, false);
	bool handled = false;
	return handled;
}


static gboolean
waveform_view_plus_motion_notify_event (GtkWidget* widget, GdkEventMotion* event)
{
	g_return_val_if_fail(event, false);
	gboolean result = false;
	return result;
}


static gboolean
waveform_view_plus_focus_in_event (GtkWidget* widget, GdkEventFocus* event)
{
	WaveformViewPlus* view = (WaveformViewPlus*)widget;

	add_key_handlers((GtkWindow*)gtk_widget_get_toplevel(widget), view, keys);

	return false;
}


static gboolean
waveform_view_plus_focus_out_event (GtkWidget* widget, GdkEventFocus* event)
{
	WaveformViewPlus* view = (WaveformViewPlus*)widget;

	remove_key_handlers((GtkWindow*)gtk_widget_get_toplevel(widget), view);

	return false;
}


static gboolean
waveform_view_plus_enter_notify_event (GtkWidget* widget, GdkEventCrossing* event)
{
	return false;
}


static gboolean
waveform_view_plus_leave_notify_event (GtkWidget* widget, GdkEventCrossing* event)
{
	return false;
}


static void
waveform_view_plus_allocate (GtkWidget* widget, GdkRectangle* allocation)
{
	PF;
	WaveformViewPlus* view = WAVEFORM_VIEW_PLUS(widget);
	g_return_if_fail (allocation);

	GTK_WIDGET_CLASS (waveform_view_plus_parent_class)->size_allocate(widget, allocation);

	if (GTK_WIDGET_REALIZED(widget)) {
		if (ROOT(view)->scrollable.x2 < 2 || ROOT(view)->scrollable.y2 < 2)
			ROOT(view)->scrollable = (AGliRegion){0, 0, allocation->width, allocation->height};

		am_promise_resolve(promise(PROMISE_DISP_READY), NULL);

		if (view->waveform)
			waveform_view_plus_allocate_wave(view);
	}
}


static void
waveform_view_plus_class_init (WaveformViewPlusClass* klass)
{
	waveform_view_plus_parent_class = g_type_class_peek_parent (klass);

	GTK_WIDGET_CLASS (klass)->button_press_event = waveform_view_plus_button_press_event;
	GTK_WIDGET_CLASS (klass)->button_release_event = waveform_view_plus_button_release_event;
	GTK_WIDGET_CLASS (klass)->motion_notify_event = waveform_view_plus_motion_notify_event;
	GTK_WIDGET_CLASS (klass)->focus_in_event = waveform_view_plus_focus_in_event;
	GTK_WIDGET_CLASS (klass)->focus_out_event = waveform_view_plus_focus_out_event;
	GTK_WIDGET_CLASS (klass)->enter_notify_event = waveform_view_plus_enter_notify_event;
	GTK_WIDGET_CLASS (klass)->leave_notify_event = waveform_view_plus_leave_notify_event;
	GTK_WIDGET_CLASS (klass)->realize = waveform_view_plus_realize;
	GTK_WIDGET_CLASS (klass)->unrealize = waveform_view_plus_unrealize;
	GTK_WIDGET_CLASS (klass)->size_allocate = waveform_view_plus_allocate;
	G_OBJECT_CLASS (klass)->finalize = waveform_view_plus_finalize;

	agl = agl_get_instance();
}


static void
waveform_view_plus_init (WaveformViewPlus* self)
{
#ifndef USE_CANVAS_SCALING
	self->zoom = 1.0;
#endif
	self->start_frame = 0;
	self->priv = waveform_view_plus_get_instance_private(self);
	self->priv->context = NULL;
	self->priv->actor = NULL;
}


static void
waveform_view_plus_finalize (GObject* obj)
{
	WaveformViewPlus* view = WAVEFORM_VIEW_PLUS(obj);
	WaveformViewPlusPrivate* v = view->priv;

	// these should really be done in dispose
	if (v->actor) {
#ifdef AGL_ACTOR_RENDER_CACHE
		/*
		 *  Temporary pending completion of moving caching to a separate behaviour
		 */
		g_clear_pointer(&((AGlActor*)v->actor)->fbo, agl_fbo_free);
#endif

		wf_actor_clear(v->actor);

		AGlActor* parent = (AGlActor*)((AGlActor*)v->actor)->root;
		if (parent) {
			agl_actor__remove_child(parent, (AGlActor*)v->actor);
		}
		v->actor = NULL;
	}

	g_clear_pointer(&v->context, wf_context_free);

	_waveform_view_plus_unset_waveform(view);
	g_clear_pointer(&v->ready, am_promise_unref);

	G_OBJECT_CLASS (waveform_view_plus_parent_class)->finalize(obj);
}


/*
 *  Returns the context that the WaveformViewPlus is using.
 *  The context provides properties such samples-per-pixel.
 */
WaveformContext*
waveform_view_plus_get_context (WaveformViewPlus* view)
{
	g_return_val_if_fail(view, NULL);
	return view->priv->context;
}


WaveformActor*
waveform_view_plus_get_actor (WaveformViewPlus* view)
{
	g_return_val_if_fail(view, NULL);
	return view->priv->actor;
}


static int
waveform_view_plus_get_width (WaveformViewPlus* view)
{
	GtkWidget* widget = (GtkWidget*)view;

	return GTK_WIDGET_REALIZED(widget) ? widget->allocation.width : 256;
}


	static KeyHold key_hold = {0, NULL};
	static bool key_down = false;
	static GHashTable* key_handlers = NULL;

	static gboolean key_hold_on_timeout (gpointer user_data)
	{
		WaveformViewPlus* waveform = user_data;
		if(key_hold.handler) key_hold.handler(waveform);
		return G_SOURCE_CONTINUE;
	}

	static gboolean key_press (GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		WaveformViewPlus* waveform = user_data;

		if (key_down) {
			// key repeat
			return true;
		}

		KeyHandler* handler = g_hash_table_lookup(key_handlers, &event->keyval);
		if (handler) {
			key_down = true;
			if (key_hold.timer) pwarn("timer already started");
			key_hold.timer = g_timeout_add(100, key_hold_on_timeout, waveform);
			key_hold.handler = handler;
	
			handler(waveform);
		}
		else dbg(1, "0x%x", event->keyval);

		return key_down;
	}

	static gboolean key_release (GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		PF;
		if (!key_down) return AGL_NOT_HANDLED; // sometimes happens at startup

		key_down = false;
		_g_source_remove0(key_hold.timer);

		return true;
	}

static void
add_key_handlers (GtkWindow* window, WaveformViewPlus* view, Key keys[])
{
	//list of keys must be terminated with a key of value zero.

	if (!key_handlers) {
		key_handlers = g_hash_table_new(g_int_hash, g_int_equal);

		int i = 0; while (true) {
			Key* key = &keys[i];
			if (i > 100 || !key->key) break;
			g_hash_table_insert(key_handlers, &key->key, key->handler);
			i++;
		}
	}

	g_signal_connect(view, "key-press-event", G_CALLBACK(key_press), view);
	g_signal_connect(view, "key-release-event", G_CALLBACK(key_release), view);
}


static void
remove_key_handlers (GtkWindow* window, WaveformViewPlus* view)
{
	g_signal_handlers_disconnect_matched (view, G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA, g_signal_lookup ("key-press-event", GTK_TYPE_WIDGET), 0, NULL, NULL, view);
	g_signal_handlers_disconnect_matched (view, G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA, g_signal_lookup ("key-release-event", GTK_TYPE_WIDGET), 0, NULL, NULL, view);
}


static void
home (WaveformViewPlus* view)
{
	waveform_view_plus_set_start(view, 0);
}


static void
zoom_in (WaveformViewPlus* view)
{
	waveform_view_plus_set_zoom(view, view->priv->context->zoom->value.f * 1.5);
}


static void
zoom_out (WaveformViewPlus* view)
{
	waveform_view_plus_set_zoom(view, view->priv->context->zoom->value.f / 1.5);
}


static void
zoom_up (WaveformViewPlus* view)
{
	WaveformActor* actor = view->priv->actor;
	wf_actor_set_vzoom(actor, actor->context->v_gain * 1.3);
}


static void
zoom_down (WaveformViewPlus* view)
{
	WaveformActor* actor = view->priv->actor;
	wf_actor_set_vzoom(actor, actor->context->v_gain / 1.3);
}


static void
waveform_view_plus_allocate_wave (WaveformViewPlus* view)
{
	WaveformViewPlusPrivate* v = view->priv;

	AGlActor* root = ROOT(view);

	int w = waveform_view_plus_get_width(view);

	if (v->context->scaled) {
		float len = wf_context_frame_to_x(v->context, v->actor->region.len);
		if (len < w) {
			wf_context_set_scale(v->context, v->context->priv->zoom.target_val.f * v->actor->region.len / w);
		}
	} else {
		wf_context_set_scale(v->context, v->actor->region.len / w);
	}
	agl_actor__set_size(root);

	waveform_actor_size((AGlActor*)v->actor);
}


	static void waveform_actor_size (AGlActor* actor)
	{
		WaveformActor* wf_actor = (WaveformActor*)actor;

		int width = agl_actor__scrollable_width(actor);
		if (!width) {
			actor->scrollable.x2 = actor->region.x2;
			actor->scrollable.y2 = actor->region.y2;
			width = agl_actor__scrollable_width(actor);
		}

		if (wf_actor->waveform) {
			uint64_t n_frames = waveform_get_n_frames(wf_actor->waveform);
			if (n_frames) {
				if (width && wf_actor->context->priv->zoom.target_val.f < 1.000001)
					wf_actor->context->samples_per_pixel = n_frames / width;
			}
		}
	}

	static AGlActorFn set_size = NULL;
	static void waveform_actor_size0 (AGlActor* actor)
	{
		waveform_actor_size(actor);

		float width = agl_actor__width(actor->parent);
		if (width > 0.0) {
#ifdef AGL_ACTOR_RENDER_CACHE
			actor->fbo = agl_fbo_new(MAX(1, width), MAX(1, agl_actor__height(actor)), 0, 0);
			actor->cache.enabled = true;
#endif

			//actor->set_size = waveform_actor_size;
			actor->set_size = set_size;
		}
	}

static AGlActor*
waveform_actor (WaveformViewPlus* view)
{
	AGlActor* actor = (AGlActor*)wf_context_add_new_actor(view->priv->context, view->waveform);

	set_size = actor->set_size;
	actor->set_size = waveform_actor_size0;

	return actor;
}


#ifdef DEBUG
static void waveform_view_on_wav_finalize (gpointer view, GObject* was)
{
	PF;
	((WaveformViewPlus*)view)->waveform = NULL;
}
#endif


static void
_waveform_view_plus_set_waveform (WaveformViewPlus* view, Waveform* waveform)
{
	view->waveform = g_object_ref(waveform);

#ifdef DEBUG
	g_object_weak_ref((GObject*)view->waveform, waveform_view_on_wav_finalize, view);
#endif
}


static void
_waveform_view_plus_unset_waveform (WaveformViewPlus* view)
{
	if (view->waveform) {
#ifdef DEBUG
		g_object_weak_unref((GObject*)view->waveform, waveform_view_on_wav_finalize, view);
#endif
		g_clear_object (&view->waveform);
	}
}
