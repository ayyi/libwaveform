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
/*
 * Copyright (C) 2008  Intel Corporation.
 *
 * Authored By: Øyvind Kolås <pippin@o-hand.com>
 *              Emmanuele Bassi <ebassi@linux.intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __text__input_h__
#define __text__input_h__

#include <pango/pango.h>
#include <graphene.h>
#include "agl/actor.h"
#include "agl/observable.h"
#include "agl/text/text_buffer.h"

G_BEGIN_DECLS

typedef struct _TextInputPrivate TextInputPrivate;

typedef struct
{
   AGlActor actor;

   char* font_family;
   AGlObservable* font;

   TextInputPrivate* priv;
} TextInput;

AGlActorClass*        text_input_get_class               ();

AGlActor*             text_input                         (gpointer);

AGlTextBuffer*        text_input_get_buffer              (TextInput*);
void                  text_input_set_buffer              (TextInput*, AGlTextBuffer*);
const gchar*          text_input_get_text                (TextInput*);
void                  text_input_set_text                (TextInput*, const gchar* text);
void                  text_input_set_markup              (TextInput*, const gchar* markup);
void                  text_input_set_color               (TextInput*, uint32_t);
uint32_t              text_input_get_color               (TextInput*);
void                  text_input_set_font_name           (TextInput*, const gchar*);
const gchar*          text_input_get_font_name           (TextInput*);
void                  text_input_set_font_description    (TextInput*, PangoFontDescription*);
PangoFontDescription* text_input_get_font_description    (TextInput*);

void                  text_input_set_ellipsize           (TextInput*, PangoEllipsizeMode);
PangoEllipsizeMode    text_input_get_ellipsize           (TextInput*);
void                  text_input_set_line_wrap           (TextInput*, bool);
bool                  text_input_get_line_wrap           (TextInput*);
void                  text_input_set_line_wrap_mode      (TextInput*, PangoWrapMode);
PangoWrapMode         text_input_get_line_wrap_mode      (TextInput*);
PangoLayout*          text_input_get_layout              (TextInput*);
void                  text_input_set_attributes          (TextInput*, PangoAttrList*);
PangoAttrList*        text_input_get_attributes          (TextInput*);
void                  text_input_set_use_markup          (TextInput*, bool);
bool                  text_input_get_use_markup          (TextInput*);
void                  text_input_set_line_alignment      (TextInput*, PangoAlignment);
PangoAlignment        text_input_get_line_alignment      (TextInput*);
void                  text_input_set_justify             (TextInput*, bool);
bool                  text_input_get_justify             (TextInput*);

void                  text_input_insert_unichar          (TextInput*, gunichar wc);
void                  text_input_delete_chars            (TextInput*, guint n_chars);
void                  text_input_insert_text             (TextInput*, const gchar* text, gssize position);
void                  text_input_delete_text             (TextInput*, gssize start_pos, gssize end_pos);
gchar*                text_input_get_chars               (TextInput*, gssize start_pos, gssize end_pos);
void                  text_input_set_editable            (TextInput*, bool);
bool                  text_input_get_editable            (TextInput*);
void                  text_input_set_activatable         (TextInput*, bool);
bool                  text_input_get_activatable         (TextInput*);

gint                  text_input_get_cursor_position     (TextInput*);
void                  text_input_set_cursor_position     (TextInput*, gint);
void                  text_input_set_cursor_visible      (TextInput*, bool);
bool                  text_input_get_cursor_visible      (TextInput*);
void                  text_input_set_cursor_color        (TextInput*, uint32_t);
uint32_t              text_input_get_cursor_color        (TextInput*);
void                  text_input_set_cursor_size         (TextInput*, gint size);
guint                 text_input_get_cursor_size         (TextInput*);
void                  text_input_get_cursor_rect         (TextInput*, graphene_rect_t*);
void                  text_input_set_selectable          (TextInput*, bool selectable);
bool                  text_input_get_selectable          (TextInput*);
void                  text_input_set_selection_bound     (TextInput*, gint);
gint                  text_input_get_selection_bound     (TextInput*);
void                  text_input_set_selection           (TextInput*, gssize start_pos, gssize end_pos);
gchar*                text_input_get_selection           (TextInput*);
void                  text_input_set_selection_color     (TextInput*, uint32_t);
uint32_t              text_input_get_selection_color     (TextInput*);
bool                  text_input_delete_selection        (TextInput*);
void                  text_input_set_password_char       (TextInput*, gunichar wc);
gunichar              text_input_get_password_char       (TextInput*);
void                  text_input_set_max_length          (TextInput*, gint max);
gint                  text_input_get_max_length          (TextInput*);
void                  text_input_set_single_line_mode    (TextInput*, bool);
bool                  text_input_get_single_line_mode    (TextInput*);

void                  text_input_set_selected_text_color (TextInput*, uint32_t);
uint32_t              text_input_get_selected_text_color (TextInput*);

gint                  text_input_coords_to_position      (TextInput*, gfloat x, gfloat y);
bool                  text_input_position_to_coords      (TextInput*, gint position, gfloat* x, gfloat* y, gfloat* line_height);

void                  text_input_set_preedit_string      (TextInput*, const gchar* preedit_str, PangoAttrList* preedit_attrs, guint cursor_pos);
void                  text_input_set_placeholder         (TextInput*, const gchar*);

void                  text_input_get_layout_offsets      (TextInput*, gint* x, gint* y);

gfloat                text_input_get_height              (TextInput*);

G_END_DECLS

#endif
