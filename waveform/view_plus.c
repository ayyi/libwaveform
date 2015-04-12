/*
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

  ---------------------------------------------------------------

  WaveformViewPlus is a Gtk widget based on GtkDrawingArea.
  It displays an audio waveform represented by a Waveform object.
  It also displays decorative title text and information text.

  When the widget is focussed, the following keyboard shortcuts are active:
    left         scroll left
    right        scroll right
    -            zoom in
    +            zoom out

*/
#define __wf_private__
#define __waveform_view_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include <ass/ass.h>
#include "agl/ext.h"
#include "agl/utils.h"
#include "agl/actor.h"
#include "waveform/peak.h"
#include "waveform/utils.h"
#include "waveform/peakgen.h"
#include "waveform/promise.h"
#include "waveform/gl_utils.h"
#include "waveform/utils.h"
#include "waveform/texture_cache.h"
#include "waveform/actor.h"
#include "waveform/grid.h"
#include "waveform/shader.h"
#include "waveform/actors/spp.h"
#include "view_plus.h"

#define DIRECT 1
#define DEFAULT_HEIGHT 64
#define DEFAULT_WIDTH 256

static AGl*          agl = NULL;

static int           instance_count = 0;

static GdkGLConfig*  glconfig = NULL;
static GdkGLContext* gl_context = NULL;
static gboolean      gl_initialised = false;

#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

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
	scroll_left,
	scroll_right;

static Key keys[] = {
	{KEY_Left,  scroll_left},
	{KEY_Right, scroll_right},
	{61,        zoom_in},
	{45,        zoom_out},
	{0},
};

//-----------------------------------------

typedef struct {
	int         width;
	int         height;
	int         y_offset;
} Title;

struct _WaveformViewPlusPrivate {
	WaveformCanvas* canvas;
	WaveformActor*  actor;
	AGlActor*       root;
	gboolean        show_grid;
	Title           title;

	AGlActor*       grid_actor;
	AGlActor*       text_actor;
	AGlActor*       spp_actor;

	AMPromise*      ready;
	gboolean        gl_init_done;
	gboolean        title_is_rendered;

	GdkGLContext*   context;
};

typedef struct {
	AGlActor    actor;
	int         baseline;
	struct {
	    GLuint  ids[1];
		int     width;
		int     height;
	}           texture;
} TextActor;

enum {
    PROMISE_DISP_READY,
    PROMISE_WAVE_READY,
    PROMISE_MAX
};
#define promise(A) ((AMPromise*)g_list_nth_data(view->priv->ready->children, A))

static int      waveform_view_plus_get_width            (WaveformViewPlus*);
static int      waveform_view_plus_get_height           (WaveformViewPlus*);

static gpointer waveform_view_plus_parent_class = NULL;

#define WAVEFORM_VIEW_PLUS_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_WAVEFORM_VIEW_PLUS, WaveformViewPlusPrivate))
enum  {
	WAVEFORM_VIEW_PLUS_DUMMY_PROPERTY
};
static gboolean waveform_view_plus_on_expose            (GtkWidget*, GdkEventExpose*);
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
static void     waveform_view_plus_set_projection       (GtkWidget*);
static void     waveform_view_plus_init_drawable        (WaveformViewPlus*);
static void     waveform_view_plus_render_text          (WaveformViewPlus*);

static void     waveform_view_plus_gl_init              (WaveformViewPlus*);
static void     waveform_view_plus_gl_on_allocate       (WaveformViewPlus*);
static void     waveform_view_plus_draw                 (WaveformViewPlus*);

static uint32_t color_gdk_to_rgba                       (GdkColor*);
static uint32_t wf_get_gtk_base_color                   (GtkWidget*, GtkStateType, char alpha);
static void     add_key_handlers                        (GtkWindow*, WaveformViewPlus*, Key keys[]);
static void     remove_key_handlers                     (GtkWindow*, WaveformViewPlus*);

static void     waveform_view_plus_add_actors           (WaveformViewPlus*);
static AGlActor* waveform_actor                         (WaveformViewPlus*);
static AGlActor* grid_actor                             (WaveformViewPlus*);
static AGlActor* text_actor                             (WaveformViewPlus*);
static AGlActor* bg_actor                               (WaveformViewPlus*);

extern AGlActor* spp_actor                              (WaveformViewPlus*);

#define FONT \
	"Droid Sans"
	//"Ubuntu"
	//"Open Sans Rg"
	//"Fontin Sans Rg"

#define FONT_SIZE 18 //TODO

char* script = 
	"[Script Info]\n"
	"ScriptType: v4.00+\n"
	"PlayResX: %i\n"
	"PlayResY: %i\n"
	"ScaledBorderAndShadow: yes\n"
	"[V4+ Styles]\n"
	"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n"
	/*
		PrimaryColour:   filling color
		SecondaryColour: for animations
		OutlineColour:   border color
		BackColour:      shadow color

		hex format appears to be AALLXXXXXX where AA=alpha (00=opaque, FF=transparent) and LL=luminance
	*/
	//"Style: Default,Fontin Sans Rg,%i,&H000000FF,&HFF0000FF,&H00FF0000,&H00000000,-1,0,0,0,100,100,0,0,1,2.5,0,1,2,2,2,1\n"
	"Style: Default," FONT ",%i,&H3FFF00FF,&HFF0000FF,&H000000FF,&H00000000,-1,0,0,0,100,100,0,0,1,2.5,0,1,2,2,2,1\n"
	"[Events]\n"
	"Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
	"Dialogue: 0,0:00:00.00,0:00:15.00,Default,,0000,0000,0000,,%s \n";

typedef struct
{
    int width, height, stride;
    unsigned char* buf;      // 8 bit alphamap
} image_t;

ASS_Library*  ass_library = NULL;
ASS_Renderer* ass_renderer = NULL;


static gboolean
__init ()
{
	dbg(2, "...");

	if(!gl_context){
		gtk_gl_init(NULL, NULL);
		if(wf_debug){
			gint major, minor;
			gdk_gl_query_version (&major, &minor);
			g_print ("GtkGLExt version %d.%d\n", major, minor);
		}
	}

	glconfig = gdk_gl_config_new_by_mode( GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE );
	if (!glconfig) { gerr ("Cannot initialise gtkglext."); return false; }

	return true;
}


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
	gtk_widget_add_events ((GtkWidget*) self, (gint) ((GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK) | GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK));
	if(!gtk_widget_set_gl_capability((GtkWidget*)self, glconfig, gl_context ? gl_context : agl_get_gl_context(), DIRECT, GDK_GL_RGBA_TYPE)){
		gwarn("failed to set gl capability");
	}

	WaveformViewPlusPrivate* priv = self->priv;
	priv->gl_init_done = false;

	return self;
}


WaveformViewPlus*
waveform_view_plus_new (Waveform* waveform)
{
	PF;
	g_return_val_if_fail(glconfig || __init(), NULL);

	WaveformViewPlus* view = construct ();
	GtkWidget* widget = (GtkWidget*)view;

	view->waveform = waveform ? g_object_ref(waveform) : NULL;

#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus(widget, TRUE);
#endif
	gtk_widget_set_size_request(widget, DEFAULT_WIDTH, DEFAULT_HEIGHT);

	bool waveform_view_plus_load_new_on_idle(gpointer _view)
	{
		WaveformViewPlus* view = _view;
		g_return_val_if_fail(view, G_SOURCE_REMOVE);
		if(!promise(PROMISE_DISP_READY)->is_resolved){
			waveform_view_plus_init_drawable(view);
			gtk_widget_queue_draw((GtkWidget*)view); //testing.
		}
		return G_SOURCE_REMOVE;
	}
	// delay initialisation to allow for additional options to be set.
	g_idle_add(waveform_view_plus_load_new_on_idle, view);

	view->priv->ready = am_promise_new(view);
	am_promise_when(view->priv->ready, am_promise_new(view), am_promise_new(view), NULL);

	return view;
}


static void
_waveform_view_plus_set_actor (WaveformViewPlus* view)
{
	WaveformActor* actor = view->priv->actor;

	waveform_view_plus_gl_on_allocate(view);

	void _waveform_view_plus_on_draw(WaveformCanvas* wfc, gpointer _view)
	{
		gtk_widget_queue_draw((GtkWidget*)_view);
	}
	actor->canvas->draw = _waveform_view_plus_on_draw;
	actor->canvas->draw_data = view;
}


static void
_show_waveform(gpointer _view, gpointer _c)
{
	// this must NOT be called until the canvas is ready

	WaveformViewPlus* view = _view;
	g_return_if_fail(view);
	WaveformViewPlusPrivate* v = view->priv;
	g_return_if_fail(v->canvas);

	if(v->actor){
		if(((AGlActor*)v->actor)->set_size) ((AGlActor*)v->actor)->set_size((AGlActor*)v->actor);

	}else{
		if(!promise(PROMISE_DISP_READY)->is_resolved) waveform_view_plus_init_drawable(view);

		if(view->waveform){ // it is valid for the widget to not have a waveform set.
			v->actor = (WaveformActor*)waveform_actor(view);
			if(v->spp_actor) ((SppActor*)v->spp_actor)->wf_actor = v->actor;

			int need_add = false;
			if(promise(PROMISE_DISP_READY)->is_resolved && !v->root->children) waveform_view_plus_add_actors(view);
			else need_add = true;

			uint64_t n_frames = waveform_get_n_frames(view->waveform);
			if(n_frames){
				wf_actor_set_region(v->actor, &(WfSampleRegion){0, n_frames});
				wf_actor_set_colour(v->actor, view->fg_colour);

				waveform_load(view->waveform);

				if(need_add){
					agl_actor__add_child(v->root, (AGlActor*)v->actor);
					((AGlActor*)v->actor)->set_size((AGlActor*)v->actor);
				}

				am_promise_resolve(promise(PROMISE_WAVE_READY), NULL);
			}
			_waveform_view_plus_set_actor(view);

		}
	}
	gtk_widget_queue_draw((GtkWidget*)view);
}


void
waveform_view_plus_load_file (WaveformViewPlus* view, const char* filename)
{
	WaveformViewPlusPrivate* v = view->priv;

	if(view->waveform){
		_g_object_unref0(view->waveform);
	}

	if(!filename){
		gtk_widget_queue_draw((GtkWidget*)view);
		return;
	}

	view->waveform = waveform_new(filename);
	if(v->actor) wf_actor_set_waveform(v->actor, view->waveform);

	am_promise_add_callback(promise(PROMISE_DISP_READY), _show_waveform, NULL);
}


void
waveform_view_plus_set_waveform (WaveformViewPlus* view, Waveform* waveform)
{
	PF;
	WaveformViewPlusPrivate* v = view->priv;

	if(v->actor && v->canvas){
		wf_canvas_remove_actor(v->canvas, v->actor);
		v->actor = NULL;
	}
	if(view->waveform){
		g_object_unref(view->waveform);
	}

	view->waveform = g_object_ref(waveform);
	view->zoom = 1.0;

	am_promise_add_callback(promise(PROMISE_DISP_READY), _show_waveform, NULL);
}


void
waveform_view_plus_set_title(WaveformViewPlus* view, const char* title)
{
	view->title = g_strdup(title);
	view->priv->title_is_rendered = false;
	gtk_widget_queue_draw((GtkWidget*)view);
}


void
waveform_view_plus_set_text(WaveformViewPlus* view, const char* text)
{
	view->text = g_strdup(text);
	gtk_widget_queue_draw((GtkWidget*)view);
}


void
waveform_view_plus_set_zoom (WaveformViewPlus* view, float zoom)
{
	#define MAX_ZOOM 51200.0 //TODO
	g_return_if_fail(view);
	WaveformViewPlusPrivate* v = view->priv;
	dbg(1, "zoom=%.2f", zoom);

	view->zoom = CLAMP(zoom, 1.0, MAX_ZOOM);

	wf_actor_set_region(v->actor, &(WfSampleRegion){
		view->start_frame,
		(waveform_get_n_frames(view->waveform) - view->start_frame) / view->zoom
	});

	((AGlActor*)v->actor)->cache.valid = false;

	if(v->grid_actor)
		v->grid_actor->cache.enabled = false; // TODO temporary

	if(v->grid_actor)
		v->grid_actor->cache.valid = false;

	if(!view->priv->actor->canvas->draw) gtk_widget_queue_draw((GtkWidget*)view);
}


void
waveform_view_plus_set_start (WaveformViewPlus* view, int64_t start_frame)
{
	uint32_t length = waveform_get_n_frames(view->waveform) / view->zoom;
	view->start_frame = CLAMP(start_frame, 0, (int64_t)waveform_get_n_frames(view->waveform) - 10);
	view->start_frame = MIN(view->start_frame, waveform_get_n_frames(view->waveform) - length);
	dbg(1, "start=%Lu", view->start_frame);
	wf_actor_set_region(view->priv->actor, &(WfSampleRegion){
		view->start_frame,
		length
	});
	if(!view->priv->actor->canvas->draw) gtk_widget_queue_draw((GtkWidget*)view);
}


void
waveform_view_plus_set_region (WaveformViewPlus* view, int64_t start_frame, int64_t end_frame)
{
	uint32_t max_len = waveform_get_n_frames(view->waveform) - start_frame;
	uint32_t length = MIN(max_len, end_frame - start_frame);

	view->start_frame = CLAMP(start_frame, 0, (int64_t)waveform_get_n_frames(view->waveform) - 10);
	view->zoom = waveform_view_plus_get_width(view) / length;
	dbg(1, "start=%Lu", view->start_frame);

	if(view->priv->actor){
		wf_actor_set_region(view->priv->actor, &(WfSampleRegion){
			view->start_frame,
			length
		});
		if(!view->priv->actor->canvas->draw) gtk_widget_queue_draw((GtkWidget*)view);
	}
}


void
waveform_view_plus_set_colour(WaveformViewPlus* view, uint32_t fg, uint32_t bg, uint32_t text1, uint32_t text2)
{
	view->fg_colour = fg;
	view->bg_colour = bg;
	if(view->priv->actor) wf_actor_set_colour(view->priv->actor, fg);

	if(agl_get_instance()->use_shaders){
		wf_shaders.ass->uniform.colour1 = view->title_colour1 = text1;
		wf_shaders.ass->uniform.colour2 = view->title_colour2 = text2;
	}
}


/*
 *  Set the current playback position in milliseconds
 */
void
waveform_view_plus_set_time(WaveformViewPlus* view, uint32_t time)
{
	void resolved(gpointer _view, gpointer time)
	{
		spp_actor_set_time((SppActor*)((WaveformViewPlus*)_view)->priv->spp_actor, GPOINTER_TO_UINT(time));
	}
	am_promise_add_callback(view->priv->ready, resolved, GUINT_TO_POINTER(time));
}


void
waveform_view_plus_set_show_rms (WaveformViewPlus* view, gboolean _show)
{
	//FIXME this idle hack is because wa is not created until realise.

	static gboolean show; show = _show;

	gboolean _on_idle(gpointer _view)
	{
		WaveformViewPlus* view = _view;
		g_return_val_if_fail(view->priv->canvas, G_SOURCE_REMOVE);

		view->priv->canvas->show_rms = show;
		gtk_widget_queue_draw((GtkWidget*)view);

		return G_SOURCE_REMOVE;
	}
	g_idle_add(_on_idle, view);
}


void
waveform_view_plus_set_show_grid (WaveformViewPlus* view, gboolean show)
{
	WaveformViewPlusPrivate* v = view->priv;

	if(show != v->show_grid){
		v->show_grid = show;
		if(gtk_widget_get_realized((GtkWidget*)view)){
			if(show)
				agl_actor__add_child((AGlActor*)v->actor, v->grid_actor = v->grid_actor ? v->grid_actor : grid_actor(view));
			else
				if(v->grid_actor) agl_actor__remove_child(v->root, v->grid_actor);

			gtk_widget_queue_draw((GtkWidget*)view);
		}
	}
}


static void
waveform_view_plus_realize (GtkWidget* base)
{
	PF2;
	WaveformViewPlus* view = (WaveformViewPlus*)base;
	GdkWindowAttr attrs = {0};
	GTK_WIDGET_SET_FLAGS (base, GTK_REALIZED);
	memset (&attrs, 0, sizeof (GdkWindowAttr));
	attrs.window_type = GDK_WINDOW_CHILD;
	attrs.width = base->allocation.width;
	attrs.wclass = GDK_INPUT_OUTPUT;
	attrs.event_mask = gtk_widget_get_events(base) | GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_ENTER_NOTIFY_MASK;
	_g_object_unref0(base->window);
	base->window = gdk_window_new (gtk_widget_get_parent_window(base), &attrs, 0);
	gdk_window_set_user_data (base->window, view);
	gtk_widget_set_style (base, gtk_style_attach(gtk_widget_get_style(base), base->window));
	gtk_style_set_background (gtk_widget_get_style (base), base->window, GTK_STATE_NORMAL);
	gdk_window_move_resize (base->window, base->allocation.x, base->allocation.y, base->allocation.width, base->allocation.height);

	if(!promise(PROMISE_DISP_READY)->is_resolved) waveform_view_plus_init_drawable(view);
}


static void
waveform_view_plus_unrealize (GtkWidget* widget)
{
	// view->waveform can not be unreffed here as it needs to be re-used if the widget is realized again.
	// The gl context and actors are now preserved through an unrealize/realize cycle - the only thing that changes is the GlDrawable.
	PF;
	WaveformViewPlus* view = (WaveformViewPlus*)widget;
	WaveformViewPlusPrivate* v = view->priv;
	gdk_window_set_user_data (widget->window, NULL);

	am_promise_unref(promise(PROMISE_DISP_READY));

	// create a new promise that will be resolved if and when the canvas is available again.
	GList* nth = g_list_nth(v->ready->children, PROMISE_DISP_READY);
	nth->data = am_promise_new(view);
	am_promise_add_callback(nth->data, _show_waveform, NULL);
}


static gboolean
waveform_view_plus_on_expose (GtkWidget* widget, GdkEventExpose* event)
{
	WaveformViewPlus* view = (WaveformViewPlus*)widget;
	g_return_val_if_fail (event, FALSE);

	if(!GTK_WIDGET_REALIZED(widget)) return true;
	if(!gl_initialised || !promise(PROMISE_DISP_READY)->is_resolved) return true;

	AGL_ACTOR_START_DRAW(view->priv->root) {
		// needed for the case of shared contexts, where one of the other contexts modifies the projection.
		waveform_view_plus_set_projection(widget);

		AGlColourFloat bg; wf_colour_rgba_to_float(&bg, view->bg_colour);
		glClearColor(bg.r, bg.g, bg.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		waveform_view_plus_draw(view);

		gdk_gl_drawable_swap_buffers(((AGlRootActor*)view->priv->root)->gl.gdk.drawable);
	} AGL_ACTOR_END_DRAW(view->priv->root)

	return true;
}


static gboolean
waveform_view_plus_button_press_event (GtkWidget* widget, GdkEventButton* event)
{
	g_return_val_if_fail (event != NULL, false);
	gboolean handled = false;

	switch (event->type){
		case GDK_BUTTON_PRESS:
			dbg(0, "GDK_BUTTON_PRESS");
			gtk_widget_grab_focus(widget);
			handled = true;
			break;
		default:
			dbg(0, "unexpected event type");
			break;
	}
	return handled;
}


static gboolean
waveform_view_plus_button_release_event (GtkWidget* widget, GdkEventButton* event)
{
	g_return_val_if_fail(event, false);
	gboolean result = false;
	return result;
}


static gboolean
waveform_view_plus_motion_notify_event (GtkWidget* widget, GdkEventMotion* event)
{
	g_return_val_if_fail(event, false);
	gboolean result = false;
	return result;
}


static gboolean
waveform_view_plus_focus_in_event(GtkWidget* widget, GdkEventFocus* event)
{
	WaveformViewPlus* view = (WaveformViewPlus*)widget;

	add_key_handlers((GtkWindow*)gtk_widget_get_toplevel(widget), view, keys);

	return false;
}


static gboolean
waveform_view_plus_focus_out_event(GtkWidget* widget, GdkEventFocus* event)
{
	WaveformViewPlus* view = (WaveformViewPlus*)widget;

	remove_key_handlers((GtkWindow*)gtk_widget_get_toplevel(widget),  view);

	return false;
}


static gboolean
waveform_view_plus_enter_notify_event(GtkWidget* widget, GdkEventCrossing* event)
{
	return false;
}


static gboolean
waveform_view_plus_leave_notify_event(GtkWidget* widget, GdkEventCrossing* event)
{
	return false;
}


static void
waveform_view_plus_add_actors(WaveformViewPlus* view)
{
	WaveformViewPlusPrivate* v = view->priv;

	g_return_if_fail(!v->root->children);

	agl_actor__add_child(v->root, bg_actor(view));
	if(v->show_grid) agl_actor__add_child(v->root, v->grid_actor = grid_actor(view));
	agl_actor__add_child(v->root, (AGlActor*)v->actor);
	agl_actor__add_child(v->root, v->spp_actor = spp_actor(view));
	agl_actor__add_child(v->root, v->text_actor = text_actor(view));

	((SppActor*)v->spp_actor)->wf_actor = v->actor;
	((SppActor*)v->spp_actor)->text_colour = view->text_colour;

	agl_actor__set_size(v->root);
}


static void
waveform_view_plus_init_drawable (WaveformViewPlus* view)
{
	GtkWidget* widget = (GtkWidget*)view;
	WaveformViewPlusPrivate* v = view->priv;

	if(!GTK_WIDGET_REALIZED(widget)) return;

	bool try_drawable_again(gpointer _view)
	{
		WaveformViewPlus* view = _view;
		if(!promise(PROMISE_DISP_READY)->is_resolved){
			waveform_view_plus_init_drawable(view);
		}
		return G_SOURCE_REMOVE;
	}

	if(!v->context){
		if(!v->root){
			if(!(v->root = agl_actor__new_root(widget))) return;
		}

		v->canvas = wf_canvas_new((AGlRootActor*)v->root);
		v->context = ((AGlRootActor*)v->root)->gl.gdk.context;

		if(!view->fg_colour)     view->fg_colour     = wf_get_gtk_fg_color(widget, GTK_STATE_NORMAL);
		if(!view->title_colour1) view->title_colour1 = wf_get_gtk_text_color(widget, GTK_STATE_NORMAL);
		if(!view->text_colour){
			//TODO because the black background is not a theme colour we need to be careful to use a contrasting colour
			if(false)
				view->text_colour = wf_get_gtk_text_color(widget, GTK_STATE_NORMAL);
			else
				view->text_colour = wf_get_gtk_base_color(widget, GTK_STATE_NORMAL, 0xaa);
		}
		view->title_colour2 = 0x0000ffff; //FIXME

		waveform_view_plus_gl_init(view);
	}else{
		GdkGLDrawable* gl_drawable = gtk_widget_get_gl_drawable((GtkWidget*)view);
		if(!gl_drawable){
			g_idle_add(try_drawable_again, view); // dont always get another realize callback, eg after reparenting.
			return;
		}

		v->context = gtk_widget_get_gl_context((GtkWidget*)view);
		((AGlRootActor*)v->root)->gl.gdk.context = v->context;
		((AGlRootActor*)v->root)->gl.gdk.drawable = gl_drawable;

		if(!v->canvas){
			v->canvas = wf_canvas_new((AGlRootActor*)v->root);
		}
	}

	waveform_view_plus_set_projection(widget);

	am_promise_resolve(g_list_nth_data(view->priv->ready->children, PROMISE_DISP_READY), NULL);
}


static void
waveform_view_plus_allocate (GtkWidget* widget, GdkRectangle* allocation)
{
	PF;
	g_return_if_fail (allocation);

	WaveformViewPlus* view = (WaveformViewPlus*)widget;
	widget->allocation = (GtkAllocation)(*allocation);
	if ((GTK_WIDGET_FLAGS (widget) & GTK_REALIZED) == 0) return;
	gdk_window_move_resize(widget->window, widget->allocation.x, widget->allocation.y, widget->allocation.width, widget->allocation.height);

	if(!promise(PROMISE_DISP_READY)->is_resolved) waveform_view_plus_init_drawable(view);

	if(!gl_initialised) return;

	waveform_view_plus_gl_on_allocate(view);

	waveform_view_plus_set_projection(widget);
}


static void
waveform_view_plus_class_init (WaveformViewPlusClass * klass)
{
	waveform_view_plus_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (WaveformViewPlusPrivate));
	GTK_WIDGET_CLASS (klass)->expose_event = waveform_view_plus_on_expose;
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

	void ass_init()
	{
		void msg_callback(int level, const char* fmt, va_list va, void* data)
		{
			if (!wf_debug || level > 6) return;
			printf("libass: ");
			vprintf(fmt, va);
			printf("\n");
		}

		ass_library = ass_library_init();
		if (!ass_library) {
			printf("ass_library_init failed!\n");
			exit(EXIT_FAILURE);
		}

		ass_set_message_cb(ass_library, msg_callback, NULL);

		ass_renderer = ass_renderer_init(ass_library);
		if (!ass_renderer) {
			printf("ass_renderer_init failed!\n");
			exit(EXIT_FAILURE);
		}

		ass_set_fonts(ass_renderer, NULL, "Sans", 1, NULL, 1);
	}
	ass_init();
}


static void
waveform_view_plus_instance_init (WaveformViewPlus * self)
{
	self->zoom = 1.0;
	self->start_frame = 0;
	self->priv = WAVEFORM_VIEW_PLUS_GET_PRIVATE (self);
	self->priv->canvas = NULL;
	self->priv->actor = NULL;
	self->priv->gl_init_done = FALSE;

	instance_count++;
}


static void
waveform_view_plus_finalize (GObject* obj)
{
	WaveformViewPlus* view = WAVEFORM_VIEW_PLUS(obj);
	WaveformViewPlusPrivate* v = view->priv;

	if(view->title) _g_free0(view->title);
	if(view->text)  _g_free0(view->text);
	//_g_free0 (self->priv->_filename);

	// these should really be done in dispose
	if(v->actor){
		wf_actor_clear(v->actor);
		wf_canvas_remove_actor(v->canvas, v->actor);
		v->actor = NULL;

	}
	if(v->canvas) wf_canvas_free0(v->canvas);
	if(view->waveform) waveform_unref0(view->waveform);

	G_OBJECT_CLASS (waveform_view_plus_parent_class)->finalize(obj);

	if(!--instance_count){
		ass_renderer_done(ass_renderer);
		ass_library_done(ass_library);
		ass_renderer = NULL;
		ass_library = NULL;
	}
}


GType
waveform_view_plus_get_type ()
{
	static volatile gsize waveform_view_plus_type_id__volatile = 0;
	if (g_once_init_enter (&waveform_view_plus_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (WaveformViewPlusClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) waveform_view_plus_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (WaveformViewPlus), 0, (GInstanceInitFunc) waveform_view_plus_instance_init, NULL };
		GType waveform_view_plus_type_id;
		waveform_view_plus_type_id = g_type_register_static (GTK_TYPE_DRAWING_AREA, "WaveformViewPlus", &g_define_type_info, 0);
		g_once_init_leave (&waveform_view_plus_type_id__volatile, waveform_view_plus_type_id);
	}
	return waveform_view_plus_type_id__volatile;
}


static void
waveform_view_plus_set_projection(GtkWidget* widget)
{
	int vx = 0;
	int vy = 0;
	int vw = waveform_view_plus_get_width((WaveformViewPlus*)widget);
	int vh = waveform_view_plus_get_height((WaveformViewPlus*)widget);
	glViewport(vx, vy, vw, vh);
	dbg (1, "viewport: %i %i %i %i", vx, vy, vw, vh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// set clipping volume (left, right, bottom, top, near, far):
	int width = vw;
	double hborder = 0 * width / 32;

	double left = -hborder;
	double right = width + hborder;
	double top   = 0.0;
	double bottom = vh;
	glOrtho (left, right, bottom, top, 10.0, -100.0);
}


/*
 *  Returns the underlying canvas that the WaveformViewPlus is using.
 */
WaveformCanvas*
waveform_view_plus_get_canvas(WaveformViewPlus* view)
{
	g_return_val_if_fail(view, NULL);
	return view->priv->canvas;
}


static int
waveform_view_plus_get_width(WaveformViewPlus* view)
{
	GtkWidget* widget = (GtkWidget*)view;

	return GTK_WIDGET_REALIZED(widget) ? widget->allocation.width : 256;
}


static int
waveform_view_plus_get_height(WaveformViewPlus* view)
{
	GtkWidget* widget = (GtkWidget*)view;

	return GTK_WIDGET_REALIZED(widget) ? widget->allocation.height : DEFAULT_HEIGHT;
}


#define _r(c)  ( (c)>>24)
#define _g(c)  (((c)>>16)&0xFF)
#define _b(c)  (((c)>> 8)&0xFF)
#define _a(c)  (((c)    )&0xFF)

#define N_CHANNELS 2 // luminance + alpha

static void
blend_single(image_t* frame, ASS_Image* img)
{
	// composite img onto frame

	int x, y;
	unsigned char opacity = 255 - _a(img->color);
	unsigned char b = _b(img->color);
	dbg(2, "  %ix%i stride=%i x=%i", img->w, img->h, img->stride, img->dst_x);

	#define LUMINANCE_CHANNEL (x * N_CHANNELS)
	#define ALPHA_CHANNEL (x * N_CHANNELS + 1)
	unsigned char* src = img->bitmap;
	unsigned char* dst = frame->buf + img->dst_y * frame->stride + img->dst_x * N_CHANNELS;
	for (y = 0; y < img->h; ++y) {
		for (x = 0; x < img->w; ++x) {
			unsigned k = ((unsigned) src[x]) * opacity / 255;
			dst[LUMINANCE_CHANNEL] = (k * b + (255 - k) * dst[LUMINANCE_CHANNEL]) / 255;
			dst[ALPHA_CHANNEL] = (k * 255 + (255 - k) * dst[ALPHA_CHANNEL]) / 255;
		}
		src += img->stride;
		dst += frame->stride;
	}
}


static void
waveform_view_plus_render_text(WaveformViewPlus* view)
{
	WaveformViewPlusPrivate* v = view->priv;
	AGlActor* actor = v->text_actor;
	TextActor* ta = (TextActor*)actor;

	PF;
	if(v->title_is_rendered) gwarn("title is already rendered");

	GError* error = NULL;
	GRegexMatchFlags flags = 0;
	static GRegex* regex = NULL;
	if(!regex) regex = g_regex_new("[_]", 0, flags, &error);
	gchar* str = g_regex_replace(regex, view->title, -1, 0, " ", flags, &error);

	char* title = g_strdup_printf("%s", str);
	g_free(str);

	int fh = agl_power_of_two(FONT_SIZE + 4);
	int fw = ta->texture.width = agl_power_of_two(strlen(title) * 20);
	ass_set_frame_size(ass_renderer, fw, fh);

	void title_render(const char* text, image_t* out, Title* t)
	{
		char* script2 = g_strdup_printf(script, fw, fh, FONT_SIZE, text);

		ASS_Track* track = ass_read_memory(ass_library, script2, strlen(script2), NULL);
		g_free(script2);
		if (!track) {
			printf("track init failed!\n");
			return;
		}

		ASS_Image* img = ass_render_frame(ass_renderer, track, 100, NULL);

		*t = (Title){
			.y_offset = fh,
		};

		ASS_Image* i = img;
		for(;i;i=i->next){
			t->height   = MAX(t->height, i->h);
			t->width    = MAX(t->width,  i->dst_x + i->w);
			t->y_offset = MIN(t->y_offset, fh - i->dst_y - i->h); // dst_y is distance from bottom.
		}

		*out = (image_t){fw, fh, fw * N_CHANNELS, g_new0(guchar, fw * fh * N_CHANNELS)};
		for(i=img;i;i=i->next){
			blend_single(out, i); // blend each glyph onto the output buffer.
		}

		ass_free_track(track);
		if(false){
			char* buf = g_new0(char, out->width * out->height * 4);
			int stride = out->width * 4;
			int y; for(y=0;y<fh;y++){
				int x; for(x=0;x<out->width;x++){
					*(buf + y * stride + x * 4    ) = *(out->buf + y * out->stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 1) = 0;
					*(buf + y * stride + x * 4 + 2) = 0;
					*(buf + y * stride + x * 4 + 3) = *(out->buf + y * out->stride + x * N_CHANNELS + 1);
				}
			}
			GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data((const guchar*)buf, GDK_COLORSPACE_RGB, /*HAS_ALPHA_*/TRUE, 8, out->width, out->height, stride, NULL, NULL);
			gdk_pixbuf_save(pixbuf, "tmp1.png", "png", NULL, NULL);
			g_object_unref(pixbuf);
			g_free(buf);
		}

	}

	// do some test renders to find where the baseline is
	{
		image_t out;
		Title t;

		title_render("iey", &out, &t);
		((TextActor*)actor)->baseline = t.height;
		fh = agl_power_of_two(t.height);

		title_render("ie", &out, &t);
		((TextActor*)actor)->baseline -= t.height -1; // 1 because of spill below baseline
	}

	image_t out;
	title_render(title, &out, &v->title);

	{
		if(!((TextActor*)actor)->texture.ids[0]){
			glGenTextures(1, ((TextActor*)actor)->texture.ids);
			if(gl_error){ gerr ("couldnt create ass_texture."); goto out; }
		}
		((TextActor*)actor)->texture.height = out.height;

		int pixel_format = GL_LUMINANCE_ALPHA;
		glBindTexture  (GL_TEXTURE_2D, ((TextActor*)actor)->texture.ids[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, out.width, out.height, 0, pixel_format, GL_UNSIGNED_BYTE, out.buf);
		gl_warn("gl error using ass texture");

#if 0
		{
			char* buf = g_new0(char, out.width * out.height * 4);
			int stride = out.width * 4;
			int y; for(y=0;y<fh;y++){
				int x; for(x=0;x<out.width;x++){
					*(buf + y * stride + x * 4    ) = *(out.buf + y * out.stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 1) = 0;//*(out.buf + y * out.stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 2) = 0;//*(out.buf + y * out.stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 3) = *(out.buf + y * out.stride + x * N_CHANNELS + 1);
				}
			}
			GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data((const guchar*)buf, GDK_COLORSPACE_RGB, /*HAS_ALPHA_*/TRUE, 8, out.width, out.height, stride, NULL, NULL);
			gdk_pixbuf_save(pixbuf, "tmp.png", "png", NULL, NULL);
			g_object_unref(pixbuf);
			g_free(buf);
		}
#endif
	}

	v->title_is_rendered = true;

  out:
	g_free(out.buf);
	g_free(title);
}


static uint32_t
color_gdk_to_rgba(GdkColor* color)
{
	return ((color->red / 0x100) << 24) + ((color->green / 0x100) << 16) + ((color->blue / 0x100) << 8) + 0xff;
}


static uint32_t
wf_get_gtk_base_color(GtkWidget* widget, GtkStateType state, char alpha)
{
	GtkStyle* style = gtk_style_copy(gtk_widget_get_style(widget));
	GdkColor c = style->base[state];
	g_object_unref(style);

	return (color_gdk_to_rgba(&c) & 0xffffff00) | alpha;
}


static void
add_key_handlers(GtkWindow* window, WaveformViewPlus* waveform, Key keys[])
{
	//list of keys must be terminated with a key of value zero.

	static KeyHold key_hold = {0, NULL};
	static bool key_down = false;

	static GHashTable* key_handlers = NULL;
	if(!key_handlers){
		key_handlers = g_hash_table_new(g_int_hash, g_int_equal);

		int i = 0; while(true){
			Key* key = &keys[i];
			if(i > 100 || !key->key) break;
			g_hash_table_insert(key_handlers, &key->key, key->handler);
			i++;
		}
	}

	gboolean key_hold_on_timeout(gpointer user_data)
	{
		WaveformViewPlus* waveform = user_data;
		if(key_hold.handler) key_hold.handler(waveform);
		return TIMER_CONTINUE;
	}

	gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		WaveformViewPlus* waveform = user_data;

		if(key_down){
			// key repeat
			return true;
		}
		key_down = true;

		KeyHandler* handler = g_hash_table_lookup(key_handlers, &event->keyval);
		if(handler){
			if(key_hold.timer) gwarn("timer already started");
			key_hold.timer = g_timeout_add(100, key_hold_on_timeout, waveform);
			key_hold.handler = handler;
	
			handler(waveform);
		}
		else dbg(1, "%i", event->keyval);

		return true;
	}

	gboolean key_release(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		PF;
		if(!key_down){ /* gwarn("key_down not set"); */ return true; } //sometimes happens at startup

		key_down = false;
		if(key_hold.timer) g_source_remove(key_hold.timer);
		key_hold.timer = 0;

		return true;
	}

	g_signal_connect(waveform, "key-press-event", G_CALLBACK(key_press), waveform);
	g_signal_connect(waveform, "key-release-event", G_CALLBACK(key_release), waveform);
}


static void
remove_key_handlers(GtkWindow* window, WaveformViewPlus* waveform)
{
	g_signal_handlers_disconnect_matched (waveform, G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA, g_signal_lookup ("key-press-event", GTK_TYPE_WIDGET), 0, NULL, NULL, waveform);
	g_signal_handlers_disconnect_matched (waveform, G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA, g_signal_lookup ("key-release-event", GTK_TYPE_WIDGET), 0, NULL, NULL, waveform);
}


static void
scroll_left(WaveformViewPlus* view)
{
	int n_visible_frames = ((float)view->waveform->n_frames) / view->zoom;
	waveform_view_plus_set_start(view, view->start_frame - n_visible_frames / 10);
}


static void
scroll_right(WaveformViewPlus* view)
{
	int n_visible_frames = ((float)view->waveform->n_frames) / view->zoom;
	waveform_view_plus_set_start(view, view->start_frame + n_visible_frames / 10);
}


static void
zoom_in(WaveformViewPlus* view)
{
	waveform_view_plus_set_zoom(view, view->zoom * 1.5);
}


static void
zoom_out(WaveformViewPlus* view)
{
	waveform_view_plus_set_zoom(view, view->zoom / 1.5);
}


static void
waveform_view_plus_gl_init(WaveformViewPlus* view)
{
	PF;
	if(gl_initialised) return;

	AGL_ACTOR_START_DRAW(view->priv->root) {

		agl_set_font_string("Roboto 10");
		if(agl_get_instance()->use_shaders){
			wf_shaders.ass->uniform.colour1 = view->title_colour1;
			wf_shaders.ass->uniform.colour2 = view->title_colour2;
		}

	} AGL_ACTOR_END_DRAW(view->priv->root)

	gl_initialised = true;
}


static void
waveform_view_plus_draw(WaveformViewPlus* view)
{
	WaveformViewPlusPrivate* v = view->priv;

#if 0 //white border
	glPushMatrix(); /* modelview matrix */
		glNormal3f(0, 0, 1); glDisable(GL_TEXTURE_2D);
		glLineWidth(1);
		glColor3f(1.0, 1.0, 1.0);

		int wid = waveform_view_plus_get_width(view);
		int h   = waveform_view_plus_get_height(view);
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 1); glVertex3f(wid, 0.0, 1);
		glVertex3f(wid, h,   1); glVertex3f(0.0,   h, 1);
		glEnd();
	glPopMatrix();
#endif

	if(view->waveform) agl_actor__paint(v->root);
}


static void
waveform_view_plus_gl_on_allocate(WaveformViewPlus* view)
{
	WaveformViewPlusPrivate* v = view->priv;

	if(!v->actor) return;

	int w = waveform_view_plus_get_width(view);
	int h = waveform_view_plus_get_height(view);

	if(w != v->root->region.x2 || h != v->root->region.y2){
		v->root->region = (AGliRegion){0, 0, w, h};
		agl_actor__set_size(v->root);
	}

	wf_canvas_set_viewport(v->canvas, NULL);
}


static AGlActor*
waveform_actor(WaveformViewPlus* view)
{
	void waveform_actor_size(AGlActor* actor)
	{
		#define V_BORDER 4

		actor->region = (AGliRegion){
			.x1 = 0,
			.y1 = V_BORDER,
			.x2 = actor->parent->region.x2,
			.y2 = actor->parent->region.y2 - V_BORDER,
		};
		wf_actor_set_rect((WaveformActor*)actor, &(WfRectangle){0, 0, actor->region.x2, actor->region.y2 - actor->region.y1});
	}

	void waveform_actor_size0(AGlActor* actor)
	{
		waveform_actor_size(actor);
#ifdef AGL_ACTOR_RENDER_CACHE
		actor->fbo = agl_fbo_new(actor->region.x2 - actor->region.x1, actor->region.y2 - actor->region.y1, 0, 0);
#endif
		actor->set_size = waveform_actor_size;
	}

	AGlActor* actor = (AGlActor*)wf_canvas_add_new_actor(view->priv->canvas, view->waveform);
	actor->colour = view->fg_colour;
	actor->set_size = waveform_actor_size0;
	return actor;
}


static AGlActor*
grid_actor(WaveformViewPlus* view)
{
	bool grid_actor_paint(AGlActor* actor)
	{
		WaveformViewPlusPrivate* v = ((WaveformViewPlus*)actor->root->widget)->priv;

		if(v->actor) wf_grid_paint(v->canvas, v->actor);

		return true;
	}

	void grid_actor_size(AGlActor* actor)
	{
		actor->region = actor->parent->region;
	}

	AGlActor* actor = agl_actor__new();
#ifdef AGL_DEBUG_ACTOR
	actor->name = "grid";
#endif
	actor->paint = grid_actor_paint;
	actor->set_size = grid_actor_size;
#ifdef AGL_ACTOR_RENDER_CACHE
	actor->fbo = agl_fbo_new(actor->region.x2 - actor->region.x1, actor->region.y2 - actor->region.y1, 0, 0);
#endif
	return actor;
}


static AGlActor*
text_actor(WaveformViewPlus* view)
{
	bool text_actor_paint(AGlActor* actor)
	{
		WaveformViewPlus* view = (WaveformViewPlus*)actor->root->widget;
		WaveformViewPlusPrivate* v = view->priv;
		TextActor* ta = (TextActor*)actor;

		agl_print(2, waveform_view_plus_get_height(view) - 16, 0, view->text_colour, view->text);

		if(view->title){
			if(!v->title_is_rendered) waveform_view_plus_render_text(view);

			// title text:
			if(agl->use_shaders){
				glEnable(GL_TEXTURE_2D);
				glActiveTexture(GL_TEXTURE0);

				agl_use_program((AGlShader*)wf_shaders.ass);

				float th = ((TextActor*)actor)->texture.height;

#undef ALIGN_TOP
#ifdef ALIGN_TOP
				float y1 = -((int)th - view->title_height - view->title_y_offset);
				agl_textured_rect(v->ass_textures[0], waveform_view_plus_get_width(view) - v->title.width - 4.0f, y, v->title.width, th, &(AGlRect){0.0, 0.0, ((float)v->title.width) / ta->texture.width, 1.0});
#else
				float y = waveform_view_plus_get_height(view) - th;
				agl_textured_rect(ta->texture.ids[0],
					waveform_view_plus_get_width(view) - v->title.width - 4.0f,
					y + ((TextActor*)actor)->baseline - 4.0f,
					v->title.width,
					th,
					&(AGlQuad){0.0, 0.0, ((float)v->title.width) / ta->texture.width, 1.0}
				);
#endif
			}
		}

		return true;
	}

	TextActor* ta = g_new0(TextActor, 1);
	AGlActor* actor = (AGlActor*)ta;
#ifdef AGL_DEBUG_ACTOR
	actor->name = "Text";
#endif
	actor->paint = text_actor_paint;
	return actor;
}


static AGlActor*
bg_actor(WaveformViewPlus* view)
{
	void agl_load_alphamap(char* buf, guint texture, int width, int height)
	{
		int pixel_format = GL_ALPHA;
		glBindTexture  (GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, buf);
		gl_warn("binding bg texture");
	}

	void create_background(AGlTextureActor* ta)
	{
		//create an alpha-map gradient texture

		g_return_if_fail(!ta->texture[0]);

		int width = 256;
		int height = 256;
		char* pbuf = g_new0(char, width * height);
#if 1
		int y; for(y=0;y<height;y++){
			int x; for(x=0;x<width;x++){
				*(pbuf + y * width + x) = ((x+y) * 0xff) / (width * 2);
			}
		}
#else
		// this gradient is brighter in the middle. It only works for stereo.
		int nc = 2;
		int c; for(c=0;c<nc;c++){
			int top = (height / nc) * c;
			int bot = height / nc;
			int mid = height / (2 * nc);
			int y; for(y=0;y<mid;y++){
				int x; for(x=0;x<width;x++){
					int y_ = top + y;
					int val = 0xff * (1.0 + sinf(((float)(-mid + 2 * y)) / mid));
					*(pbuf + y_ * width + x) = ((val) / 8 + ((x+y_) * 0xff) / (width * 2)) / 2;
					y_ = top + bot - y - 1;
					*(pbuf + (y_) * width + x) = ((val) / 8 + ((x+y_) * 0xff) / (width * 2)) / 2;
				}
			}
		} 
#endif

		glEnable(GL_TEXTURE_2D);

		glGenTextures(1, ta->texture);
		if(glGetError() != GL_NO_ERROR){ gerr ("couldnt create bg_texture."); goto out; }

		agl_load_alphamap(pbuf, ta->texture[0], width, height);

	  out:
		g_free(pbuf);
	}

	void bg_actor_set_state(AGlActor* actor)
	{
		if(agl->use_shaders){
#if 0
			((AlphaMapShader*)actor->program)->uniform.fg_colour = 0x4488ffff; // TODO use theme colour, or pass as argument.
#else
			((AlphaMapShader*)actor->program)->uniform.fg_colour = 0x666666ff;
#endif
		}else{
			glColor4f(0.4, 0.4, 0.4, 1.0);
		}
	}

	bool bg_actor_paint(AGlActor* actor)
	{
		agl_textured_rect(((AGlTextureActor*)actor)->texture[0],
			0,
			0,
			actor->parent->region.x2,
			actor->parent->region.y2,
			NULL
		);

		return true;
	}

	AGlTextureActor* ta = g_new0(AGlTextureActor, 1);
	AGlActor* actor = (AGlActor*)ta;
#ifdef AGL_DEBUG_ACTOR
	actor->name = "background";
#endif
	actor->set_state = bg_actor_set_state;
	actor->paint = bg_actor_paint;
	actor->program = (AGlShader*)agl->shaders.alphamap;

	create_background(ta);

	return actor;
}


