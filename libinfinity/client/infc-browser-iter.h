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

#ifndef __INFC_BROWSER_ITER_H__
#define __INFC_BROWSER_ITER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define INFC_TYPE_BROWSER_ITER            (infc_browser_iter_get_type())

typedef struct _InfcBrowserIter InfcBrowserIter;
struct _InfcBrowserIter {
  guint node_id;
  gpointer node;
};

GType
infc_browser_iter_get_type(void) G_GNUC_CONST;

/* TODO: Do these two need to be public? */
InfcBrowserIter*
infc_browser_iter_copy(InfcBrowserIter* iter);

void
infc_browser_iter_free(InfcBrowserIter* iter);

G_END_DECLS

#endif /* __INFC_BROWSER_ITER_H__ */

/* vim:set et sw=2 ts=2: */
