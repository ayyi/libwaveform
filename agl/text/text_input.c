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

/**
 * #TextInput is an actor that displays custom text using Pango
 * as the text rendering engine.
 *
 * #TextInput also allows inline editing of the text if the
 * actor is set editable using text_input_set_editable().
 *
 * Selection using keyboard or pointers can be enabled using
 * text_input_set_selectable().
 */
#include "config.h"
#include <string.h>
#include <math.h>
#include "agl/debug.h"
#include "agl/shader.h"
#include "agl/behaviours/key.h"
#include "agl/text/pango.h"
#include "agl/text/text_input.h"

/* cursor width in pixels */
#define DEFAULT_CURSOR_SIZE 2

/* vertical padding for the cursor */
#define CURSOR_Y_PADDING 2

#define TEXT_PADDING 2

typedef struct
{
   AGlActorClass parent;
} TextInputActorClass;

AGl* agl = NULL;

static void text_input_free      (AGlActor*);
static bool text_input_draw      (AGlActor*);
static void text_input_layout    (AGlActor*);
static bool text_input_event     (AGlActor*, GdkEvent*, AGliPt);
static bool text_input_press     (AGlActor*, GdkEvent*, AGliPt);
static bool text_input_release   (TextInput*, GdkEvent*);
static bool text_input_key_press (TextInput*, GdkEvent*);
static bool text_input_move      (TextInput*, GdkEvent*, AGliPt);

static bool text_input_move_left  (AGlActor*, GdkModifierType);
static bool text_input_move_right (AGlActor*, GdkModifierType);
static bool text_input_line_start (AGlActor*, GdkModifierType);
static bool text_input_line_end   (AGlActor*, GdkModifierType);
static bool text_input_del_next   (AGlActor*, GdkModifierType);
static bool text_input_del_prev   (AGlActor*, GdkModifierType);
static bool text_input_activate   (AGlActor*, GdkModifierType);
static bool text_input_move_up    (AGlActor*, GdkModifierType);
static bool text_input_move_down  (AGlActor*, GdkModifierType);
static bool text_input_select_all (AGlActor*, GdkModifierType);

static PangoLayout* text_input_create_layout_no_cache (TextInput*, gint width, gint height, PangoEllipsizeMode);
static inline void  text_input_ensure_cursor_position (TextInput*);
static void         selection_paint                   (TextInput*);
static TextBuffer*  get_buffer                        (TextInput*);
static void         text_input_dirty_cache            (TextInput*);
static void         text_input_get_preferred_width    (TextInput*, gfloat for_height, gfloat* min_width_p, gfloat* natural_width_p);
static void         text_input_get_preferred_height   (TextInput*, gfloat for_height, gfloat* min_width_p, gfloat* natural_width_p);
static void         text_input_allocate               (TextInput*);
static void         text_input_real_insert_text       (TextInput*, guint start_pos, const gchar*, guint n_chars);
static void         text_input_key_focus_change       (TextInput*);
static bool         text_input_is_empty               (TextInput*);


static TextInputActorClass actor_class = {{0, "Text", (AGlActorNew*)text_input, text_input_free}};


AGlActorClass*
text_input_get_class ()
{
	static bool init_done = false;

	if(!init_done){
		agl = agl_get_instance();

		agl_actor_class__add_behaviour(&actor_class.parent, key_get_class());

		init_done = true;
	}

	return (AGlActorClass*)&actor_class;
}


/* We need at least three cached layouts to run the allocation without
 * regenerating a new layout. First the layout will be generated at
 * full width to get the preferred width, then it will be generated at
 * the preferred width to get the preferred height and then it might
 * be regenerated at a different width to get the height for the
 * actual allocated width
 *
 * since we might get multiple queries from layout managers doing a
 * double-pass allocations, like tabular ones, we should use 6 slots
 */
#define N_CACHED_LAYOUTS 6

typedef struct _LayoutCache LayoutCache;

struct _LayoutCache
{
	/* Cached layout. Pango internally caches the computed extents
	 * when they are requested so there is no need to cache that as
	 * well
	 */
	PangoLayout* layout;

	/* A number representing the age of this cache (so that when a
	 * new layout is needed the last used cache is replaced)
	 */
	guint age;
};

struct _TextInputPrivate
{
	PangoFontDescription* font_desc;

	/* the displayed text */
	TextBuffer* buffer;

	gchar* font_name;

	gchar* preedit_str;

	uint32_t text_color;

	LayoutCache cached_layouts[N_CACHED_LAYOUTS];
	guint cache_age;

	/* These are the attributes set by the attributes property */
	PangoAttrList* attrs;
	/* These are the attributes derived from the text when the
	   use-markup property is set */
	PangoAttrList* markup_attrs;
	/* This is the combination of the above two lists. It is set to NULL
	   whenever either of them changes and then regenerated by merging
	   the two lists whenever a layout is needed */
	PangoAttrList* effective_attrs;
	/* These are the attributes for the preedit string. These are merged
	   with the effective attributes into a temporary list before
	   creating a layout */
	PangoAttrList* preedit_attrs;

	/* current cursor position */
	gint position;

	/* current 'other end of selection' position */
	gint selection_bound;

	/* the x position in the PangoLayout, used to
	 * avoid drifting when repeatedly moving up|down
	 */
	gint x_pos;

	/* the x position of the PangoLayout when in
	 * single line mode, to scroll the contents of the
	 * text actor
	 */
	gint text_x;

	/* the y position of the PangoLayout, fixed to 0 by
	 * default for now */
	gint text_y;

	/* Where to draw the cursor */
	graphene_rect_t cursor_rect;
	uint32_t cursor_color;
	guint cursor_size;

	guint preedit_cursor_pos;
	gint preedit_n_chars;

	gchar* placeholder;

	uint32_t selection_color;

	uint32_t selected_text_color;

	gunichar password_char;

	guint password_hint_id;
	guint password_hint_timeout;

	/* bitfields */
	guint alignment : 2;
	guint wrap : 1;
	guint use_underline : 1;
	guint use_markup : 1;
	guint ellipsize : 3;
	guint single_line_mode : 1;
	guint wrap_mode : 3;
	guint justify : 1;
	guint editable : 1;
	guint cursor_visible : 1;
	guint activatable : 1;
	guint selectable : 1;
	guint selection_color_set : 1;
	guint in_select_drag : 1;
	guint cursor_color_is_set : 1;
	guint preedit_set : 1;
	guint is_default_font : 1;
	guint selected_text_color_set : 1;
	guint show_password_hint : 1;
	guint password_hint_visible : 1;
	guint resolved_direction : 4;
};


static const uint32_t default_cursor_color = 0xffffffff;
static const uint32_t default_selection_color = 0x008888ff;
static const uint32_t default_text_color = 0xffffffff;
static const uint32_t default_selected_text_color = 0xff99ffff;

static ActorKey keys[] = {
	{XK_Left,     text_input_move_left},
	{XK_KP_Left,  text_input_move_left},
	{XK_Right,    text_input_move_right},
	{XK_KP_Right, text_input_move_right},
	{XK_Home,     text_input_line_start},
	{XK_KP_Home,  text_input_line_start},
	{XK_KP_Begin, text_input_line_start},
	{XK_End,      text_input_line_end},
	{XK_KP_End,   text_input_line_end},
	{XK_Delete,   text_input_del_next},
	{XK_BackSpace,text_input_del_prev},
	{XK_Return,   text_input_activate},
	{XK_KP_Enter, text_input_activate},
	{XK_ISO_Enter,text_input_activate},
	{XK_Up,       text_input_move_up},
	{XK_KP_Up,    text_input_move_up},
	{XK_Down,     text_input_move_down},
	{XK_KP_Down,  text_input_move_down},
	{XK_a,        text_input_select_all},
	{0,}
};

#define KEYS(A) ((KeyBehaviour*)(A)->behaviours[0])

AGlActor*
text_input (gpointer user_data)
{
	text_input_get_class();

	TextInput* node = agl_actor__new(TextInput,
		.actor = {
			.class    = (AGlActorClass*)&actor_class,
			.name     = actor_class.parent.name,
			.set_size = text_input_layout,
			.on_event = text_input_event,
			.paint    = text_input_draw,
		},
		.font_family = "Roboto",
		.font = agl_observable_new()
	);

	int password_hint_time = 10;

	agl_observable_set (node->font, 12);

	node->priv = AGL_NEW (TextInputPrivate,
		.alignment = PANGO_ALIGN_LEFT,
		.wrap = false,
		.wrap_mode = PANGO_WRAP_WORD,
		.ellipsize = PANGO_ELLIPSIZE_NONE,
		.single_line_mode = true,
		.use_underline = false,
		.use_markup = false,
		.justify = false,

		/* default to "" so that text_input_get_text() will
		 * return a valid string and we can safely call strlen()
		 * or strcmp() on it
		 */
		.buffer = NULL,

		.text_color = default_text_color,
		.cursor_color = default_cursor_color,
		.selection_color = default_selection_color,
		.selected_text_color = default_selected_text_color,

		.font_name = g_strdup_printf("%s %i", node->font_family, node->font->value),
		.font_desc = pango_font_description_from_string (node->font_family),
		.is_default_font = true,

		.position = -1,
		.selection_bound = -1,

		.x_pos = -1,
		.cursor_visible = true,
		.editable = true,
		.selectable = true,

		.selection_color_set = false,

		.cursor_color_is_set = false,
		.selected_text_color_set = false,
		.preedit_set = false,

		.password_char = 0,
		.show_password_hint = password_hint_time > 0,
		.password_hint_timeout = password_hint_time,

		.text_y = 0,

		.cursor_size = DEFAULT_CURSOR_SIZE,
	);

	KEYS((AGlActor*)node)->keys = &keys;

	void on_font (AGlObservable* observable, int val, gpointer _node)
	{
		TextInput* node = _node;

		const gchar* font_name = g_strdup_printf("Sans %i", val);
		text_input_set_font_name (node, font_name);
		g_free((char*)font_name);
	}
	agl_observable_subscribe(node->font, on_font, node);

	return (AGlActor*)node;
}


static void
text_input_free (AGlActor* actor)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	/* get rid of the entire cache */
	text_input_dirty_cache (self);

	if (priv->password_hint_id) {
		g_source_remove (priv->password_hint_id);
		priv->password_hint_id = 0;
	}

	if (priv->font_desc)
		pango_font_description_free (priv->font_desc);

	if (priv->attrs)
		pango_attr_list_unref (priv->attrs);
	if (priv->markup_attrs)
		pango_attr_list_unref (priv->markup_attrs);
	if (priv->effective_attrs)
		pango_attr_list_unref (priv->effective_attrs);
	if (priv->preedit_attrs)
		pango_attr_list_unref (priv->preedit_attrs);

	if(priv->placeholder)
		g_free(priv->placeholder);

	text_input_set_buffer (self, NULL);
	g_free (priv->font_name);

	g_free(actor);
}


static bool
text_input_draw (AGlActor* actor)
{
	TextInput* input = (TextInput*)actor;
	TextInputPrivate* priv = input->priv;

	PangoLayout* layout = text_input_create_layout_no_cache (input, agl_actor__width(actor), agl_actor__height(actor), PANGO_ELLIPSIZE_NONE);

	agl_pango_show_layout (layout, 0, 0, 0., text_input_is_empty(input) ? 0x777777ff : priv->text_color);

	if (priv->editable && priv->cursor_visible)
		text_input_ensure_cursor_position (input);

	selection_paint (input);

	return true;
}


static void
text_input_layout (AGlActor* actor)
{
	TextInput* self = (TextInput*)actor;

	// these are not used at the moment (only single-line mode tested)
	gfloat for_height = 0, min_width_p = 0, natural_width_p = 0;
	text_input_get_preferred_width (self, for_height, &min_width_p, &natural_width_p);
	gfloat for_width = 0;
	text_input_get_preferred_height (self, for_width, &min_width_p, &natural_width_p);

	text_input_allocate (self);
}


static bool
text_input_event (AGlActor* actor, GdkEvent* event, AGliPt xy)
{
	switch(event->type){
		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
			return text_input_press (actor, event, xy);
		case GDK_MOTION_NOTIFY:
			return text_input_move ((TextInput*)actor, event, xy);
		case GDK_BUTTON_RELEASE:
			return text_input_release ((TextInput*)actor, event);

		case GDK_ENTER_NOTIFY:
		case GDK_LEAVE_NOTIFY:
			return AGL_NOT_HANDLED;

		case GDK_KEY_PRESS:
			return text_input_key_press ((TextInput*)actor, event);
		case GDK_KEY_RELEASE:
			break;

		case GDK_FOCUS_CHANGE:
			text_input_key_focus_change ((TextInput*)actor);
			break;

		default:
			break;
	}
	return AGL_NOT_HANDLED;
}


static inline void
text_input_queue_redraw (TextInput* self)
{
	agl_actor__invalidate ((AGlActor*)self);
}


static gint
offset_to_bytes (const gchar* text, gint pos)
{
	const gchar* ptr;

	if (pos < 0)
		return strlen (text);

	/* Loop over each character in the string until we either reach the
	   end or the requested position */
	for (ptr = text; *ptr && pos-- > 0; ptr = g_utf8_next_char (ptr))
		;

	return ptr - text;
}


#define bytes_to_offset(t, p) (g_utf8_pointer_to_offset ((t), (t) + (p)))

static inline void
text_input_clear_selection (TextInput* self)
{
	TextInputPrivate* priv = self->priv;

	if (priv->selection_bound != priv->position) {
		priv->selection_bound = priv->position;
		agl_actor__invalidate ((AGlActor*)self);
	}
}


static bool
text_input_is_empty (TextInput* self)
{
	if (self->priv->buffer == NULL)
		return true;

	if (agl_text_buffer_get_length (self->priv->buffer) == 0)
		return true;

	return false;
}


static gchar*
text_input_get_display_text (TextInput* self)
{
	TextInputPrivate* priv = self->priv;

	/* short-circuit the case where the buffer is unset or it's empty,
	 * to avoid creating a pointless TextBuffer and emitting
	 * notifications with it
	 */
	if (text_input_is_empty (self))
		return g_strdup(priv->placeholder ? priv->placeholder : "");

	TextBuffer* buffer = get_buffer (self);
	const gchar* text = agl_text_buffer_get_text (buffer);

	/* simple short-circuit to avoid going through GString
	 * with an empty text and a password char set
	 */
	if (text[0] == '\0')
		return g_strdup ("");

	if (G_LIKELY (priv->password_char == 0))
		return g_strdup (text);
	else {
		guint n_chars = agl_text_buffer_get_length (buffer);
		GString* str = g_string_sized_new (agl_text_buffer_get_bytes (buffer));
		gunichar invisible_char = priv->password_char;

		/* we need to convert the string built of invisible
		 * characters into UTF-8 for it to be fed to the Pango
		 * layout
		 */
		gchar buf[7];
		memset (buf, 0, sizeof (buf));
		gint char_len = g_unichar_to_utf8 (invisible_char, buf);

		if (priv->show_password_hint && priv->password_hint_visible) {

			for (int i = 0; i < n_chars - 1; i++)
				g_string_append_len (str, buf, char_len);

			char* last_char = g_utf8_offset_to_pointer (text, n_chars - 1);
			g_string_append (str, last_char);
		} else {
			for (int i = 0; i < n_chars; i++)
				g_string_append_len (str, buf, char_len);
		}

		return g_string_free (str, FALSE);
	}
}


static inline void
text_input_ensure_effective_attributes (TextInput* self)
{
	TextInputPrivate* priv = self->priv;

	/* If we already have the effective attributes then we don't need to
	   do anything */
	if (priv->effective_attrs != NULL)
		return;

	/* Same as if we don't have any attribute at all.
	 * We also ignore markup attributes for editable. */
	if (priv->attrs == NULL && (priv->editable || priv->markup_attrs == NULL))
		return;

	if (priv->attrs != NULL) {
		/* If there are no markup attributes, or if this is editable (in which
		 * case we ignore markup), then we can just use these attrs directly */
		if (priv->editable || priv->markup_attrs == NULL)
			priv->effective_attrs = pango_attr_list_ref (priv->attrs);
		else {
			/* Otherwise we need to merge the two lists */
			PangoAttrIterator* iter;
			GSList *attributes, *l;

			priv->effective_attrs = pango_attr_list_copy (priv->markup_attrs);

			iter = pango_attr_list_get_iterator (priv->attrs);
			do {
				attributes = pango_attr_iterator_get_attrs (iter);

				for (l = attributes; l != NULL; l = l->next) {
					PangoAttribute* attr = l->data;

					pango_attr_list_insert (priv->effective_attrs, attr);
				}

				g_slist_free (attributes);
			} while (pango_attr_iterator_next (iter));

			pango_attr_iterator_destroy (iter);
		}
	} else if (priv->markup_attrs != NULL) {
		/* We can just use the markup attributes directly */
		priv->effective_attrs = pango_attr_list_ref (priv->markup_attrs);
	}
}


static PangoLayout*
text_input_create_layout_no_cache (TextInput* text, gint width, gint height, PangoEllipsizeMode ellipsize)
{
	TextInputPrivate* priv = text->priv;

	PangoLayout* layout = pango_layout_new (agl_pango_get_context ());

	pango_layout_set_font_description (layout, priv->font_desc);

	gchar* contents = text_input_get_display_text (text);
	gsize contents_len = strlen (contents);

	if (priv->editable && priv->preedit_set) {
		GString* tmp = g_string_new (contents);
		PangoAttrList* tmp_attrs = pango_attr_list_new ();

		gint cursor_index = (priv->position == 0)
			? 0
			: offset_to_bytes (contents, priv->position);

		g_string_insert (tmp, cursor_index, priv->preedit_str);

		pango_layout_set_text (layout, tmp->str, tmp->len);

		if (priv->preedit_attrs != NULL) {
			pango_attr_list_splice (tmp_attrs, priv->preedit_attrs, cursor_index, strlen (priv->preedit_str));

			pango_layout_set_attributes (layout, tmp_attrs);
		}

		g_string_free (tmp, TRUE);
		pango_attr_list_unref (tmp_attrs);
	} else {

		PangoDirection pango_dir = (priv->password_char != 0)
			? PANGO_DIRECTION_NEUTRAL
			: pango_find_base_dir (contents, contents_len);

		if (pango_dir == PANGO_DIRECTION_NEUTRAL) {
			pango_dir = PANGO_DIRECTION_LTR;
		}

		pango_context_set_base_dir (agl_pango_get_context (), pango_dir);

		priv->resolved_direction = pango_dir;

		pango_layout_set_text (layout, contents, contents_len);
	}

	/* This will merge the markup attributes and the attributes
	 * property if needed */
	text_input_ensure_effective_attributes (text);

	if (priv->effective_attrs != NULL)
		pango_layout_set_attributes (layout, priv->effective_attrs);

	pango_layout_set_alignment (layout, priv->alignment);
	pango_layout_set_single_paragraph_mode (layout, priv->single_line_mode);
	pango_layout_set_justify (layout, priv->justify);
	pango_layout_set_wrap (layout, priv->wrap_mode);

	pango_layout_set_ellipsize (layout, ellipsize);
	if(priv->wrap){
		pango_layout_set_width (layout, width);
		pango_layout_set_height (layout, height);
	}

	g_free (contents);

	return layout;
}


static void
text_input_dirty_cache (TextInput* text)
{
	TextInputPrivate* priv = text->priv;

	/* Delete the cached layouts so they will be recreated the next time they are needed */
	for (int i = 0; i < N_CACHED_LAYOUTS; i++){
		if (priv->cached_layouts[i].layout) {
			g_object_unref (priv->cached_layouts[i].layout);
			priv->cached_layouts[i].layout = NULL;
		}
	}
}


/*
 * text_input_set_font_description_internal:
 * @self: a #TextInput
 * @desc: a #PangoFontDescription
 *
 * Sets @desc as the font description to be used by the #TextInput
 * actor. The #PangoFontDescription is copied.
 *
 * This function will also set the :font-name field as a side-effect
 *
 * This function will evict the layout cache, and queue a relayout if
 * the #TextInput actor has contents.
 */
static inline void
text_input_set_font_description_internal (TextInput* self, PangoFontDescription* desc, gboolean is_default_font)
{
	TextInputPrivate* priv = self->priv;

	priv->is_default_font = is_default_font;

	if (priv->font_desc == desc || pango_font_description_equal (priv->font_desc, desc))
		return;

	if (priv->font_desc != NULL)
		pango_font_description_free (priv->font_desc);

	priv->font_desc = pango_font_description_copy (desc);

	/* update the font name string we use */
	g_free (priv->font_name);
	priv->font_name = pango_font_description_to_string (priv->font_desc);

	text_input_dirty_cache (self);

	if (agl_text_buffer_get_length (get_buffer (self)))
		agl_actor__invalidate ((AGlActor*)self);
}


/*
 * text_input_create_layout:
 * @text: a #TextInput
 * @allocation_width: the allocation width
 * @allocation_height: the allocation height
 *
 * Like text_input_create_layout_no_cache(), but will also ensure
 * the glyphs cache. If a previously cached layout generated using the
 * same width is available then that will be used instead of
 * generating a new one.
 */
static PangoLayout*
text_input_create_layout (TextInput* text, gfloat allocation_width, gfloat allocation_height)
{
	TextInputPrivate* priv = text->priv;
	LayoutCache* oldest_cache = priv->cached_layouts;
	bool found_free_cache = false;
	gint width = -1;
	gint height = -1;
	PangoEllipsizeMode ellipsize = PANGO_ELLIPSIZE_NONE;

	/* First determine the width, height, and ellipsize mode that
	 * we need for the layout. The ellipsize mode depends on
	 * allocation_width/allocation_size as follows:
	 *
	 * Cases, assuming ellipsize != NONE on actor:
	 *
	 * Width request: ellipsization can be set or not on layout,
	 * doesn't matter.
	 *
	 * Height request: ellipsization must never be set on layout
	 * if wrap=true, because we need to measure the wrapped
	 * height. It must always be set if wrap=false.
	 *
	 * Allocate: ellipsization must always be set.
	 *
	 * See http://bugzilla.gnome.org/show_bug.cgi?id=560931
	 */

	if (priv->ellipsize != PANGO_ELLIPSIZE_NONE) {
		if (allocation_height < 0 && priv->wrap)
			; /* must not set ellipsization on wrap=true height request */
		else {
			if (!priv->editable)
				ellipsize = priv->ellipsize;
		}
	}

	/* When painting, we always need to set the width, since
	 * we might need to align to the right. When getting the
	 * height, however, there are some cases where we know that
	 * the width won't affect the width.
	 *
	 * - editable, single-line text actors, since those can
	 *   scroll the layout.
	 * - non-wrapping, non-ellipsizing actors.
	 */
	if (allocation_width >= 0 && (allocation_height >= 0 || !((priv->editable && priv->single_line_mode) || (priv->ellipsize == PANGO_ELLIPSIZE_NONE && !priv->wrap)))) {
		width = allocation_width * 1024 + 0.5f;
	}

	/* Pango only uses height if ellipsization is enabled, so don't set
	 * height if ellipsize isn't set. Pango implicitly enables wrapping
	 * if height is set, so don't set height if wrapping is disabled.
	 * In other words, only set height if we want to both wrap then
	 * ellipsize and we're not in single line mode.
	 *
	 * See http://bugzilla.gnome.org/show_bug.cgi?id=560931 if this
	 * seems odd.
	 */
	if (allocation_height >= 0 && priv->wrap && priv->ellipsize != PANGO_ELLIPSIZE_NONE && !priv->single_line_mode) {
		height = allocation_height * 1024 + 0.5f;
	}

	/* Search for a cached layout with the same width and keep
	 * track of the oldest one
	 */
	for (int i = 0; i < N_CACHED_LAYOUTS; i++) {
		if (priv->cached_layouts[i].layout == NULL) {
			/* Always prefer free cache spaces */
			found_free_cache = true;
			oldest_cache = priv->cached_layouts + i;
		} else {
			PangoLayout* cached = priv->cached_layouts[i].layout;
			gint cached_width = pango_layout_get_width (cached);
			gint cached_height = pango_layout_get_height (cached);
			gint cached_ellipsize = pango_layout_get_ellipsize (cached);

			if (cached_width == width && cached_height == height && cached_ellipsize == ellipsize) {
				/* If this cached layout is using the same size then we can
				 * just return that directly
				 */
				dbg (2, "TextInput: %p: cache hit for size %.2fx%.2f at %i", text, allocation_width, allocation_height, i);

				return priv->cached_layouts[i].layout;
			}

			/* When getting the preferred height for a specific width,
			 * we might be able to reuse the layout from getting the
			 * preferred width. If the width that the layout gives
			 * unconstrained is less than the width that we are using
			 * than the height will be unaffected by that width.
			 */
			if (allocation_height < 0 && cached_width == -1 && cached_ellipsize == ellipsize) {
				PangoRectangle logical_rect;

				pango_layout_get_extents (priv->cached_layouts[i].layout, NULL, &logical_rect);

				if (logical_rect.width <= width) {
					/* We've been asked for our height for the width we gave as a result
					 * of a _get_preferred_width call
					 */
					dbg(2, "TextInput: %p: cache hit for size %.2fx%.2f (unwrapped width narrower than given width)", text, allocation_width, allocation_height);

					return priv->cached_layouts[i].layout;
				}
			}

			if (!found_free_cache && (priv->cached_layouts[i].age < oldest_cache->age)) {
				oldest_cache = priv->cached_layouts + i;
			}
		}
	}

	dbg(2, "TextInput: %p: cache miss for size %.2fx%.2f", text, allocation_width, allocation_height);

	/* If we make it here then we didn't have a cached version so we
	   need to recreate the layout */
	if (oldest_cache->layout)
		g_object_unref (oldest_cache->layout);

	oldest_cache->layout = text_input_create_layout_no_cache (text, width, height, ellipsize);

	/* Mark the 'time' this cache was created and advance the time */
	oldest_cache->age = priv->cache_age++;

	return oldest_cache->layout;
}


/**
 * text_input_coords_to_position:
 * @self: a #TextInput
 * @x: the X coordinate, relative to the actor
 * @y: the Y coordinate, relative to the actor
 *
 * Retrieves the position of the character at the given coordinates.
 *
 * Return: the position of the character
 */
gint
text_input_coords_to_position (TextInput* self, gfloat x, gfloat y)
{
	/* Take any offset due to scrolling into account, and normalize
	 * the coordinates to PangoScale units
	 */
	gint px = (x - self->priv->text_x) * PANGO_SCALE;
	gint py = (y - self->priv->text_y) * PANGO_SCALE;

	gint index_;
	gint trailing;
	pango_layout_xy_to_index (text_input_get_layout (self), px, py, &index_, &trailing);

	return index_ + trailing;
}


/**
 * text_input_position_to_coords:
 * @self: a #TextInput
 * @position: position in characters
 * @x: (out): return location for the X coordinate, or %NULL
 * @y: (out): return location for the Y coordinate, or %NULL
 * @line_height: (out): return location for the line height, or %NULL
 *
 * Retrieves the coordinates of the given @position.
 *
 * Return value: %TRUE if the conversion was successful
 */
bool
text_input_position_to_coords (TextInput* self, gint position, gfloat* x, gfloat* y, gfloat* line_height)
{
	TextInputPrivate* priv = self->priv;

	gint password_char_bytes = 1;
	gint index_;

	gint n_chars = agl_text_buffer_get_length (get_buffer (self));
	if (priv->preedit_set)
		n_chars += priv->preedit_n_chars;

	if (position < -1 || position > n_chars)
		return false;

	if (priv->password_char != 0)
		password_char_bytes = g_unichar_to_utf8 (priv->password_char, NULL);

	if (position == -1) {
		if (priv->password_char == 0) {
			gsize n_bytes = agl_text_buffer_get_bytes (get_buffer (self));
			if (priv->editable && priv->preedit_set)
				index_ = n_bytes + strlen (priv->preedit_str);
			else
				index_ = n_bytes;
		} else
			index_ = n_chars * password_char_bytes;
	} else if (position == 0) {
		index_ = 0;
	} else {
		gchar* text = text_input_get_display_text (self);
		GString* tmp = g_string_new (text);
		gint cursor_index;

		cursor_index = offset_to_bytes (text, priv->position);

		if (priv->preedit_str != NULL)
			g_string_insert (tmp, cursor_index, priv->preedit_str);

		if (priv->password_char == 0)
			index_ = offset_to_bytes (tmp->str, position);
		else
			index_ = position * password_char_bytes;

		g_free (text);
		g_string_free (tmp, true);
	}

	PangoRectangle rect;
	pango_layout_get_cursor_pos (text_input_get_layout (self), index_, &rect, NULL);

	if (x) {
		*x = (gfloat)rect.x / 1024.0f;

		/* Take any offset due to scrolling into account */
		if (priv->single_line_mode)
			*x += priv->text_x;
	}

	if (y)
		*y = (gfloat)rect.y / 1024.0f;

	if (line_height)
		*line_height = (gfloat)rect.height / 1024.0f;

	return true;
}


static inline void
text_input_ensure_cursor_position (TextInput* self)
{
	TextInputPrivate* priv = self->priv;
	graphene_rect_t cursor_rect = {0,};

	gint position = priv->position;

	if (priv->editable && priv->preedit_set) {
		if (position == -1)
			position = agl_text_buffer_get_length (get_buffer (self));

		position += priv->preedit_cursor_pos;
	}

	dbg (2, "Cursor at %d (preedit %s at pos: %d)", position, priv->preedit_set ? "set" : "unset", priv->preedit_set ? priv->preedit_cursor_pos : 0);

	gfloat x = 0, cursor_height = 0, y = 0;
	text_input_position_to_coords (self, position, &x, &y, &cursor_height);

	graphene_rect_init (&cursor_rect, x, y + CURSOR_Y_PADDING, priv->cursor_size, cursor_height - 2 * CURSOR_Y_PADDING);

	if (!graphene_rect_equal (&priv->cursor_rect, &cursor_rect)) {
		priv->cursor_rect = cursor_rect;
	}
}


/**
 * text_input_delete_selection:
 * @self: a #TextInput
 *
 * Deletes the currently selected text
 *
 * This function is only useful in subclasses of #TextInput
 *
 * Return value: %TRUE if text was deleted or if the text actor
 *   is empty, and %FALSE otherwise
 */
bool
text_input_delete_selection (TextInput* self)
{
	TextInputPrivate* priv = self->priv;

	guint n_chars = agl_text_buffer_get_length (get_buffer (self));
	if (n_chars == 0)
		return true;

	gint start_index = priv->position == -1 ? n_chars : priv->position;
	gint end_index = priv->selection_bound == -1 ? n_chars : priv->selection_bound;

	if (end_index == start_index)
		return false;

	if (end_index < start_index) {
		gint temp = start_index;
		start_index = end_index;
		end_index = temp;
	}

	text_input_delete_text (self, start_index, end_index);

	priv->position = start_index;
	priv->selection_bound = start_index;

	return true;
}


/*
 * Utility function to update both cursor position and selection bound
 * at once
 */
static inline void
text_input_set_positions (TextInput* self, gint new_pos, gint new_bound)
{
	text_input_set_cursor_position (self, new_pos);
	text_input_set_selection_bound (self, new_bound);
}


static inline void
text_input_set_markup_internal (TextInput* self, const gchar* str)
{
	TextInputPrivate* priv = self->priv;

	g_assert (str);

	gchar* text = NULL;
	GError* error = NULL;
	PangoAttrList* attrs = NULL;
	bool res = pango_parse_markup (str, -1, 0, &attrs, &text, NULL, &error);

	if (!res) {
		if (G_LIKELY (error != NULL)) {
			g_warning ("Failed to set the markup of the actor '%s': %s", ((AGlActor*)self)->name, error->message);
			g_error_free (error);
		} else
			g_warning ("Failed to set the markup of the actor '%s'", ((AGlActor*)self)->name);

		return;
	}

	if (text) {
		agl_text_buffer_set_text (get_buffer (self), text, -1);
		g_free (text);
	}

	/* Store the new markup attributes */
	if (priv->markup_attrs != NULL)
		pango_attr_list_unref (priv->markup_attrs);

	priv->markup_attrs = attrs;

	/*  Clear the effective attributes so they will be regenerated when a
	 *  layout is created
	 */
	if (priv->effective_attrs != NULL) {
		pango_attr_list_unref (priv->effective_attrs);
		priv->effective_attrs = NULL;
	}
}


typedef void (*TextInputSelectionFunc) (TextInput* text, const AGlfRegion* box, gpointer user_data);

static void
text_input_foreach_selection_rectangle (TextInput* self, TextInputSelectionFunc func, gpointer user_data)
{
	TextInputPrivate* priv = self->priv;
	PangoLayout* layout = text_input_get_layout (self);
	gchar* utf8 = text_input_get_display_text (self);

	gint start_index;
	if (priv->position == 0)
		start_index = 0;
	else
		start_index = offset_to_bytes (utf8, priv->position);

	gint end_index = (priv->selection_bound == 0)
		? 0
		: offset_to_bytes (utf8, priv->selection_bound);

	if (start_index > end_index) {
		gint temp = start_index;
		start_index = end_index;
		end_index = temp;
	}

	gint lines = pango_layout_get_line_count (layout);

	for (gint line_no = 0; line_no < lines; line_no++) {
		gint n_ranges;
		gint* ranges;
		gint index_;
		gint maxindex;
		gfloat y, height;

		PangoLayoutLine* line = pango_layout_get_line_readonly (layout, line_no);
		pango_layout_line_x_to_index (line, G_MAXINT, &maxindex, NULL);
		if (maxindex < start_index)
			continue;

		pango_layout_line_get_x_ranges (line, start_index, end_index, &ranges, &n_ranges);
		pango_layout_line_x_to_index (line, 0, &index_, NULL);

		text_input_position_to_coords (self, bytes_to_offset (utf8, index_), NULL, &y, &height);

		AGlfRegion box = {
			.y1 = y,
			.y2 = y + height
		};

		for (int i = 0; i < n_ranges; i++) {
			gfloat range_x = ranges[i * 2] / PANGO_SCALE;

			/* Account for any scrolling in single line mode */
			if (priv->single_line_mode)
				range_x += priv->text_x;

			gfloat range_width = ((gfloat)ranges[i * 2 + 1] - (gfloat)ranges[i * 2]) / PANGO_SCALE;

			box.x1 = range_x;
			box.x2 = ceilf (range_x + range_width + .5f);

			func (self, &box, user_data);
		}

		g_free (ranges);
	}

	g_free (utf8);
}


static void
add_selection_rectangle_to_path (TextInput* text, const AGlfRegion* box, gpointer user_data)
{
	AGlRect* rect = (AGlRect*)user_data;
	*rect = (AGlRect){
		box->x1,
		box->y1,
		box->x2 - box->x1,
		box->y2 - box->y1
	};

	agl_rect (
		box->x1,
		box->y1,
		box->x2 - box->x1,
		box->y2 - box->y1
	);
}

/* Draws the selected text, its background, and the cursor */
static void
selection_paint (TextInput* self)
{
	TextInputPrivate* priv = self->priv;
	AGlActor* actor = (AGlActor*)self;

	if (actor->root->selected != actor)
		return;

	if (priv->editable && priv->cursor_visible) {
		uint32_t color;
		gint position = priv->position;

		if (position == priv->selection_bound) {
			/* No selection, just draw the cursor */
			if (priv->cursor_color_is_set)
				color = priv->cursor_color;
			else
				color = priv->text_color;

			agl->shaders.plain->uniform.colour = color;
			agl_use_program((AGlShader*)agl->shaders.plain);

			agl_rect (
				priv->cursor_rect.origin.x,
				priv->cursor_rect.origin.y,
				priv->cursor_rect.size.width,
				priv->cursor_rect.size.height
			);
		} else {
			/* Paint selection background */
			if (priv->selection_color_set)
				color = priv->selection_color;
			else if (priv->cursor_color_is_set)
				color = priv->cursor_color;
			else
				color = priv->text_color;

			agl->shaders.plain->uniform.colour = color;
			agl_use_program((AGlShader*)agl->shaders.plain);

			AGlRect rect = {0,}; // single rect will only work for single line
			text_input_foreach_selection_rectangle (self, add_selection_rectangle_to_path, &rect);

			if (priv->selected_text_color_set)
				color = priv->selected_text_color;
			else
				color = priv->text_color;

			PangoLayout* layout = text_input_get_layout (self);

			agl_push_clip(rect.x, rect.y, rect.w, rect.h);
			agl_pango_show_layout (layout, 0, 0, 0., priv->text_color);
			agl_pop_clip();
		}
	}
}


static gint
text_input_move_word_backward (TextInput* self, gint start)
{
	gint retval = start;

	if (agl_text_buffer_get_length (get_buffer (self)) > 0 && start > 0) {
		PangoLayout* layout = text_input_get_layout (self);
		PangoLogAttr* log_attrs = NULL;
		gint n_attrs = 0;

		pango_layout_get_log_attrs (layout, &log_attrs, &n_attrs);

		retval = start - 1;
		while (retval > 0 && !log_attrs[retval].is_word_start)
			retval -= 1;

		g_free (log_attrs);
	}

	return retval;
}


static gint
text_input_move_word_forward (TextInput* self, gint start)
{
	gint retval = start;
	guint n_chars;

	n_chars = agl_text_buffer_get_length (get_buffer (self));
	if (n_chars > 0 && start < n_chars) {
		PangoLayout* layout = text_input_get_layout (self);
		PangoLogAttr* log_attrs = NULL;
		gint n_attrs = 0;

		pango_layout_get_log_attrs (layout, &log_attrs, &n_attrs);

		retval = start + 1;
		while (retval < n_chars && !log_attrs[retval].is_word_end)
			retval += 1;

		g_free (log_attrs);
	}

	return retval;
}


static gint
text_input_move_line_start (TextInput* self, gint start)
{
	gint line_no;
	gint index_;

	PangoLayout* layout = text_input_get_layout (self);
	const gchar* text = agl_text_buffer_get_text (get_buffer (self));

	if (start == 0)
		index_ = 0;
	else
		index_ = offset_to_bytes (text, start);

	pango_layout_index_to_line_x (layout, index_, 0, &line_no, NULL);

	PangoLayoutLine* layout_line;
	layout_line = pango_layout_get_line_readonly (layout, line_no);
	if (!layout_line)
		return false;

	pango_layout_line_x_to_index (layout_line, 0, &index_, NULL);

	return bytes_to_offset (text, index_);
}


static gint
text_input_move_line_end (TextInput* self, gint start)
{
	TextInputPrivate* priv = self->priv;

	PangoLayout* layout = text_input_get_layout (self);
	const gchar* text = agl_text_buffer_get_text (get_buffer (self));

	gint index_;
	if (start == 0)
		index_ = 0;
	else
		index_ = offset_to_bytes (text, priv->position);

	gint line_no;
	pango_layout_index_to_line_x (layout, index_, 0, &line_no, NULL);

	PangoLayoutLine* layout_line = pango_layout_get_line_readonly (layout, line_no);
	if (!layout_line)
		return FALSE;

	gint trailing;
	pango_layout_line_x_to_index (layout_line, G_MAXINT, &index_, &trailing);
	index_ += trailing;

	return bytes_to_offset (text, index_);
}


static void
text_input_select_word (TextInput* self)
{
	gint cursor_pos = self->priv->position;

	gint start_pos = text_input_move_word_backward (self, cursor_pos);
	gint end_pos = text_input_move_word_forward (self, cursor_pos);

	text_input_set_selection (self, start_pos, end_pos);
}


static void
text_input_select_line (TextInput* self)
{
	TextInputPrivate* priv = self->priv;
	gint cursor_pos = priv->position;
	gint start_pos, end_pos;

	if (priv->single_line_mode) {
		start_pos = 0;
		end_pos = -1;
	} else {
		start_pos = text_input_move_line_start (self, cursor_pos);
		end_pos = text_input_move_line_end (self, cursor_pos);
	}

	text_input_set_selection (self, start_pos, end_pos);
}


static bool
text_input_press (AGlActor* actor, GdkEvent* event, AGliPt xy)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	/* if a TextInput is just used for display purposes, then we
	 * should ignore the events we receive
	 */
	if (!priv->editable)
		return AGL_NOT_HANDLED;

	actor->root->selected = actor;

	/* if the actor is empty we just reset everything and not
	 * set up the dragging of the selection since there's nothing
	 * to select
	 */
	if (agl_text_buffer_get_length (get_buffer (self)) == 0) {
		text_input_set_positions (self, -1, -1);

		return AGL_HANDLED;
	}

	gint index_ = text_input_coords_to_position (self, xy.x, xy.y);
	const char* text = agl_text_buffer_get_text (get_buffer (self));
	int offset = bytes_to_offset (text, index_);

	/* what we select depends on the number of button clicks we
	 * receive, and whether we are selectable:
	 *
	 *   1: just position the cursor and the selection
	 *   2: select the current word
	 *   3: select the contents of the whole actor
	 */
	switch(event->type){
		case GDK_BUTTON_PRESS:
			text_input_set_positions (self, offset, offset);
			break;
		case GDK_2BUTTON_PRESS:
			if (priv->selectable)
				text_input_select_word (self);
			break;
		case GDK_3BUTTON_PRESS:
			if (priv->selectable)
				text_input_select_line (self);
			break;
		default:
			/* touch events do not have click count */
			text_input_set_positions (self, offset, offset);
			break;
	}

	/* we don't need to go any further if we're not selectable */
	if (!priv->selectable)
		return AGL_HANDLED;

	/* grab the pointer */
	priv->in_select_drag = true;

	if (event->type == GDK_BUTTON_PRESS) {
		agl_actor__grab (actor);
	}

	return AGL_HANDLED;
}


static bool
text_input_move (TextInput* self, GdkEvent* event, AGliPt xy)
{
	TextInputPrivate* priv = self->priv;

	if (!priv->in_select_drag)
		return AGL_NOT_HANDLED;

	gint index_ = text_input_coords_to_position (self, xy.x, xy.y);
	const gchar* text = agl_text_buffer_get_text (get_buffer (self));
	gint offset = bytes_to_offset (text, index_);

	if (priv->selectable)
		text_input_set_cursor_position (self, offset);
	else
		text_input_set_positions (self, offset, offset);

	return AGL_HANDLED;
}


static bool
text_input_release (TextInput* self, GdkEvent* event)
{
	TextInputPrivate* priv = self->priv;

	if (priv->in_select_drag) {
		if (event->type == GDK_BUTTON_RELEASE) {
			agl_actor__grab (NULL);
			priv->in_select_drag = false;

			return AGL_HANDLED;
		}
	}

	return AGL_NOT_HANDLED;
}


static gboolean
text_input_remove_password_hint (gpointer data)
{
	TextInput* self = data;

	self->priv->password_hint_visible = FALSE;
	self->priv->password_hint_id = 0;

	text_input_dirty_cache (data);
	text_input_queue_redraw (data);

	return G_SOURCE_REMOVE;
}


static bool
text_input_key_press (TextInput* self, GdkEvent* event)
{
	TextInputPrivate* priv = self->priv;
	GdkEventKey* e = (GdkEventKey*)event;

	if (!priv->editable)
		return AGL_NOT_HANDLED;

	if(e->keyval == XK_Escape)
		return AGL_NOT_HANDLED;

	if ((e->state & GDK_CONTROL_MASK) == 0) {
		/* truncate the eventual selection so that the
		 * new character can replace it
		 */
		text_input_delete_selection (self);

		char buffer[8] = {e->keyval,};
		text_input_real_insert_text (self, priv->position, buffer, 1);

		if (priv->show_password_hint) {
			if (priv->password_hint_id != 0)
				g_source_remove (priv->password_hint_id);

			priv->password_hint_visible = true;
			priv->password_hint_id = g_timeout_add (priv->password_hint_timeout, text_input_remove_password_hint, self);
		}

		return AGL_HANDLED;
	}

	return AGL_NOT_HANDLED;
}


static void
text_input_get_preferred_width (TextInput* self, gfloat for_height, gfloat* min_width_p, gfloat* natural_width_p)
{
	TextInputPrivate* priv = self->priv;
	PangoRectangle logical_rect = {0,};

	PangoLayout* layout = text_input_create_layout (self, -1, -1);

	pango_layout_get_extents (layout, NULL, &logical_rect);

	/* the X coordinate of the logical rectangle might be non-zero
	 * according to the Pango documentation; hence, we need to offset
	 * the width accordingly
	 */
	gint logical_width = logical_rect.x + logical_rect.width;

	gfloat layout_width = logical_width > 0 ? ceilf (logical_width / 1024.0f) : 1;

	if (min_width_p) {
		if (priv->wrap || priv->ellipsize || priv->editable)
			*min_width_p = 1;
		else
			*min_width_p = layout_width;
	}

	if (natural_width_p) {
		if (priv->editable && priv->single_line_mode)
			*natural_width_p = layout_width + TEXT_PADDING * 2;
		else
			*natural_width_p = layout_width;
	}
}


static void
text_input_get_preferred_height (TextInput* self, gfloat for_width, gfloat* min_height_p, gfloat* natural_height_p)
{
	TextInputPrivate* priv = self->priv;

	if (for_width == 0) {
		if (min_height_p)
			*min_height_p = 0;

		if (natural_height_p)
			*natural_height_p = 0;
	} else {
		PangoLayout* layout;
		PangoRectangle logical_rect = {
		    0,
		};
		gint logical_height;
		gfloat layout_height;

		if (priv->single_line_mode)
			for_width = -1;

		layout = text_input_create_layout (self, for_width, -1);

		pango_layout_get_extents (layout, NULL, &logical_rect);

		/* the Y coordinate of the logical rectangle might be non-zero
		 * according to the Pango documentation; hence, we need to offset
		 * the height accordingly
		 */
		logical_height = logical_rect.y + logical_rect.height;
		layout_height = ceilf (logical_height / 1024.0f);

		if (min_height_p) {
			/* if we wrap and ellipsize then the minimum height is
			 * going to be at least the size of the first line
			 */
			if ((priv->ellipsize && priv->wrap) && !priv->single_line_mode) {
				PangoLayoutLine* line;
				gfloat line_height;

				line = pango_layout_get_line_readonly (layout, 0);
				pango_layout_line_get_extents (line, NULL, &logical_rect);

				logical_height = logical_rect.y + logical_rect.height;
				line_height = ceilf (logical_height / 1024.0f);

				*min_height_p = line_height;
			} else
				*min_height_p = layout_height;
		}

		if (natural_height_p)
			*natural_height_p = layout_height;
	}
}


static void
text_input_allocate (TextInput* text)
{
	/* Ensure that there is a cached layout with the right width so
	 * that we don't need to create the text during the paint run
	 *
	 * if the Text is editable and in single line mode we don't want
	 * to have any limit on the layout size, since the paint will clip
	 * it to the allocation of the actor
	 */
	if (text->priv->editable && text->priv->single_line_mode)
		text_input_create_layout (text, -1, -1);
	else
		text_input_create_layout (text, agl_actor__width((AGlActor*)text), agl_actor__height((AGlActor*)text));
}


static void
text_input_key_focus_change (TextInput* self)
{
	agl_actor__invalidate ((AGlActor*)self);
}


static bool
text_input_move_left (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	gint pos = priv->position;
	gint new_pos = 0;

	gint len = agl_text_buffer_get_length (get_buffer (self));

	if (pos != 0 && len != 0) {
		if (modifiers & GDK_CONTROL_MASK) {
			if (pos == -1)
				new_pos = text_input_move_word_backward (self, len);
			else
				new_pos = text_input_move_word_backward (self, pos);
		} else {
			if (pos == -1)
				new_pos = len - 1;
			else
				new_pos = pos - 1;
		}

		text_input_set_cursor_position (self, new_pos);
	}

	if (!(priv->selectable && (modifiers & GDK_SHIFT_MASK)))
		text_input_clear_selection (self);

	return true;
}


static bool
text_input_move_right (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	gint pos = priv->position;
	gint len = agl_text_buffer_get_length (get_buffer (self));
	gint new_pos = 0;

	if (pos != -1 && len != 0) {
		if (modifiers & GDK_CONTROL_MASK) {
			if (pos != len)
				new_pos = text_input_move_word_forward (self, pos);
		} else {
			if (pos != len)
				new_pos = pos + 1;
		}

		text_input_set_cursor_position (self, new_pos);
	}

	if (!(priv->selectable && (modifiers & GDK_SHIFT_MASK)))
		text_input_clear_selection (self);

	return true;
}


static bool
text_input_move_up (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	PangoLayout* layout = text_input_get_layout (self);
	const gchar* text = agl_text_buffer_get_text (get_buffer (self));

	gint index_ = (priv->position == 0)
		? 0
		: offset_to_bytes (text, priv->position);

	gint line_no, x;
	pango_layout_index_to_line_x (layout, index_, 0, &line_no, &x);

	line_no -= 1;
	if (line_no < 0)
		return false;

	if (priv->x_pos != -1)
		x = priv->x_pos;

	PangoLayoutLine* layout_line = pango_layout_get_line_readonly (layout, line_no);
	if (!layout_line)
		return false;

	gint trailing;
	pango_layout_line_x_to_index (layout_line, x, &index_, &trailing);

	gint pos = bytes_to_offset (text, index_);
	text_input_set_cursor_position (self, pos + trailing);

	/* Store the target x position to avoid drifting left and right when
	   moving the cursor up and down */
	priv->x_pos = x;

	if (!(priv->selectable && (modifiers & GDK_SHIFT_MASK)))
		text_input_clear_selection (self);

	return true;
}


static bool
text_input_move_down (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	PangoLayout* layout = text_input_get_layout (self);
	const gchar* text = agl_text_buffer_get_text (get_buffer (self));

	gint index_ = (priv->position == 0) ? 0 : offset_to_bytes (text, priv->position);

	gint x, line_no;
	pango_layout_index_to_line_x (layout, index_, 0, &line_no, &x);

	if (priv->x_pos != -1)
		x = priv->x_pos;

	PangoLayoutLine* layout_line = pango_layout_get_line_readonly (layout, line_no + 1);
	if (!layout_line)
		return false;

	gint trailing;
	pango_layout_line_x_to_index (layout_line, x, &index_, &trailing);

	gint pos = bytes_to_offset (text, index_);
	text_input_set_cursor_position (self, pos + trailing);

	/* Store the target x position to avoid drifting left and right when
	   moving the cursor up and down */
	priv->x_pos = x;

	if (!(priv->selectable && (modifiers & GDK_SHIFT_MASK)))
		text_input_clear_selection (self);

	return true;
}


static bool
text_input_line_start (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	gint position = text_input_move_line_start (self, priv->position);
	text_input_set_cursor_position (self, position);

	if (!(priv->selectable && (modifiers & GDK_SHIFT_MASK)))
		text_input_clear_selection (self);

	return true;
}


static bool
text_input_line_end (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	gint position = text_input_move_line_end (self, priv->position);
	text_input_set_cursor_position (self, position);

	if (!(priv->selectable && (modifiers & GDK_SHIFT_MASK)))
		text_input_clear_selection (self);

	return true;
}


static bool
text_input_select_all (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;

	if (modifiers & GDK_CONTROL_MASK){
		guint n_chars = agl_text_buffer_get_length (get_buffer (self));
		text_input_set_positions (self, 0, n_chars);

		return AGL_HANDLED;
	}
	return AGL_NOT_HANDLED;
}


static bool
text_input_real_del_word_next (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	gint pos = priv->position;
	gint len = agl_text_buffer_get_length (get_buffer (self));

	if (len && pos != -1 && pos < len) {
		gint end = text_input_move_word_forward (self, pos);
		text_input_delete_text (self, pos, end);

		if (priv->selection_bound >= end) {
			gint new_bound;

			new_bound = priv->selection_bound - (end - pos);
			text_input_set_selection_bound (self, new_bound);
		} else if (priv->selection_bound > pos) {
			text_input_set_selection_bound (self, pos);
		}
	}

	return true;
}


static bool
text_input_del_next (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	if (text_input_delete_selection (self))
		return true;

	if(modifiers & GDK_CONTROL_MASK){
		return text_input_real_del_word_next (actor, modifiers);
	}

	gint pos = priv->position;
	gint len = agl_text_buffer_get_length (get_buffer (self));

	if (len && pos != -1 && pos < len)
		text_input_delete_text (self, pos, pos + 1);

	return true;
}


static bool
text_input_real_del_word_prev (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	gint pos = priv->position;
	gint len = agl_text_buffer_get_length (get_buffer (self));

	if (pos != 0 && len != 0) {
		gint new_pos;

		if (pos == -1) {
			new_pos = text_input_move_word_backward (self, len);
			text_input_delete_text (self, new_pos, len);

			text_input_set_positions (self, -1, -1);
		} else {
			new_pos = text_input_move_word_backward (self, pos);
			text_input_delete_text (self, new_pos, pos);

			text_input_set_cursor_position (self, new_pos);
			if (priv->selection_bound >= pos) {
				gint new_bound;

				new_bound = priv->selection_bound - (pos - new_pos);
				text_input_set_selection_bound (self, new_bound);
			} else if (priv->selection_bound >= new_pos) {
				text_input_set_selection_bound (self, new_pos);
			}
		}
	}

	return true;
}


static bool
text_input_del_prev (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	if (text_input_delete_selection (self))
		return true;

	if(modifiers & GDK_CONTROL_MASK){
		return text_input_real_del_word_prev (actor, modifiers);
	}

	gint pos = priv->position;
	gint len = agl_text_buffer_get_length (get_buffer (self));

	if (pos != 0 && len != 0) {
		if (pos == -1) {
			text_input_delete_text (self, len - 1, len);

			text_input_set_positions (self, -1, -1);
		} else {
			text_input_delete_text (self, pos - 1, pos);

			text_input_set_positions (self, pos - 1, pos - 1);
		}
	}

	return true;
}


static bool
text_input_activate (AGlActor* actor, GdkModifierType modifiers)
{
	TextInput* self = (TextInput*)actor;
	TextInputPrivate* priv = self->priv;

	if (priv->activatable) {
		//g_signal_emit (self, text_signals[ACTIVATE], 0);
		return true;
	}

	return false;
}


static TextBuffer*
get_buffer (TextInput* self)
{
	TextInputPrivate* priv = self->priv;

	if (!priv->buffer) {
		TextBuffer* buffer = agl_text_buffer_new ();
		text_input_set_buffer (self, buffer);
		g_object_unref (buffer);
	}

	return priv->buffer;
}


/*
 *  TextBuffer signal handlers
 */
static void
buffer_inserted_text (TextBuffer* buffer, guint position, const gchar* chars, guint n_chars, TextInput* self)
{
	TextInputPrivate* priv = self->priv;

	if (priv->position >= 0 || priv->selection_bound >= 0) {
		gint new_position = priv->position;
		gint new_selection_bound = priv->selection_bound;

		if (position <= new_position)
			new_position += n_chars;
		if (position <= new_selection_bound)
			new_selection_bound += n_chars;

		if (priv->position != new_position || priv->selection_bound != new_selection_bound)
			text_input_set_positions (self, new_position, new_selection_bound);
	}

	/* TODO: What are we supposed to with the out value of position? */
}


static void
buffer_deleted_text (TextBuffer* buffer, guint position, guint n_chars, TextInput* self)
{
	TextInputPrivate* priv = self->priv;

	if (priv->position >= 0 || priv->selection_bound >= 0) {
		gint new_position = priv->position;
		gint new_selection_bound = priv->selection_bound;

		if (position < new_position)
			new_position -= n_chars;
		if (position < new_selection_bound)
			new_selection_bound -= n_chars;

		if (priv->position != new_position || priv->selection_bound != new_selection_bound)
			text_input_set_positions (self, new_position, new_selection_bound);
	}
}


static void
buffer_notify_text (TextBuffer* buffer, GParamSpec* spec, TextInput* self)
{
	TextInputPrivate* priv = self->priv;

	text_input_dirty_cache (self);

	if(!priv->single_line_mode){
		dbg(0, "TODO do layout");
	}

	agl_actor__invalidate((AGlActor*)self);
}


static void
buffer_notify_max_length (TextBuffer* buffer, GParamSpec* spec, TextInput* self)
{
}


static void
buffer_connect_signals (TextInput* self)
{
	TextInputPrivate* priv = self->priv;
	g_signal_connect (priv->buffer, "inserted-text", G_CALLBACK (buffer_inserted_text), self);
	g_signal_connect (priv->buffer, "deleted-text", G_CALLBACK (buffer_deleted_text), self);
	g_signal_connect (priv->buffer, "notify::text", G_CALLBACK (buffer_notify_text), self);
	g_signal_connect (priv->buffer, "notify::max-length", G_CALLBACK (buffer_notify_max_length), self);
}


static void
buffer_disconnect_signals (TextInput* self)
{
	TextInputPrivate* priv = self->priv;
	g_signal_handlers_disconnect_by_func (priv->buffer, buffer_inserted_text, self);
	g_signal_handlers_disconnect_by_func (priv->buffer, buffer_deleted_text, self);
	g_signal_handlers_disconnect_by_func (priv->buffer, buffer_notify_text, self);
	g_signal_handlers_disconnect_by_func (priv->buffer, buffer_notify_max_length, self);
}


/**
 * text_input_get_buffer:
 * @self: a #TextInput
 *
 * Get the #TextBuffer object which holds the text for
 * this widget.
 *
 * Returns: (transfer none): A #GtkEntryBuffer object.
 */
TextBuffer*
text_input_get_buffer (TextInput* self)
{
	g_return_val_if_fail (self, NULL);

	return get_buffer (self);
}


/**
 * text_input_set_buffer:
 * @self: a #TextInput
 * @buffer: a #TextBuffer
 *
 * Set the #TextBuffer object which holds the text for
 * this widget.
 */
void
text_input_set_buffer (TextInput* self, TextBuffer* buffer)
{
	TextInputPrivate* priv = self->priv;

	if (buffer) {
		g_return_if_fail (AGL_IS_TEXT_BUFFER (buffer));
		g_object_ref (buffer);
	}

	if (priv->buffer) {
		buffer_disconnect_signals (self);
		g_object_unref (priv->buffer);
	}

	priv->buffer = buffer;

	if (priv->buffer)
		buffer_connect_signals (self);
}


/**
 * text_input_set_editable:
 * @self: a #TextInput
 * @editable: whether the #TextInput should be editable
 *
 * Sets whether the #TextInput actor should be editable.
 *
 * An editable #TextInput with key focus
 * will receive key events and will update its contents accordingly.
 */
void
text_input_set_editable (TextInput* self, bool editable)
{
	TextInputPrivate* priv = self->priv;

	g_return_if_fail (self);

	if (priv->editable != editable) {
		priv->editable = editable;

		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_editable:
 * @self: a #TextInput
 *
 * Retrieves whether a #TextInput is editable or not.
 *
 * Return value: %TRUE if the actor is editable
 */
bool
text_input_get_editable (TextInput* self)
{
	g_return_val_if_fail (self, false);

	return self->priv->editable;
}


/**
 * text_input_set_selectable:
 * @self: a #TextInput
 * @selectable: whether the #TextInput actor should be selectable
 *
 * Sets whether a #TextInput actor should be selectable.
 *
 * A selectable #TextInput will allow selecting its contents using
 * the pointer or the keyboard.
 */
void
text_input_set_selectable (TextInput* self, bool selectable)
{
	TextInputPrivate* priv = self->priv;

	if (priv->selectable != selectable) {
		priv->selectable = selectable;

		text_input_queue_redraw (self);
	}
}


/**
 * text_input_get_selectable:
 * @self: a #TextInput
 *
 * Retrieves whether a #TextInput is selectable or not.
 *
 * Return value: %TRUE if the actor is selectable
 */
bool
text_input_get_selectable (TextInput* self)
{
	return self->priv->selectable;
}


/**
 * text_input_set_activatable:
 * @self: a #TextInput
 * @activatable: whether the #TextInput actor should be activatable
 *
 * Sets whether a #TextInput actor should be activatable.
 *
 * An activatable #TextInput actor will emit the #TextInput::activate
 * signal whenever the 'Enter' (or 'Return') key is pressed; if it is not
 * activatable, a new line will be appended to the current content.
 *
 * An activatable #TextInput must also be set as editable using
 * text_input_set_editable().
 */
void
text_input_set_activatable (TextInput* self, bool activatable)
{
	TextInputPrivate* priv = self->priv;

	g_return_if_fail (self);

	if (priv->activatable != activatable) {
		priv->activatable = activatable;

		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_activatable:
 * @self: a #TextInput
 *
 * Retrieves whether a #TextInput is activatable or not.
 *
 * Return value: %TRUE if the actor is activatable
 */
bool
text_input_get_activatable (TextInput* self)
{
	g_return_val_if_fail (self, true);

	return self->priv->activatable;
}


/**
 * text_input_set_cursor_visible:
 * @self: a #TextInput
 * @cursor_visible: whether the cursor should be visible
 *
 * Sets whether the cursor of a #TextInput actor should be
 * visible or not.
 *
 * The color of the cursor will be the same as the text color
 * unless text_input_set_cursor_color() has been called.
 *
 * The size of the cursor can be set using text_input_set_cursor_size().
 *
 * The position of the cursor can be changed programmatically using
 * text_input_set_cursor_position().
 */
void
text_input_set_cursor_visible (TextInput* self, bool cursor_visible)
{
	TextInputPrivate* priv = self->priv;

	g_return_if_fail (self);

	if (priv->cursor_visible != cursor_visible) {
		priv->cursor_visible = cursor_visible;

		text_input_dirty_cache (self);
		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_cursor_visible:
 * @self: a #TextInput
 *
 * Retrieves whether the cursor of a #TextInput actor is visible.
 *
 * Return value: %TRUE if the cursor is visible
 */
bool
text_input_get_cursor_visible (TextInput* self)
{
	g_return_val_if_fail (self, true);

	return self->priv->cursor_visible;
}


/**
 * text_input_set_cursor_color:
 * @self: a #TextInput
 * @color: (allow-none): the color of the cursor, or %NULL to unset it
 *
 * Sets the color of the cursor of a #TextInput actor.
 *
 * If @color is %NULL, the cursor color will be the same as the
 * text color.
 */
void
text_input_set_cursor_color (TextInput* self, uint32_t color)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	if (color) {
		priv->cursor_color = color;
		priv->cursor_color_is_set = true;
	} else
		priv->cursor_color_is_set = false;
}


/**
 * text_input_get_cursor_color:
 * @self: a #TextInput
 *
 * Retrieves the color of the cursor of a #TextInput actor.
 */
uint32_t
text_input_get_cursor_color (TextInput* self)
{
	g_return_val_if_fail (self, 0);

	TextInputPrivate* priv = self->priv;

	return priv->cursor_color;
}


/**
 * text_input_set_selection:
 * @self: a #TextInput
 * @start_pos: start of the selection, in characters
 * @end_pos: end of the selection, in characters
 *
 * Selects the region of text between @start_pos and @end_pos.
 *
 * This function changes the position of the cursor to match
 * @start_pos and the selection bound to match @end_pos.
 */
void
text_input_set_selection (TextInput* self, gssize start_pos, gssize end_pos)
{
	guint n_chars = agl_text_buffer_get_length (get_buffer (self));
	if (end_pos < 0)
		end_pos = n_chars;

	start_pos = MIN (n_chars, start_pos);
	end_pos = MIN (n_chars, end_pos);

	text_input_set_positions (self, start_pos, end_pos);
}


/**
 * text_input_get_selection:
 * @self: a #TextInput
 *
 * Retrieves the currently selected text.
 *
 * Return value: a newly allocated string containing the currently
 *   selected text, or %NULL. Use g_free() to free the returned
 *   string.
 */
gchar*
text_input_get_selection (TextInput* self)
{
	g_return_val_if_fail (self, NULL);

	TextInputPrivate* priv = self->priv;

	gint start_index = priv->position;
	gint end_index = priv->selection_bound;

	if (end_index == start_index)
		return g_strdup ("");

	if ((end_index != -1 && end_index < start_index) || start_index == -1) {
		gint temp = start_index;
		start_index = end_index;
		end_index = temp;
	}

	const gchar* text = agl_text_buffer_get_text (get_buffer (self));
	gint start_offset = offset_to_bytes (text, start_index);
	gint end_offset = offset_to_bytes (text, end_index);
	gint len = end_offset - start_offset;

	gchar* str = g_malloc (len + 1);
	g_utf8_strncpy (str, text + start_offset, end_index - start_index);

	return str;
}


/**
 * text_input_set_selection_bound:
 * @self: a #TextInput
 * @selection_bound: the position of the end of the selection, in characters
 *
 * Sets the other end of the selection, starting from the current
 * cursor position.
 *
 * If @selection_bound is -1, the selection unset.
 */
void
text_input_set_selection_bound (TextInput* self, gint selection_bound)
{
	TextInputPrivate* priv = self->priv;

	if (priv->selection_bound != selection_bound) {
		gint len = agl_text_buffer_get_length (get_buffer (self));
		;

		if (selection_bound < 0 || selection_bound >= len)
			priv->selection_bound = -1;
		else
			priv->selection_bound = selection_bound;

		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_selection_bound:
 * @self: a #TextInput
 *
 * Retrieves the other end of the selection of a #TextInput actor,
 * in characters from the current cursor position.
 *
 * Return value: the position of the other end of the selection
 */
gint
text_input_get_selection_bound (TextInput* self)
{
	g_return_val_if_fail (self, -1);

	return self->priv->selection_bound;
}

/**
 * text_input_set_selection_color:
 * @self: a #TextInput
 * @color: (allow-none): the color of the selection, or %NULL to unset it
 *
 * Sets the color of the selection of a #TextInput actor.
 *
 * If @color is %NULL, the selection color will be the same as the
 * cursor color, or if no cursor color is set either then it will be
 * the same as the text color.
 */
void
text_input_set_selection_color (TextInput* self, uint32_t color)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	if (color) {
		priv->selection_color = color;
		priv->selection_color_set = TRUE;
	} else
		priv->selection_color_set = FALSE;
}


/**
 * text_input_get_selection_color:
 * @self: a #TextInput
 *
 * Retrieves the color of the selection of a #TextInput actor.
 */
uint32_t
text_input_get_selection_color (TextInput* self)
{

	g_return_val_if_fail (self, 0);

	TextInputPrivate* priv = self->priv;

	return priv->selection_color;
}


/**
 * text_input_set_selected_text_color:
 * @self: a #TextInput
 * @color: (allow-none): the selected text color, or %NULL to unset it
 *
 * Sets the selected text color of a #TextInput actor.
 *
 * If @color is %NULL, the selected text color will be the same as the
 * selection color, which then falls back to cursor, and then text color.
 */
void
text_input_set_selected_text_color (TextInput* self, uint32_t color)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	if (color) {
		priv->selected_text_color = color;
		priv->selected_text_color_set = true;
	} else
		priv->selected_text_color_set = false;

	agl_actor__invalidate ((AGlActor*)self);
}


/**
 * text_input_get_selected_text_color:
 * @self: a #TextInput
 *
 * Retrieves the color of selected text of a #TextInput actor.
 */
uint32_t
text_input_get_selected_text_color (TextInput* self)
{
	g_return_val_if_fail (self, 0);

	return self->priv->selected_text_color;
}


/**
 * text_input_set_font_description:
 * @self: a #TextInput
 * @font_desc: a #PangoFontDescription
 *
 * Sets @font_desc as the font description for a #TextInput
 *
 * The #PangoFontDescription is copied by the #TextInput actor
 * so you can safely call pango_font_description_free() on it after
 * calling this function.
 */
void
text_input_set_font_description (TextInput* self, PangoFontDescription* font_desc)
{
	g_return_if_fail (self);

	text_input_set_font_description_internal (self, font_desc, font_desc == NULL);
}


/**
 * text_input_get_font_description:
 * @self: a #TextInput
 *
 * Retrieves the #PangoFontDescription used by @self
 *
 * Return value: a #PangoFontDescription. The returned value is owned
 *   by the #TextInput actor and it should not be modified or freed
 */
PangoFontDescription*
text_input_get_font_description (TextInput* self)
{
	g_return_val_if_fail (self, NULL);

	return self->priv->font_desc;
}


/**
 * text_input_get_font_name:
 * @self: a #TextInput
 *
 * Retrieves the font name as set by text_input_set_font_name().
 *
 * Return value: a string containing the font name. The returned
 *   string is owned by the #TextInput actor and should not be
 *   modified or freed
 */
const gchar*
text_input_get_font_name (TextInput* text)
{
	g_return_val_if_fail (text, NULL);

	return text->priv->font_name;
}


/**
 * text_input_set_font_name:
 * @self: a #TextInput
 * @font_name: (allow-none): a font name, or %NULL to set the default font name
 *
 * Sets the font used by a #TextInput. The @font_name string
 * must either be %NULL, which means that the default font name
 * will be used; or be something that can be parsed by
 * the pango_font_description_from_string() function, like:
 *
 * |[
 *   // Set the font to the system's Sans, 10 points
 *   text_input_set_font_name (text, "Sans 10");
 *
 *   // Set the font to the system's Serif, 16 pixels
 *   text_input_set_font_name (text, "Serif 16px");
 *
 *   // Set the font to Helvetica, 10 points
 *   text_input_set_font_name (text, "Helvetica 10");
 * ]|
 */
void
text_input_set_font_name (TextInput* self, const gchar* font_name)
{
	TextInputPrivate* priv = self->priv;
	bool is_default_font;

	if (!font_name || font_name[0] == '\0') {
		font_name = g_strdup ("Sans 12");
		is_default_font = true;
	} else
		is_default_font = false;

	if (!g_strcmp0 (priv->font_name, font_name))
		goto out;

	PangoFontDescription* desc = pango_font_description_from_string (font_name);
	if (!desc) {
		g_warning ("Attempting to create a PangoFontDescription for font name '%s', but failed.", font_name);
		goto out;
	}

	/* this will set the font_name field as well */
	text_input_set_font_description_internal (self, desc, is_default_font);

	pango_font_description_free (desc);

out:
	if (is_default_font)
		g_free ((gchar*)font_name);
}


/**
 * text_input_get_text:
 * @self: a #TextInput
 *
 * Retrieves a pointer to the current contents of a #TextInput
 * actor.
 *
 * If you need a copy of the contents for manipulating, either
 * use g_strdup() on the returned string, or use:
 *
 * |[
 *    copy = text_input_get_chars (text, 0, -1);
 * ]|
 *
 * Which will return a newly allocated string.
 *
 * If the #TextInput actor is empty, this function will return
 * an empty string, and not %NULL.
 *
 * Return value: (transfer none): the contents of the actor. The returned
 *   string is owned by the #TextInput actor and should never be modified
 *   or freed
 */
const gchar*
text_input_get_text (TextInput* self)
{
	g_return_val_if_fail (self, NULL);

	return agl_text_buffer_get_text (get_buffer (self));
}


static inline void
text_input_set_use_markup_internal (TextInput* self, bool use_markup)
{
	TextInputPrivate* priv = self->priv;

	if (priv->use_markup != use_markup) {
		priv->use_markup = use_markup;

		/* reset the attributes lists so that they can be
		 * re-generated
		 */
		if (priv->effective_attrs != NULL) {
			pango_attr_list_unref (priv->effective_attrs);
			priv->effective_attrs = NULL;
		}

		if (priv->markup_attrs) {
			pango_attr_list_unref (priv->markup_attrs);
			priv->markup_attrs = NULL;
		}
	}
}


/**
 * text_input_set_text:
 * @self: a #TextInput
 * @text: (allow-none): the text to set. Passing %NULL is the same
 *   as passing "" (the empty string)
 *
 * Sets the contents of a #TextInput actor.
 *
 * If the #TextInput:use-markup property was set to %TRUE it
 * will be reset to %FALSE as a side effect. If you want to
 * maintain the #TextInput:use-markup you should use the
 * text_input_set_markup() function instead
 */
void
text_input_set_text (TextInput* self, const gchar* text)
{
	/* if the text is editable (i.e. there is not markup flag to reset) then
	 * changing the contents will result in selection and cursor changes that
	 * we should avoid
	 */
	if (self->priv->editable) {
		if (g_strcmp0 (agl_text_buffer_get_text (get_buffer (self)), text) == 0)
			return;
	}

	text_input_set_use_markup_internal (self, FALSE);
	agl_text_buffer_set_text (get_buffer (self), text ? text : "", -1);
}


/**
 * text_input_set_markup:
 * @self: a #TextInput
 * @markup: (allow-none): a string containing Pango markup.
 *   Passing %NULL is the same as passing "" (the empty string)
 *
 * Sets @markup as the contents of a #TextInput.
 *
 * This is a convenience function for setting a string containing
 * Pango markup, and it is logically equivalent to:
 *
 * |[
 *   /&ast; the order is important &ast;/
 *   text_input_set_text (actor, markup);
 *   text_input_set_use_markup (actor, TRUE);
 * ]|
 */
void
text_input_set_markup (TextInput* self, const gchar* markup)
{
	g_return_if_fail (self);

	text_input_set_use_markup_internal (self, TRUE);
	if (markup != NULL && *markup != '\0')
		text_input_set_markup_internal (self, markup);
	else
		agl_text_buffer_set_text (get_buffer (self), "", 0);
}


/**
 * text_input_get_layout:
 * @self: a #TextInput
 *
 * Retrieves the current #PangoLayout used by a #TextInput actor.
 *
 * Return value: (transfer none): a #PangoLayout. The returned object is owned by
 *   the #TextInput actor and should not be modified or freed
 */
PangoLayout*
text_input_get_layout (TextInput* self)
{
	AGlActor* actor = (AGlActor*)self;

	if (self->priv->editable && self->priv->single_line_mode)
		return text_input_create_layout (self, -1, -1);

	return text_input_create_layout (self, agl_actor__width(actor), agl_actor__height(actor));
}


/**
 * text_input_set_color:
 * @self: a #TextInput
 * @color: a uint32_t color
 *
 * Sets the color of the contents of a #TextInput actor.
 *
 * The overall opacity of the #TextInput actor will be the
 * result of the alpha value of @color and the composited
 * opacity of the actor itself on the scenegraph.
 */
void
text_input_set_color (TextInput* self, uint32_t color)
{
	g_return_if_fail (self);

	self->priv->text_color = color;

	agl_actor__invalidate ((AGlActor*)self);
}


/**
 * text_input_get_color:
 * @self: a #TextInput
 *
 * Retrieves the text color as set by text_input_set_color().
 */
uint32_t
text_input_get_color (TextInput* self)
{
	g_return_val_if_fail (self, 0);

	return self->priv->text_color;
}


/**
 * text_input_set_ellipsize:
 * @self: a #TextInput
 * @mode: a #PangoEllipsizeMode
 *
 * Sets the mode used to ellipsize (add an ellipsis: "...") to the
 * text if there is not enough space to render the entire contents
 * of a #TextInput actor
 */
void
text_input_set_ellipsize (TextInput* self, PangoEllipsizeMode mode)
{
	g_return_if_fail (self);
	g_return_if_fail (mode >= PANGO_ELLIPSIZE_NONE && mode <= PANGO_ELLIPSIZE_END);

	TextInputPrivate* priv = self->priv;

	if ((PangoEllipsizeMode)priv->ellipsize != mode) {
		priv->ellipsize = mode;

		text_input_dirty_cache (self);

		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_ellipsize:
 * @self: a #TextInput
 *
 * Returns the ellipsizing position of a #TextInput actor, as
 * set by text_input_set_ellipsize().
 *
 * Return value: #PangoEllipsizeMode
 */
PangoEllipsizeMode
text_input_get_ellipsize (TextInput* self)
{
	g_return_val_if_fail (self, PANGO_ELLIPSIZE_NONE);

	return self->priv->ellipsize;
}


/**
 * text_input_get_line_wrap:
 * @self: a #TextInput
 *
 * Retrieves the value set using text_input_set_line_wrap().
 *
 * Return value: %TRUE if the #TextInput actor should wrap
 *   its contents
 */
bool
text_input_get_line_wrap (TextInput* self)
{
	g_return_val_if_fail (self, false);

	return self->priv->wrap;
}


/**
 * text_input_set_line_wrap:
 * @self: a #TextInput
 * @line_wrap: whether the contents should wrap
 *
 * Sets whether the contents of a #TextInput actor should wrap,
 * if they don't fit the size assigned to the actor.
 */
void
text_input_set_line_wrap (TextInput* self, bool line_wrap)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	if (priv->wrap != line_wrap) {
		priv->wrap = line_wrap;

		text_input_dirty_cache (self);

		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_set_line_wrap_mode:
 * @self: a #TextInput
 * @wrap_mode: the line wrapping mode
 *
 * If line wrapping is enabled (see text_input_set_line_wrap()) this
 * function controls how the line wrapping is performed. The default is
 * %PANGO_WRAP_WORD which means wrap on word boundaries.
 */
void
text_input_set_line_wrap_mode (TextInput* self, PangoWrapMode wrap_mode)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	if (priv->wrap_mode != wrap_mode) {
		priv->wrap_mode = wrap_mode;

		text_input_dirty_cache (self);

		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_line_wrap_mode:
 * @self: a #TextInput
 *
 * Retrieves the line wrap mode used by the #TextInput actor.
 *
 * See text_input_set_line_wrap_mode ().
 *
 * Return value: the wrap mode used by the #TextInput
 */
PangoWrapMode
text_input_get_line_wrap_mode (TextInput* self)
{
	g_return_val_if_fail (self, PANGO_WRAP_WORD);

	return self->priv->wrap_mode;
}


/**
 * text_input_set_attributes:
 * @self: a #TextInput
 * @attrs: (allow-none): a #PangoAttrList or %NULL to unset the attributes
 *
 * Sets the attributes list that are going to be applied to the
 * #TextInput contents.
 *
 * The #TextInput actor will take a reference on the #PangoAttrList
 * passed to this function.
 */
void
text_input_set_attributes (TextInput* self, PangoAttrList* attrs)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	/* While we should probably test for equality, Pango doesn't
	 * provide us an easy method to check for AttrList equality.
	 */
	if (priv->attrs == attrs)
		return;

	if (attrs)
		pango_attr_list_ref (attrs);

	if (priv->attrs)
		pango_attr_list_unref (priv->attrs);

	priv->attrs = attrs;

	/* Clear the effective attributes so they will be regenerated when a
	   layout is created */
	if (priv->effective_attrs) {
		pango_attr_list_unref (priv->effective_attrs);
		priv->effective_attrs = NULL;
	}

	text_input_dirty_cache (self);

	agl_actor__invalidate ((AGlActor*)self);
}


/**
 * text_input_get_attributes:
 * @self: a #TextInput
 *
 * Gets the attribute list that was set on the #TextInput actor
 * text_input_set_attributes(), if any.
 *
 * Return value: (transfer none): the attribute list, or %NULL if none was set. The
 *  returned value is owned by the #TextInput and should not be unreferenced.
 */
PangoAttrList*
text_input_get_attributes (TextInput* self)
{
	g_return_val_if_fail (self, NULL);

	return self->priv->attrs;
}


/**
 * text_input_set_line_alignment:
 * @self: a #TextInput
 * @alignment: A #PangoAlignment
 *
 * Sets the way that the lines of a wrapped label are aligned with
 * respect to each other. This does not affect the overall alignment
 * of the label within its allocated or specified width.
 *
 * To align a #TextInput actor you should add it to a container
 * that supports alignment, or use the anchor point.
 */
void
text_input_set_line_alignment (TextInput* self, PangoAlignment alignment)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	if (priv->alignment != alignment) {
		priv->alignment = alignment;

		text_input_dirty_cache (self);

		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_line_alignment:
 * @self: a #TextInput
 *
 * Retrieves the alignment of a #TextInput, as set by
 * text_input_set_line_alignment().
 *
 * Return value: a #PangoAlignment
 */
PangoAlignment
text_input_get_line_alignment (TextInput* self)
{
	g_return_val_if_fail (self, PANGO_ALIGN_LEFT);

	return self->priv->alignment;
}


/**
 * text_input_set_use_markup:
 * @self: a #TextInput
 * @setting: %TRUE if the text should be parsed for markup.
 *
 * Sets whether the contents of the #TextInput actor contains markup
 * in <link linkend="PangoMarkupFormat">Pango's text markup language</link>.
 *
 * Setting #TextInput:use-markup on an editable #TextInput will
 * not have any effect except hiding the markup.
 */
void
text_input_set_use_markup (TextInput* self, bool setting)
{
	g_return_if_fail (self);

	const gchar* text = agl_text_buffer_get_text (get_buffer (self));

	text_input_set_use_markup_internal (self, setting);

	if (setting)
		text_input_set_markup_internal (self, text);

	text_input_dirty_cache (self);

	agl_actor__invalidate ((AGlActor*)self);
}


/**
 * text_input_get_use_markup:
 * @self: a #TextInput
 *
 * Retrieves whether the contents of the #TextInput actor should be
 * parsed for the Pango text markup.
 *
 * Return value: %TRUE if the contents will be parsed for markup
 */
bool
text_input_get_use_markup (TextInput* self)
{
	g_return_val_if_fail (self, false);

	return self->priv->use_markup;
}


/**
 * text_input_set_justify:
 * @self: a #TextInput
 * @justify: whether the text should be justified
 *
 * Sets whether the text of the #TextInput actor should be justified
 * on both margins. This setting is ignored with Pango &lt; 1.18.
 */
void
text_input_set_justify (TextInput* self, bool justify)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	if (priv->justify != justify) {
		priv->justify = justify;

		text_input_dirty_cache (self);

		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_justify:
 * @self: a #TextInput
 *
 * Retrieves whether the #TextInput actor should justify its contents
 * on both margins.
 *
 * Return value: %TRUE if the text should be justified
 */
bool
text_input_get_justify (TextInput* self)
{
	g_return_val_if_fail (self, FALSE);

	return self->priv->justify;
}


/**
 * text_input_get_cursor_position:
 * @self: a #TextInput
 *
 * Retrieves the cursor position.
 *
 * Return value: the cursor position, in characters
 */
gint
text_input_get_cursor_position (TextInput* self)
{
	g_return_val_if_fail (self, -1);

	return self->priv->position;
}


/**
 * text_input_set_cursor_position:
 * @self: a #TextInput
 * @position: the new cursor position, in characters
 *
 * Sets the cursor of a #TextInput actor at @position.
 *
 * The position is expressed in characters, not in bytes.
 */
void
text_input_set_cursor_position (TextInput* self, gint position)
{
	TextInputPrivate* priv = self->priv;

	if (priv->position == position)
		return;

	gint len = agl_text_buffer_get_length (get_buffer (self));

	if (position < 0 || position >= len)
		priv->position = -1;
	else
		priv->position = position;

	/* Forget the target x position so that it will be recalculated next
	   time the cursor is moved up or down */
	priv->x_pos = -1;

	agl_actor__invalidate ((AGlActor*)self);
}


/**
 * text_input_set_cursor_size:
 * @self: a #TextInput
 * @size: the size of the cursor, in pixels, or -1 to use the
 *   default value
 *
 * Sets the size of the cursor of a #TextInput. The cursor
 * will only be visible if the #TextInput:cursor-visible property
 * is set to %TRUE.
 */
void
text_input_set_cursor_size (TextInput* self, gint size)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	if (priv->cursor_size != size) {
		if (size < 0)
			size = DEFAULT_CURSOR_SIZE;

		priv->cursor_size = size;

		text_input_queue_redraw (self);
	}
}


/**
 * text_input_get_cursor_size:
 * @self: a #TextInput
 *
 * Retrieves the size of the cursor of a #TextInput actor.
 *
 * Return value: the size of the cursor, in pixels
 */
guint
text_input_get_cursor_size (TextInput* self)
{
	g_return_val_if_fail (self, DEFAULT_CURSOR_SIZE);

	return self->priv->cursor_size;
}


/**
 * text_input_set_password_char:
 * @self: a #TextInput
 * @wc: a Unicode character, or 0 to unset the password character
 *
 * Sets the character to use in place of the actual text in a
 * password text actor.
 *
 * If @wc is 0 the text will be displayed as it is entered in the
 * #TextInput actor.
 */
void
text_input_set_password_char (TextInput* self, gunichar wc)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	if (priv->password_char != wc) {
		priv->password_char = wc;

		text_input_dirty_cache (self);
		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_password_char:
 * @self: a #TextInput
 *
 * Retrieves the character to use in place of the actual text
 * as set by text_input_set_password_char().
 *
 * Return value: a Unicode character or 0 if the password
 *   character is not set
 */
gunichar
text_input_get_password_char (TextInput* self)
{
	g_return_val_if_fail (self, 0);

	return self->priv->password_char;
}


/**
 * text_input_set_max_length:
 * @self: a #TextInput
 * @max: the maximum number of characters allowed in the text actor; 0
 *   to disable or -1 to set the length of the current string
 *
 * Sets the maximum allowed length of the contents of the actor. If the
 * current contents are longer than the given length, then they will be
 * truncated to fit.
 */
void
text_input_set_max_length (TextInput* self, gint max)
{
	g_return_if_fail (self);

	agl_text_buffer_set_max_length (get_buffer (self), max);
}


/**
 * text_input_get_max_length:
 * @self: a #TextInput
 *
 * Gets the maximum length of text that can be set into a text actor.
 *
 * See text_input_set_max_length().
 *
 * Return value: the maximum number of characters.
 */
gint
text_input_get_max_length (TextInput* self)
{
	g_return_val_if_fail (self, 0);

	return agl_text_buffer_get_max_length (get_buffer (self));
}


static void
text_input_real_insert_text (TextInput* self, guint start_pos, const gchar* chars, guint n_chars)
{
	/*
	 * The actual insertion from the buffer. This will end firing the
	 * following signal handlers: buffer_inserted_text(),
	 * buffer_notify_text(), buffer_notify_max_length()
	 */
	agl_text_buffer_insert_text (get_buffer (self), start_pos, chars, n_chars);
}


/**
 * text_input_insert_unichar:
 * @self: a #TextInput
 * @wc: a Unicode character
 *
 * Inserts @wc at the current cursor position of a
 * #TextInput actor.
 */
void
text_input_insert_unichar (TextInput* self, gunichar wc)
{
	TextInputPrivate* priv = self->priv;

	GString* new = g_string_new ("");
	g_string_append_unichar (new, wc);

	text_input_real_insert_text (self, priv->position, new->str, 1);

	g_string_free (new, TRUE);
}


/**
 * text_input_insert_text:
 * @self: a #TextInput
 * @text: the text to be inserted
 * @position: the position of the insertion, or -1
 *
 * Inserts @text at the given position.
 *
 * If @position is a negative number, the text will be appended
 * at the end of the current contents of the #TextInput.
 *
 * The position is expressed in characters, not in bytes.
 */
void
text_input_insert_text (TextInput* self, const gchar* text, gssize position)
{
	g_return_if_fail (self);
	g_return_if_fail (text != NULL);

	text_input_real_insert_text (self, position, text, g_utf8_strlen (text, -1));
}


static void
text_input_real_delete_text (TextInput* self, gssize start_pos, gssize end_pos)
{
	/*
	 * The actual deletion from the buffer. This will end firing the
	 * following signal handlers: buffer_deleted_text(),
	 * buffer_notify_text(), buffer_notify_max_length()
	 */
	agl_text_buffer_delete_text (get_buffer (self), start_pos, end_pos - start_pos);
}


/**
 * text_input_delete_text:
 * @self: a #TextInput
 * @start_pos: starting position
 * @end_pos: ending position
 *
 * Deletes the text inside a #TextInput actor between @start_pos
 * and @end_pos.
 *
 * The starting and ending positions are expressed in characters,
 * not in bytes.
 */
void
text_input_delete_text (TextInput* self, gssize start_pos, gssize end_pos)
{
	text_input_real_delete_text (self, start_pos, end_pos);
}


/**
 * text_input_delete_chars:
 * @self: a #TextInput
 * @n_chars: the number of characters to delete
 *
 * Deletes @n_chars inside a #TextInput actor, starting from the
 * current cursor position.
 *
 * Somewhat awkwardly, the cursor position is decremented by the same
 * number of characters you've deleted.
 */
#if 0
void
text_input_delete_chars (TextInput* self, guint n_chars)
{
	TextInputPrivate* priv = self->priv;

	text_input_real_delete_text (self, priv->position, priv->position + n_chars);

	if (priv->position > 0)
		text_input_set_cursor_position (self, priv->position - n_chars);
}
#endif


/**
 * text_input_get_chars:
 * @self: a #TextInput
 * @start_pos: start of text, in characters
 * @end_pos: end of text, in characters
 *
 * Retrieves the contents of the #TextInput actor between
 * @start_pos and @end_pos, but not including @end_pos.
 *
 * The positions are specified in characters, not in bytes.
 *
 * Return value: a newly allocated string with the contents of
 *   the text actor between the specified positions. Use g_free()
 *   to free the resources when done
 */
gchar*
text_input_get_chars (TextInput* self, gssize start_pos, gssize end_pos)
{
	guint n_chars = agl_text_buffer_get_length (get_buffer (self));
	const gchar* text = agl_text_buffer_get_text (get_buffer (self));

	if (end_pos < 0)
		end_pos = n_chars;

	start_pos = MIN (n_chars, start_pos);
	end_pos = MIN (n_chars, end_pos);

	gint start_index = g_utf8_offset_to_pointer (text, start_pos) - text;
	gint end_index = g_utf8_offset_to_pointer (text, end_pos) - text;

	return g_strndup (text + start_index, end_index - start_index);
}


/**
 * text_input_set_single_line_mode:
 * @self: a #TextInput
 * @single_line: whether to enable single line mode
 *
 * Sets whether a #TextInput actor should be in single line mode
 * or not. Only editable #TextInput<!-- -->s can be in single line
 * mode.
 *
 * A text actor in single line mode will not wrap text and will clip
 * the visible area to the predefined size. The contents of the
 * text actor will scroll to display the end of the text if its length
 * is bigger than the allocated width.
 *
 * When setting the single line mode the #TextInput:activatable
 * property is also set as a side effect. Instead of entering a new
 * line character, the text actor will emit the #TextInput::activate
 * signal.
 */
void
text_input_set_single_line_mode (TextInput* self, bool single_line)
{
	TextInputPrivate* priv = self->priv;

	if (priv->single_line_mode != single_line) {
		priv->single_line_mode = single_line;

		if (priv->single_line_mode) {
			priv->activatable = TRUE;
		}

		text_input_dirty_cache (self);
		agl_actor__invalidate ((AGlActor*)self);
	}
}


/**
 * text_input_get_single_line_mode:
 * @self: a #TextInput
 *
 * Retrieves whether the #TextInput actor is in single line mode.
 *
 * Return value: %TRUE if the #TextInput actor is in single line mode
 */
bool
text_input_get_single_line_mode (TextInput* self)
{
	g_return_val_if_fail (self, FALSE);

	return self->priv->single_line_mode;
}


/**
 * text_input_set_preedit_string:
 * @self: a #TextInput
 * @preedit_str: (allow-none): the pre-edit string, or %NULL to unset it
 * @preedit_attrs: (allow-none): the pre-edit string attributes
 * @cursor_pos: the cursor position for the pre-edit string
 *
 * Sets, or unsets, the pre-edit string. This function is useful
 * for input methods to display a string (with eventual specific
 * Pango attributes) before it is entered inside the #TextInput
 * buffer.
 *
 * The preedit string and attributes are ignored if the #TextInput
 * actor is not editable.
 *
 * This function should not be used by applications
 */
void
text_input_set_preedit_string (TextInput* self, const gchar* preedit_str, PangoAttrList* preedit_attrs, guint cursor_pos)
{
	g_return_if_fail (self);

	TextInputPrivate* priv = self->priv;

	g_free (priv->preedit_str);
	priv->preedit_str = NULL;

	if (priv->preedit_attrs != NULL) {
		pango_attr_list_unref (priv->preedit_attrs);
		priv->preedit_attrs = NULL;
	}

	priv->preedit_n_chars = 0;
	priv->preedit_cursor_pos = 0;

	if (preedit_str == NULL || *preedit_str == '\0')
		priv->preedit_set = FALSE;
	else {
		priv->preedit_str = g_strdup (preedit_str);

		if (priv->preedit_str != NULL)
			priv->preedit_n_chars = g_utf8_strlen (priv->preedit_str, -1);
		else
			priv->preedit_n_chars = 0;

		if (preedit_attrs != NULL)
			priv->preedit_attrs = pango_attr_list_ref (preedit_attrs);

		priv->preedit_cursor_pos = CLAMP (cursor_pos, 0, priv->preedit_n_chars);

		priv->preedit_set = TRUE;
	}

	text_input_dirty_cache (self);
	agl_actor__invalidate ((AGlActor*)self);
}


void
text_input_set_placeholder (TextInput* self, const gchar* placeholder)
{
	TextInputPrivate* priv = self->priv;

	g_free(priv->placeholder);
	priv->placeholder = g_strdup(placeholder);

	agl_actor__invalidate ((AGlActor*)self);
}


/**
 * text_input_get_layout_offsets:
 * @self: a #TextInput
 * @x: (out): location to store X offset of layout, or %NULL
 * @y: (out): location to store Y offset of layout, or %NULL
 *
 * Obtains the coordinates where the #TextInput will draw the #PangoLayout
 * representing the text.
 */
void
text_input_get_layout_offsets (TextInput* self, gint* x, gint* y)
{
	TextInputPrivate* priv = self->priv;

	g_return_if_fail (self);

	if (x != NULL)
		*x = priv->text_x;

	if (y != NULL)
		*y = priv->text_y;
}


/**
 * text_input_get_cursor_rect:
 * @self: a #TextInput
 * @rect: (out caller-allocates): return location of a #graphene_rect_t
 *
 * Retrieves the rectangle that contains the cursor.
 *
 * The coordinates of the rectangle's origin are in actor-relative
 * coordinates.
 */
void
text_input_get_cursor_rect (TextInput* self, graphene_rect_t* rect)
{
	g_return_if_fail (self);
	g_return_if_fail (rect);

	*rect = self->priv->cursor_rect;
}
