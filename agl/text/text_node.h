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
#ifndef __agl_textnode_h__
#define __agl_textnode_h__

#include "agl/actor.h"

typedef struct {
    AGlActor         actor;
    char*            text;
    struct {
        char* name;
        int   size;
    }                font;
    PangoGlyphString glyphs;
} TextNode;

AGlActor* text_node          (gpointer);
void      text_node_set_text (TextNode*, const char*);

#endif
