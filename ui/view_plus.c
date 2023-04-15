/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2023 Tim Orford <tim@orford.org>                  |
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
 +----------------------------------------------------------------------+
 |
 */

#define __wf_private__
#define __waveform_view_private__
#define __wf_canvas_priv__

#include "config.h"
#include <gtk/gtk.h>
#include "agl/debug.h"
#include "agl/utils.h"
#include "agl/event.h"
#include "agl/fbo.h"
#include "agl/behaviours/throttled_key.h"
#include "waveform/ui-utils.h"
#include "waveform/actor.h"
#include "view_plus.h"

#define DIRECT 1
#define DEFAULT_HEIGHT 64
#define DEFAULT_WIDTH 128

static AGl*          agl = NULL;

static GdkGLContext* gl_context = NULL;

#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_source_remove0(S) {if(S) g_source_remove(S); S = 0;}

static ActorKeyHandler
	zoom_in,
	zoom_out,
	scroll_left,
	scroll_right,
	zoom_up,
	zoom_down,
	home;

static ActorKey keys[] = {
	{61,        zoom_in},
	{45,        zoom_out},
	{XK_Left,   scroll_left},
	{XK_Right,  scroll_right},
	{'0',       zoom_up},
	{'9',       zoom_down},
	{XK_Home,   home},
	{0},
};

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

G_DEFINE_TYPE_WITH_PRIVATE (WaveformViewPlus, waveform_view_plus, AGL_TYPE_GTK_AREA)
enum  {
	WAVEFORM_VIEW_PLUS_DUMMY_PROPERTY
};
static void     waveform_view_plus_button_press_event   (GtkGestureClick*, guint, double, double, GtkWidget*);
static void     waveform_view_plus_focus_in_event       (GtkWidget*);
static void     waveform_view_plus_focus_out_event      (GtkWidget*);
static void     waveform_view_plus_realize              (GtkWidget*);
static void     waveform_view_plus_unrealize            (GtkWidget*);
static void     waveform_view_plus_allocate             (GtkWidget*, int, int, int);
static void     waveform_view_plus_finalize             (GObject*);
static bool     waveform_view_plus_display_maybe_ready  (WaveformViewPlus*);
static void    _waveform_view_plus_set_waveform         (WaveformViewPlus*, Waveform*);
static void    _waveform_view_plus_unset_waveform       (WaveformViewPlus*);

static void     waveform_view_plus_gl_on_allocate       (WaveformViewPlus*);

static AGlActor* waveform_actor                         (WaveformViewPlus*);
static void      waveform_actor_size                    (AGlActor*);

#define SCENE(VIEW) ((AGlActor*)((AGlGtkArea*)VIEW)->scene)

/*
 *  For use where the widget needs to share an opengl context with other items.
 *  Should be called before any widgets are instantiated.
 *
 *  Alternatively, just use the context returned by agl_get_gl_context() for other widgets.
 */
void
waveform_view_plus_set_gl (GdkGLContext* _gl_context)
{
	gl_context = _gl_context;
}


static WaveformViewPlus*
construct ()
{
	WaveformViewPlus* self = (WaveformViewPlus*) g_object_new (TYPE_WAVEFORM_VIEW_PLUS, NULL);

	return self;
}


	static gboolean waveform_view_plus_load_new_on_idle (gpointer _view)
	{
		WaveformViewPlus* view = _view;
		g_return_val_if_fail(view, G_SOURCE_REMOVE);
		waveform_view_plus_display_maybe_ready(view);

		return G_SOURCE_REMOVE;
	}

	void _waveform_view_plus_on_draw (AGlScene* scene, gpointer view)
	{
		agl_gtk_area_queue_render((AGlGtkArea*)view);
	}

WaveformViewPlus*
waveform_view_plus_new (Waveform* waveform)
{
	PF;

	WaveformViewPlus* view = construct ();
	GtkWidget* widget = (GtkWidget*)view;
	WaveformViewPlusPrivate* v = view->priv;

	if (waveform) _waveform_view_plus_set_waveform(view, waveform);

	gtk_widget_set_size_request(widget, DEFAULT_WIDTH, DEFAULT_HEIGHT);

	// delay initialisation to allow for additional options to be set.
	g_idle_add(waveform_view_plus_load_new_on_idle, view);

	AGlActor* root = SCENE(view);
	v->context = wf_context_new(root);

	v->actor = (WaveformActor*)waveform_actor(view);
	((AGlActor*)v->actor)->z = 2;

	AGlScene* scene = (AGlScene*)root;
	scene->draw = _waveform_view_plus_on_draw;
	scene->user_data = view;

	return view;
}


				static void _waveform_view_plus__show_waveform_done (Waveform* w, gpointer _view)
				{
					WaveformViewPlus* view = _view;
					WaveformViewPlusPrivate* v = view->priv;

					if (w == view->waveform) { // it may have changed during load
						if (!g_list_find (SCENE(view)->children, v->actor)) {
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

	g_assert(promise(PROMISE_DISP_READY)->is_resolved);

	if (!(actor->parent)) {
		if (view->waveform) { // it is valid for the widget to not have a waveform set.
			agl_actor__add_child(SCENE(view), actor);

			agl_actor__set_size(actor);

			uint64_t n_frames = waveform_get_n_frames(view->waveform);
			if (n_frames) {
				if (!v->actor->region.len) {
					wf_actor_set_region(v->actor, &(WfSampleRegion){0, n_frames});
				}

				g_signal_connect (view->waveform, "peakdata-ready", (GCallback)_waveform_view_plus__show_waveform_done, view);
			}
			waveform_view_plus_gl_on_allocate(view);
		}
	}

	agl_gtk_area_queue_render((AGlGtkArea*)view);
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
		agl_gtk_area_queue_render((AGlGtkArea*)view);
		return;
	}

	_waveform_view_plus_set_waveform(view, waveform_new(filename));

	if (v->actor) {
		wf_actor_set_waveform(v->actor, view->waveform, waveform_view_plus_load_file_done, AGL_NEW(WfClosure,
			.callback = callback,
			.user_data = user_data
		));
	}

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
	dbg(1, "zoom=%.2f", zoom);

#ifdef USE_CANVAS_SCALING
	if((zoom = CLAMP(zoom, 1.0, WF_CONTEXT_MAX_ZOOM)) == v->context->zoom->value.f) return;

	wf_context_set_zoom(v->context, zoom);

	int64_t n_frames = waveform_get_n_frames(view->waveform);

	// if wav is shorter than previous, it may need to be scrolled into view.
	// this is done un-animated.
	if (v->actor->region.start >= n_frames) {
		int64_t delta = v->actor->region.start;
		v->actor->region.start = 0;
		v->actor->region.len -= delta;
	}

	// It is not strictly neccesary to set the region, as it will anyway be clipped when rendering
	int64_t region_len = v->context->samples_per_pixel * agl_actor__width(((AGlActor*)v->actor)) / v->context->priv->zoom.target_val.f;
	int64_t max_start = n_frames - region_len;
	wf_actor_set_region(v->actor, &(WfSampleRegion){
		MIN(view->start_frame, max_start),
		//(waveform_get_n_frames(view->waveform) - view->start_frame) // oversized
		region_len
	});

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

	agl_gtk_area_queue_render((AGlGtkArea*)view);
}


void
waveform_view_plus_set_colour (WaveformViewPlus* view, uint32_t fg, uint32_t bg)
{
	WaveformViewPlusPrivate* v = view->priv;

	((AGlGtkArea*)view)->bg_colour = bg;
	if (view->priv->actor) wf_actor_set_colour(v->actor, fg);
}

	static bool show;

	static gboolean _on_idle(gpointer _view)
	{
		WaveformViewPlus* view = _view;
		if (!view->priv->context) return G_SOURCE_CONTINUE;

		view->priv->context->show_rms = show;
		agl_gtk_area_queue_render((AGlGtkArea*)view);

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
	agl_actor__add_child(SCENE(view), actor);

	return actor;
}


AGlActor*
waveform_view_plus_get_layer (WaveformViewPlus* view, int z)
{
	return agl_actor__find_by_z(SCENE(view), z);
}


void
waveform_view_plus_remove_layer (WaveformViewPlus* view, AGlActor* actor)
{
	g_return_if_fail(actor);

	agl_actor__remove_child(SCENE(view), actor);
}


static void
waveform_view_plus_realize (GtkWidget* widget)
{
	PF2;
	WaveformViewPlus* view = (WaveformViewPlus*)widget;

	GTK_WIDGET_CLASS (waveform_view_plus_parent_class)->realize (widget);
	AGlActor* actor = (AGlActor*)view->priv->actor;

	if (!actor->colour) {
		// currently the waveform background is always dark, so a light colour is needed for the foreground
		uint32_t base_colour = wf_get_gtk_base_color(widget, 0xaa);
		wf_actor_set_colour(view->priv->actor,
			wf_colour_is_dark_rgba(base_colour)
				? wf_get_gtk_fg_color(widget)
				: base_colour
		);
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		GtkStyleContext* context = gtk_widget_get_style_context (widget);
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
		GdkRGBA color;
		gtk_style_context_lookup_color (context, "background-color", &color);
	}

	waveform_view_plus_display_maybe_ready (view);
}


static void
waveform_view_plus_unrealize (GtkWidget* widget)
{
	// view->waveform can not be unreffed here as it needs to be re-used if the widget is realized again.
	// The gl context and actors are now preserved through an unrealize/realize cycle - the only thing that changes is the GlDrawable.

	PF;
	WaveformViewPlus* view = (WaveformViewPlus*)widget;
	WaveformViewPlusPrivate* v = view->priv;

	am_promise_unref(promise(PROMISE_DISP_READY));

	// create a new promise that will be resolved if and when the canvas is available again.
	GList* nth = g_list_nth(v->ready->children, PROMISE_DISP_READY);
	nth->data = am_promise_new(view);
	am_promise_add_callback(nth->data, _waveform_view_plus__show_waveform, NULL);

	GTK_WIDGET_CLASS (waveform_view_plus_parent_class)->unrealize (widget);
}


static void
waveform_view_plus_button_press_event (GtkGestureClick* gesture, guint n_press, double x, double y, GtkWidget* widget)
{
	gtk_widget_grab_focus(widget);
}


static void
waveform_view_plus_focus_in_event (GtkWidget* widget)
{
}


static void
waveform_view_plus_focus_out_event (GtkWidget* widget)
{
}


static bool
waveform_view_plus_display_maybe_ready (WaveformViewPlus* view)
{
	void
	waveform_view_plus_display_ready (WaveformViewPlus* view)
	{
		dbg(1, "READY");

		am_promise_resolve (promise(PROMISE_DISP_READY), NULL);
		agl_gtk_area_queue_render((AGlGtkArea*)view);
	}


	GtkWidget* widget = (GtkWidget*)view;

	if (promise(PROMISE_DISP_READY)->is_resolved) return true;

	if (gtk_widget_get_realized(widget) && ((AGlRootActor*)SCENE(view))->gl.gdk.context) {
		waveform_view_plus_display_ready(view);
		return true;
	}

	return false;
}


static void
waveform_view_plus_allocate (GtkWidget* widget, int width, int height, int baseline)
{
	PF;

	WaveformViewPlus* view = (WaveformViewPlus*)widget;

	GTK_WIDGET_CLASS (waveform_view_plus_parent_class)->size_allocate(widget, width, height, baseline);

	waveform_view_plus_gl_on_allocate(view);
}


static void
waveform_view_plus_class_init (WaveformViewPlusClass* klass)
{
	waveform_view_plus_parent_class = g_type_class_peek_parent (klass);

	GTK_WIDGET_CLASS (klass)->realize = waveform_view_plus_realize;
	GTK_WIDGET_CLASS (klass)->unrealize = waveform_view_plus_unrealize;
	GTK_WIDGET_CLASS (klass)->size_allocate = waveform_view_plus_allocate;
	G_OBJECT_CLASS (klass)->finalize = waveform_view_plus_finalize;

	agl = agl_get_instance();
}


static void
waveform_view_plus_init (WaveformViewPlus* self)
{
	GtkWidget* widget = (GtkWidget*)self;
	WaveformViewPlus* view = self;

#ifndef USE_CANVAS_SCALING
	self->zoom = 1.0;
#endif
	self->start_frame = 0;

	WaveformViewPlusPrivate* v = self->priv = waveform_view_plus_get_instance_private(self);
	v->context = NULL;
	v->actor = NULL;

	gtk_widget_set_focusable (widget, true);

	self->click = gtk_gesture_click_new ();
	gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (self->click), false);
	gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (self->click), GDK_BUTTON_PRIMARY);
	g_signal_connect (self->click, "pressed", G_CALLBACK (waveform_view_plus_button_press_event), self);
	gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (self->click), GTK_PHASE_CAPTURE);
	gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (self->click));

	GtkEventController* controller = gtk_event_controller_focus_new ();
	g_signal_connect_swapped (controller, "enter", G_CALLBACK (waveform_view_plus_focus_in_event), widget);
	g_signal_connect_swapped (controller, "leave", G_CALLBACK (waveform_view_plus_focus_out_event), widget);
	gtk_widget_add_controller (widget, controller);

	((AGlActor*)((AGlGtkArea*)self)->scene)->behaviours[0] = throttled_key_behaviour();
	#define KEYS(A) ((KeyBehaviour*)((AGlActor*)(A))->behaviours[0])
	KEYS(((AGlGtkArea*)self)->scene)->keys = &keys;

	v->ready = am_promise_new(view);
	am_promise_when(v->ready, am_promise_new(view), am_promise_new(view), NULL);
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

	if (v->context) wf_context_free0(v->context);

	_waveform_view_plus_unset_waveform(view);
	g_clear_pointer(&v->ready, am_promise_unref);

	G_OBJECT_CLASS (waveform_view_plus_parent_class)->finalize(obj);
}


/*
 *  Returns the underlying canvas that the WaveformViewPlus is using.
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


static bool
scroll_left (AGlActor* actor, AGlModifierType state)
{
	WaveformViewPlus* view = (WaveformViewPlus*)actor->root->gl.gdk.widget;
#ifdef USE_CANVAS_SCALING
	WaveformViewPlusPrivate* v = view->priv;
#endif

	if (!view->waveform) return AGL_NOT_HANDLED;

#ifdef USE_CANVAS_SCALING
	int64_t n_frames_visible = agl_actor__width(((AGlActor*)v->actor)) * v->context->samples_per_pixel / v->context->zoom->value.f;
#else
	int n_frames_visible = ((float)view->waveform->n_frames) / view->zoom;
#endif
	waveform_view_plus_set_start(view, view->start_frame - n_frames_visible / 10);

	return AGL_HANDLED;
}


static bool
scroll_right (AGlActor* actor, AGlModifierType state)
{
	WaveformViewPlus* view = (WaveformViewPlus*)actor->root->gl.gdk.widget;
#ifdef USE_CANVAS_SCALING
	WaveformViewPlusPrivate* v = view->priv;
#endif

	if (!view->waveform) return AGL_NOT_HANDLED;

#ifdef USE_CANVAS_SCALING
	int64_t n_frames_visible = agl_actor__width(((AGlActor*)v->actor)) * v->context->samples_per_pixel / v->context->zoom->value.f;
#else
	int n_frames_visible = ((float)view->waveform->n_frames) / view->zoom;
#endif
	waveform_view_plus_set_start(view, view->start_frame + n_frames_visible / 10);

	return AGL_HANDLED;
}


static bool
home (AGlActor* actor, AGlModifierType state)
{
	WaveformViewPlus* view = (WaveformViewPlus*)actor->root->gl.gdk.widget;

	waveform_view_plus_set_start(view, 0);

	return AGL_HANDLED;
}


static bool
zoom_in (AGlActor* actor, AGlModifierType state)
{
	WaveformViewPlus* view = (WaveformViewPlus*)actor->root->gl.gdk.widget;

	waveform_view_plus_set_zoom(view, view->priv->context->zoom->value.f * 1.5);

	return AGL_HANDLED;
}


static bool
zoom_out (AGlActor* actor, AGlModifierType state)
{
	WaveformViewPlus* view = (WaveformViewPlus*)actor->root->gl.gdk.widget;

	waveform_view_plus_set_zoom(view, view->priv->context->zoom->value.f / 1.5);

	return AGL_HANDLED;
}


static bool
zoom_up (AGlActor* scene, AGlModifierType state)
{
	WaveformViewPlus* view = (WaveformViewPlus*)scene->root->gl.gdk.widget;
	WaveformActor* actor = view->priv->actor;

	wf_actor_set_vzoom(actor, actor->context->v_gain * 1.3);

	return AGL_HANDLED;
}


static bool
zoom_down (AGlActor* scene, AGlModifierType state)
{
	WaveformViewPlus* view = (WaveformViewPlus*)scene->root->gl.gdk.widget;
	WaveformActor* actor = view->priv->actor;

	wf_actor_set_vzoom(actor, actor->context->v_gain / 1.3);

	return AGL_HANDLED;
}


static void
waveform_view_plus_gl_on_allocate (WaveformViewPlus* view)
{
	WaveformViewPlusPrivate* v = view->priv;

	if (!v->actor) return;

	if (v->context->scaled) {
		wf_context_set_scale(v->context, v->context->priv->zoom.target_val.f * v->actor->region.len / agl_actor__width(SCENE(view)));
	}

	waveform_actor_size((AGlActor*)v->actor);
}


	static void waveform_actor_size (AGlActor* actor)
	{
		#define V_BORDER 4.
		WaveformActor* wf_actor = (WaveformActor*)actor;

		if (!actor->parent) return;

		actor->region = (AGlfRegion){
			0,
			V_BORDER,
			agl_actor__width(actor->parent),
			agl_actor__height(actor->parent) - V_BORDER
		};

		uint64_t n_frames = waveform_get_n_frames(wf_actor->waveform);
		if (n_frames) {
			wf_actor->context->samples_per_pixel = n_frames / agl_actor__width(actor);
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
