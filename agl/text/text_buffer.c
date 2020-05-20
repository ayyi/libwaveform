/* Copyright (C) 2011 Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Stef Walter <stefw@collabora.co.uk>
 */

#include "config.h"
#include <string.h>
#include "agl/debug.h"
#include "text_buffer.h"

#define P_(A) (A)
#define I_(A) (A)

#ifdef G_ENABLE_DEBUG
#define g_marshal_value_peek_uint(v)     g_value_get_uint (v)
#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#else
/* WARNING: This code accesses GValues directly, which is UNSUPPORTED API.
 *          Do not access GValues directly in your code. Instead, use the
 *          g_value_get_*() functions
 */
#define g_marshal_value_peek_uint(v)     (v)->data[0].v_uint
#define g_marshal_value_peek_string(v)   (v)->data[0].v_pointer
#endif

/* VOID:UINT,STRING,UINT (./clutter-marshal.list:34) */
void
_clutter_marshal_VOID__UINT_STRING_UINT (GClosure     *closure,
                                         GValue       *return_value G_GNUC_UNUSED,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      invocation_hint G_GNUC_UNUSED,
                                         gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_STRING_UINT) (gpointer data1,
                                                       guint arg1,
                                                       gpointer arg2,
                                                       guint arg3,
                                                       gpointer data2);
  GCClosure *cc = (GCClosure *) closure;
  gpointer data1, data2;
  GMarshalFunc_VOID__UINT_STRING_UINT callback;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_STRING_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_string (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            data2);
}

/* VOID:UINT,UINT (./clutter-marshal.list:35) */
void
_clutter_marshal_VOID__UINT_UINT (GClosure     *closure,
                                  GValue       *return_value G_GNUC_UNUSED,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint G_GNUC_UNUSED,
                                  gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_UINT) (gpointer data1,
                                                guint arg1,
                                                guint arg2,
                                                gpointer data2);
  GCClosure *cc = (GCClosure *) closure;
  gpointer data1, data2;
  GMarshalFunc_VOID__UINT_UINT callback;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            data2);
}

/**
 * SECTION:clutter-text-buffer
 * @title: AGlTextBuffer
 * @short_description: Text buffer for ClutterText
 *
 * The #AGlTextBuffer class contains the actual text displayed in a
 * #ClutterText widget.
 *
 * A single #AGlTextBuffer object can be shared by multiple #ClutterText
 * widgets which will then share the same text content, but not the cursor
 * position, visibility attributes, icon etc.
 *
 * #AGlTextBuffer may be derived from. Such a derived class might allow
 * text to be stored in an alternate location, such as non-pageable memory,
 * useful in the case of important passwords. Or a derived class could
 * integrate with an application's concept of undo/redo.
 */

/* Initial size of buffer, in bytes */
#define MIN_SIZE 16

enum {
  PROP_0,
  PROP_TEXT,
  PROP_LENGTH,
  PROP_MAX_LENGTH,
  PROP_LAST
};

static GParamSpec *obj_props[PROP_LAST] = { NULL, };

enum {
  INSERTED_TEXT,
  DELETED_TEXT,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

struct _AGlTextBufferPrivate
{
  gint  max_length;

  /* Only valid if this class is not derived */
  gchar *normal_text;
  gsize  normal_text_size;
  gsize  normal_text_bytes;
  guint  normal_text_chars;
};

G_DEFINE_TYPE_WITH_PRIVATE (AGlTextBuffer, agl_text_buffer, G_TYPE_OBJECT)

static void agl_text_buffer_emit_inserted_text (TextBuffer*, guint position, const gchar* chars, guint n_chars);
static void agl_text_buffer_emit_deleted_text  (TextBuffer*, guint position, guint n_chars);

/* --------------------------------------------------------------------------------
 * DEFAULT IMPLEMENTATIONS OF TEXT BUFFER
 *
 * These may be overridden by a derived class, behavior may be changed etc...
 * The normal_text and normal_text_xxxx fields may not be valid when
 * this class is derived from.
 */

/* Overwrite a memory that might contain sensitive information. */
static void
trash_area (gchar *area,
            gsize  len)
{
  volatile gchar *varea = (volatile gchar *)area;
  while (len-- > 0)
    *varea++ = 0;
}

static const gchar*
agl_text_buffer_normal_get_text (AGlTextBuffer *buffer,
                                  gsize          *n_bytes)
{
  if (n_bytes)
    *n_bytes = buffer->priv->normal_text_bytes;
  if (!buffer->priv->normal_text)
      return "";
  return buffer->priv->normal_text;
}

static guint
agl_text_buffer_normal_get_length (AGlTextBuffer *buffer)
{
  return buffer->priv->normal_text_chars;
}

static guint
agl_text_buffer_normal_insert_text (AGlTextBuffer *buffer,
                                     guint           position,
                                     const gchar    *chars,
                                     guint           n_chars)
{
  AGlTextBufferPrivate *pv = buffer->priv;
  gsize prev_size;
  gsize n_bytes;
  gsize at;

  n_bytes = g_utf8_offset_to_pointer (chars, n_chars) - chars;

  /* Need more memory */
  if (n_bytes + pv->normal_text_bytes + 1 > pv->normal_text_size)
    {
      gchar *et_new;

      prev_size = pv->normal_text_size;

      /* Calculate our new buffer size */
      while (n_bytes + pv->normal_text_bytes + 1 > pv->normal_text_size)
        {
          if (pv->normal_text_size == 0)
            pv->normal_text_size = MIN_SIZE;
          else
            {
              if (2 * pv->normal_text_size < AGL_TEXT_BUFFER_MAX_SIZE)
                pv->normal_text_size *= 2;
              else
                {
                  pv->normal_text_size = AGL_TEXT_BUFFER_MAX_SIZE;
                  if (n_bytes > pv->normal_text_size - pv->normal_text_bytes - 1)
                    {
                      n_bytes = pv->normal_text_size - pv->normal_text_bytes - 1;
                      n_bytes = g_utf8_find_prev_char (chars, chars + n_bytes + 1) - chars;
                      n_chars = g_utf8_strlen (chars, n_bytes);
                    }
                  break;
                }
            }
        }

      /* Could be a password, so can't leave stuff in memory. */
      et_new = g_malloc (pv->normal_text_size);
      memcpy (et_new, pv->normal_text, MIN (prev_size, pv->normal_text_size));
      trash_area (pv->normal_text, prev_size);
      g_free (pv->normal_text);
      pv->normal_text = et_new;
    }

  /* Actual text insertion */
  at = g_utf8_offset_to_pointer (pv->normal_text, position) - pv->normal_text;
  memmove (pv->normal_text + at + n_bytes, pv->normal_text + at, pv->normal_text_bytes - at);
  memcpy (pv->normal_text + at, chars, n_bytes);

  /* Book keeping */
  pv->normal_text_bytes += n_bytes;
  pv->normal_text_chars += n_chars;
  pv->normal_text[pv->normal_text_bytes] = '\0';

  agl_text_buffer_emit_inserted_text (buffer, position, chars, n_chars);
  return n_chars;
}

static guint
agl_text_buffer_normal_delete_text (AGlTextBuffer *buffer,
                                     guint           position,
                                     guint           n_chars)
{
  AGlTextBufferPrivate *pv = buffer->priv;
  gsize start, end;

  if (position > pv->normal_text_chars)
    position = pv->normal_text_chars;
  if (position + n_chars > pv->normal_text_chars)
    n_chars = pv->normal_text_chars - position;

  if (n_chars > 0)
    {
      start = g_utf8_offset_to_pointer (pv->normal_text, position) - pv->normal_text;
      end = g_utf8_offset_to_pointer (pv->normal_text, position + n_chars) - pv->normal_text;

      memmove (pv->normal_text + start, pv->normal_text + end, pv->normal_text_bytes + 1 - end);
      pv->normal_text_chars -= n_chars;
      pv->normal_text_bytes -= (end - start);

      /*
       * Could be a password, make sure we don't leave anything sensitive after
       * the terminating zero.  Note, that the terminating zero already trashed
       * one byte.
       */
      trash_area (pv->normal_text + pv->normal_text_bytes + 1, end - start - 1);

      agl_text_buffer_emit_deleted_text (buffer, position, n_chars);
    }

  return n_chars;
}

/* --------------------------------------------------------------------------------
 *
 */

static void
agl_text_buffer_real_inserted_text (AGlTextBuffer *buffer,
                                     guint           position,
                                     const gchar    *chars,
                                     guint           n_chars)
{
  g_object_notify (G_OBJECT (buffer), "text");
  g_object_notify (G_OBJECT (buffer), "length");
}

static void
agl_text_buffer_real_deleted_text (AGlTextBuffer *buffer,
                                    guint           position,
                                    guint           n_chars)
{
  g_object_notify (G_OBJECT (buffer), "text");
  g_object_notify (G_OBJECT (buffer), "length");
}

/* --------------------------------------------------------------------------------
 *
 */

static void
agl_text_buffer_init (AGlTextBuffer *self)
{
  self->priv = agl_text_buffer_get_instance_private (self);

  self->priv->normal_text = NULL;
  self->priv->normal_text_chars = 0;
  self->priv->normal_text_bytes = 0;
  self->priv->normal_text_size = 0;
}

static void
agl_text_buffer_finalize (GObject *obj)
{
  AGlTextBuffer *buffer = AGL_TEXT_BUFFER (obj);
  AGlTextBufferPrivate *pv = buffer->priv;

  if (pv->normal_text)
    {
      trash_area (pv->normal_text, pv->normal_text_size);
      g_free (pv->normal_text);
      pv->normal_text = NULL;
      pv->normal_text_bytes = pv->normal_text_size = 0;
      pv->normal_text_chars = 0;
    }

  G_OBJECT_CLASS (agl_text_buffer_parent_class)->finalize (obj);
}

static void
agl_text_buffer_set_property (GObject      *obj,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AGlTextBuffer *buffer = AGL_TEXT_BUFFER (obj);

  switch (prop_id)
    {
    case PROP_MAX_LENGTH:
      agl_text_buffer_set_max_length (buffer, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
}

static void
agl_text_buffer_get_property (GObject    *obj,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AGlTextBuffer *buffer = AGL_TEXT_BUFFER (obj);

  switch (prop_id)
    {
    case PROP_TEXT:
      g_value_set_string (value, agl_text_buffer_get_text (buffer));
      break;
    case PROP_LENGTH:
      g_value_set_uint (value, agl_text_buffer_get_length (buffer));
      break;
    case PROP_MAX_LENGTH:
      g_value_set_int (value, agl_text_buffer_get_max_length (buffer));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
}

static void
agl_text_buffer_class_init (AGlTextBufferClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = agl_text_buffer_finalize;
  gobject_class->set_property = agl_text_buffer_set_property;
  gobject_class->get_property = agl_text_buffer_get_property;

  klass->get_text = agl_text_buffer_normal_get_text;
  klass->get_length = agl_text_buffer_normal_get_length;
  klass->insert_text = agl_text_buffer_normal_insert_text;
  klass->delete_text = agl_text_buffer_normal_delete_text;

  klass->inserted_text = agl_text_buffer_real_inserted_text;
  klass->deleted_text = agl_text_buffer_real_deleted_text;

  /**
   * AGlTextBuffer:text:
   *
   * The contents of the buffer.
   */
  obj_props[PROP_TEXT] =
      g_param_spec_string ("text",
                           P_("Text"),
                           P_("The contents of the buffer"),
                           "",
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AGlTextBuffer:length:
   *
   * The length (in characters) of the text in buffer.
   */
  obj_props[PROP_LENGTH] =
      g_param_spec_uint ("length",
                         P_("Text length"),
                         P_("Length of the text currently in the buffer"),
                         0, AGL_TEXT_BUFFER_MAX_SIZE, 0,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AGlTextBuffer:max-length:
   *
   * The maximum length (in characters) of the text in the buffer.
   */
  obj_props[PROP_MAX_LENGTH] =
      g_param_spec_int ("max-length",
                        P_("Maximum length"),
                        P_("Maximum number of characters for this entry. Zero if no maximum"),
                        0, AGL_TEXT_BUFFER_MAX_SIZE, 0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, PROP_LAST, obj_props);

  /**
   * AGlTextBuffer::inserted-text:
   * @buffer: a #AGlTextBuffer
   * @position: the position the text was inserted at.
   * @chars: The text that was inserted.
   * @n_chars: The number of characters that were inserted.
   *
   * This signal is emitted after text is inserted into the buffer.
   */
  signals[INSERTED_TEXT] =
    g_signal_new (I_("inserted-text"),
                  AGL_TYPE_TEXT_BUFFER,
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (AGlTextBufferClass, inserted_text),
                  NULL, NULL,
                  _clutter_marshal_VOID__UINT_STRING_UINT,
                  G_TYPE_NONE, 3,
                  G_TYPE_UINT,
                  G_TYPE_STRING,
                  G_TYPE_UINT);

  /**
   * AGlTextBuffer::deleted-text:
   * @buffer: a #AGlTextBuffer
   * @position: the position the text was deleted at.
   * @n_chars: The number of characters that were deleted.
   *
   * This signal is emitted after text is deleted from the buffer.
   */
  signals[DELETED_TEXT] =
    g_signal_new (I_("deleted-text"),
                  AGL_TYPE_TEXT_BUFFER,
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (AGlTextBufferClass, deleted_text),
                  NULL, NULL,
                  _clutter_marshal_VOID__UINT_UINT,
                  G_TYPE_NONE, 2,
                  G_TYPE_UINT,
                  G_TYPE_UINT);
}

/* --------------------------------------------------------------------------------
 *
 */

/**
 * agl_text_buffer_new:
 *
 * Create a new AGlTextBuffer object.
 *
 * Return value: A new AGlTextBuffer object.
 **/
AGlTextBuffer*
agl_text_buffer_new (void)
{
  return g_object_new (AGL_TYPE_TEXT_BUFFER, NULL);
}


/**
 * agl_text_buffer_new_with_text:
 * @text: (allow-none): initial buffer text
 * @text_len: initial buffer text length, or -1 for null-terminated.
 *
 * Create a new AGlTextBuffer object with some text.
 *
 * Return value: A new AGlTextBuffer object.
 **/
AGlTextBuffer*
agl_text_buffer_new_with_text (const gchar   *text,
                                   gssize         text_len)
{
  AGlTextBuffer *buffer = agl_text_buffer_new ();
  agl_text_buffer_set_text (buffer, text, text_len);
  return buffer;
}


/**
 * agl_text_buffer_get_length:
 * @buffer: a #AGlTextBuffer
 *
 * Retrieves the length in characters of the buffer.
 *
 * Return value: The number of characters in the buffer.
 **/
guint
agl_text_buffer_get_length (AGlTextBuffer *buffer)
{

  g_return_val_if_fail (AGL_IS_TEXT_BUFFER (buffer), 0);

  AGlTextBufferClass *klass = AGL_TEXT_BUFFER_GET_CLASS (buffer);
  g_return_val_if_fail (klass->get_length != NULL, 0);

  return (*klass->get_length) (buffer);
}

/**
 * agl_text_buffer_get_bytes:
 * @buffer: a #AGlTextBuffer
 *
 * Retrieves the length in bytes of the buffer.
 * See agl_text_buffer_get_length().
 *
 * Return value: The byte length of the buffer.
 **/
gsize
agl_text_buffer_get_bytes (AGlTextBuffer *buffer)
{
  AGlTextBufferClass *klass;
  gsize bytes = 0;

  g_return_val_if_fail (AGL_IS_TEXT_BUFFER (buffer), 0);

  klass = AGL_TEXT_BUFFER_GET_CLASS (buffer);
  g_return_val_if_fail (klass->get_text != NULL, 0);

  (*klass->get_text) (buffer, &bytes);
  return bytes;
}

/**
 * agl_text_buffer_get_text:
 * @buffer: a #AGlTextBuffer
 *
 * Retrieves the contents of the buffer.
 *
 * The memory pointer returned by this call will not change
 * unless this object emits a signal, or is finalized.
 *
 * Return value: a pointer to the contents of the widget as a
 *      string. This string points to internally allocated
 *      storage in the buffer and must not be freed, modified or
 *      stored.
 **/
const gchar*
agl_text_buffer_get_text (AGlTextBuffer *buffer)
{
  AGlTextBufferClass *klass;

  g_return_val_if_fail (AGL_IS_TEXT_BUFFER (buffer), NULL);

  klass = AGL_TEXT_BUFFER_GET_CLASS (buffer);
  g_return_val_if_fail (klass->get_text != NULL, NULL);

  return (*klass->get_text) (buffer, NULL);
}

/**
 * agl_text_buffer_set_text:
 * @buffer: a #AGlTextBuffer
 * @chars: the new text
 * @n_chars: the number of characters in @text, or -1
 *
 * Sets the text in the buffer.
 *
 * This is roughly equivalent to calling agl_text_buffer_delete_text()
 * and agl_text_buffer_insert_text().
 *
 * Note that @n_chars is in characters, not in bytes.
 **/
void
agl_text_buffer_set_text (TextBuffer* buffer, const gchar* chars, gint n_chars)
{
  g_return_if_fail (AGL_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (chars != NULL);

  g_object_freeze_notify (G_OBJECT (buffer));
  agl_text_buffer_delete_text (buffer, 0, -1);
  agl_text_buffer_insert_text (buffer, 0, chars, n_chars);
  g_object_thaw_notify (G_OBJECT (buffer));
}

/**
 * agl_text_buffer_set_max_length:
 * @buffer: a #AGlTextBuffer
 * @max_length: the maximum length of the entry buffer, or 0 for no maximum.
 *   (other than the maximum length of entries.) The value passed in will
 *   be clamped to the range [ 0, %AGL_TEXT_BUFFER_MAX_SIZE ].
 *
 * Sets the maximum allowed length of the contents of the buffer. If
 * the current contents are longer than the given length, then they
 * will be truncated to fit.
 **/
void
agl_text_buffer_set_max_length (AGlTextBuffer *buffer,
                                    gint               max_length)
{
  g_return_if_fail (AGL_IS_TEXT_BUFFER (buffer));

  max_length = CLAMP (max_length, 0, AGL_TEXT_BUFFER_MAX_SIZE);

  if (max_length > 0 && agl_text_buffer_get_length (buffer) > max_length)
    agl_text_buffer_delete_text (buffer, max_length, -1);

  buffer->priv->max_length = max_length;
  g_object_notify (G_OBJECT (buffer), "max-length");
}

/**
 * agl_text_buffer_get_max_length:
 * @buffer: a #AGlTextBuffer
 *
 * Retrieves the maximum allowed length of the text in
 * @buffer. See agl_text_buffer_set_max_length().
 *
 * Return value: the maximum allowed number of characters
 *               in #AGlTextBuffer, or 0 if there is no maximum.
 */
gint
agl_text_buffer_get_max_length (AGlTextBuffer *buffer)
{
  g_return_val_if_fail (AGL_IS_TEXT_BUFFER (buffer), 0);
  return buffer->priv->max_length;
}

/**
 * agl_text_buffer_insert_text:
 * @buffer: a #AGlTextBuffer
 * @position: the position at which to insert text.
 * @chars: the text to insert into the buffer.
 * @n_chars: the length of the text in characters, or -1
 *
 * Inserts @n_chars characters of @chars into the contents of the
 * buffer, at position @position.
 *
 * If @n_chars is negative, then characters from chars will be inserted
 * until a null-terminator is found. If @position or @n_chars are out of
 * bounds, or the maximum buffer text length is exceeded, then they are
 * coerced to sane values.
 *
 * Note that the position and length are in characters, not in bytes.
 *
 * Returns: The number of characters actually inserted.
 */
guint
agl_text_buffer_insert_text (AGlTextBuffer *buffer,
                                 guint              position,
                                 const gchar       *chars,
                                 gint               n_chars)
{
  AGlTextBufferClass *klass;
  AGlTextBufferPrivate *pv;
  guint length;

  g_return_val_if_fail (AGL_IS_TEXT_BUFFER (buffer), 0);

  length = agl_text_buffer_get_length (buffer);
  pv = buffer->priv;

  if (n_chars < 0)
    n_chars = g_utf8_strlen (chars, -1);

  /* Bring position into bounds */
  if (position > length)
    position = length;

  /* Make sure not entering too much data */
  if (pv->max_length > 0)
    {
      if (length >= pv->max_length)
        n_chars = 0;
      else if (length + n_chars > pv->max_length)
        n_chars -= (length + n_chars) - pv->max_length;
    }

  klass = AGL_TEXT_BUFFER_GET_CLASS (buffer);
  g_return_val_if_fail (klass->insert_text != NULL, 0);

  return (klass->insert_text) (buffer, position, chars, n_chars);
}

/**
 * agl_text_buffer_delete_text:
 * @buffer: a #AGlTextBuffer
 * @position: position at which to delete text
 * @n_chars: number of characters to delete
 *
 * Deletes a sequence of characters from the buffer. @n_chars characters are
 * deleted starting at @position. If @n_chars is negative, then all characters
 * until the end of the text are deleted.
 *
 * If @position or @n_chars are out of bounds, then they are coerced to sane
 * values.
 *
 * Note that the positions are specified in characters, not bytes.
 *
 * Returns: The number of characters deleted.
 */
guint
agl_text_buffer_delete_text (AGlTextBuffer *buffer,
                                 guint              position,
                                 gint               n_chars)
{
  AGlTextBufferClass *klass;
  guint length;

  g_return_val_if_fail (AGL_IS_TEXT_BUFFER (buffer), 0);

  length = agl_text_buffer_get_length (buffer);
  if (n_chars < 0)
    n_chars = length;
  if (position > length)
    position = length;
  if (position + n_chars > length)
    n_chars = length - position;

  klass = AGL_TEXT_BUFFER_GET_CLASS (buffer);
  g_return_val_if_fail (klass->delete_text != NULL, 0);

  return (klass->delete_text) (buffer, position, n_chars);
}

/**
 * agl_text_buffer_emit_inserted_text:
 * @buffer: a #AGlTextBuffer
 * @position: position at which text was inserted
 * @chars: text that was inserted
 * @n_chars: number of characters inserted
 *
 * Emits the #AGlTextBuffer::inserted-text signal on @buffer.
 *
 * Used when subclassing #AGlTextBuffer
 */
static void
agl_text_buffer_emit_inserted_text (AGlTextBuffer *buffer,
                                        guint              position,
                                        const gchar       *chars,
                                        guint              n_chars)
{
  g_return_if_fail (AGL_IS_TEXT_BUFFER (buffer));
  g_signal_emit (buffer, signals[INSERTED_TEXT], 0, position, chars, n_chars);
}

/**
 * agl_text_buffer_emit_deleted_text:
 * @buffer: a #AGlTextBuffer
 * @position: position at which text was deleted
 * @n_chars: number of characters deleted
 *
 * Emits the #AGlTextBuffer::deleted-text signal on @buffer.
 *
 * Used when subclassing #AGlTextBuffer
 */
static void
agl_text_buffer_emit_deleted_text (AGlTextBuffer *buffer,
                                       guint              position,
                                       guint              n_chars)
{
  g_return_if_fail (AGL_IS_TEXT_BUFFER (buffer));
  g_signal_emit (buffer, signals[DELETED_TEXT], 0, position, n_chars);
}
