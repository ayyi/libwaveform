/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2015 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __agl_actor_c__
#define __gl_canvas_priv__
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <gtk/gtk.h>
#include "agl/utils.h"
#include "agl/ext.h"
#include "waveform/utils.h"
#include "agl/shader.h"
#include "agl/actor.h"
#ifdef AGL_ACTOR_RENDER_CACHE
#include "agl/fbo.h"
#endif

#define HANDLED TRUE
#define NOT_HANDLED FALSE
#define CURSOR_NORMAL 0

#ifdef DEBUG
#define AGL_DEBUG if(agl->debug)
#else
#define AGL_DEBUG if(false)
#endif
static AGl* agl = NULL;

static bool   agl_actor__is_onscreen  (AGlActor*);
static bool  _actor__on_event    (AGlActor*, GdkEvent*, AGliPt);
static void   agl_actor__init    (AGlActor*);              // called once when gl context is available. and again if gl context changes, eg after re-realize.
#ifdef USE_FRAME_CLOCK
static bool   agl_actor__is_animating (AGlActor*);
#endif


AGlActor*
agl_actor__new()
{
	agl = agl_get_instance();

	AGlActor* a = g_new0(AGlActor, 1);
	a->paint = agl_actor__null_painter;
	return a;
}


AGlActor*
agl_actor__new_root(GtkWidget* widget)
{
	void agl_actor__have_drawable(AGlRootActor* a, GdkGLDrawable* drawable)
	{
		g_return_if_fail(!a->gl.gdk.drawable);
		g_return_if_fail(!a->gl.gdk.context);

		static bool first_time = true;

		a->viewport.w = a->widget->allocation.width;
		a->viewport.h = a->widget->allocation.height;

		a->gl.gdk.drawable = drawable;
		a->gl.gdk.context = gtk_widget_get_gl_context(a->widget);

		if(first_time){
			agl_get_extensions();

			gdk_gl_drawable_gl_begin (a->gl.gdk.drawable, a->gl.gdk.context);

				int version = 0;
				const char* _version = (const char*)glGetString(GL_VERSION);
				if(_version){
					gchar** split = g_strsplit(_version, ".", 2);
					if(split){
						version = atoi(split[0]);
						printf("gl version: %i\n", version);
						g_strfreev(split);
					}
				}

			agl_gl_init();

			gdk_gl_drawable_gl_end(a->gl.gdk.drawable);
		}

		gdk_gl_drawable_gl_begin (a->gl.gdk.drawable, a->gl.gdk.context);

		agl_actor__init((AGlActor*)a);

		gdk_gl_drawable_gl_end(a->gl.gdk.drawable);

		first_time = false;
	}

	bool agl_actor__try_drawable(gpointer _actor)
	{
		AGlRootActor* a = _actor;
		if(!a->gl.gdk.drawable){
			GdkGLDrawable* drawable = gtk_widget_get_gl_drawable(a->widget);
			if(drawable){
				agl_actor__have_drawable(a, drawable);
			}
		}
		return G_SOURCE_REMOVE;
	}

	void agl_actor__on_unrealise(GtkWidget* widget, gpointer _actor)
	{
		AGlRootActor* a = _actor;

		a->gl.gdk.drawable = NULL;
		a->gl.gdk.context  = NULL;
	}

	void agl_actor__on_realise(GtkWidget* widget, gpointer _actor)
	{
		AGlRootActor* a = _actor;

		agl_actor__try_drawable(a);
		if(!a->gl.gdk.drawable){
			g_idle_add(agl_actor__try_drawable, a);
		}
	}

	AGlRootActor* a = (AGlRootActor*)agl_actor__new_root_(CONTEXT_TYPE_GTK);
	a->widget = widget;

	if(GTK_WIDGET_REALIZED(widget)){
		agl_actor__on_realise(widget, a);
	}
	g_signal_connect((gpointer)widget, "realize", G_CALLBACK(agl_actor__on_realise), a);
	g_signal_connect((gpointer)widget, "unrealize", G_CALLBACK(agl_actor__on_unrealise), a);

	return (AGlActor*)a;
}


AGlActor*
agl_actor__new_root_(ContextType type)
{
	agl = agl_get_instance();

	AGlRootActor* a = g_new0(AGlRootActor, 1);
	a->type = type;
#ifdef AGL_DEBUG_ACTOR
	((AGlActor*)a)->name = "ROOT";
#endif
	((AGlActor*)a)->root = a;
	((AGlActor*)a)->paint = agl_actor__null_painter;

	return (AGlActor*)a;
}


void
agl_actor__free(AGlActor* actor)
{
	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* child = l->data;
		agl_actor__free(child);
	}
	g_list_free0(actor->children);

#ifdef AGL_ACTOR_RENDER_CACHE
	if(actor->fbo) agl_fbo_free(actor->fbo);
#endif

	if(actor->free)
		actor->free(actor);
	else
		g_free(actor);
}


AGlActor*
agl_actor__add_child(AGlActor* actor, AGlActor* child)
{
	g_return_val_if_fail(actor && child, NULL);
#ifdef DEBUG
	g_return_val_if_fail(!g_list_find(actor->children, child), NULL);
#endif

	GList* pos = NULL;
	GList* l = actor->children;
	for(;l;l=l->next){
		if(child->z < ((AGlActor*)l->data)->z){
			pos = l->prev;
			break;
		}
	}
	actor->children = pos ? g_list_insert_before (actor->children, l, child) : g_list_append(actor->children, child);

	child->parent = actor;

	if(actor->root){
		child->root = actor->root;

		void set_child_roots(AGlActor* actor)
		{
			GList* l = actor->children;
			for(;l;l=l->next){
				AGlActor* child = l->data;
				if(!child->root){
					child->root = actor->root;
					set_child_roots(child);
				}
			}
		}
		set_child_roots(child);

		// TODO need some way to detect type?
		#define READY_FOR_INIT(A) (A->root->widget ? GTK_WIDGET_REALIZED(A->root->widget) : true)
		if(child->init && READY_FOR_INIT(child)) child->init(child);
	}

	return child;
}


void
agl_actor__remove_child(AGlActor* actor, AGlActor* child)
{
	g_return_if_fail(actor && child);
	g_return_if_fail(g_list_find(actor->children, child));

	actor->children = g_list_remove(actor->children, child);

	agl_actor__free(child);

	agl_actor__invalidate(actor);
}


AGlActor*
agl_actor__replace_child(AGlActor* actor, AGlActor* child, AGlActor* new_child)
{
	GList* l = g_list_find(actor->children, child);
	if(l){
		agl_actor__add_child(actor, new_child);

		// update the children list such that the original order is preserved
		actor->children = g_list_remove(actor->children, new_child);
		GList* j = g_list_find(actor->children, child);
		j->data = new_child;

		agl_actor__free(child);

		return new_child;
	}
	return NULL;
}


static void
agl_actor__init(AGlActor* a)
{
	// note that this may be called more than once, eg on settings change.

	if(agl->use_shaders && a->program && !a->program->program) agl_create_program(a->program);

	call(a->init, a);

	GList* l = a->children;
	for(;l;l=l->next) agl_actor__init((AGlActor*)l->data);
}


static bool
agl_actor__is_onscreen(AGlActor* a)
{
	if(a->root->widget){
		int h = a->root->widget->allocation.height; // TODO use viewport->height
		AGlRect* viewport = &a->root->viewport;
		AGliPt offset = agl_actor__find_offset(a);
		//gwarn("   %s %i %.1f x %i %.1f, offset: x=%i y=%i viewport=%.1f %.1f", a->name, w, viewport->width, h, viewport->height, offset.x, offset.y, viewport->x, viewport->y);
		//if(offset.y + a->region.y2  < viewport->y || offset.y > viewport->y + h) dbg(0, "         offscreen: %s", a->name);
		return !(offset.y + a->region.y2  < viewport->y || offset.y > viewport->y + h);
	}
	return true;
}


#ifdef AGL_ACTOR_RENDER_CACHE
static void
render_from_fbo(AGlActor* a)
{
	// render the FBO to screen

	AGlFBO* fbo = a->fbo;
	g_return_if_fail(fbo);

	// TODO fix the real agl_textured_rect so that it doesnt call glBlendFunc
	void agl_textured_rect_(guint texture, float x, float y, float w, float h, AGlQuad* _t)
	{
		glBindTexture(GL_TEXTURE_2D, texture);

		AGlQuad t = _t ? *_t : (AGlQuad){0.0, 0.0, 1.0, 1.0};

		glBegin(GL_QUADS);
		glTexCoord2d(t.x0, t.y0); glVertex2d(x,     y);
		glTexCoord2d(t.x1, t.y0); glVertex2d(x + w, y);
		glTexCoord2d(t.x1, t.y1); glVertex2d(x + w, y + h);
		glTexCoord2d(t.x0, t.y1); glVertex2d(x,     y + h);
		glEnd();
	}

	if(agl->use_shaders){
		agl->shaders.texture->uniform.fg_colour = 0xffffffff;
		agl_use_program((AGlShader*)agl->shaders.texture);
	}else{
		agl_enable(0); // TODO find out why this is needed.
		agl_enable(AGL_ENABLE_TEXTURE_2D | AGL_ENABLE_BLEND);
		glColor4f(1.0, 1.0, 1.0, 1.0); // seems to make a difference for alpha
	}
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	float w = fbo->width;
	float h = fbo->height;

	agl_textured_rect_(fbo->texture, 0.0, 0.0, w, h, &(AGlQuad){0.0, h / agl_power_of_two(h), w / agl_power_of_two(w), 0.0}); // TODO y is reversed - why needed?

#undef FBO_MARKER // show red dot in top left corner of fbos for debugging
#ifdef FBO_MARKER
	agl->shaders.plain->uniform.colour = 0xff0000ff;
	agl_use_program((AGlShader*)agl->shaders.plain);
	glRectf(2, 2, 8, 8);
#endif

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl_warn("");
}
#endif


void
agl_actor__paint(AGlActor* a)
{
	if(!a->root) return;

	if(!agl_actor__is_onscreen(a)) return;

	if(a->region.x1 || a->region.y1){
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(a->region.x1, a->region.y1, 0.0);
	}

	// TODO remove special case - create agl_root_actor__paint() ?
	if(a == (AGlActor*)a->root) glTranslatef(0, -((AGlRootActor*)a)->viewport.y, 0);

#ifdef AGL_ACTOR_RENDER_CACHE
	// TODO check case where actor is translated and is partially offscreen.
	// TODO actor may be huge. do we need to crop to viewport ?
	if(a->fbo && a->cache.enabled && !(a->region.x2 - a->region.x1 > AGL_MAX_FBO_WIDTH)){
		if(!a->cache.valid){
			agl_draw_to_fbo(a->fbo) {
				glClearColor(0.0, 0.0, 0.0, 0.0); // background colour must be same as foreground for correct antialiasing
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				call(a->set_state, a);
				if(agl->use_shaders && a->program) agl_use_program(a->program);

				a->paint(a);

				// TODO check assumptions are correct, eg check children not also cached to fbo.
				GList* l = a->children;
				for(;l;l=l->next){
					agl_actor__paint((AGlActor*)l->data);
				}
			} agl_end_draw_to_fbo;
			a->cache.valid = true;
		}

		render_from_fbo(a);
	}else{
#else
	if(true){
#endif
		call(a->set_state, a);
		if(agl->use_shaders && a->program) agl_use_program(a->program);

		a->paint(a);
	}

#ifdef AGL_ACTOR_RENDER_CACHE
	if(!a->cache.valid){
#endif
		GList* l = a->children;
		for(;l;l=l->next){
			agl_actor__paint((AGlActor*)l->data);
		}
#ifdef AGL_ACTOR_RENDER_CACHE
	}
#endif

	if(a->region.x1 || a->region.y1){
		glPopMatrix();
	}

#undef SHOW_ACTOR_BORDERS
#ifdef SHOW_ACTOR_BORDERS
	static int depth; depth = -1;
	static uint32_t colours[10] = {0xff000099, 0xff990099, 0x00ff0099, 0x0000ff99, 0xff000099, 0xffff0099, 0x00ffff99};

	void paint_border(AGlActor* a)
	{
		depth++;
		glPushMatrix();
		glTranslatef(a->region.x1, a->region.y1, 0.0);
		agl->shaders.plain->uniform.colour = colours[depth];
		agl_use_program((AGlShader*)agl->shaders.plain);

		#define W 1
		AGliRegion r = a->region;
		float width = r.x2 - r.x1;
		float height = MAX(0.0, r.y2 - r.y1);
		agl_rect(0.0,       0.0,        width, W     );
		agl_rect(0.0,       0.0,        W,     height);
		agl_rect(0.0,       height - W, width, W     );
		agl_rect(width - W, 0.0,        W,     height);

		GList* l = a->children;
		for(;l;l=l->next){
			paint_border((AGlActor*)l->data);
		}

		glPopMatrix();
		depth--;
	}

	if(!a->parent){
		AGl* agl = agl_get_instance();
		agl->shaders.plain->uniform.colour = 0xff0000ff;
		agl_use_program((AGlShader*)agl->shaders.plain);
		paint_border(a);
	}
#endif
}


void
agl_actor__set_size(AGlActor* actor)
{
	call(actor->set_size, actor); // actors need to update their own regions.

#ifdef AGL_ACTOR_RENDER_CACHE
	if(actor->fbo){
		int w = actor->region.x2 - actor->region.x1;
		int h = actor->region.y2 - actor->region.y1;

		agl_fbo_set_size (actor->fbo, w, h);
		actor->cache.valid = false;
	}
#endif

	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* a = l->data;
		agl_actor__set_size(a);
	}
}


void
agl_actor__set_use_shaders (AGlRootActor* actor, gboolean val)
{
	AGl* agl = agl_get_instance();

	bool changed = (val != agl->use_shaders);
	agl->pref_use_shaders = val;
	if(((AGlRootActor*)actor)->gl.gdk.drawable && !val) agl_use_program(NULL); // must do before set use_shaders
	if(!val) agl->use_shaders = false;

	if(((AGlRootActor*)actor)->gl.gdk.drawable){
		agl->use_shaders = val;

		if(changed){
			agl_actor__init((AGlActor*)actor);
		}
	}
}


static bool
_actor__on_event(AGlActor* a, GdkEvent* event, AGliPt xy)
{
	// xy is the event coordinates relative to the top left of the actor's parent.
	// ie the coordinate system is the same one as the actor's region.

	if(!a) return NOT_HANDLED;

	bool handled = NOT_HANDLED;

	AGlActor* find_handler_actor(AGlActor* a, AGliPt* xy)
	{
		// iterate up until an actor with an event handler is found.

		int i = 0;
		while(a && i++ < 256){
			if(a->on_event) return a;
			a = a->parent;
		}
		return NULL;
	}

	AGliPt scroll_offset = {0, 0}; //{actor->root->viewport->x, actor->root->viewport->y}; // FIXME this should not be needed. is it still used?

	AGlActor* h = find_handler_actor(a, &xy);
	if(h){
		if(h->on_event(h, event, xy, scroll_offset)){
			return HANDLED;
		}

		while(h->parent){
			xy.x += h->region.x1;
			xy.y += h->region.y1;
			h = h->parent;
			if(h->on_event && (handled = h->on_event(h, event, xy, scroll_offset))) break;
		}
	}

	return handled;
}


bool
agl_actor__on_event(AGlRootActor* root, GdkEvent* event)
{
	AGlActor* actor = (AGlActor*)root;
	GtkWidget* widget = actor->root->widget;

	AGliPt xy = {event->button.x + actor->root->viewport.x, event->button.y + actor->root->viewport.y};

	bool region_match(AGliRegion* r, int x, int y)
	{
		bool match = x > r->x1 && x < r->x2 && y > r->y1 && y < r->y2;
		//if(match) dbg(0, "x=%i x2=%i match=%i", x, r->x2, match);
		return match;
	}

	AGlActor* child_region_hit(AGlActor* actor, AGliPt xy)
	{
		GList* l = g_list_last(actor->children); // iterate backwards so that the 'top' actor get the events first.
		for(;l;l=l->prev){
			AGlActor* child = l->data;
			if(!child->disabled && region_match(&child->region, xy.x, xy.y)){
				AGlActor* sub = child_region_hit(child, (AGliPt){xy.x - child->region.x1, xy.y - child->region.y1});
				if(sub) return sub;
				//printf("  match. y=%i y0=%i y1=%i x=%i-->%i type=%i\n", xy.y, child->region.y1, child->region.y2, child->region.x1, child->region.x2, child->type);
				return child;
			}
		}
		return NULL;
	}

	if(actor_context.grabbed){
		AGliPt offset = (actor_context.grabbed->parent == actor) ? (AGliPt){0, 0} : agl_actor__find_offset(actor_context.grabbed->parent);
		bool handled = _actor__on_event(actor_context.grabbed, event, (AGliPt){xy.x - offset.x, xy.y - offset.y});

		if(event->type == GDK_BUTTON_RELEASE){
			actor_context.grabbed = NULL;
			gdk_window_set_cursor(widget->window, NULL);
		}

		return handled;
	}

	static AGlActor* hovered = NULL;

	AGlActor* a = child_region_hit(actor, xy);

	switch(event->type){
		case GDK_MOTION_NOTIFY:
			if(hovered){
				if(!a || (a && a != hovered)){
					//if(hovered) hovered->hover = false;
					//hovered = NULL;
					GdkEvent leave = {
						.type = GDK_LEAVE_NOTIFY,
					};
					_actor__on_event(hovered, &leave, xy);
				}
			}

			break;
		case GDK_BUTTON_RELEASE:
			return _actor__on_event(a, event, xy);
		default:
			break;
	}

	if(a && a != actor){
		if(event->type == GDK_MOTION_NOTIFY){
			if(a != hovered && !actor_context.grabbed){
				if(hovered) hovered->hover = false;
				hovered = a;
				a->hover = true;

				GdkEvent enter = {
					.type = GDK_ENTER_NOTIFY,
				};
				return _actor__on_event(a, &enter, xy);
			}
		}

		AGliPt offset = (a->parent == actor) ? (AGliPt){0, 0} : agl_actor__find_offset(a->parent);
		return _actor__on_event(a, event, (AGliPt){xy.x - offset.x, xy.y - offset.y});

	}else{
		if(event->type == GDK_MOTION_NOTIFY){
			if(hovered){
				hovered->hover = false;
				hovered = NULL;
				gdk_window_set_cursor(widget->window, NULL);
				gtk_widget_queue_draw(widget); // TODO not always needed
			}
		}
	}

	return NOT_HANDLED;
}


/*
 *  Utility function for use as a gtk "expose-event" handler.
 *  Application must pass the root actor as user_data when connecting the signal.
 */
bool
agl_actor__on_expose(GtkWidget* widget, GdkEventExpose* event, gpointer user_data)
{
	void set_projection(AGlActor* actor)
	{
		int vx = 0;
		int vy = 0;
		int vw = MAX(64, actor->region.x2);
		int vh = MAX(64, actor->region.y2);
		glViewport(vx, vy, vw, vh);
		AGL_DEBUG printf("viewport: %i %i %i %i\n", vx, vy, vw, vh);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho (actor->root->viewport.x, actor->root->viewport.x + actor->root->viewport.w, actor->root->viewport.y + actor->root->viewport.h, actor->root->viewport.y, 256.0, -256.0);
	}

	AGlRootActor* root = user_data;
	g_return_val_if_fail(root, false);

	if(!GTK_WIDGET_REALIZED(widget)) return true;

	AGL_ACTOR_START_DRAW(root) {
		set_projection((AGlActor*)root);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		agl_actor__paint((AGlActor*)root);

#undef SHOW_BOUNDING_BOX
#ifdef SHOW_BOUNDING_BOX
		glDisable(GL_TEXTURE_2D);

		int w = GL_WIDTH;
		int h = GL_HEIGHT/2;
		glBegin(GL_QUADS);
		glVertex3f(-0.2, -0.2, 1); glVertex3f(w, -0.2, 1);
		glVertex3f(w, h, 1);       glVertex3f(-0.2, h, 1);
		glEnd();
		glEnable(GL_TEXTURE_2D);
#endif
		gdk_gl_drawable_swap_buffers(root->gl.gdk.drawable);
	} AGL_ACTOR_END_DRAW(root)

	return true;
}


AGlActor*
agl_actor__find_by_z(AGlActor* actor, int z)
{
	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* a = l->data;
		if(a->z == z) return a;
	}
	return NULL;
}


bool
agl_actor__null_painter(AGlActor* actor)
{
	return true;
}


void
agl_actor__grab(AGlActor* actor)
{
	actor_context.grabbed = actor;
}


void
agl_actor__invalidate(AGlActor* actor)
{
	void _agl_actor__invalidate(AGlActor* actor)
	{
#ifdef AGL_ACTOR_RENDER_CACHE
		actor->cache.valid = false;
#endif
		call(actor->invalidate, actor);

		GList* l = actor->children;
		for(;l;l=l->next){
			AGlActor* a = l->data;
			_agl_actor__invalidate(a);
		}

		while(actor){
#ifdef AGL_ACTOR_RENDER_CACHE
			actor->cache.valid = false;
#endif
			// should probably also call actor->invalidate here
			actor = actor->parent;
		}
	}
	_agl_actor__invalidate(actor);
	if(actor->root && actor->root->type == CONTEXT_TYPE_GTK) gtk_widget_queue_draw(actor->root->widget);
}


/*
 *  Enable / disable caching of all actors that the given actor participates in.
 *  Note that the caching of child actors and actors in other parts of the tree are not affected.
 */
void
agl_actor__enable_cache(AGlActor* actor, bool enable)
{
#ifdef AGL_ACTOR_RENDER_CACHE
	while(actor){
		actor->cache.enabled = enable;
		if(!enable) actor->cache.valid = false;
		actor = actor->parent;
	}
#endif
}


/*
 *   Set initial values of animatables and start the transition.
 *
 *   The 'val' of each animatable is assumed to be already set. 'start_val' will be set here.
 *
 *   If the actor has any other transitions using the same animatable, these animatables
 *   are removed from that transition.
 *
 *   @param animatables - ownership of this list is transferred to the WfAnimation.
 */
void
agl_actor__start_transition(AGlActor* actor, GList* animatables, AnimationFn done, gpointer user_data)
{
	// TODO handle the case here where animating is disabled.

	g_return_if_fail(actor);

	// set initial value
	GList* l = animatables;
	for(;l;l=l->next){
		WfAnimatable* animatable = l->data;
		animatable->start_val.b = animatable->val.b;
	}

	typedef struct {
		AGlActor*      actor;
		AnimationFn    done;
		gpointer       user_data;
	} C;
	C* c = g_new0(C, 1);
	*c = (C){
		.actor = actor,
		.done = done,
		.user_data = user_data
	};

	void agl_actor_on_frame(WfAnimation* animation, int time)
	{
		C* c = animation->user_data;

		agl_actor__invalidate(c->actor);
	}

	void
	actor_on_transition_finished(WfAnimation* animation, gpointer _actor)
	{
		g_return_if_fail(animation);
		g_return_if_fail(_actor);
		AGlActor* a = _actor;

#ifdef DEBUG
		int l = g_list_length(a->transitions);
#endif
		a->transitions = g_list_remove(a->transitions, animation);
#ifdef DEBUG
		if(g_list_length(a->transitions) != l - 1) gwarn("animation not removed. len=%i-->%i", l, g_list_length(a->transitions));
#endif
#ifdef USE_FRAME_CLOCK
		if(a->root) a->root->is_animating = agl_actor__is_animating((AGlActor*)a->root);
#endif

		agl_actor__enable_cache(a, true);
	}

	void _on_animation_finished(WfAnimation* animation, gpointer user_data)
	{
		g_return_if_fail(user_data);
		g_return_if_fail(animation);
		C* c = user_data;

		if(c->done) c->done(animation, c->user_data);

		actor_on_transition_finished(animation, c->actor);
		g_free(c);
	}

	l = actor->transitions;
	for(;l;l=l->next){
		// remove animatables we are replacing. let others finish.
		GList* k = animatables;
		for(;k;k=k->next){
			if(wf_animation_remove_animatable((WfAnimation*)l->data, (WfAnimatable*)k->data)) break;
		}
	}

	if(animatables){
		WfAnimation* animation = wf_animation_new(_on_animation_finished, c);
		animation->on_frame = agl_actor_on_frame;
		actor->transitions = g_list_append(actor->transitions, animation);
#ifdef USE_FRAME_CLOCK
		if(actor->root) actor->root->is_animating = true;
#endif
		wf_transition_add_member(animation, animatables);
		wf_animation_start(animation);
		agl_actor__enable_cache(actor, false);
	}
}


AGliPt
agl_actor__find_offset(AGlActor* a)
{
	AGliPt of = {0, 0};
	do {
		of.x += a->region.x1;
		of.y += a->region.y1;
	} while((a = a->parent));
	return of;
}


bool
agl_actor__is_disabled(AGlActor* a)
{
	do{
		if(a->disabled) return true;
	} while((a = a->parent));

	return false;
}


#ifdef USE_FRAME_CLOCK
static bool
agl_actor__is_animating(AGlActor* a)
{
	if(a->transitions) return true;

	GList* l = a->children;
	for(;l;l=l->next){
		if(agl_actor__is_animating((AGlActor*)l->data)) return true;
	}
	return false;
}
#endif


#ifdef DEBUG
void
agl_actor__print_tree (AGlActor* actor)
{
	g_return_if_fail(actor);

	printf("scene graph:\n");
	static int indent; indent = 1;

	void _print(AGlActor* actor)
	{
		g_return_if_fail(actor);
		int i; for(i=0;i<indent;i++) printf("  ");
#ifdef AGL_DEBUG_ACTOR
#ifdef AGL_ACTOR_RENDER_CACHE
		if(actor->name) printf("%s (%i,%i)\n", actor->name, actor->cache.enabled, actor->cache.valid);
#else
		if(actor->name) printf("%s\n", actor->name);
#endif
		if(!actor->name) printf("(%i,%i)\n", actor->region.x1, actor->region.y1);
#else
		printf("%i\n", actor->region.x1);
#endif
		indent++;
		GList* l = actor->children;
		for(;l;l=l->next){
			AGlActor* child = l->data;
			_print(child);
		}
		indent--;
	}

	_print(actor);
}
#endif
