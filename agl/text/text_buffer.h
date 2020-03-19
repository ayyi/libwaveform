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

#ifndef __text_buffer_h__
#define __text_buffer_h__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define AGL_TYPE_TEXT_BUFFER            (agl_text_buffer_get_type ())
#define AGL_TEXT_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AGL_TYPE_TEXT_BUFFER, AGlTextBuffer))
#define AGL_TEXT_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AGL_TYPE_TEXT_BUFFER, AGlTextBufferClass))
#define AGL_IS_TEXT_BUFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGL_TYPE_TEXT_BUFFER))
#define AGL_IS_TEXT_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AGL_TYPE_TEXT_BUFFER))
#define AGL_TEXT_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), AGL_TYPE_TEXT_BUFFER, AGlTextBufferClass))

/**
 * AGL_TEXT_BUFFER_MAX_SIZE:
 *
 * Maximum size of text buffer, in bytes.
 */
#define AGL_TEXT_BUFFER_MAX_SIZE G_MAXUSHORT

typedef struct _AGlTextBuffer          AGlTextBuffer;
typedef struct _AGlTextBuffer          TextBuffer;
typedef struct _AGlTextBufferClass     AGlTextBufferClass;
typedef struct _AGlTextBufferPrivate   AGlTextBufferPrivate;

/**
 * TextBuffer:
 *
 * The #AGlTextBuffer structure contains private
 * data and it should only be accessed using the provided API.
 */
struct _AGlTextBuffer
{
   GObject parent_instance;

   AGlTextBufferPrivate* priv;
};

/**
 * TextBufferClass:
 * @inserted_text: default handler for the #AGlTextBuffer::inserted-text signal
 * @deleted_text: default hanlder for the #AGlTextBuffer::deleted-text signal
 * @get_text: virtual function
 * @get_length: virtual function
 * @insert_text: virtual function
 * @delete_text: virtual function
 *
 * The #TextBufferClass structure contains
 * only private data.
 */
struct _AGlTextBufferClass
{
  GObjectClass parent_class;

  /* Signals */
  void         (*inserted_text)  (TextBuffer*, guint position, const gchar* chars, guint n_chars);
  void         (*deleted_text)   (TextBuffer*, guint position, guint n_chars);

  /* Virtual Methods */
  const gchar* (*get_text)       (TextBuffer*, gsize* n_bytes);
  guint        (*get_length)     (TextBuffer*);
  guint        (*insert_text)    (TextBuffer*, guint position, const gchar*, guint n_chars);
  guint        (*delete_text)    (TextBuffer*, guint position, guint n_chars);
};

GType        agl_text_buffer_get_type       (void) G_GNUC_CONST;

TextBuffer*  agl_text_buffer_new            (void);
TextBuffer*  agl_text_buffer_new_with_text  (const gchar* text, gssize text_len);

gsize        agl_text_buffer_get_bytes      (TextBuffer*);
guint        agl_text_buffer_get_length     (TextBuffer*);
const gchar* agl_text_buffer_get_text       (TextBuffer*);
void         agl_text_buffer_set_text       (TextBuffer*, const gchar*, gint n_chars);
void         agl_text_buffer_set_max_length (TextBuffer*, gint max_length);
gint         agl_text_buffer_get_max_length (TextBuffer*);

guint        agl_text_buffer_insert_text    (TextBuffer*, guint position, const gchar* chars, gint n_chars);
guint        agl_text_buffer_delete_text    (TextBuffer*, guint position, gint n_chars);

G_END_DECLS

#endif
