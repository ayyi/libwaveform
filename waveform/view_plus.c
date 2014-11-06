/*
  copyright (C) 2012-2014 Tim Orford <tim@orford.org>

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
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include <ass/ass.h>
#include "waveform/peak.h"
#include "waveform/utils.h"
#include "waveform/peakgen.h"

#include "waveform/gl_utils.h"
#include "agl/utils.h"
#include "waveform/utils.h"
#include "waveform/texture_cache.h"
#include "waveform/actor.h"
#include "waveform/grid.h"
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

struct _WaveformViewPlusPrivate {
	WaveformCanvas* canvas;
	WaveformActor*  actor;
	gboolean        show_grid;
	GLuint          ass_textures[1];
	int             title_texture_width;

	gboolean        gl_init_done;
	gboolean        canvas_init_done;
	gboolean        title_is_rendered;
};

static int      waveform_view_plus_get_width            (WaveformViewPlus*);
static int      waveform_view_plus_get_height           (WaveformViewPlus*);

static gpointer waveform_view_plus_parent_class = NULL;

#define WAVEFORM_VIEW_PLUS_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_WAVEFORM_VIEW_PLUS, WaveformViewPlusPrivate))
enum  {
	WAVEFORM_VIEW_PLUS_DUMMY_PROPERTY
};
static gboolean waveform_view_plus_configure_event      (GtkWidget*, GdkEventConfigure*);
static gboolean waveform_view_plus_on_expose            (GtkWidget*, GdkEventExpose*);
static gboolean waveform_view_plus_button_press_event   (GtkWidget*, GdkEventButton*);
static gboolean waveform_view_plus_button_release_event (GtkWidget*, GdkEventButton*);
static gboolean waveform_view_plus_motion_notify_event  (GtkWidget*, GdkEventMotion*);
static gboolean waveform_view_plus_focus_in_event       (GtkWidget*, GdkEventFocus*);
static gboolean waveform_view_plus_focus_out_event      (GtkWidget*, GdkEventFocus*);
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
	gtk_widget_add_events ((GtkWidget*) self, (gint) ((GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK) | GDK_POINTER_MOTION_MASK));
	//GdkGLConfig* glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE);
	if(!gtk_widget_set_gl_capability((GtkWidget*)self, glconfig, gl_context, DIRECT, GDK_GL_RGBA_TYPE)){
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

	gboolean waveform_view_plus_load_new_on_idle(gpointer _view)
	{
		WaveformViewPlus* view = _view;
		g_return_val_if_fail(view, IDLE_STOP);
		if(!view->priv->canvas_init_done){
			waveform_view_plus_init_drawable(view);
			gtk_widget_queue_draw((GtkWidget*)view); //testing.
		}
		return IDLE_STOP;
	}
	g_idle_add(waveform_view_plus_load_new_on_idle, view);
	return view;
}


static void
_waveform_view_plus_set_actor (WaveformViewPlus* view)
{
	WaveformActor* actor = view->priv->actor;

#if 0
	int width = waveform_view_plus_get_width(view);
	wf_actor_set_rect(actor, &(WfRectangle){0, 0, width, waveform_view_plus_get_height(view)});
#else
	waveform_view_plus_gl_on_allocate(view);
#endif

	void _waveform_view_plus_on_draw(WaveformCanvas* wfc, gpointer _view)
	{
		gtk_widget_queue_draw((GtkWidget*)_view);
	}
	actor->canvas->draw = _waveform_view_plus_on_draw;
	actor->canvas->draw_data = view;
}


void
waveform_view_plus_load_file (WaveformViewPlus* view, const char* filename)
{
	WaveformViewPlusPrivate* _view = view->priv;

	if(_view->actor){
		wf_canvas_remove_actor(_view->canvas, _view->actor);
		_view->actor = NULL;
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

	typedef struct
	{
		WaveformViewPlus* view;
		Waveform*     waveform;
	} C;
	C* c = g_new0(C, 1);
	c->view = view;
	c->waveform = view->waveform;

	gboolean waveform_view_plus_load_file_on_idle(gpointer _c)
	{
		C* c = _c;
		WaveformViewPlus* view = c->view;
		g_return_val_if_fail(view, IDLE_STOP);

		if(c->waveform == view->waveform){
			if(!view->priv->canvas_init_done) waveform_view_plus_init_drawable(view);

			WaveformActor* actor = view->priv->actor = wf_canvas_add_new_actor(c->view->priv->canvas, c->view->waveform);

			uint64_t n_frames = waveform_get_n_frames(c->view->waveform);
			if(n_frames){
				wf_actor_set_region(actor, &(WfSampleRegion){0, n_frames});
				wf_actor_set_colour(view->priv->actor, view->fg_colour, view->bg_colour);

				waveform_load(c->view->waveform);
			}
			_waveform_view_plus_set_actor(c->view);

			gtk_widget_queue_draw((GtkWidget*)c->view);
		}else{
			dbg(2, "waveform changed. ignoring...");
		}
		g_free(c);
		return IDLE_STOP;
	}
	g_idle_add(waveform_view_plus_load_file_on_idle, c);
}


void
waveform_view_plus_set_waveform (WaveformViewPlus* view, Waveform* waveform)
{
	PF;
	WaveformViewPlusPrivate* _view = view->priv;

	if(__wf_drawing) gwarn("set_waveform called while already drawing");
	if(_view->actor && _view->canvas){
		wf_canvas_remove_actor(view->priv->canvas, view->priv->actor);
		_view->actor = NULL;
	}
	if(view->waveform){
		g_object_unref(view->waveform);
	}
	gboolean need_init = !_view->canvas_init_done;
	if(!_view->canvas_init_done) waveform_view_plus_init_drawable(view);

	view->waveform = g_object_ref(waveform);
	view->zoom = 1.0;
	view->priv->actor = wf_canvas_add_new_actor(view->priv->canvas, waveform);
	wf_actor_set_region(view->priv->actor, &(WfSampleRegion){0, waveform_get_n_frames(view->waveform)});

	_waveform_view_plus_set_actor(view);

	if(need_init) waveform_view_plus_set_projection((GtkWidget*)view);
	gtk_widget_queue_draw((GtkWidget*)view);
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
	dbg(1, "zoom=%.2f", zoom);
	view->zoom = CLAMP(zoom, 1.0, MAX_ZOOM);

	WfSampleRegion region = {view->start_frame, (waveform_get_n_frames(view->waveform) - view->start_frame) / view->zoom};
	wf_actor_set_region(view->priv->actor, &region);

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
	if(view->priv->actor) wf_actor_set_colour(view->priv->actor, fg, bg);

	if(agl_get_instance()->use_shaders){
		wf_shaders.ass->uniform.colour1 = view->title_colour1 = text1;
		wf_shaders.ass->uniform.colour2 = view->title_colour2 = text2;
	}
}


void
waveform_view_plus_set_show_rms (WaveformViewPlus* view, gboolean _show)
{
	//FIXME this idle hack is because wa is not created until realise.

	static gboolean show; show = _show;

	gboolean _on_idle(gpointer _view)
	{
		WaveformViewPlus* view = _view;

		view->priv->canvas->show_rms = show;
		gtk_widget_queue_draw((GtkWidget*)view);

		return IDLE_STOP;
	}
	g_idle_add(_on_idle, view);
}


void
waveform_view_plus_set_show_grid (WaveformViewPlus* view, gboolean show)
{
	view->priv->show_grid = show;
	gtk_widget_queue_draw((GtkWidget*)view);
}


static void
waveform_view_plus_realize (GtkWidget* base)
{
	PF2;
	WaveformViewPlus* self = (WaveformViewPlus*)base;
	GdkWindowAttr attrs = {0};
	GTK_WIDGET_SET_FLAGS (base, GTK_REALIZED);
	memset (&attrs, 0, sizeof (GdkWindowAttr));
	attrs.window_type = GDK_WINDOW_CHILD;
	attrs.width = base->allocation.width;
	attrs.wclass = GDK_INPUT_OUTPUT;
	attrs.event_mask = gtk_widget_get_events(base) | GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK;
	_g_object_unref0(base->window);
	base->window = gdk_window_new (gtk_widget_get_parent_window(base), &attrs, 0);
	gdk_window_set_user_data (base->window, self);
	gtk_widget_set_style (base, gtk_style_attach(gtk_widget_get_style(base), base->window));
	gtk_style_set_background (gtk_widget_get_style (base), base->window, GTK_STATE_NORMAL);
	gdk_window_move_resize (base->window, base->allocation.x, base->allocation.y, base->allocation.width, base->allocation.height);

	if(!self->priv->canvas_init_done) waveform_view_plus_init_drawable(self);
}


static void
waveform_view_plus_unrealize (GtkWidget* widget)
{
	PF;
	WaveformViewPlus* self = (WaveformViewPlus*)widget;
	gdk_window_set_user_data (widget->window, NULL);

	if(self->priv->actor){
		wf_canvas_remove_actor(self->priv->canvas, self->priv->actor);
		self->priv->actor = 0;
		if(self->waveform) _g_object_unref0(self->waveform); // is unreffed by wf_actor_free, but the view also needs to release its reference.
	}

	if(self->priv->canvas) wf_canvas_free0(self->priv->canvas);
	self->priv->canvas_init_done = false;
}


static gboolean
waveform_view_plus_configure_event (GtkWidget* base, GdkEventConfigure* event)
{
	gboolean result = FALSE;
#if 0
	WaveformViewPlus* self = (WaveformViewPlus*) base;
	g_return_val_if_fail (event != NULL, FALSE);
	GdkGLContext* _tmp0_ = gtk_widget_get_gl_context(base);
	GdkGLContext* glcontext = _g_object_ref0 (_tmp0_);
	GdkGLDrawable* _tmp2_ = gtk_widget_get_gl_drawable ((GtkWidget*) self);
	GdkGLDrawable* gldrawable = _g_object_ref0 (_tmp2_);
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
		result = FALSE;
		_g_object_unref0 (gldrawable);
		_g_object_unref0 (glcontext);
		return result;
	}
	GtkAllocation _tmp7_ = ((GtkWidget*) self)->allocation;
	gint _tmp8_ = _tmp7_.width;
	GtkAllocation _tmp9_ = ((GtkWidget*) self)->allocation;
	gint _tmp10_ = _tmp9_.height;
	glViewport ((GLint) 0, (GLint) 0, (GLsizei) _tmp8_, (GLsizei) _tmp10_);
	gboolean _tmp11_ = self->priv->gl_init_done;
	if (!_tmp11_) {
		glEnable ((GLenum) GL_TEXTURE_2D);
		self->priv->gl_init_done = TRUE;
	}
	gdk_gl_drawable_gl_end (gldrawable);
	result = TRUE;
	_g_object_unref0 (gldrawable);
	_g_object_unref0 (glcontext);
#endif
	return result;
}


static gboolean
waveform_view_plus_on_expose (GtkWidget* widget, GdkEventExpose* event)
{
	WaveformViewPlus* view = (WaveformViewPlus*)widget;
	g_return_val_if_fail (event, FALSE);

	if(!GTK_WIDGET_REALIZED(widget)) return true;
	if(!gl_initialised || !view->priv->canvas_init_done) return true;

	WF_VIEW_START_DRAW {
		// needed for the case of shared contexts, where one of the other contexts modifies the projection.
		waveform_view_plus_set_projection(widget);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		waveform_view_plus_draw(view);

		gdk_gl_drawable_swap_buffers(view->priv->canvas->gl.gdk.drawable);
	} WF_VIEW_END_DRAW

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
	dbg(0, "");
	gboolean result = false;
	return result;
}


static gboolean
waveform_view_plus_motion_notify_event (GtkWidget* widget, GdkEventMotion* event)
{
	g_return_val_if_fail(event, false);
	dbg(0, "");
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


static void
waveform_view_plus_init_drawable (WaveformViewPlus* view)
{
	GtkWidget* widget = (GtkWidget*)view;

	if(!GTK_WIDGET_REALIZED(widget)) return;

	if(!(view->priv->canvas = wf_canvas_new_from_widget(widget))) return;

	if(!view->fg_colour)     view->fg_colour     = wf_get_gtk_fg_color(widget, GTK_STATE_NORMAL);
	if(!view->title_colour1) view->title_colour1 = wf_get_gtk_text_color(widget, GTK_STATE_NORMAL);
	if(!view->text_colour){
		//TODO because the black background is not a theme colour we need to be careful to use a contrasting colour
		if(false)
			view->text_colour   = wf_get_gtk_text_color(widget, GTK_STATE_NORMAL);
		else
			view->text_colour   = wf_get_gtk_base_color(widget, GTK_STATE_NORMAL, 0xaa);
	}
	view->title_colour2 = 0x0000ffff; //FIXME

	waveform_view_plus_gl_init(view);
	waveform_view_plus_set_projection(widget);

	view->priv->canvas_init_done = true;
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

	if(!view->priv->canvas_init_done) waveform_view_plus_init_drawable(view);

	if(!gl_initialised) return;

	waveform_view_plus_gl_on_allocate(view);

	waveform_view_plus_set_projection(widget);
}


static void
waveform_view_plus_class_init (WaveformViewPlusClass * klass)
{
	waveform_view_plus_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (WaveformViewPlusPrivate));
	GTK_WIDGET_CLASS (klass)->configure_event = waveform_view_plus_configure_event;
	GTK_WIDGET_CLASS (klass)->expose_event = waveform_view_plus_on_expose;
	GTK_WIDGET_CLASS (klass)->button_press_event = waveform_view_plus_button_press_event;
	GTK_WIDGET_CLASS (klass)->button_release_event = waveform_view_plus_button_release_event;
	GTK_WIDGET_CLASS (klass)->motion_notify_event = waveform_view_plus_motion_notify_event;
	GTK_WIDGET_CLASS (klass)->focus_in_event = waveform_view_plus_focus_in_event;
	GTK_WIDGET_CLASS (klass)->focus_out_event = waveform_view_plus_focus_out_event;
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
	if(view->title) _g_free0(view->title);
	if(view->text)  _g_free0(view->text);
	//_g_free0 (self->priv->_filename);
	//TODO free actor?
	if(view->waveform) waveform_unref0(view->waveform); //TODO should be done in dispose?
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

	PF;
	if(v->title_is_rendered) gwarn("title is already rendered");

	GError* error = NULL;
	GRegexMatchFlags flags = 0;
	static GRegex* regex = NULL;
	if(!regex) regex = g_regex_new("[_]", 0, flags, &error);
	gchar* str = g_regex_replace(regex, view->title, -1, 0, " ", flags, &error);

	char* title = g_strdup_printf("%s", str);
	g_free(str);

	int fh = 32;
	int fw = agl_power_of_two(strlen(title) * 20);
	ass_set_frame_size(ass_renderer, fw, fh);

	char* script_ = g_strdup_printf(script, fw, fh, FONT_SIZE, title);

	//TODO this defines the height, but we dont yet know the height, so really we need to do a quick prerender.
	ASS_Track* track = ass_read_memory(ass_library, script_, strlen(script_), NULL);
	g_free(script_);
	if (!track) {
		printf("track init failed!\n");
		return;
	}

	// ASS_Image is a list of alphamaps each with a uint32 RGBA colour/alpha
	ASS_Image* img = ass_render_frame(ass_renderer, track, 100, NULL);

	int height = 0, width = 0, y_unused = 64;
	ASS_Image* i = img;
	for(;i;i=i->next){
		height = MAX(height, i->h);
		width  = MAX(width,  i->dst_x + i->w);
		y_unused = MIN(y_unused, 64 - i->dst_y - i->h);
	}
	//dbg(0, "width=%i height=%i y_unused=%i", width, height, y_unused);
	view->title_width = width;
	v->title_texture_width = fw;
	view->title_height = height;
	view->title_y_offset = y_unused;

	// ass output will be composited into this buffer.
	// 2 bytes per pixel for use with GL_LUMINANCE8_ALPHA8 mode.
	//int w = agl_power_of_two(width);
	//int h = agl_power_of_two(height + y_unused); //TODO
	//dbg(0, "width: guess=%i actual=%i texture=%i", strlen(title) * 20, width, w);
	image_t out = {fw, fh, fw * N_CHANNELS, g_new0(guchar, fw * fh * N_CHANNELS)};

	//clear the background to border colour for better antialiasing at edges
	// ...no, this is not needed.
#if 0
	int x, y; for(y=0;y<out.height;y++){
		for(x=0;x<out.width;x++){
			*(out.buf + y * out.stride + x * N_CHANNELS) = 0xff;
		}
	}
#endif

	int cnt = 0;
	for(i=img;i;i=i->next,cnt++){
		blend_single(&out, i); // blend each glyph onto the output buffer.
	}
	dbg(1, "%d images blended", cnt);

	{
		if(!v->ass_textures[0]){
			glGenTextures(1, v->ass_textures);
			if(gl_error){ gerr ("couldnt create ass_texture."); return; } //TODO leaks
		}

		int pixel_format = GL_LUMINANCE_ALPHA;
		glBindTexture  (GL_TEXTURE_2D, v->ass_textures[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, out.width, out.height, 0, pixel_format, GL_UNSIGNED_BYTE, out.buf);
		gl_warn("gl error using ass texture");

#if 0
		{
			char* buf = g_new0(char, w * h * 4);
			int stride = w * 4;
			for(y=0;y<h;y++){
				for(x=0;x<w;x++){
					*(buf + y * stride + x * 4    ) = *(out.buf + y * out.stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 1) = 0;//*(out.buf + y * out.stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 2) = 0;//*(out.buf + y * out.stride + x * N_CHANNELS);
					*(buf + y * stride + x * 4 + 3) = 0xff;
				}
			}
			GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data((const guchar*)buf, GDK_COLORSPACE_RGB, /*HAS_ALPHA_*/TRUE, 8, out.width, out.height, stride, NULL, NULL);
			gdk_pixbuf_save(pixbuf, "tmp.png", "png", NULL, NULL);
			g_object_unref(pixbuf);
			g_free(buf);
		}
#endif
	}

	ass_free_track(track);
	g_free(out.buf);
	g_free(title);

	v->title_is_rendered = true;
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
	if(!key_handlers) key_handlers = g_hash_table_new(g_int_hash, g_int_equal);

	int i = 0; while(true){
		Key* key = &keys[i];
		if(i > 100 || !key->key) break;
		g_hash_table_insert(key_handlers, &key->key, key->handler);
		i++;
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

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), waveform);
	g_signal_connect(window, "key-release-event", G_CALLBACK(key_release), waveform);
}


static void
remove_key_handlers(GtkWindow* window, WaveformViewPlus* waveform)
{
	#warning TODO remove_key_handlers
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

	WF_VIEW_START_DRAW {

		agl_set_font_string("Roboto 10");
		if(agl_get_instance()->use_shaders){
			wf_shaders.ass->uniform.colour1 = view->title_colour1;
			wf_shaders.ass->uniform.colour2 = view->title_colour2;
		}

	} WF_VIEW_END_DRAW

	gl_initialised = true;
}


typedef void    (Renderer)       (WaveformViewPlus*);

Renderer text_render;

static void
waveform_view_plus_draw(WaveformViewPlus* view)
{
	WaveformViewPlusPrivate* v = view->priv;
	Waveform* w = view->waveform;
	WaveformActor* actor = view->priv->actor;

#if 0 //white border
	glPushMatrix(); /* modelview matrix */
		glNormal3f(0, 0, 1); glDisable(GL_TEXTURE_2D);
		glLineWidth(1);
		glColor3f(1.0, 1.0, 1.0);

		int wid = waveform_view_plus_get_width(view);;
		int h   = waveform_view_plus_get_height(view);
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 1); glVertex3f(wid, 0.0, 1);
		glVertex3f(wid, h,   1); glVertex3f(0.0,   h, 1);
		glEnd();
	glPopMatrix();
#endif

	if(!w) return;

	if(actor) wf_actor_paint(actor);

	if(v->show_grid){
		WfViewPort viewport; wf_actor_get_viewport(actor, &viewport);

		WfSampleRegion region = {view->start_frame, w->n_frames};
		wf_grid_paint(view->priv->canvas, &region, &viewport);
	}

	agl_print(2, waveform_view_plus_get_height(view) - 16, 0, view->text_colour, view->text);

																		// TODO render from renderer list
	text_render(view);
}


void
text_render(WaveformViewPlus* view)
{
	WaveformViewPlusPrivate* v = view->priv;

	if(!v->title_is_rendered) waveform_view_plus_render_text(view);

	// title text:
	if(agl->use_shaders){
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);

		agl_use_program((AGlShader*)wf_shaders.ass);

		//texture size
		float th = agl_power_of_two(view->title_height + view->title_y_offset);

#undef ALIGN_TOP
#ifdef ALIGN_TOP
		float y1 = -((int)th - view->title_height - view->title_y_offset);
		agl_textured_rect(v->ass_textures[0], waveform_view_plus_get_width(view) - view->title_width - 4.0f, y, view->title_width, th, &(AGlRect){0.0, 0.0, ((float)view->title_width) / v->title_texture_width, 1.0});
#else
		float y = waveform_view_plus_get_height(view) - th;
		agl_textured_rect(v->ass_textures[0],
			waveform_view_plus_get_width(view) - view->title_width - 4.0f,
			y,
			view->title_width,
			th,
			&(AGlQuad){0.0, 0.0, ((float)view->title_width) / v->title_texture_width, 1.0}
		);
#endif
	}
}


static void
waveform_view_plus_gl_on_allocate(WaveformViewPlus* view)
{
	if(!view->priv->actor) return;

	WfRectangle rect = {0, 0, waveform_view_plus_get_width(view), waveform_view_plus_get_height(view)};
	wf_actor_set_rect(view->priv->actor, &rect);

	wf_canvas_set_viewport(view->priv->canvas, NULL);
}


