/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

	gboolean    title_is_rendered;
} TextActor;

AGlActor* text_actor            (WaveformActor*);
void      text_actor_set_colour (TextActor*, uint32_t, uint32_t);
void      text_actor_set_text   (TextActor*, char* title, char* text);

#endif
