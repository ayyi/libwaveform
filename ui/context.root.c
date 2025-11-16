/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 | This handles some legacy gl initialisation common to all WfActor
 | instances.
 | The preferred place for this would be in a renderer object.
 |
 */

#include "config.h"
#include "debug/debug.h"
#include "agl/utils.h"
#include "agl/actor.h"
#include "context.h"

#ifdef USE_GTK
#define WAVEFORM_START_DRAW(wfc) \
	if (actor_not_is_gtk(wfc->root->root) || gdk_gl_drawable_make_current (wfc->root->root->gl.gdk.drawable, wfc->root->root->gl.gdk.context))
#else
#define WAVEFORM_START_DRAW(wfc) \
	;
#endif

#define WAVEFORM_END_DRAW(wa) \
	;


static void
wf_context_init_gl (WaveformContext* wfc)
{
	PF;

	AGl* agl = agl_get_instance();

	if (!agl->pref_use_shaders) {
		wfc->use_1d_textures = false;
		return;
	}

	WAVEFORM_START_DRAW(wfc) {

		if (!wfc->root) {
			agl_gl_init();
		}

		if (!agl->use_shaders) {
			agl_use_program(NULL);
			wfc->use_1d_textures = false;
		}

	} WAVEFORM_END_DRAW(wfc);
}


AGlBehaviour* init_hook      ();
void          init_hook_init (AGlBehaviour*, AGlActor*);

typedef struct {
   AGlBehaviour     behaviour;
   WaveformContext* wfc;
} InitHookBehaviour;

static AGlBehaviourClass klass = {
	.new = init_hook,
	.init = init_hook_init,
};


AGlBehaviour*
init_hook ()
{
	return (AGlBehaviour*)AGL_NEW(InitHookBehaviour,
		.behaviour = {
			.klass = &klass,
		}
	);
}


AGlBehaviour*
init_hook_new (WaveformContext* wfc)
{
	AGlBehaviour* hook = init_hook();
	((InitHookBehaviour*)hook)->wfc = wfc;

	return hook;
}


void
init_hook_init (AGlBehaviour* behaviour, AGlActor* actor)
{
	InitHookBehaviour* hook = (InitHookBehaviour*)behaviour;
	AGlScene* scene = actor->root;
	AGl* agl = agl_get_instance();

	wf_context_init_gl(hook->wfc);

	if (scene->draw) wf_context_queue_redraw(hook->wfc);
	hook->wfc->use_1d_textures = agl->use_shaders;
}


