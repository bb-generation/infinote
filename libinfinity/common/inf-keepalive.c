/* libinfinity - a GObject-based infinote implementation
 * Copyright (C) 2007-2011 Armin Burgmeier <armin@arbur.net>
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

#include <libinfinity/common/inf-keepalive.h>


InfKeepalive*
inf_keepalive_new(void)
{
  InfKeepalive* keepalive;
  keepalive = g_slice_new(InfKeepalive);
  keepalive->use_keepalive = 1;
  keepalive->keepalive_time = 15;
  keepalive->keepalive_interval = 15;
  keepalive->keepalive_probes = 2;
  return keepalive;
}

GType
inf_keepalive_get_type(void)
{
  static GType keepalive_type = 0;

  if(!keepalive_type)
  {
      keepalive_type = g_boxed_type_register_static(
      "InfKeepalive",
      (GBoxedCopyFunc)inf_keepalive_copy,
      (GBoxedFreeFunc)inf_keepalive_free
    );
  }

  return keepalive_type;
}

InfKeepalive*
inf_keepalive_copy(const InfKeepalive* keepalive)
{
  InfKeepalive* keepalive_new = inf_keepalive_new();
  keepalive_new->use_keepalive = keepalive->use_keepalive;
  keepalive_new->keepalive_time = keepalive->keepalive_time;
  keepalive_new->keepalive_interval = keepalive->keepalive_interval;
  keepalive_new->keepalive_probes = keepalive->keepalive_probes;
  return keepalive_new;
}

void
inf_keepalive_free(InfKeepalive* keepalive)
{
  g_slice_free(InfKeepalive, keepalive);
}


