/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2013-2015 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

#ifndef __agl_actor_h__
#define __agl_actor_h__
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#ifdef USE_SDL
#  include "SDL2/SDL.h"
#endif
#include "transition/transition.h"
#include "agl/typedefs.h"
#include "gtk/gtk.h"

#undef AGL_DEBUG_ACTOR
#define AGL_ACTOR_RENDER_CACHE

typedef AGlActor* (AGlActorNew)       (GtkWidget*);
typedef void      (*AGlActorSetState) (AGlActor*);
typedef bool      (*AGlActorPaint)    (AGlActor*);
typedef bool      (*AGlActorOnEvent)  (AGlActor*, GdkEvent*, AGliPt xy, AGliPt scroll_offset);
typedef void      (*AGlActorFn)       (AGlActor*);

typedef struct _AGlActorContext AGlActorContext;

typedef enum {
	CONTEXT_TYPE_GTK = 0,
	CONTEXT_TYPE_SDL,
	CONTEXT_TYPE_GLX,
} ContextType;

struct _AGlActor {
#ifdef AGL_DEBUG_ACTOR
	char*            name;
#endif
	AGlActor*        parent;
	AGlRootActor*    root;

	AGlActorFn       init;            // called once when gl context is available.
	AGlActorFn       set_size;        // called when the parent widget is resized.
	AGlActorSetState set_state;       // called once per expose
	AGlActorFn       invalidate;      // clear fbo caches (and most likely other cached render information too)
	AGlActorPaint    paint;           // called multiple times per expose, once for each object.
	AGlActorOnEvent  on_event;
	AGlActorFn       free;

	AGliRegion       region;          // position and size. {int x1, y1, x2, y2}
	AGlShader*       program;
	uint32_t         colour;          // rgba
	int              z;               // controls the order objects with the same parent are drawn.
	bool             disabled;        // when disabled, actor and children are greyed-out and are non-interactive.
	GList*           children;        // type AGlActor
	GList*           transitions;     // list of WfAnimation*'s that are currently active.
#ifdef AGL_ACTOR_RENDER_CACHE
	AGlFBO*          fbo;
	struct {
	    bool         enabled;         // if false, caching must be disabled. if true, caching is used only if fbo is present.
	    bool         valid;
	}                cache;
#endif
	bool             hover;
};


AGlActor* agl_actor__new             ();
AGlActor* agl_actor__new_root        (GtkWidget*);
AGlActor* agl_actor__new_root_       (ContextType);
void      agl_actor__free            (AGlActor*);
AGlActor* agl_actor__add_child       (AGlActor*, AGlActor*);
void      agl_actor__remove_child    (AGlActor*, AGlActor*);
AGlActor* agl_actor__replace_child   (AGlActor*, AGlActor*, AGlActor*);
void      agl_actor__paint           (AGlActor*);
void      agl_actor__set_size        (AGlActor*);
void      agl_actor__set_use_shaders (AGlRootActor*, gboolean);
void      agl_actor__grab            (AGlActor*);
void      agl_actor__invalidate      (AGlActor*);
void      agl_actor__enable_cache    (AGlActor*, bool);
void      agl_actor__start_transition(AGlActor*, GList* animatables, AnimationFn done, gpointer);
bool      agl_actor__is_disabled     (AGlActor*);
bool      agl_actor__on_event        (AGlRootActor*, GdkEvent*);
AGlActor* agl_actor__find_by_z       (AGlActor*, int);
bool      agl_actor__null_painter    (AGlActor*);
bool      agl_actor__on_expose       (GtkWidget*, GdkEventExpose*, gpointer);

#ifdef DEBUG
void      agl_actor__print_tree      (AGlActor*);
#endif

struct _AGlRootActor {
   AGlActor          actor;
   GtkWidget*        widget;
   AGlRect           viewport;       // { float x, y, w, h; } TODO clarify how this differs from root->region. region is int but viewport is float.

   void              (*draw)(AGlScene*, gpointer); // application callback - called when the application needs to initiate a redraw.

   gpointer          user_data;

   union {
		struct {
			GdkGLContext*  context;
			GdkGLDrawable* drawable;
		}          gdk;
#ifdef USE_SDL
		struct {
			SDL_GLContext context;
		}          sdl;
#endif
		struct {
		}            glx;
   }                 gl;
   ContextType       type;

#ifdef USE_FRAME_CLOCK
   bool              is_animating;
#endif
};

struct _AGlTextureActor {
   AGlActor          actor;
   guint             texture[1];
};

struct _AGlActorContext {
   AGlActor*         grabbed;         // to enable dragging outside an actors boundaries.
};

#ifdef __agl_actor_c__
AGlActorContext actor_context;
#else
extern AGlActorContext actor_context;
#endif

#ifdef USE_SDL
#  define actor_is_sdl(RA) (RA && RA->type == CONTEXT_TYPE_SDL)
#else
#  define actor_is_sdl(RA) false
#endif
#define actor_not_is_gtk(RA) (RA && RA->type != CONTEXT_TYPE_GTK)

#define AGL_ACTOR_START_DRAW(A) \
	if(__wf_drawing){ gwarn("AGL_ACTOR_START_DRAW: already drawing"); } \
	__draw_depth++; \
	__wf_drawing = TRUE; \
	if (actor_is_sdl(((AGlRootActor*)A)) || (__draw_depth > 1) || gdk_gl_drawable_gl_begin (((AGlRootActor*)A)->gl.gdk.drawable, ((AGlRootActor*)A)->gl.gdk.context)) {

#define AGL_ACTOR_END_DRAW(A) \
	__draw_depth--; \
	if(actor_is_sdl(((AGlRootActor*)A))){ \
		if(!__draw_depth) gdk_gl_drawable_gl_end(((AGlRootActor*)A)->gl.gdk.drawable); \
		else { gwarn("!! gl_begin fail"); } \
	} \
	} \
	(__wf_drawing = FALSE);

#endif
