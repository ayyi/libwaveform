/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __agl_actor_c__
#define __gl_canvas_priv__
#define __wf_private__
#include "config.h"
#ifdef USE_GTK
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#else
#include <gdk/gdk.h>
#endif
#include "agl/ext.h"
#include "agl/utils.h"
#include "agl/debug.h"
#include "wf/utils.h"
#include "agl/shader.h"
#include "agl/transform.h"
#include "text/renderops.h"
#include "text/renderer.h"
#include "text/roundedrect.h"
#include "agl/actor.h"
#ifdef AGL_ACTOR_RENDER_CACHE
#include "agl/fbo.h"
#endif

#define CURSOR_NORMAL 0

#define call(FN, A, ...) if(FN) (FN)(A, ##__VA_ARGS__)

static AGl* agl = NULL;
static AGlActorClass root_actor_class = {0, "ROOT"};

#define SCENE_IS_GTK(A) ((A)->root->type == CONTEXT_TYPE_GTK)

#define IS_DRAWABLE(A) (!(!agl_actor__is_onscreen(A) || ((agl_actor__width(A) < 1 || agl_actor__height(A) < 1))))

bool         _agl_actor__paint        (AGlActor*);
bool          agl_actor__is_onscreen  (AGlActor*);

static bool  _agl_actor__on_event     (AGlActor*, GdkEvent*, AGliPt);
static void   agl_actor__init         (AGlActor*);        // called once when gl context is available. and again if gl context changes, eg after re-realize.
#ifdef USE_FRAME_CLOCK
static bool   agl_actor__is_animating (AGlActor*);
#endif
#ifdef AGL_ACTOR_RENDER_CACHE
bool          agl_actor__is_cached    (AGlActor*);
#endif
static AGliPt _agl_actor__find_offset (AGlActor*);


#ifdef USE_GTK
static void
agl_actor__have_drawable (AGlRootActor* a, GdkGLDrawable* drawable)
{
	g_return_if_fail(!a->gl.gdk.drawable);
	g_return_if_fail(!a->gl.gdk.context);

	static bool first_time = true;

	((AGlActor*)a)->scrollable.x2 = a->gl.gdk.widget->allocation.width;
	((AGlActor*)a)->scrollable.y2 = a->gl.gdk.widget->allocation.height;

	a->gl.gdk.drawable = drawable;
	a->gl.gdk.context = agl_get_gl_context();

#ifdef USE_SYSTEM_GTKGLEXT
	gdk_gl_drawable_make_current (a->gl.gdk.drawable, a->gl.gdk.context);
#else
	g_assert(G_OBJECT_TYPE(a->gl.gdk.drawable) == GDK_TYPE_GL_WINDOW);
	gdk_gl_window_make_context_current (a->gl.gdk.drawable, a->gl.gdk.context);
#endif

	if(first_time){
		agl_gl_init();

#ifdef DEBUG
		if(agl->debug){
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
		}
#endif
	}

	agl_actor__init((AGlActor*)a);

	first_time = false;
}
#endif


#ifdef USE_GTK
static gboolean
agl_actor__try_drawable(gpointer _actor)
{
	AGlRootActor* a = _actor;
	if(!a->gl.gdk.drawable){
		GdkGLDrawable* drawable = gtk_widget_get_gl_drawable(a->gl.gdk.widget);
		if(drawable){
			agl_actor__have_drawable(a, drawable);
		}
	}
	return G_SOURCE_REMOVE;
}
#endif


#ifdef USE_GTK
static void
agl_actor__on_unrealise(GtkWidget* widget, gpointer _actor)
{
	AGlRootActor* a = _actor;

	a->gl.gdk.drawable = NULL;
	a->gl.gdk.context  = NULL;
}
#endif


#ifdef USE_GTK
static void
agl_actor__on_realise(GtkWidget* widget, gpointer _actor)
{
	AGlRootActor* a = _actor;

	agl_actor__try_drawable(a);
	if(!a->gl.gdk.drawable){
		g_idle_add(agl_actor__try_drawable, a);
	}
}
#endif


AGlActorClass*
agl_scene_get_class ()
{
	return &root_actor_class;
}


#ifdef USE_GTK
AGlActor*
agl_actor__new_root(GtkWidget* widget)
{
	AGlRootActor* a = (AGlRootActor*)agl_actor__new_root_(CONTEXT_TYPE_GTK);
	a->gl.gdk.widget = widget;

	if(GTK_WIDGET_REALIZED(widget)){
		agl_actor__on_realise(widget, a);
	}
	g_signal_connect((gpointer)widget, "realize", G_CALLBACK(agl_actor__on_realise), a);
	g_signal_connect((gpointer)widget, "unrealize", G_CALLBACK(agl_actor__on_unrealise), a);

	return (AGlActor*)a;
}
#endif


AGlActor*
agl_actor__new_root_(ContextType type)
{
	agl = agl_get_instance();

	AGlRootActor* a = agl_actor__new(AGlRootActor,
		.actor = {
			.class = &root_actor_class,
			.name = "ROOT",
			.root = a,
			.paint = agl_actor__null_painter
		},
		.type = type,
		.bg_colour = 0x000000ff,
		.enable_animations = true,
	);

#ifdef USE_GTK
		#define READY_FOR_INIT(A) (SCENE_IS_GTK(A) ? ((A)->root->gl.gdk.widget && GTK_WIDGET_REALIZED((A)->root->gl.gdk.widget)) : true)
#else
		#define READY_FOR_INIT(A) true
#endif
	if(READY_FOR_INIT((AGlActor*)a)) agl_actor__init((AGlActor*)a);

	return (AGlActor*)a;
}


void
agl_actor__free (AGlActor* actor)
{
	g_return_if_fail(actor);

	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* child = l->data;
		agl_actor__free(child);
	}
	g_list_free0(actor->children);

#ifdef AGL_ACTOR_RENDER_CACHE
	if(actor->fbo) agl_fbo_free0(actor->fbo);
#endif

	while(actor->transitions)
		wf_animation_remove(actor->transitions->data);

	int i; for(i=0;i<AGL_ACTOR_N_BEHAVIOURS;i++){
		AGlBehaviour* behaviour = actor->behaviours[i];
		if(!behaviour)
			break;
		if(behaviour->klass->free)
			behaviour->klass->free(behaviour);
		else
			g_free(behaviour);
		actor->behaviours[i] = NULL;
	}

	if(actor->class && actor->class->free)
		actor->class->free(actor);
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

		agl_actor__invalidate(actor);

		if(READY_FOR_INIT(child)) agl_actor__init(child);
	}

	return child;
}


AGlActor*
agl_actor__insert_child (AGlActor* actor, AGlActor* child, int position)
{
	agl_actor__add_child(actor, child);
	actor->children = g_list_remove(actor->children, child);
	actor->children = g_list_insert_before(actor->children, g_list_nth(actor->children, position), child);
	return child;
}


void
agl_actor__remove_child (AGlActor* actor, AGlActor* child)
{
	g_return_if_fail(actor && child);
	g_return_if_fail(g_list_find(actor->children, child));

	if(actor->root->selected == child) actor->root->selected = NULL;
	if(actor->root->hovered == child) actor->root->hovered = NULL;

	actor->children = g_list_remove(actor->children, child);

	while(child->children){
		agl_actor__remove_child(child, child->children->data);
	}

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
_agl_actor__init (AGlActor* actor)
{
	if(agl->use_shaders && actor->program && !actor->program->program) agl_create_program(actor->program);

	call(actor->init, actor);

	for(int i = 0; i < AGL_ACTOR_N_BEHAVIOURS; i++){
		AGlBehaviour* behaviour = actor->behaviours[i];
		if(!behaviour)
			break;

		if(behaviour->klass->init) agl_behaviour_init(behaviour, actor);
	}

	for(GList* l=actor->children;l;l=l->next)
		_agl_actor__init((AGlActor*)l->data);
}


/*
 *  Note that agl_actor__init may be called more than once, eg on settings change.
 */
static void
agl_actor__init (AGlActor* actor)
{
	// agl_create_programs can sometimes end up being called multiple
	// times to allow for actors to add global shaders
	extern void agl_create_programs ();
	agl_create_programs ();

	_agl_actor__init (actor);
}


bool
agl_actor__is_onscreen (AGlActor* a)
{
	int h = ((AGlActor*)a->root)->scrollable.y2 - ((AGlActor*)a->root)->scrollable.y1;
	int w = ((AGlActor*)a->root)->scrollable.x2 - ((AGlActor*)a->root)->scrollable.x1;
	if(h && w){
		AGliRegion* scrollable = &a->scrollable;
		AGliPt offset = agl_actor__find_offset(a);
		if(!scrollable->y2){
			// scrollable size is NOT set
			return !(
				offset.x + agl_actor__width(a)  < 0 || // actor right is before window left
				offset.x                        > w || // actor left is before window right
				offset.y + agl_actor__height(a) < 0 || // actor botton is before window top
				offset.y                        > h    // actor top is after window bottom
			);
		}else{
			// scrollable is set
			// note: if scrollable.y1 is negative, contents are scrolled upwards
			AGliSize size = {
				a->scrollable.x2 - a->scrollable.x1,
				a->scrollable.y2 - a->scrollable.y1
			};
			if(
				offset.x            < w && // actor left is before window right
				offset.x + size.w   > 0 && // actor right is after after window left
				offset.y            < h && // actor top is before window bottom
				offset.y + size.h   > 0    // actor bottom is after window top
			){
				return true;
			}else{
				return false;
			}
		}
	}
	return true;
}


#ifdef AGL_ACTOR_RENDER_CACHE
/*
 * Render the FBO to screen
 */
void
agl_actor__render_from_fbo (AGlActor* a)
{
	AGlFBO* fbo = a->fbo;
	g_return_if_fail(fbo);

	g_return_if_fail(a->cache.valid);

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

	float w = agl_actor__width(a);
	float h = MIN(fbo->height, agl_actor__height(a));
	float h2 = fbo->height - h;
	AGlfPt tsize = {agl_power_of_two(w), agl_power_of_two(fbo->height)};

	float start = ((float)-a->cache.offset.x);
	float top = -a->cache.offset.y;

	// The FBO is upside down so y has to be reversed. TODO render the FBO so that it is not upside down
	agl_textured_rect_(fbo->texture, a->cache.position.x, top * 2, w, h, &(AGlQuad){
		start / tsize.x,
		(-top + h + h2) / tsize.y,
		(start + w) / tsize.x,
		(-top + h2) / tsize.y
	});

#undef FBO_MARKER // show red dots in corner of fbos for debugging
#ifdef FBO_MARKER
	agl->shaders.plain->uniform.colour = 0xff0000ff;
	agl_use_program((AGlShader*)agl->shaders.plain);
	#define INSET 2
	glRectf (INSET,         INSET,         INSET + 6, INSET + 6);
	glRectf (w - INSET - 6, INSET,         w - INSET, INSET + 6);
	glRectf (INSET,         h - INSET - 6, 8,         h - INSET);
	glRectf (w - INSET - 6, h - INSET - 6, w - INSET, h - INSET);
	#undef INSET
#endif

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl_warn("");
}
#endif


static bool
__draw (AGlActor* a, bool use_fbo)
{
	bool good = true;
#ifdef AGL_ACTOR_RENDER_CACHE
	// TODO check case where actor is translated and is partially offscreen.
	if(use_fbo){
		if(!a->cache.valid){
			g_return_val_if_fail(a->fbo->width > 0.5 && a->fbo->height > 0.5, false);

			agl_draw_to_fbo(a->fbo) {
				glTranslatef(- a->cache.position.x, 0.0, 0.0);
				glClearColor(0.0, 0.0, 0.0, 0.0); // background colour must be same as foreground for correct antialiasing
				glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				call(a->set_state, a);
				if(agl->use_shaders && a->program) agl_use_program(a->program);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

				renderer_push_builder();

				ops_set_viewport (builder(), &(graphene_rect_t){
					.size.width = a->fbo->width,
					.size.height = a->fbo->height
				});

				ops_push_clip (builder(), &AGL_ROUNDED_RECT_INIT (0, 0, ((float)a->fbo->width) * 1.0, ((float)a->fbo->height) * 1.0));

				good &= a->paint(a);

				GList* l = a->children;
				for(;l;l=l->next){
					AGlActor* a = l->data;
					if(IS_DRAWABLE(a)){
						good &= _agl_actor__paint(a);
					}
				}
				glTranslatef(a->cache.position.x, 0.0, 0.0);

				ops_pop_clip (builder());

				builder()->target = a->fbo->id;
				renderer_render (builder());

				renderer_pop_builder();
			} agl_end_draw_to_fbo;

			a->cache.valid = good;
		}
		if(a->cache.valid)
			agl_actor__render_from_fbo(a);

#undef SHOW_FBO_BORDERS
#ifdef SHOW_FBO_BORDERS
		agl->shaders.plain->uniform.colour = 0x3333ffaa;
		agl_use_program((AGlShader*)agl->shaders.plain);
		AGlRect r = {
			.w = a->fbo->width,
			.h = agl_actor__height(a)
		};
		agl_box(1, r.x, r.y, r.w, r.h);
#endif
	}else{
#else
	if(true){
#endif
		call(a->set_state, a);
		if(agl->use_shaders && a->program) agl_use_program(a->program);

		good &= a->paint(a);
	}

#ifdef AGL_ACTOR_RENDER_CACHE
	if(!a->cache.valid){
#endif
		GList* l = a->children;
		for(;l;l=l->next){
			if(IS_DRAWABLE((AGlActor*)l->data))
				good &= _agl_actor__paint((AGlActor*)l->data);
		}
#ifdef AGL_ACTOR_RENDER_CACHE
	}
#endif
	return good;
}


bool
_agl_actor__paint (AGlActor* a)
{
#ifdef AGL_ACTOR_RENDER_CACHE
	bool use_fbo = a->fbo && a->cache.enabled && !(agl_actor__width(a) > AGL_MAX_FBO_WIDTH);
#else
	#define use_fbo false;
#endif

	AGliPt offset = {
		.x = a->region.x1,
		.y = a->region.y1,
	};

#ifdef AGL_ACTOR_RENDER_CACHE
	if(!use_fbo){
#else
	if(true){
#endif
		// Offset so that actors can always draw objects at the same position,
		// irrespective of scroll position
		offset.x += a->scrollable.x1;
		offset.y += a->scrollable.y1;
	}
	if(offset.x || offset.y){
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(offset.x, offset.y, 0.0);

		builder()->offset.x += offset.x;
		builder()->offset.y += offset.y;
	}

	static int j; j = -1;
	static bool f; f = use_fbo;

	bool decorator_next (AGlActor* a)
	{
		AGlBehaviour* behaviour = a->behaviours[++j];
		if(behaviour){
			if(behaviour->klass->draw){
				return behaviour->klass->draw(behaviour, a, decorator_next);
			}else{
				return decorator_next(a);
			}
		}
		return __draw(a, f);
	}

#ifdef AGL_ACTOR_RENDER_CACHE
	bool good = use_fbo && a->cache.valid
#else
	bool good = false
#endif
		? __draw(a, f)
		: decorator_next(a);

	if(offset.x || offset.y){
		glPopMatrix();

		builder()->offset.x -= offset.x;
		builder()->offset.y -= offset.y;
	}

#ifdef DEBUG
	AGL_DEBUG if(wf_debug > 2 && a == (AGlActor*)a->root) agl_actor__print_tree (a);
#endif

#undef SHOW_ACTOR_BORDERS
#ifdef SHOW_ACTOR_BORDERS
	static int depth; depth = -1;
	#define MAX_COLOURS 10
	static uint32_t colours[MAX_COLOURS] = {0xff000088, 0xff990088, 0x00ff0088, 0x3333ff88, 0xff000088, 0xffff0088, 0x00ffff88};

	int n_no_x_offset_parents (AGlActor* a)
	{
		int n = 0;
		AGlActor* _a = a;
		while(_a && !_a->region.x1) n++, _a = _a->parent;
		return n;
	}

	int n_no_y_offset_parents (AGlActor* a)
	{
		int n = 0;
		AGlActor* _a = a;
		while(_a && !_a->region.y1) n++, _a = _a->parent;
		return n;
	}

	int n_no_x_offset_parents2 (AGlActor* a)
	{
		int n = 0;
		AGlActor* _a = a;
		while(_a && _a->parent && _a->region.x2 == agl_actor__width(_a->parent))
			n++,
			_a = _a->parent;
		return n;
	}

	int n_no_y_offset_parents2 (AGlActor* a)
	{
		int n = 0;
		AGlActor* _a = a;
		while(_a && _a->parent && _a->region.y2 == agl_actor__height(_a->parent))
			n++,
			_a = _a->parent;
		return n;
	}

	void paint_border(AGlActor* a)
	{
		depth++;
		glPushMatrix();
		glTranslatef(a->region.x1, a->region.y1, 0.0);
		agl->shaders.plain->uniform.colour = colours[MIN(depth, MAX_COLOURS - 1)];
		agl_use_program((AGlShader*)agl->shaders.plain);

		float x = 0.5 * n_no_x_offset_parents(a);
		float y1 = 0.5 * n_no_y_offset_parents(a);
		agl_box(1, x, y1, agl_actor__width(a) - x - 0.5 * n_no_x_offset_parents2(a), MAX(0, agl_actor__height(a) - y1) - 0.5 * n_no_y_offset_parents2(a));

		GList* l = a->children;
		for(;l;l=l->next){
			paint_border((AGlActor*)l->data);
		}

		glPopMatrix();
		depth--;
	}

	if(!a->parent){
		agl->shaders.plain->uniform.colour = 0xff0000ff;
		agl_use_program((AGlShader*)agl->shaders.plain);
		paint_border(a);
	}
#endif

	return good;
}


bool
agl_actor__paint (AGlActor* a)
{
	if(!a->root) return false;

	if(!agl_actor__is_onscreen(a) || ((agl_actor__width(a) < 1 || agl_actor__height(a) < 1) && a->paint != agl_actor__null_painter)) return false;

	if(a->root == (AGlScene*)a){
		ops_push_clip (builder(), &AGL_ROUNDED_RECT_INIT (0, 0, a->region.x2, a->region.y2));
		ops_set_program (builder(), &renderer.coloring_program);

		AGlTransform* transform = agl_transform_new();
		ops_set_modelview (builder(), transform);

		if(!a->region.x2 || !a->region.y2){
			return false;
		}

		ops_set_viewport (builder(), &(graphene_rect_t){.size.width = a->region.x2, .size.height = a->region.y2});
	}

	bool good =  _agl_actor__paint(a);

	if(a->root == (AGlScene*)a && builder()->vertices->len){
		renderer_render (builder());

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // restore normal blend function
	}

	return good;
}


void
agl_actor__set_size (AGlActor* actor)
{
	for(int i = 0; i < AGL_ACTOR_N_BEHAVIOURS; i++){
		AGlBehaviour* behaviour = actor->behaviours[i];
		if(!behaviour)
			break;
		if(behaviour->klass->layout) behaviour->klass->layout(behaviour, actor);
	}

	call(actor->set_size, actor); // actors need to update their own regions.

#ifdef AGL_ACTOR_RENDER_CACHE
	if(actor->fbo){
		AGliPt size = actor->cache.size_request.x ? actor->cache.size_request : (AGliPt){agl_actor__width(actor), agl_actor__height(actor)};
		if(size.x != actor->fbo->width || size.y != actor->fbo->height){
#if 0
			// Although resizing of fbos should work, it is not reliable
			// and people often advice against it.
			agl_fbo_set_size (actor->fbo, size.x, size.y);
#else
			if(agl_power_of_two(size.x) != agl_power_of_two(actor->fbo->width) || agl_power_of_two(size.y) != agl_power_of_two(actor->fbo->height)){
				AGlFBOFlags flags = actor->fbo->flags;
				agl_fbo_free(actor->fbo);
				actor->fbo = agl_fbo_new(size.x, size.y, 0, flags);
			}else{
				agl_fbo_set_size (actor->fbo, size.x, size.y);
			}
#endif
			actor->cache.valid = false;
		}
	}
#endif

	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* a = l->data;
		agl_actor__set_size(a);
	}
}


void
agl_actor__scroll_to (AGlActor* actor, AGliPt pt)
{
	if(pt.x > -1){
		int width_inner = agl_actor__width(actor);
		int width_outer = actor->scrollable.x2 - actor->scrollable.x1;

		if(width_outer > width_inner){
			pt.x = CLAMP(pt.x, 0, width_outer - width_inner);
			actor->scrollable.x1 = -pt.x;
			actor->scrollable.x2 = -pt.x + width_outer;
		}
	}

	if(pt.y > -1){
		int height_inner = agl_actor__height(actor);
		int height_outer = actor->scrollable.y2 - actor->scrollable.y1;

		if(height_outer > height_inner){
			pt.y = CLAMP(pt.y, 0, height_outer - height_inner);
			actor->scrollable.y1 = -pt.y;
			actor->scrollable.y2 = -pt.y + height_outer;
		}
	}
}


void
agl_actor__set_use_shaders (AGlRootActor* actor, gboolean val)
{
	AGl* agl = agl_get_instance();

#ifdef USE_GTK
	bool changed = (val != agl->use_shaders);
#endif
	agl->pref_use_shaders = val;
#ifdef USE_GTK
	if(((AGlRootActor*)actor)->gl.gdk.drawable && !val) agl_use_program(NULL); // must do before set use_shaders
#endif
	if(!val) agl->use_shaders = false;

#ifdef USE_GTK
	if(((AGlRootActor*)actor)->gl.gdk.drawable){
		agl->use_shaders = val;

		if(changed){
			agl_actor__init((AGlActor*)actor);
		}
	}
#endif
}


static bool
_agl_actor__on_event(AGlActor* a, GdkEvent* event, AGliPt xy)
{
	// xy is the event coordinates relative to the top left of the actor's parent.
	// ie the coordinate system is the same one as the actor's region.

	if(!a) return AGL_NOT_HANDLED;

	bool handled = AGL_NOT_HANDLED;

	AGlActor* find_handler_actor(AGlActor* a, AGliPt* xy)
	{
		// iterate up until an actor with an event handler is found.

		int i = 0;
		while(a && i++ < 256){
			if(a->on_event) return a;

			behaviour_foreach(a)
				if(behaviour->klass->event) return a;
			}

			xy->x += a->region.x1;
			xy->y += a->region.y1;
			a = a->parent;
		}
		return NULL;
	}

	AGlActor* h = find_handler_actor(a, &xy);
	if(h){
		do {
			behaviour_foreach(h)
				if(behaviour->klass->event){
					if(agl_behaviour_event(behaviour, h, event)){
						handled = true;
						break;
					}
				}
			}
			if(handled) break;

			if(h->on_event && (handled = h->on_event(h, event, xy))) break;

			xy.x += h->region.x1;
			xy.y += h->region.y1;
		} while((h = h->parent));
	}

	return handled;
}


static bool
region_match (AGlfRegion* r, float x, float y)
{
	bool match = x > r->x1 && x < r->x2 && y > r->y1 && y < r->y2;
	//printf("     x=%.0f x2=%.0f  y=%.0f %.0f-->%.0f match=%i\n", x, r->x2, y, r->y1, r->y2, match);
	return match;
}


/*
 *  Find the child of the actor at the given coordinate.
 *  The coordinate is relative to the actor, ie all position offsets have been applied.
 */
static AGlActor*
child_region_hit (AGlActor* actor, AGliPt xy)
{
	GList* l = g_list_last(actor->children); // iterate backwards so that the 'top' actor get the events first.
	for(;l;l=l->prev){
		AGlActor* child = l->data;
		if(!child->disabled && region_match(&child->region, xy.x, xy.y)){
			return child_region_hit(child, (AGliPt){
				xy.x - child->region.x1 - child->scrollable.x1,
				xy.y - child->region.y1 - child->scrollable.y1
			});
		}
	}
	return actor;
}


/*
 * agl_actor__on_event can be used by clients that handle their own events to forward them on to the AGlScene.
 */
bool
agl_actor__on_event (AGlScene* root, GdkEvent* event)
{
	AGlActor* actor = (AGlActor*)root;
#ifdef USE_GTK
	GtkWidget* widget = actor->root->gl.gdk.widget;
#endif

	switch(event->type){
		case GDK_KEY_PRESS:
		case GDK_KEY_RELEASE:
		case GDK_FOCUS_CHANGE:
			AGL_DEBUG printf("%s: keypress: key=%i\n", __func__, ((GdkEventKey*)event)->keyval);
			if(root->selected){
				return _agl_actor__on_event(root->selected, event, (AGliPt){});
			}
			return AGL_NOT_HANDLED;
		case GDK_EXPOSE:
		case GDK_VISIBILITY_NOTIFY:
			return AGL_NOT_HANDLED;
		default:
			;
	}

	AGliPt xy = {event->button.x + actor->scrollable.x1, event->button.y + actor->scrollable.y1};

	if(event->type == GDK_LEAVE_NOTIFY){
		if(root->hovered){
			GdkEventCrossing leave = {
				.type = GDK_LEAVE_NOTIFY,
				.detail = GDK_NOTIFY_ANCESTOR
			};
			_agl_actor__on_event(root->hovered, (GdkEvent*)&leave, xy);
			root->hovered = NULL;
		}
		return AGL_HANDLED;
	}

	if(actor_context.grabbed){
		AGliPt offset = (false && actor_context.grabbed->parent == actor) // test if grabbed is direct descendent of root
			? (AGliPt){0, 0} // why? if this needs to be restored, pls document why (was removed for part-resize-right).
			: _agl_actor__find_offset(actor_context.grabbed);
		bool handled = _agl_actor__on_event(actor_context.grabbed, event, (AGliPt){xy.x - offset.x, xy.y - offset.y});

		if(event->type == GDK_BUTTON_RELEASE){
			actor_context.grabbed = NULL;
#ifdef USE_GTK
			if(SCENE_IS_GTK(actor)){
				gdk_window_set_cursor(widget->window, NULL);
			}
#endif
		}

		return handled;
	}

	AGlActor* hovered = root->hovered;

	AGlActor* a = child_region_hit(actor, xy);

	switch(event->type){
		case GDK_MOTION_NOTIFY:
			if(hovered){
				if(!a || (a && a != hovered)){
					// INFERIOR = "left towards an inferior" / in
					// ANCESTOR = "left towards an ancestor" / out
					int direction = hovered->parent == a ? GDK_NOTIFY_ANCESTOR : GDK_NOTIFY_INFERIOR;
					GdkEventCrossing leave = {
						.type = GDK_LEAVE_NOTIFY,
						.detail = direction
					};
					// actors should always return NOT_HANDLED for leave events.
					_agl_actor__on_event(hovered, (GdkEvent*)&leave, xy);
				}
			}

			break;
		case GDK_BUTTON_PRESS:
#ifdef USE_GTK
			if(SCENE_IS_GTK(actor)) gtk_window_set_focus((GtkWindow*)gtk_widget_get_toplevel(root->gl.gdk.widget), root->gl.gdk.widget);
#endif
		case GDK_BUTTON_RELEASE:
			if(root->selected != a){
				if(root->selected) agl_actor__invalidate(root->selected);
				root->selected = a; // TODO almost certainly not always correct
				if(a) agl_actor__invalidate(a);
			}
			if(a){
				AGliPt offset = _agl_actor__find_offset(a);
				return _agl_actor__on_event(a, event, (AGliPt){xy.x - offset.x, xy.y - offset.y});
			}else{
				return AGL_HANDLED;
			}
		default:
			break;
	}

	if(a && a != actor){
		if(event->type == GDK_MOTION_NOTIFY){
			if(a != hovered && !actor_context.grabbed){
				root->hovered = a;

				GdkEvent enter = {
					.type = GDK_ENTER_NOTIFY,
				};
				return _agl_actor__on_event(a, &enter, xy);
			}
		}

		AGliPt offset = _agl_actor__find_offset(a);
		return _agl_actor__on_event(a, event, (AGliPt){xy.x - offset.x, xy.y - offset.y});

	}else{
		if(event->type == GDK_MOTION_NOTIFY){
			if(hovered){
				root->hovered = NULL;
#ifdef USE_GTK
				if(SCENE_IS_GTK(actor)){
					gdk_window_set_cursor(widget->window, NULL);
					gtk_widget_queue_draw(widget); // TODO not always needed
				}
#endif
			}
		}
	}

	return AGL_NOT_HANDLED;
}


bool
agl_actor__xevent (AGlRootActor* scene, XEvent* xevent)
{
	static Time previous1 = 0;
	static Time previous2 = 0;

	switch (xevent->type) {
		case ButtonPress:
			{
				GdkEvent event = {
					.button = {
						.type = GDK_BUTTON_PRESS,
						.x = (double)xevent->xbutton.x,
						.y = (double)xevent->xbutton.y,
						.button = xevent->xbutton.button,
					},
				};

				if(xevent->xbutton.time - previous1 < 250){
					if(xevent->xbutton.time - previous2 < 400)
						event.button.type = GDK_3BUTTON_PRESS;
					else
						event.button.type = GDK_2BUTTON_PRESS;
				}

				agl_actor__on_event(scene, &event);

				previous2 = previous1;
				previous1 = xevent->xbutton.time;
			}
			break;
		case ButtonRelease:
			agl_actor__on_event(scene, &(GdkEvent){
				.button = {
					.type = GDK_BUTTON_RELEASE,
					.x = (double)xevent->xbutton.x,
					.y = (double)xevent->xbutton.y,
					.button = xevent->xbutton.button,
					.state = xevent->xbutton.state,
				},
			});
			break;
		case MotionNotify:
			{
				GdkEventMotion event = {
					.type = GDK_MOTION_NOTIFY,
					.x = (double)xevent->xbutton.x,
					.y = (double)xevent->xbutton.y,
				};
				event.type = GDK_MOTION_NOTIFY;

				agl_actor__on_event(scene, (GdkEvent*)&event);
			}
			break;
		case KeyPress:
		case KeyRelease:
			{
				KeySym key;
				XLookupString(&xevent->xkey, NULL, 0, &key, NULL);

				if(!(key >= 0xffe1 && key <= 0xffee)){ // ignore modifier keys
					GdkEventKey event = {
						.type = xevent->type == KeyPress ? GDK_KEY_PRESS : GDK_KEY_RELEASE,
						.state = ((XKeyEvent*)xevent)->state,
						.keyval = key
					};
					agl_actor__on_event(scene, (GdkEvent*)&event);
				}
			}
			break;
		case FocusOut:
			agl_actor__on_event(scene, &(GdkEvent){.type = GDK_FOCUS_CHANGE});
			break;
	}
	return AGL_NOT_HANDLED;
}


/*
 *  Utility function for use as a gtk "expose-event" handler.
 *  Application must pass the root actor as user_data when connecting the signal.
 */
#ifdef USE_GTK
bool
agl_actor__on_expose (GtkWidget* widget, GdkEventExpose* event, gpointer user_data)
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

		glOrtho (actor->scrollable.x1, actor->scrollable.x2, actor->scrollable.y2, actor->scrollable.y1, 256.0, -256.0);
	}

	AGlScene* root = user_data;
	g_return_val_if_fail(root, false);

	if(!GTK_WIDGET_REALIZED(widget)) return true;

	AGL_ACTOR_START_DRAW(root) {
		set_projection((AGlActor*)root);
		agl_bg_colour_rbga(root->bg_colour);
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

#if USE_SYSTEM_GTKGLEXT
		gdk_gl_drawable_swap_buffers(root->gl.gdk.drawable);
#else
		gdk_gl_window_swap_buffers(root->gl.gdk.drawable);
#endif
	} AGL_ACTOR_END_DRAW(root)

	return true;
}
#endif


AGlActor*
agl_actor__find_by_name (AGlActor* actor, const char* name)
{
	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* a = l->data;
		if(!strcmp(a->name, name) || (a = agl_actor__find_by_name(a, name))) return a;
	}
	return NULL;
}


AGlActor*
agl_actor__find_by_class (AGlActor* actor, AGlActorClass* klass)
{
	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* a = l->data;
		if(a->class == klass || (a = agl_actor__find_by_class(a, klass))) return a;
	}
	return NULL;
}


AGlActor*
agl_actor__find_by_z (AGlActor* actor, int z)
{
	GList* l = actor->children;
	for(;l;l=l->next){
		AGlActor* a = l->data;
		if(a->z == z) return a;
	}
	return NULL;
}


/*
 *  Searches up then down
 *
 *  This can be more efficient than starting at the root,
 *  but could conceivably return a concealled actor
 *  To avoid this, pass the root actor as the argument.
 */
AGlActor*
agl_actor__pick (AGlActor* actor, AGliPt _pt)
{
	AGlfPt pt = (AGlfPt){_pt.x, _pt.y};
	AGlfRegion region = actor->region;
	AGlActor* parent = actor;
	while((parent = parent->parent)){
		float y = (float)pt.y + region.y1;
		pt = (AGlfPt){(float)pt.x + region.x1, y};
		bool match = region_match(&parent->region, pt.x, pt.y);
		if(match){
			return child_region_hit(parent, (AGliPt){pt.x, pt.y});
		}
		region = parent->region;
	}

	return NULL;
}


AGlBehaviour*
agl_actor__find_behaviour (AGlActor* actor, AGlBehaviourClass* klass)
{
	for(int i = 0; i < AGL_ACTOR_N_BEHAVIOURS; i++){
		AGlBehaviour* behaviour = actor->behaviours[i];
		if(!behaviour)
			break;

		if(behaviour->klass == klass)
			return behaviour;
	}
	return NULL;
}


bool
agl_actor__null_painter (AGlActor* actor)
{
	return true;
}


bool
agl_actor__solid_painter (AGlActor* actor)
{
	agl->shaders.plain->uniform.colour = 0xff000055;
	agl_use_program((AGlShader*)agl->shaders.plain);

	agl_rect_((AGlRect){.w = agl_actor__width(actor), .h = agl_actor__height(actor)});

	return true;
}


void
agl_actor__grab (AGlActor* actor)
{
	actor_context.grabbed = actor;
}


/*
 *  Remove render caches for the actor and all parents
 */
void
agl_actor__invalidate (AGlActor* actor)
{
	g_return_if_fail(actor);

	void _agl_actor__invalidate (AGlActor* actor)
	{
#ifdef AGL_ACTOR_RENDER_CACHE
		actor->cache.valid = false;
#endif
		call(actor->invalidate, actor);

		#if 0
		GList* l = actor->children;
		for(;l;l=l->next){
			AGlActor* a = l->data;
			_agl_actor__invalidate(a);
		}
		#endif

		while(actor){
#ifdef AGL_ACTOR_RENDER_CACHE
			actor->cache.valid = false;
#endif
			// should probably also call actor->invalidate here
			actor = actor->parent;
		}
	}
	_agl_actor__invalidate(actor);

	if(actor->root){
#ifdef USE_GTK
		if(actor->root->type == CONTEXT_TYPE_GTK) gtk_widget_queue_draw(actor->root->gl.gdk.widget);
#else
		if(false);
#endif
		else call(actor->root->draw, actor->root, actor->root->user_data);
	}
}


/*
 *  Invalidate both parents and children
 */
void
agl_actor__invalidate_down (AGlActor* actor)
{
	agl_actor__invalidate(actor);

	void _agl_actor__invalidate_down(AGlActor* actor)
	{
#ifdef AGL_ACTOR_RENDER_CACHE
		actor->cache.valid = false;
#endif
		call(actor->invalidate, actor);

		GList* l = actor->children;
		for(;l;l=l->next){
			_agl_actor__invalidate_down((AGlActor*)l->data);
		}
	}

	_agl_actor__invalidate_down(actor);
}


/*
 *  Enable / disable caching of all actors that the given actor participates in.
 *  Note that the caching of child actors and actors in other parts of the tree are not affected.
 */
void
agl_actor__enable_cache (AGlActor* actor, bool enable)
{
#ifdef AGL_ACTOR_RENDER_CACHE
	while(actor){
		actor->cache.enabled = enable;
		if(!enable) actor->cache.valid = false;
		actor = actor->parent;
	}
#endif
}


	typedef struct {
		AGlActor*      actor;
		AnimationFn    done;
		gpointer       user_data;
	} C;

	void agl_actor_on_frame (WfAnimation* animation, int time)
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
		if(g_list_length(a->transitions) != l - 1) pwarn("animation not removed. len=%i-->%i", l, g_list_length(a->transitions));
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
WfAnimation*
agl_actor__start_transition (AGlActor* actor, GList* animatables, AnimationFn done, gpointer user_data)
{
	g_return_val_if_fail(actor, NULL);

	if(!actor->root->enable_animations/* || !actor->root->draw*/){ //if we cannot initiate painting we cannot animate.
		GList* l = animatables;
		for(;l;l=l->next){
			WfAnimatable* animatable = l->data;
			*animatable->val.f = animatable->target_val.f;
		}
		g_list_free(animatables);
		return NULL;
	}

	// set initial value
	GList* l = animatables;
	for(;l;l=l->next){
		WfAnimatable* animatable = l->data;
		animatable->start_val.b = *animatable->val.b;
	}

	l = actor->transitions;
	GList* next = NULL;
	for(;l;l=next){
		next = l->next; // store before the link is freed

		// remove animatables we are replacing. let others finish.
		GList* k = animatables;
		for(;k;k=k->next){
			if(wf_animation_remove_animatable((WfAnimation*)l->data, (WfAnimatable*)k->data)) break;
		}
	}

	if(animatables){
		WfAnimation* animation = wf_animation_new(_on_animation_finished, AGL_NEW(C,
			.actor = actor,
			.done = done,
			.user_data = user_data
		));
		animation->on_frame = agl_actor_on_frame;
		actor->transitions = g_list_append(actor->transitions, animation);
#ifdef USE_FRAME_CLOCK
		if(actor->root) actor->root->is_animating = true;
#endif
		wf_transition_add_member(animation, animatables);
		wf_animation_start(animation);
		agl_actor__enable_cache(actor, false);

		return animation;
	}

	return NULL;
}


/*
 *  Will be removed - use agl_actor__find_offset instead
 */
static AGliPt
_agl_actor__find_offset (AGlActor* a)
{
	AGliPt of = {0, 0};
	do {
		of.x += a->region.x1;
		of.y += a->region.y1;
	} while((a = a->parent));
	return of;
}


/*
 *  Return the distance from the actor top left to the root top left
 */
AGliPt
agl_actor__find_offset (AGlActor* a)
{
	AGliPt of = {0, 0};
	do {
		of.x += a->region.x1 + a->scrollable.x1;
		of.y += a->region.y1 + a->scrollable.y1;
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


/*
 *  Used externally but not published
 */
#ifdef AGL_ACTOR_RENDER_CACHE
bool
agl_actor__is_cached(AGlActor* a)
{
	do {
		if(a->cache.enabled && a->cache.valid) return true;
	} while((a = a->parent));

	return false;
}
#endif


void
agl_actor_class__add_behaviour (AGlActorClass* K, AGlBehaviourClass* B)
{
	int i; for(i=0;i<AGL_ACTOR_N_BEHAVIOURS;i++){
		AGlBehaviourClass* behaviour = K->behaviour_classes[i];
		if(!behaviour){
			K->behaviour_classes[i] = B;
			break;
		}
	}
}


AGlBehaviour*
agl_actor__get_behaviour (AGlActor* actor, AGlBehaviourClass* klass)
{
	for(int i = 0; i < AGL_ACTOR_N_BEHAVIOURS; i++){
		AGlBehaviour* behaviour = actor->behaviours[i];
		if(!behaviour)
			break;
		if(behaviour->klass == klass)
			return behaviour;
	}
	return NULL;
}


#ifdef DEBUG
void
agl_actor__print_tree (AGlActor* actor)
{
#ifdef AGL_ACTOR_RENDER_CACHE
	char white [16] = "\x1b[0;39m";
	char lgrey [16] = "\x1b[38;5;244m";
	char dgrey [16] = "\x1b[38;5;238m";
#endif

	g_return_if_fail(actor);

	printf("scene graph:\n");
	static int indent; indent = 1;

	void _print(AGlActor* actor)
	{
		g_return_if_fail(actor);
		int i; for(i=0;i<indent;i++) printf("  ");

		bool is_onscreen = agl_actor__is_onscreen(actor);
		char* offscreen = is_onscreen ? "" : " OFFSCREEN";
		char* zero_size = agl_actor__width(actor) ? "" : " ZEROSIZE";
#ifdef AGL_ACTOR_RENDER_CACHE
		char* negative_size = (agl_actor__width(actor) < 1 || agl_actor__height(actor) < 1) ? " NEGATIVESIZE" : "";
		char* disabled = agl_actor__is_disabled(actor) ?  " DISABLED" :  "";
#endif

		char scrollablex[32] = {0};
		if(actor->scrollable.x1 || actor->scrollable.x2)
			sprintf(scrollablex, " scrollable.x(%i,%i)", actor->scrollable.x1, actor->scrollable.x2);
		char scrollabley[32] = {0};
		if(actor->scrollable.y1 || actor->scrollable.y2)
			sprintf(scrollabley, " scrollable.y(%i,%i)", actor->scrollable.y1, actor->scrollable.y2);

#ifdef AGL_ACTOR_RENDER_CACHE
		char* colour = !agl_actor__width(actor) || !is_onscreen
			? dgrey
			: agl_actor__is_cached(actor)
				? lgrey
				: "";
		AGliPt offset = _agl_actor__find_offset(actor);
		if(actor->name) printf("%s%s:%s%s%s%s cache(%i,%i) region(%0f,%0f,%0f,%0f) offset(%i,%i)%s%s%s\n", colour, actor->name, offscreen, zero_size, negative_size, disabled, actor->cache.enabled, actor->cache.valid, actor->region.x1, actor->region.y1, actor->region.x2, actor->region.y2, offset.x, offset.y, scrollablex, scrollabley, white);
#else
		if(actor->name) printf("%s\n", actor->name);
#endif
		if(!actor->name) printf("%s%s (%0f,%0f)\n", offscreen, zero_size, actor->region.x1, actor->region.y1);
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
