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
#define __gl_actor_c__
#define __gl_canvas_priv__
#include "global.h"
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <gtk/gtk.h>
#include "agl/utils.h"
#include "agl/ext.h"
#include "waveform/utils.h"
#include "agl/actor.h"

#define HANDLED TRUE
#define NOT_HANDLED FALSE
#define CURSOR_NORMAL 0

static AGl* agl = NULL;

static bool   actor__is_onscreen (AGlActor*);
static AGliPt actor__find_offset (AGlActor*);
static bool  _actor__on_event    (AGlActor*, void* widget, GdkEvent*, AGliPt);


AGlActor*
actor__new()
{
	agl = agl_get_instance();

	AGlActor* a = g_new0(AGlActor, 1);
	a->paint = actor__null_painter;
	return a;
}


AGlActor*
actor__new_root(GtkWidget* widget)
{
	agl = agl_get_instance();

	RootActor* a = g_new0(RootActor, 1);
#ifdef DEBUG_ACTOR
	((AGlActor*)a)->name = "ROOT";
#endif
	a->widget = widget;
	((AGlActor*)a)->root = a;
	((AGlActor*)a)->paint = actor__null_painter;

	a->viewport.w = widget->allocation.width;
	a->viewport.h = widget->allocation.height;

	return (AGlActor*)a;
}


void
actor__free(AGlActor* actor)
{
	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* child = l->data;
		actor__free(child);
	}
	g_list_free0(actor->children);

	if(actor->free)
		actor->free(actor);
	else
		g_free(actor);
}


AGlActor*
actor__add_child(AGlActor* actor, AGlActor* child)
{
	g_return_val_if_fail(actor && child, NULL);
	actor->children = g_list_append(actor->children, child);
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
	}

	return child;
}


void
actor__remove_child(AGlActor* actor, AGlActor* child)
{
	g_return_if_fail(actor && child);
	g_return_if_fail(g_list_find(actor->children, child));
	call(child->free, child);
	actor->children = g_list_remove(actor->children, child);
}


AGlActor*
actor__replace_child(AGlActor* actor, AGlActor* child, ActorNew constructor)
{
	GList* l = g_list_find(actor->children, child);
	if(l){
		actor__free(child);

		//Actor* new_child = constructor((AyyiPanel*)actor->root->widget);
		AGlActor* new_child = actor__add_child(actor, constructor(actor->root->widget));

		// update the children list such that the original order is preserved
		actor->children = g_list_remove(actor->children, new_child);
		GList* j = g_list_find(actor->children, child);
		j->data = new_child;

		return new_child;
	}
	return NULL;
}


void
actor__init(AGlActor* a, gpointer user_data)
{
	call(a->init, a, user_data);

	GList* l = a->children;
	for(;l;l=l->next) actor__init((AGlActor*)l->data, user_data);
}


static bool
actor__is_onscreen(AGlActor* a)
{
	int h = a->root->widget->allocation.height;
	AGlRect* viewport = &a->root->viewport;
	AGliPt offset = actor__find_offset(a);
	return !(offset.y + a->region.y2  < viewport->y || offset.y > viewport->y + h);
}


void
actor__paint(AGlActor* a)
{
	if(!a->root) return;

	if(!actor__is_onscreen(a)) return;

	// FIXME is calling root widget multiple times?
	call(a->set_state, a->root->widget); //TODO find optimum place

	glPushMatrix();
	glTranslatef(a->region.x1, a->region.y1, 0.0);

	a->paint(a);

	GList* l = a->children;
	for(;l;l=l->next){
		actor__paint((AGlActor*)l->data);
	}

	glPopMatrix();

#undef SHOW_ACTOR_BORDERS
#ifdef SHOW_ACTOR_BORDERS
	static int depth; depth = 0;
	static uint32_t colours[10] = {0xff000099, 0xff990099, 0x00ff0099, 0x0000ff99, 0xff000099, 0xffff0099, 0x00ffff99};

	void paint_border(AGlActor* a)
	{
		depth++;
		glPushMatrix();
		glTranslatef(a->region.x1, a->region.y1, 0.0);
		agl->shaders.plain->uniform.colour = colours[depth];
		agl_use_program((AGlShader*)agl->shaders.plain);

		#define W 1
		iRegion r = a->region;
		float width = r.x2 - r.x1;
		float height = r.y2 - r.y1;
		agl_rect(0.0,       0.0,        width, W     );
		agl_rect(0.0,       0.0,        W,     height);
		agl_rect(0.0,       height - W, width, W     );
		agl_rect(width - W, 0,          W,     height);

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
actor__set_size(AGlActor* actor)
{
	//pass each actor the size of its parent --- but actors can access this themselves if they want. no need to pass it. *** remove args 3 and 4
	call(actor->set_size, actor, -1, -1);

	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* a = l->data;
		actor__set_size(a);
	}
}


static bool
_actor__on_event(AGlActor* a, void* widget, GdkEvent* event, AGliPt xy)
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
actor__on_event(RootActor* root, GdkEvent* event)
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
			if(region_match(&child->region, xy.x, xy.y)){
				AGlActor* sub = child_region_hit(child, (AGliPt){xy.x - child->region.x1, xy.y - child->region.y1});
				if(sub) return sub;
				//dbg(0, "  match. y=%i y0=%i y1=%i x=%i-->%i type=%i", y, child->region.y1, child->region.y2, child->region.x1, child->region.x2, child->type);
				return child;
			}
		}
		return NULL;
	}

	if(actor_context.grabbed){
		AGliPt offset = (actor_context.grabbed->parent == actor) ? (AGliPt){0, 0} : actor__find_offset(actor_context.grabbed->parent);
		bool handled = _actor__on_event(actor_context.grabbed, widget, event, (AGliPt){xy.x - offset.x, xy.y - offset.y});

		if(event->type == GDK_BUTTON_RELEASE){
			actor_context.grabbed = NULL;
			gdk_window_set_cursor(widget->window, NULL);
		}

		return handled;
	}

	static AGlActor* hovered = NULL;

	AGlActor* a = child_region_hit(actor, xy);

	if(a && a != actor){
		if(event->type == GDK_MOTION_NOTIFY){
			if(a != hovered && !actor_context.grabbed){
				if(hovered) hovered->hover = false;
				hovered = a;
				a->hover = true;
				gdk_window_set_cursor(widget->window, NULL);
				gtk_widget_queue_draw(widget); // TODO not always needed
			}
		}

		AGliPt offset = (a->parent == actor) ? (AGliPt){0, 0} : actor__find_offset(a->parent);
		return _actor__on_event(a, widget, event, (AGliPt){xy.x - offset.x, xy.y - offset.y});

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


void
actor__null_painter(AGlActor* actor)
{
}


void
actor__grab(AGlActor* actor)
{
	actor_context.grabbed = actor;
}


void
actor__invalidate(AGlActor* actor)
{
	call(actor->invalidate, actor);

	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* a = l->data;
		actor__invalidate(a);
	}
}


static AGliPt
actor__find_offset(AGlActor* a)
{
	AGliPt of = {0, 0};
	do {
		of.x += a->region.x1;
		of.y += a->region.y1;
	} while((a = a->parent));
	return of;
}


