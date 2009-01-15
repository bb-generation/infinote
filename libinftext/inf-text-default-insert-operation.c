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

#include <libinftext/inf-text-default-insert-operation.h>
#include <libinftext/inf-text-default-delete-operation.h>
#include <libinftext/inf-text-insert-operation.h>
#include <libinftext/inf-text-delete-operation.h>
#include <libinftext/inf-text-buffer.h>

#include <libinfinity/adopted/inf-adopted-operation.h>

typedef struct _InfTextDefaultInsertOperationPrivate
  InfTextDefaultInsertOperationPrivate;
struct _InfTextDefaultInsertOperationPrivate {
  guint position;
  InfTextChunk* chunk;
};

enum {
  PROP_0,

  PROP_POSITION,
  PROP_CHUNK
};

#define INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), INF_TEXT_TYPE_DEFAULT_INSERT_OPERATION, InfTextDefaultInsertOperationPrivate))

static GObjectClass* parent_class;

static void
inf_text_default_insert_operation_init(GTypeInstance* instance,
                                        gpointer g_class)
{
  InfTextDefaultInsertOperation* operation;
  InfTextDefaultInsertOperationPrivate* priv;

  operation = INF_TEXT_DEFAULT_INSERT_OPERATION(instance);
  priv = INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation);

  priv->position = 0;
  priv->chunk = NULL;
}

static void
inf_text_default_insert_operation_finalize(GObject* object)
{
  InfTextDefaultInsertOperation* operation;
  InfTextDefaultInsertOperationPrivate* priv;

  operation = INF_TEXT_DEFAULT_INSERT_OPERATION(object);
  priv = INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation);

  inf_text_chunk_free(priv->chunk);

  G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void
inf_text_default_insert_operation_set_property(GObject* object,
                                               guint prop_id,
                                               const GValue* value,
                                               GParamSpec* pspec)
{
  InfTextDefaultInsertOperation* operation;
  InfTextDefaultInsertOperationPrivate* priv;

  operation = INF_TEXT_DEFAULT_INSERT_OPERATION(object);
  priv = INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation);

  switch(prop_id)
  {
  case PROP_POSITION:
    priv->position = g_value_get_uint(value);
    break;
  case PROP_CHUNK:
    g_assert(priv->chunk == NULL); /* construct only */
    priv->chunk = (InfTextChunk*)g_value_dup_boxed(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(value, prop_id, pspec);
    break;
  }
}

static void
inf_text_default_insert_operation_get_property(GObject* object,
                                               guint prop_id,
                                               GValue* value,
                                               GParamSpec* pspec)
{
  InfTextDefaultInsertOperation* operation;
  InfTextDefaultInsertOperationPrivate* priv;

  operation = INF_TEXT_DEFAULT_INSERT_OPERATION(object);
  priv = INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation);

  switch(prop_id)
  {
  case PROP_POSITION:
    g_value_set_uint(value, priv->position);
    break;
  case PROP_CHUNK:
    g_value_set_boxed(value, priv->chunk);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static gboolean
inf_text_default_insert_operation_need_concurrency_id(
  InfAdoptedOperation* operation,
  InfAdoptedOperation* against)
{
  g_assert(INF_TEXT_IS_DEFAULT_INSERT_OPERATION(operation));

  return inf_text_insert_operation_need_concurrency_id(
    INF_TEXT_INSERT_OPERATION(operation),
    against
  );
}

static InfAdoptedConcurrencyId
inf_text_default_insert_operation_get_concurrency_id(
  InfAdoptedOperation* operation,
  InfAdoptedOperation* against)
{
  g_assert(INF_TEXT_IS_DEFAULT_INSERT_OPERATION(operation));

  return inf_text_insert_operation_get_concurrency_id(
    INF_TEXT_INSERT_OPERATION(operation),
    against
  );
}


static InfAdoptedOperation*
inf_text_default_insert_operation_transform(InfAdoptedOperation* operation,
                                            InfAdoptedOperation* against,
                                            InfAdoptedConcurrencyId cid)
{
  g_assert(INF_TEXT_IS_DEFAULT_INSERT_OPERATION(operation));

  if(INF_TEXT_IS_INSERT_OPERATION(against))
  {
    return inf_text_insert_operation_transform_insert(
      INF_TEXT_INSERT_OPERATION(operation),
      INF_TEXT_INSERT_OPERATION(against),
      cid
    );
  }
  else if(INF_TEXT_IS_DELETE_OPERATION(against))
  {
    return inf_text_insert_operation_transform_delete(
      INF_TEXT_INSERT_OPERATION(operation),
      INF_TEXT_DELETE_OPERATION(against),
      cid
    );
  }
  else
  {
    g_assert_not_reached();
    return NULL;
  }
}

static InfAdoptedOperation*
inf_text_default_insert_operation_copy(InfAdoptedOperation* operation)
{
  InfTextDefaultInsertOperationPrivate* priv;
  priv = INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation);

  return INF_ADOPTED_OPERATION(
    g_object_new(
      INF_TEXT_TYPE_DEFAULT_INSERT_OPERATION,
      "position", priv->position,
      "chunk", priv->chunk,
      NULL
    )
  );
}

static InfAdoptedOperationFlags
inf_text_default_insert_operation_get_flags(InfAdoptedOperation* operation)
{
  return INF_ADOPTED_OPERATION_AFFECTS_BUFFER |
         INF_ADOPTED_OPERATION_REVERSIBLE;
}

static void
inf_text_default_insert_operation_apply(InfAdoptedOperation* operation,
                                        InfAdoptedUser* by,
                                        InfBuffer* buffer)
{
  InfTextDefaultInsertOperationPrivate* priv;

  g_assert(INF_TEXT_IS_DEFAULT_INSERT_OPERATION(operation));
  g_assert(INF_TEXT_IS_BUFFER(buffer));

  priv = INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation);

  inf_text_buffer_insert_chunk(
    INF_TEXT_BUFFER(buffer),
    priv->position,
    priv->chunk,
    INF_USER(by)
  );
}

static InfAdoptedOperation*
inf_text_default_insert_operation_revert(InfAdoptedOperation* operation)
{
  InfTextDefaultInsertOperationPrivate* priv;
  priv = INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation);

  return INF_ADOPTED_OPERATION(
    inf_text_default_delete_operation_new(priv->position, priv->chunk)
  );
}

static guint
inf_text_default_insert_operation_get_position(InfTextInsertOperation* op)
{
  return INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(op)->position;
}

static guint
inf_text_default_insert_operation_get_length(
  InfTextInsertOperation* operation)
{
  return inf_text_chunk_get_length(
    INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation)->chunk
  );
}

static InfTextInsertOperation*
inf_text_default_insert_operation_transform_position(
  InfTextInsertOperation* operation,
  guint position)
{
  InfTextDefaultInsertOperationPrivate* priv;
  GObject* result;

  priv = INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation);

  result = g_object_new(
    INF_TEXT_TYPE_DEFAULT_INSERT_OPERATION,
    "position", position,
    "chunk", priv->chunk,
    NULL
  );

  return INF_TEXT_INSERT_OPERATION(result);
}

static void
inf_text_default_insert_operation_class_init(gpointer g_class,
                                             gpointer class_data)
{
  GObjectClass* object_class;
  object_class = G_OBJECT_CLASS(g_class);

  parent_class = G_OBJECT_CLASS(g_type_class_peek_parent(g_class));
  g_type_class_add_private(
    g_class,
    sizeof(InfTextDefaultInsertOperationPrivate)
  );

  object_class->finalize = inf_text_default_insert_operation_finalize;
  object_class->set_property =
    inf_text_default_insert_operation_set_property;
  object_class->get_property =
    inf_text_default_insert_operation_get_property;

  g_object_class_install_property(
    object_class,
    PROP_POSITION,
    g_param_spec_uint(
      "position",
      "Position",
      "Insertion position",
      0,
      G_MAXUINT,
      0,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
    )
  );

  g_object_class_install_property(
    object_class,
    PROP_CHUNK,
    g_param_spec_boxed(
      "chunk",
      "Chunk",
      "The text to insert",
      INF_TEXT_TYPE_CHUNK,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
    )
  );
}

static void
inf_text_default_insert_operation_operation_init(gpointer g_iface,
                                                 gpointer iface_data)
{
  InfAdoptedOperationIface* iface;
  iface = (InfAdoptedOperationIface*)g_iface;

  iface->need_concurrency_id =
    inf_text_default_insert_operation_need_concurrency_id;
  iface->get_concurrency_id =
    inf_text_default_insert_operation_get_concurrency_id;
  iface->transform = inf_text_default_insert_operation_transform;
  iface->copy = inf_text_default_insert_operation_copy;
  iface->get_flags = inf_text_default_insert_operation_get_flags;
  iface->apply = inf_text_default_insert_operation_apply;
  iface->revert = inf_text_default_insert_operation_revert;
  /* DefaultInsertOperation is always reversible */
  iface->make_reversible = NULL;
}

static void
inf_text_default_insert_operation_insert_operation_init(gpointer g_iface,
                                                        gpointer iface_data)
{
  InfTextInsertOperationIface* iface;
  iface = (InfTextInsertOperationIface*)g_iface;

  iface->get_position = inf_text_default_insert_operation_get_position;
  iface->get_length = inf_text_default_insert_operation_get_length;
  iface->transform_position =
    inf_text_default_insert_operation_transform_position;
}

GType
inf_text_default_insert_operation_get_type(void)
{
  static GType default_insert_operation_type = 0;

  if(!default_insert_operation_type)
  {
    static const GTypeInfo default_insert_operation_type_info = {
      sizeof(InfTextDefaultInsertOperationClass),   /* class_size */
      NULL,                                         /* base_init */
      NULL,                                         /* base_finalize */
      inf_text_default_insert_operation_class_init, /* class_init */
      NULL,                                         /* class_finalize */
      NULL,                                         /* class_data */
      sizeof(InfTextDefaultInsertOperation),        /* instance_size */
      0,                                            /* n_preallocs */
      inf_text_default_insert_operation_init,       /* instance_init */
      NULL                                          /* value_table */
    };

    static const GInterfaceInfo operation_info = {
      inf_text_default_insert_operation_operation_init,
      NULL,
      NULL
    };

    static const GInterfaceInfo insert_operation_info = {
      inf_text_default_insert_operation_insert_operation_init,
      NULL,
      NULL
    };

    default_insert_operation_type = g_type_register_static(
      G_TYPE_OBJECT,
      "InfTextDefaultInsertOperation",
      &default_insert_operation_type_info,
      0
    );

    g_type_add_interface_static(
      default_insert_operation_type,
      INF_ADOPTED_TYPE_OPERATION,
      &operation_info
    );

    g_type_add_interface_static(
      default_insert_operation_type,
      INF_TEXT_TYPE_INSERT_OPERATION,
      &insert_operation_info
    );
  }

  return default_insert_operation_type;
}

/**
 * inf_text_default_insert_operation_new:
 * @pos: The position at which to insert text.
 * @chunk: The text to insert.
 *
 * Creates a new insert operation that, when applied, inserts @chunk
 * at @pos.
 *
 * Return Value: A new #InfTextDefaultInsertOperation.
 **/
InfTextDefaultInsertOperation*
inf_text_default_insert_operation_new(guint pos,
                                      InfTextChunk* chunk)
{
  GObject* object;

  g_return_val_if_fail(chunk != NULL, NULL);

  object = g_object_new(
    INF_TEXT_TYPE_DEFAULT_INSERT_OPERATION,
    "position", pos,
    "chunk", chunk,
    NULL
  );

  return INF_TEXT_DEFAULT_INSERT_OPERATION(object);
}

/**
 * inf_text_default_insert_operation_get_chunk:
 * @operation: A #InfTextDefaultInsertOperation.
 *
 * Returns the text inserted by @operation.
 *
 * Return Value: A #InfTextChunk, owned by the operation.
 **/
InfTextChunk*
inf_text_default_insert_operation_get_chunk(InfTextDefaultInsertOperation* operation)
{
  g_return_val_if_fail(INF_TEXT_IS_DEFAULT_INSERT_OPERATION(operation), NULL);
  return INF_TEXT_DEFAULT_INSERT_OPERATION_PRIVATE(operation)->chunk;
}

/* vim:set et sw=2 ts=2: */
