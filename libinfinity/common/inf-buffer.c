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

/**
 * SECTION:inf-buffer
 * @title: InfBuffer
 * @short_description: Abstract document interface
 * @include: libinfinity/common/inf-buffer.h
 *
 * #InfBuffer represents a document containing a session's content. It does
 * not cope with keeping its content in-sync with other participants but just
 * offers an interface to modify the document.
 *
 * The #InfBuffer interface itself is probably not too useful, but actual
 * documents implementing functionality (such as text editing or graphics
 * editing) need to implement this interface to be passed to #InfSession.
 **/

#include <libinfinity/common/inf-buffer.h>

static void
inf_buffer_base_init(gpointer g_class)
{
  static gboolean initialized = FALSE;

  if(!initialized)
  {
    g_object_interface_install_property(
      g_class,
      g_param_spec_boolean(
        "modified",
        "Modified",
        "Whether the buffer was modified since it has been saved",
        FALSE,
        G_PARAM_READWRITE
      )
    );

    initialized = TRUE;
  }
}

GType
inf_buffer_get_type(void)
{
  static GType buffer_type = 0;

  if(!buffer_type)
  {
    static const GTypeInfo buffer_info = {
      sizeof(InfBufferIface),        /* class_size */
      inf_buffer_base_init,          /* base_init */
      NULL,                          /* base_finalize */
      NULL,                          /* class_init */
      NULL,                          /* class_finalize */
      NULL,                          /* class_data */
      0,                             /* instance_size */
      0,                             /* n_preallocs */
      NULL,                          /* instance_init */
      NULL                           /* value_table */
    };

    buffer_type = g_type_register_static(
      G_TYPE_INTERFACE,
      "InfBuffer",
      &buffer_info,
      0
    );

    g_type_interface_add_prerequisite(buffer_type, G_TYPE_OBJECT);
  }

  return buffer_type;
}

/**
 * inf_buffer_get_modified:
 * @buffer: A #InfBuffer.
 *
 * Indicates whether the buffer has been modified since the last call to
 * inf_buffer_set_modified() set the modification flag to %FALSE.
 *
 * Returns: Whether the buffer has been modified.
 */
gboolean
inf_buffer_get_modified(InfBuffer* buffer)
{
  InfBufferIface* iface;
  gboolean modified;

  g_return_val_if_fail(INF_IS_BUFFER(buffer), FALSE);

  iface = INF_BUFFER_GET_IFACE(buffer);
  if(iface->get_modified != NULL)
  {
    return iface->get_modified(buffer);
  }
  else
  {
    g_object_get(G_OBJECT(buffer), "modified", &modified, NULL);
    return modified;
  }
}

/**
 * inf_buffer_set_modified:
 * @buffer: A #InfBuffer.
 * @modified: Whether the buffer is considered modified or not.
 *
 * Sets the modification flag of @buffer to @modified. You should normally set
 * the flag to %FALSE every time the document is saved onto disk. The buffer
 * itself will set it to %TRUE when it has been changed.
 *
 * To get notified when the modification flag changes, connect to
 * GObject::notify for the InfBuffer:modified property.
 */
void
inf_buffer_set_modified(InfBuffer* buffer,
                        gboolean modified)
{
  InfBufferIface* iface;

  g_return_if_fail(INF_IS_BUFFER(buffer));

  iface = INF_BUFFER_GET_IFACE(buffer);
  if(iface->set_modified != NULL)
  {
    iface->set_modified(buffer, modified);
  }
  else
  {
    g_object_set(G_OBJECT(buffer), "modified", modified, NULL);
  }
}

/* vim:set et sw=2 ts=2: */
