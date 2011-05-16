/* libinfinity - a GObject-based infinote implementation
 * Copyright (C) 2007-2010 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef __INF_KEEPALIVE_H__
#define __INF_KEEPALIVE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define INF_KEEPALIVE_USE_DEFAULT 1
#define INF_KEEPALIVE_TIME_DEFAULT 15
#define INF_KEEPALIVE_INTERVAL_DEFAULT 15
#define INF_KEEPALIVE_PROBES_DEFAULT 2


#define INF_TYPE_KEEPALIVE                 (inf_keepalive_get_type())

typedef struct _InfKeepalive InfKeepalive;

struct _InfKeepalive {
  gint use_keepalive;
  gint keepalive_time;
  gint keepalive_interval;
  gint keepalive_probes;
};

GType
inf_keepalive_get_type(void) G_GNUC_CONST;

InfKeepalive*
inf_keepalive_new(void);

InfKeepalive*
inf_keepalive_copy(const InfKeepalive* keepalive);

void
inf_keepalive_free(InfKeepalive* keepalive);

G_END_DECLS

#endif /* __INF_KEEPALIVE_H__ */
