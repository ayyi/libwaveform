/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. https://www.ayyi.org          |
* | copyright (C) 2012-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

#ifndef __text_actor_h__
#define __text_actor_h__

#include <GL/gl.h>

typedef struct {
	int         width;
	int         height;
	int         y_offset;
} Title;

typedef struct {
    AGlActor    actor;

    char*       title;
    Title       _title;
    char*       text;
    uint32_t    text_colour;
    uint32_t    title_colour1;
    uint32_t    title_colour2;

    int         baseline;
	struct {
        GLuint  ids[1];
        int     width;
        int     height;
    }           texture;

	bool        title_is_rendered;
} TextActor;

AGlActorClass* text_actor_get_class  ();
AGlActor*      text_actor            (WaveformActor*);
void           text_actor_set_colour (TextActor*, uint32_t, uint32_t);
void           text_actor_set_text   (TextActor*, char* title, char* text);

#endif
