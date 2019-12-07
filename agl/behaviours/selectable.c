/**
* +----------------------------------------------------------------------+
* | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
* | copyright (C) 2019-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "selectable.h"

void selectable_init (AGlBehaviour*, AGlActor*);

static AGlBehaviourClass klass = {
	.init = selectable_init
};

AGlBehaviour*
selectable ()
{
	SelectBehaviour* a = AGL_NEW(SelectBehaviour,
		.behaviour = {
			.klass = &klass,
		},
		.observable = agl_observable_new()
	);

	return (AGlBehaviour*)a;
}


void
selectable_init (AGlBehaviour* behaviour, AGlActor* actor)
{
	SelectBehaviour* selectable = (SelectBehaviour*)behaviour;

	agl_observable_subscribe (selectable->observable, selectable->on_select, actor);
}
