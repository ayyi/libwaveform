/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2025-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | HoverActor displays time and level for the current pointer position. |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include "debug/debug.h"
#include "waveform/actor.h"
#include "waveform/ui-utils.h"
#include "waveform/hover.h"

static AGl* agl;

/*
 *  EventSpy attaches to a different actor in order to get its events
 */
typedef struct
{
    AGlBehaviourClass class;
} EventSpyClass;

typedef struct
{
    AGlBehaviour behaviour;
    AGliPt       xy;
} EventSpy;

void
event_spy_free (AGlBehaviour* b)
{
	// override the default free as there is nothing to free
}

static bool event_spy_event (AGlBehaviour*, AGlActor*, GdkEvent*);

static EventSpyClass event_spy_class = {
	.class = {
		.free = event_spy_free,
		.event = event_spy_event
	}
};


typedef struct {
    AGlActor         actor;
    WaveformActor*   wf_actor;
    WaveformContext* context;
    EventSpy         eventspy;
} HoverActor;

static bool
event_spy_event (AGlBehaviour* behaviour, AGlActor* actor, GdkEvent* event)
{
	EventSpy* spy = (EventSpy*)behaviour;

	switch (event->type) {
		case GDK_MOTION_NOTIFY:
			spy->xy = (AGliPt){event->button.x, event->button.y};
			agl_actor__invalidate((AGlActor*)actor->root);
			break;
		case GDK_LEAVE_NOTIFY:
			spy->xy = (AGliPt){-1, -1};
			agl_actor__invalidate((AGlActor*)actor->root);
		default:
			break;
	}
	return AGL_NOT_HANDLED;
}


static bool hover_actor_paint (AGlActor*);

static AGlActorClass actor_class = {0, "Hover", (AGlActorNew*)hover_actor};


static void
hover_actor_size (AGlActor* actor)
{
	AGlActor* parent = actor->parent;

	actor->region = (AGlfRegion){
		parent->region.x2 - 56.,
		8.,
		parent->region.x2,
		30.,
	};
}


static void
hover_actor_init (AGlActor* actor)
{
	HoverActor* hover = (HoverActor*)actor;

	agl_actor__add_behaviour((AGlActor*)hover->wf_actor, &hover->eventspy.behaviour);
}


AGlActor*
hover_actor (WaveformActor* wf_actor)
{
	g_return_val_if_fail(wf_actor, NULL);

	agl = agl_get_instance();

	HoverActor* hover = agl_actor__new(HoverActor,
		.actor = {
			.class = &actor_class,
			.colour = 0xffffffff,
			.init = hover_actor_init,
			.paint = hover_actor_paint,
			.set_size = hover_actor_size,
		},
		.wf_actor = wf_actor,
		.context = wf_actor->context,
		.eventspy = {
			.behaviour = {
				.klass = &event_spy_class.class
			},
			.xy = { -1, -1 },
		}
	);

	return (AGlActor*)hover;
}


static bool
hover_actor_paint (AGlActor* actor)
{
	HoverActor* hover = (HoverActor*)actor;

	if (hover->eventspy.xy.y > -1) {
		int wave_height = agl_actor__height((AGlActor*)hover->wf_actor);
		int n_channels = hover->wf_actor->waveform->n_channels;
		int ch_height = wave_height / n_channels;
		int pk_height = ch_height / 2;
		int y = (hover->eventspy.xy.y - (int)((AGlActor*)hover->wf_actor)->region.y1) % ch_height;

		agl_set_font_string("Sans 7.5");
		agl_print_with_background(0, 0, 0, 0xccccccff, 0x0000005f, "%.2f dB ", wf_int2db((SHRT_MAX * (y - pk_height)) / pk_height));
	}

	if (hover->eventspy.xy.x > -1) {
		agl_set_font_string("Sans 7.5");
		agl_print_with_background(0, 14, 0, 0xccccccff, 0x0000005f, "%s", wf_context_print_time(hover->context, hover->eventspy.xy.x));
	}

	return true;
}
