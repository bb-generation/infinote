/* libinfinity - a GObject-based infinote implementation
 * Copyright (C) 2007, 2008, 2009 Armin Burgmeier <armin@arbur.net>
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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <libinftext/inf-text-buffer.h>
#include <libinfinity/common/inf-buffer.h>
#include <libinfinity/inf-marshal.h>

enum {
  INSERT_TEXT,
  ERASE_TEXT,

  LAST_SIGNAL
};

static guint text_buffer_signals[LAST_SIGNAL];

static void
inf_text_buffer_base_init(gpointer g_class)
{
  static gboolean initialized = FALSE;

  if(!initialized)
  {
    text_buffer_signals[INSERT_TEXT] = g_signal_new(
      "insert-text",
      INF_TEXT_TYPE_BUFFER,
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(InfTextBufferIface, insert_text),
      NULL, NULL,
      inf_marshal_VOID__UINT_BOXED_OBJECT,
      G_TYPE_NONE,
      3,
      G_TYPE_UINT,
      INF_TEXT_TYPE_CHUNK | G_SIGNAL_TYPE_STATIC_SCOPE,
      INF_TYPE_USER
    );

    text_buffer_signals[ERASE_TEXT] = g_signal_new(
      "erase-text",
      INF_TEXT_TYPE_BUFFER,
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(InfTextBufferIface, erase_text),
      NULL, NULL,
      inf_marshal_VOID__UINT_UINT_OBJECT,
      G_TYPE_NONE,
      3,
      G_TYPE_UINT,
      G_TYPE_UINT,
      INF_TYPE_USER
    );

    initialized = TRUE;
  }
}

GType
inf_text_buffer_get_type(void)
{
  static GType text_buffer_type = 0;

  if(!text_buffer_type)
  {
    static const GTypeInfo text_buffer_info = {
      sizeof(InfTextBufferIface),    /* class_size */
      inf_text_buffer_base_init,     /* base_init */
      NULL,                          /* base_finalize */
      NULL,                          /* class_init */
      NULL,                          /* class_finalize */
      NULL,                          /* class_data */
      0,                             /* instance_size */
      0,                             /* n_preallocs */
      NULL,                          /* instance_init */
      NULL                           /* value_table */
    };

    text_buffer_type = g_type_register_static(
      G_TYPE_INTERFACE,
      "InfTextBuffer",
      &text_buffer_info,
      0
    );

    g_type_interface_add_prerequisite(text_buffer_type, INF_TYPE_BUFFER);
  }

  return text_buffer_type;
}

/**
 * inf_text_buffer_get_encoding:
 * @buffer: A #InfTextBuffer.
 *
 * Returns the character encoding that the buffer uses. This means that all
 * #InfTextChunk return values are encoded in this encoding and all
 * #InfTextChunk parameters are expected to be encoded in that encoding.
 *
 * Return Value: The character encoding for @buffer.
 **/
const gchar*
inf_text_buffer_get_encoding(InfTextBuffer* buffer)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), NULL);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->get_encoding != NULL, NULL);

  return iface->get_encoding(buffer);
}

/**
 * inf_text_buffer_get_length:
 * @buffer: A #InfTextBuffer.
 *
 * Returns the number of characters in @buffer.
 *
 * Return Value: The length of @buffer.
 **/
guint
inf_text_buffer_get_length(InfTextBuffer* buffer)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), 0);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->get_length != NULL, 0);

  return iface->get_length(buffer);
}

/**
 * inf_text_buffer_get_slice:
 * @buffer: A #InfTextBuffer.
 * @pos: Character offset of where to start extracting.
 * @len: Number of characters to extract.
 *
 * Reads @len characters, starting at @pos, from the buffer, and returns them
 * as a #InfTextChunk.
 *
 * Return Value: A #InfTextChunk.
 **/
InfTextChunk*
inf_text_buffer_get_slice(InfTextBuffer* buffer,
                          guint pos,
                          guint len)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), NULL);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->get_slice != NULL, NULL);

  return iface->get_slice(buffer, pos, len);
}

/**
 * inf_text_buffer_insert_text:
 * @buffer: A #InfTextBuffer.
 * @pos: A character offset into @buffer.
 * @text: A pointer to the text to insert.
 * @len: The length (in characters) of @text.
 * @bytes: The length (in bytes) of @text.
 * @user: A #InfUser that has inserted the new text, or %NULL.
 *
 * Inserts @text into @buffer as written by @author. @text must be encoded in
 * the character encoding of the buffer, see inf_text_buffer_get_encoding().
 **/
void
inf_text_buffer_insert_text(InfTextBuffer* buffer,
                            guint pos,
                            gconstpointer text,
                            gsize bytes,
                            guint len,
                            InfUser* user)
{
  InfTextChunk* chunk;

  g_return_if_fail(INF_TEXT_IS_BUFFER(buffer));
  g_return_if_fail(text != NULL);
  g_return_if_fail(user == NULL || INF_IS_USER(user));

  chunk = inf_text_chunk_new(inf_text_buffer_get_encoding(buffer));

  inf_text_chunk_insert_text(
    chunk,
    0,
    text,
    bytes,
    len,
    user == NULL ? 0 : inf_user_get_id(user)
  );

  g_signal_emit(
    G_OBJECT(buffer),
    text_buffer_signals[INSERT_TEXT],
    0,
    pos,
    chunk,
    user
  );

  inf_text_chunk_free(chunk);
}

/**
 * inf_text_buffer_insert_chunk:
 * @buffer: A #InfTextBuffer.
 * @pos: A character offset into @buffer.
 * @chunk: A #InfTextChunk.
 * @user: A #InfUser inserting @chunk, or %NULL.
 *
 * Inserts a #InfTextChunk into @buffer. @user must not necessarily be the
 * author of @chunk (@chunk may even consist of multiple segments). This
 * happens when undoing a delete operation that erased another user's text.
 **/
void
inf_text_buffer_insert_chunk(InfTextBuffer* buffer,
                             guint pos,
                             InfTextChunk* chunk,
                             InfUser* user)
{
  g_return_if_fail(INF_TEXT_IS_BUFFER(buffer));
  g_return_if_fail(chunk != NULL);
  g_return_if_fail(user == NULL || INF_IS_USER(user));

  g_signal_emit(
    G_OBJECT(buffer),
    text_buffer_signals[INSERT_TEXT],
    0,
    pos,
    chunk,
    user
  );
}

/**
 * inf_text_buffer_erase_text:
 * @buffer: A #InfTextBuffer.
 * @pos: The position to begin deleting characters from.
 * @len: The amount of characters to delete.
 * @user: A #InfUser that erases the text, or %NULL.
 *
 * Erases characters from the text buffer.
 **/
void
inf_text_buffer_erase_text(InfTextBuffer* buffer,
                           guint pos,
                           guint len,
                           InfUser* user)
{
  g_return_if_fail(INF_TEXT_IS_BUFFER(buffer));
  g_return_if_fail(user == NULL || INF_IS_USER(user));

  g_signal_emit(
    G_OBJECT(buffer),
    text_buffer_signals[ERASE_TEXT],
    0,
    pos,
    len,
    user
  );
}

/**
 * inf_text_buffer_create_iter:
 * @buffer: A #InfTextBuffer.
 *
 * Creates a #InfTextBufferIter pointing to the first segmnet of @buffer.
 * A #InfTextBufferIter is used to traverse the buffer contents in steps of
 * so-called segments each of which is written by the same user. The function
 * returns %NULL if there are no segments (i.e. the buffer is empty).
 *
 * The iterator stays valid as long as the buffer remains unmodified and
 * must be freed with inf_text_buffer_destroy_iter() before.
 *
 * Return Value: A #InfTextBufferIter to be freed by
 * inf_text_buffer_destroy_iter() when done using it, or %NULL.
 **/
InfTextBufferIter*
inf_text_buffer_create_iter(InfTextBuffer* buffer)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), NULL);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->create_iter != NULL, NULL);

  return iface->create_iter(buffer);
}

/**
 * inf_text_buffer_destroy_iter:
 * @buffer: A #InfTextBuffer.
 * @iter: A #InfTextBufferIter pointing into @buffer.
 *
 * Destroys a #InfTextBufferIter created by inf_text_buffer_create_iter().
 **/
void
inf_text_buffer_destroy_iter(InfTextBuffer* buffer,
                             InfTextBufferIter* iter)
{
  InfTextBufferIface* iface;

  g_return_if_fail(INF_TEXT_IS_BUFFER(buffer));
  g_return_if_fail(iter != NULL);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_if_fail(iface->destroy_iter != NULL);

  iface->destroy_iter(buffer, iter);
}

/**
 * inf_text_buffer_iter_next:
 * @buffer: A #InfTextBuffer.
 * @iter: A #InfTextBufferIter pointing into @buffer.
 *
 * Moves @iter to point to the next segment in the buffer. If @iter already
 * points to the last segment, @iter is left unmodified and the function
 * returns %FALSE.
 *
 * Return Value: Whether @iter was moved.
 **/
gboolean
inf_text_buffer_iter_next(InfTextBuffer* buffer,
                          InfTextBufferIter* iter)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), FALSE);
  g_return_val_if_fail(iter != NULL, FALSE);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->iter_next != NULL, FALSE);

  return iface->iter_next(buffer, iter);
}

/**
 * inf_text_buffer_iter_prev:
 * @buffer: A #InfTextBuffer.
 * @iter: A #InfTextBufferIter pointing into @buffer.
 *
 * Moves @iter to point to the previous segment in the buffer. If @iter
 * already points to the first segment, @iter is left unmodified and the
 * function returns %FALSE.
 *
 * Return Value: Whether @iter was moved.
 **/
gboolean
inf_text_buffer_iter_prev(InfTextBuffer* buffer,
                          InfTextBufferIter* iter)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), FALSE);
  g_return_val_if_fail(iter != NULL, FALSE);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->iter_prev != NULL, FALSE);

  return iface->iter_prev(buffer, iter);
}

/**
 * inf_text_buffer_iter_get_text:
 * @buffer: A #InfTextBuffer.
 * @iter: A #InfTextBufferIter pointing into @buffer.
 *
 * Returns the text of the segment @iter points to. It is encoded in
 * @buffer's encoding (see inf_text_buffer_get_encoding()).
 *
 * Return Value: The text of the segment @iter points to. Free with g_free()
 * when done using it.
 **/
gpointer
inf_text_buffer_iter_get_text(InfTextBuffer* buffer,
                              InfTextBufferIter* iter)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), NULL);
  g_return_val_if_fail(iter != NULL, NULL);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->iter_get_text != NULL, NULL);

  return iface->iter_get_text(buffer, iter);
}

/**
 * inf_text_buffer_iter_get_length:
 * @buffer: A #InfTextBuffer.
 * @iter: A #InfTextBufferIter pointing into @buffer.
 *
 * Returns the length of the segment @iter points to, in characters.
 *
 * Return Value: The number of characters of the segment @iter points to.
 **/
guint
inf_text_buffer_iter_get_length(InfTextBuffer* buffer,
                                InfTextBufferIter* iter)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), 0);
  g_return_val_if_fail(iter != NULL, 0);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->iter_get_length != NULL, 0);

  return iface->iter_get_length(buffer, iter);
}

/**
 * inf_text_buffer_iter_get_bytes:
 * @buffer: A #InfTextBuffer.
 * @iter: A #InfTextBufferIter pointing into @buffer.
 *
 * Returns the length of the segment @iter points to, in bytes.
 *
 * Return Value: The number of bytes of the segment @iter points to.
 **/
gsize
inf_text_buffer_iter_get_bytes(InfTextBuffer* buffer,
                               InfTextBufferIter* iter)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), 0);
  g_return_val_if_fail(iter != NULL, 0);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->iter_get_bytes != NULL, 0);

  return iface->iter_get_bytes(buffer, iter);
}

/**
 * inf_text_buffer_iter_get_author:
 * @buffer: A #InfTextBuffer.
 * @iter: A #InfTextBufferIter pointing into @buffer.
 *
 * Returns the user ID of the user that has written the segment @iter points
 * to.
 *
 * Return Value: The user ID of the user that wrote the segment @iter points
 * to.
 **/
guint
inf_text_buffer_iter_get_author(InfTextBuffer* buffer,
                                InfTextBufferIter* iter)
{
  InfTextBufferIface* iface;

  g_return_val_if_fail(INF_TEXT_IS_BUFFER(buffer), 0);
  g_return_val_if_fail(iter != NULL, 0);

  iface = INF_TEXT_BUFFER_GET_IFACE(buffer);
  g_return_val_if_fail(iface->iter_get_author != NULL, 0);

  return iface->iter_get_author(buffer, iter);
}

/* vim:set et sw=2 ts=2: */
