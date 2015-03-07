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

#ifndef __gl_actor_h__
#define __gl_actor_h__
#include "transition/transition.h"
#include "agl/typedefs.h"
#include "gtk/gtk.h"

#undef AGL_DEBUG_ACTOR
#define AGL_ACTOR_RENDER_CACHE

#ifndef bool
#  define bool int
#endif

typedef AGlActor* (ActorNew)          (GtkWidget*);
typedef void      (*ActorInit)        (AGlActor*, gpointer);
typedef void      (*ActorSetState)    (AGlActor*);
typedef bool      (*ActorPaint)       (AGlActor*);
typedef bool      (*ActorOnEvent)     (AGlActor*, GdkEvent*, AGliPt xy, AGliPt scroll_offset);
typedef void      (*ActorFn)          (AGlActor*);

typedef struct _AGlRootActor  AGlRootActor;
typedef struct _actor_context AGlActorContext;

struct _AGlActor {
#ifdef AGL_DEBUG_ACTOR
	char*            name;
#endif
	AGlActor*        parent;
	AGlRootActor*    root;
	ActorInit        init;            // called once when gl context is available.
	ActorFn          set_size;        // called when the parent widget is resized.
	ActorSetState    set_state;       // called once per expose
	ActorFn          invalidate;      // clear fbo caches (and most likely other cached render information too)
	ActorPaint       paint;           // called multiple times per expose, once for each object.
	ActorOnEvent     on_event;
	ActorFn          free;
	AGliRegion       region;
	AGlShader*       program;
	uint32_t         colour;
	GList*           transitions;     // list of WfAnimation*
	GList*           children;        // type AGlActor
#ifdef AGL_ACTOR_RENDER_CACHE
	AGlFBO*          fbo;
	struct {
	    bool         enabled;         // if false caching must be disabled. if true, caching is used only if fbo is present.
	    bool         valid;
	}                cache;
#endif
	bool             hover;
};


AGlActor* agl_actor__new             ();
AGlActor* agl_actor__new_root        (GtkWidget*);
void      agl_actor__free            (AGlActor*);
AGlActor* agl_actor__add_child       (AGlActor*, AGlActor*);
void      agl_actor__remove_child    (AGlActor*, AGlActor*);
AGlActor* agl_actor__replace_child   (AGlActor*, AGlActor*, ActorNew);
void      agl_actor__init            (AGlActor*, gpointer);              // called once when gl context is available.
void      agl_actor__paint           (AGlActor*);
void      agl_actor__set_size        (AGlActor*);
void      agl_actor__grab            (AGlActor*);
void      agl_actor__invalidate      (AGlActor*);
void      agl_actor__enable_cache    (AGlActor*, bool);
void      agl_actor__start_transition(AGlActor*, GList* animatables, AnimationFn done, gpointer);
bool      agl_actor__on_event        (AGlRootActor*, GdkEvent*);
bool      agl_actor__null_painter    (AGlActor*);

#ifdef DEBUG
void      agl_actor__print_tree      (AGlActor*);
#endif

struct _AGlRootActor {
   AGlActor          actor;
   GtkWidget*        widget;
   AGlRect           viewport;
};

struct _AGlTextureActor {
   AGlActor          actor;
   guint             texture[1];
};

struct _actor_context {
   AGlActor*         grabbed;         // to enable dragging outside an actors boundaries.
};

#ifdef __gl_actor_c__
AGlActorContext actor_context;
#else
extern AGlActorContext actor_context;
#endif

#endif
