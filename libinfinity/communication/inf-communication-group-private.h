/* libinfinity - a GObject-based infinote implementation
 * Copyright (C) 2007, 2008 Armin Burgmeier <armin@arbur.net>
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

#ifndef __INF_COMMUNICATION_GROUP_PRIVATE_H__
#define __INF_COMMUNICATION_GROUP_PRIVATE_H__

#include <libinfinity/communication/inf-communication-group.h>

void
_inf_communication_group_add_member(InfCommunicationGroup* group,
                                    InfXmlConnection* connection);

void
_inf_communication_group_remove_member(InfCommunicationGroup* group,
                                       InfXmlConnection* connection);

#endif /* __INF_COMMUNICATION_GROUP_PRIVATE_H__ */

/* vim:set et sw=2 ts=2: */
