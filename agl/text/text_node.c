/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2020-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "config.h"
#include <stdbool.h>
#include <GL/gl.h>
#include "agl/debug.h"
#include "agl/shader.h"
#include "text/pango.h"
#include "agl/behaviours/clip.h"
#include "text/text_node.h"

#define g_free0(var) ((var == NULL) ? NULL : (var = (g_free(var), NULL)))

static AGl* agl = NULL;

static void text_node_free (AGlActor*);
static void text_node_init (AGlActor*);
static bool text_node_draw (AGlActor*);

static AGlActorClass actor_class = {0, "Text", (AGlActorNew*)text_node, text_node_free};


AGlActorClass*
text_node_get_class ()
{
	static bool init_done = false;

	if(!init_done){
		agl = agl_get_instance();

		agl_actor_class__add_behaviour(&actor_class, clip_get_class());

		init_done = true;
	}

	return &actor_class;
}


AGlActor*
text_node (gpointer user_data)
{
	text_node_get_class();

	TextNode* node = agl_actor__new(TextNode,
		.actor = {
			.class  = &actor_class,
			.name   = actor_class.name,
			.init   = text_node_init,
			.paint  = text_node_draw,
		},
		.font = {
			.name = "Roboto"
		}
	);

	return (AGlActor*)node;
}


static void
text_node_free (AGlActor* actor)
{
	g_free0(((TextNode*)actor)->text);
	g_free(actor);
}


/*
 *  Ownership is taken of arg text
 */
void
text_node_set_text (TextNode* node, const char* text)
{
	if(node->text)
		g_free(node->text);

	node->text = (char*)text;
}


static void
text_node_init (AGlActor* actor)
{
#ifdef AGL_ACTOR_RENDER_CACHE
	actor->fbo = agl_fbo_new(agl_actor__width(actor), agl_actor__height(actor), 0, 0);
	actor->cache.enabled = true;
#endif
}


static bool
text_node_draw (AGlActor* actor)
{
	TextNode* text = (TextNode*)actor;

	agl_set_font(text->font.name, text->font.size, PANGO_WEIGHT_NORMAL);

	agl_print(0, 0, 0, actor->colour, ((TextNode*)actor)->text);

	return true;
}

