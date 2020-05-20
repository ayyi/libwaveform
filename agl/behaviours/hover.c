/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2020-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "config.h"
#undef USE_GTK
#include "hover.h"

static bool hover_behaviour_handle_event (AGlBehaviour*, AGlActor*, GdkEvent*);

static AGlBehaviourClass klass = {
	.new = hover_behaviour,
	.event = hover_behaviour_handle_event,
};


AGlBehaviourClass*
hover_get_class ()
{
	return &klass;
}


AGlBehaviour*
hover_behaviour ()
{
	return AGL_NEW(AGlBehaviour,
		.klass = &klass,
	);
}


static bool
hover_behaviour_handle_event (AGlBehaviour* behaviour, AGlActor* actor, GdkEvent* event)
{
	switch(event->type){
		case GDK_ENTER_NOTIFY:
		case GDK_LEAVE_NOTIFY:
			agl_actor__invalidate(actor);
			break;
		default:
			break;
	}

	return AGL_NOT_HANDLED;
}
