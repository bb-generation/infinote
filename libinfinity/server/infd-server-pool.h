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

#ifndef __INFD_SERVER_POOL_H__
#define __INFD_SERVER_POOL_H__

#include <libinfinity/server/infd-directory.h>
#include <libinfinity/server/infd-xmpp-server.h>
#include <libinfinity/server/infd-xml-server.h>
#include <libinfinity/common/inf-local-publisher.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define INFD_TYPE_SERVER_POOL                 (infd_server_pool_get_type())
#define INFD_SERVER_POOL(obj)                 (G_TYPE_CHECK_INSTANCE_CAST((obj), INFD_TYPE_SERVER_POOL, InfdServerPool))
#define INFD_SERVER_POOL_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST((klass), INFD_TYPE_SERVER_POOL, InfdServerPoolClass))
#define INFD_IS_SERVER_POOL(obj)              (G_TYPE_CHECK_INSTANCE_TYPE((obj), INFD_TYPE_SERVER_POOL))
#define INFD_IS_SERVER_POOL_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE((klass), INFD_TYPE_SERVER_POOL))
#define INFD_SERVER_POOL_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS((obj), INFD_TYPE_SERVER_POOL, InfdServerPoolClass))

typedef struct _InfdServerPool InfdServerPool;
typedef struct _InfdServerPoolClass InfdServerPoolClass;

struct _InfdServerPoolClass {
  GObjectClass parent_class;
};

struct _InfdServerPool {
  GObject parent;
};

GType
infd_server_pool_get_type(void) G_GNUC_CONST;

InfdServerPool*
infd_server_pool_new(InfdDirectory* directory);

void
infd_server_pool_add_server(InfdServerPool* server_pool,
                            InfdXmlServer* server);

void
infd_server_pool_add_local_publisher(InfdServerPool* server_pool,
                                     InfdXmppServer* server,
                                     InfLocalPublisher* publisher);

G_END_DECLS

#endif /* __INFD_SERVER_POOL_H__ */

/* vim:set et sw=2 ts=2: */
