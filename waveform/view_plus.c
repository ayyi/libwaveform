/*
  copyright (C) 2012 Tim Orford <tim@orford.org>

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

static GdkGLConfig*  glconfig = NULL;
static GdkGLContext* gl_context = NULL;
static gboolean      gl_initialised = FALSE;

#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _WaveformViewPlusPrivate {
	gboolean        gl_init_done;
	WaveformCanvas* canvas;
	WaveformActor*  actor;
	gboolean        show_grid;
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
static void     waveform_view_plus_realize              (GtkWidget*);
static void     waveform_view_plus_unrealize            (GtkWidget*);
static void     waveform_view_plus_allocate             (GtkWidget*, GdkRectangle*);
static void     waveform_view_plus_finalize             (GObject*);
static void     waveform_view_plus_set_projection       (GtkWidget*);
static void     waveform_view_plus_init_drawable        (WaveformViewPlus*);
static void     waveform_view_plus_gl_on_allocate       (WaveformViewPlus*);
static void     waveform_view_plus_render_text          (WaveformViewPlus*);

static const int frame_w = 512;
static const int frame_h =  64;
#define FONT \
	"Droid Sans"
	//"Ubuntu"
	//"Open Sans Rg"
	//"Fontin Sans Rg"

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
GLuint        ass_textures[] = {0, 0};

#include "view_plus_gl.c"


static gboolean canvas_init_done = false;


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
		if(!canvas_init_done){
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
	wf_actor_allocate(actor, &(WfRectangle){0, 0, width, waveform_view_plus_get_height(view)});
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
		g_object_unref(view->waveform);
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
			if(!canvas_init_done) waveform_view_plus_init_drawable(view);

			WaveformActor* actor = view->priv->actor = wf_canvas_add_new_actor(c->view->priv->canvas, c->view->waveform);
			wf_actor_set_region(actor, &(WfSampleRegion){0, waveform_get_n_frames(c->view->waveform)});
			wf_actor_set_colour(view->priv->actor, view->fg_colour, view->bg_colour);

			waveform_load(c->view->waveform);

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
	wf_canvas_remove_actor(view->priv->canvas, view->priv->actor);
	_view->actor = NULL;
	if(view->waveform){
		g_object_unref(view->waveform);
	}
	gboolean need_init = !canvas_init_done;
	if(!canvas_init_done) waveform_view_plus_init_drawable(view);

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
waveform_view_plus_set_colour(WaveformViewPlus* w, uint32_t fg, uint32_t bg, uint32_t text1, uint32_t text2)
{
	w->fg_colour = fg;
	w->bg_colour = bg;
	if(w->priv->actor) wf_actor_set_colour(w->priv->actor, fg, bg);

	w->title_colour1 = text1;
	w->title_colour2 = text2;
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

	if(!canvas_init_done) waveform_view_plus_init_drawable(self);
}


static void
waveform_view_plus_unrealize (GtkWidget* widget)
{
	PF0;
	WaveformViewPlus* self = (WaveformViewPlus*)widget;
	gdk_window_set_user_data (widget->window, NULL);

	wf_canvas_free0(self->priv->canvas);
	canvas_init_done = false;
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
	if(!gl_initialised) return true;

	WF_START_DRAW {
		// needed for the case of shared contexts, where one of the other contexts modifies the projection.
		waveform_view_plus_set_projection(widget);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw(view);

		gdk_gl_drawable_swap_buffers(view->priv->canvas->gl_drawable);
	} WF_END_DRAW

	return true;
}


static gboolean
waveform_view_plus_button_press_event (GtkWidget* base, GdkEventButton* event)
{
	g_return_val_if_fail (event != NULL, false);
	gboolean result = false;
	return result;
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


static void
waveform_view_plus_init_drawable (WaveformViewPlus* view)
{
	GtkWidget* widget = (GtkWidget*)view;

	if(!GTK_WIDGET_REALIZED(widget)) return;

	if(!(view->priv->canvas = wf_canvas_new_from_widget(widget))) return;

	if(!view->fg_colour)     view->fg_colour     = wf_get_gtk_fg_color(widget, GTK_STATE_NORMAL);
	if(!view->title_colour1) view->title_colour1 = wf_get_gtk_text_color(widget, GTK_STATE_NORMAL);
	if(!view->text_colour)   view->text_colour   = wf_get_gtk_text_color(widget, GTK_STATE_NORMAL);
	view->title_colour2 = 0x0000ffff; //FIXME
	dbg(0, "colour=0x%x", view->title_colour1);

	waveform_view_plus_gl_init(view);
	waveform_view_plus_set_projection(widget);

	canvas_init_done = true;
}


static void
waveform_view_plus_allocate (GtkWidget* widget, GdkRectangle* allocation)
{
	PF;
	g_return_if_fail (allocation);

	WaveformViewPlus* wv = (WaveformViewPlus*)widget;
	widget->allocation = (GtkAllocation)(*allocation);
	if ((GTK_WIDGET_FLAGS (widget) & GTK_REALIZED) == 0) return;
	gdk_window_move_resize(widget->window, widget->allocation.x, widget->allocation.y, widget->allocation.width, widget->allocation.height);

	if(!canvas_init_done) waveform_view_plus_init_drawable(wv);

	if(!gl_initialised) return;

	waveform_view_plus_gl_on_allocate(wv);

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
	GTK_WIDGET_CLASS (klass)->realize = waveform_view_plus_realize;
	GTK_WIDGET_CLASS (klass)->unrealize = waveform_view_plus_unrealize;
	GTK_WIDGET_CLASS (klass)->size_allocate = waveform_view_plus_allocate;
	G_OBJECT_CLASS (klass)->finalize = waveform_view_plus_finalize;

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

		ass_set_frame_size(ass_renderer, frame_w, frame_h);
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
}


static void
waveform_view_plus_finalize (GObject* obj)
{
	WaveformViewPlus* view = WAVEFORM_VIEW_PLUS(obj);
	if(view->title) _g_free0(view->title);
	if(view->text)  _g_free0(view->text);
	//_g_free0 (self->priv->_filename);
	//TODO free actor?
	waveform_unref0(view->waveform); //TODO should be done in dispose?
	G_OBJECT_CLASS (waveform_view_plus_parent_class)->finalize(obj);
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
	PF;
	char* script_ = g_strdup_printf(script, frame_w, frame_h, 28, view->title);
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
	dbg(0, "width=%i height=%i y_unused=%i", width, height, y_unused);
	view->title_width = width;
	view->title_height = height;
	view->title_y_offset = y_unused;

	// ass output will be composited into this buffer.
	// 2 bytes per pixel for use with GL_LUMINANCE8_ALPHA8 mode.
	int w = agl_power_of_two(width);
	int h = agl_power_of_two(height + y_unused);
	image_t out = {w, h, w * N_CHANNELS, g_new0(guchar, w * h * N_CHANNELS)};

#if 0 //BACKGROUND
	int width  = out.width;
	int height = out.height;
	int y; for(y=0;y<height;y++){
		int x; for(x=0;x<width;x++){
			*(out.buf + y * width + x) = ((x+y) * 0xff) / (width * 2);
		}
	}
#else
	//clear the background to border colour for better antialiasing at edges
	int x, y; for(y=0;y<out.height;y++){
		for(x=0;x<out.width;x++){
			*(out.buf + y * out.stride + N_CHANNELS * x) = 0xff; //TODO should be border colour.
		}
	}
#endif

	int cnt = 0;
	for(i=img;i;i=i->next,cnt++){
		blend_single(&out, i); // each one of these is a single glyph
	}
	dbg(1, "%d images blended", cnt);

	{
		glGenTextures(1, ass_textures);
		if(gl_error){ gerr ("couldnt create ass_texture."); return; }

		int pixel_format = GL_LUMINANCE_ALPHA;
		glBindTexture  (GL_TEXTURE_2D, ass_textures[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, out.width, out.height, 0, pixel_format, GL_UNSIGNED_BYTE, out.buf);
		gl_warn("gl error binding ass texture");
	}

	ass_free_track(track);
	ass_renderer_done(ass_renderer);
	ass_library_done(ass_library);
	g_free(out.buf);
}

