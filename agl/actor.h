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

#ifndef __gl_actor_h__
#define __gl_actor_h__
#include "agl/typedefs.h"
#include "gtk/gtk.h"

#undef DEBUG_ACTOR

#ifndef bool
#  define bool int
#endif

typedef struct _actor AGlActor;

typedef AGlActor* (ActorNew)          (GtkWidget*);
typedef void      (*ActorInit)        (AGlActor*, gpointer);
typedef void      (*ActorSize)        (AGlActor*, int, int);
typedef void      (*ActorSetState)    ();
typedef void      (*ActorPaint)       (AGlActor*);
typedef bool      (*ActorOnEvent)     (AGlActor*, GdkEvent*, AGliPt xy, AGliPt scroll_offset);
typedef void      (*ActorFn)          (AGlActor*);

typedef struct _RootActor     RootActor;
typedef struct _actor_context ActorContext;

struct _actor {
#ifdef DEBUG_ACTOR
	char*            name;
#endif
	AGlActor*        parent;
	RootActor*       root;
	ActorInit        init;            // called once when gl context is available.
	ActorSize        set_size;        // called when the parent widget is resized.
	ActorSetState    set_state;       // called once per expose
	ActorFn          invalidate;      // clear fbo caches
	ActorPaint       paint;           // called multiple times per expose, once for each object.
	ActorOnEvent     on_event;
	ActorFn          free;
	AGliRegion       region;
	GList*           children;        // type AGlActor
	bool             hover;
};


AGlActor* actor__new             ();
AGlActor* actor__new_root        (GtkWidget*);
void      actor__free            (AGlActor*);
AGlActor* actor__add_child       (AGlActor*, AGlActor*);
void      actor__remove_child    (AGlActor*, AGlActor*);
AGlActor* actor__replace_child   (AGlActor*, AGlActor*, ActorNew);
void      actor__init            (AGlActor*, gpointer);              // called once when gl context is available.
void      actor__paint           (AGlActor*);
void      actor__set_size        (AGlActor*);
void      actor__grab            (AGlActor*);
void      actor__invalidate      (AGlActor*);
bool      actor__on_event        (RootActor*, GdkEvent*);
void      actor__null_painter    (AGlActor*);


struct _RootActor {
   AGlActor          actor;
   GtkWidget*        widget;
   AGlRect           viewport;
};

struct _actor_context {
   AGlActor*         grabbed;         // to enable dragging outside an actors boundaries.
};

#ifdef __gl_actor_c__
ActorContext actor_context;
#else
extern ActorContext actor_context;
#endif

#endif
