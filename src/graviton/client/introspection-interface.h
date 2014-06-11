/*
 * This file is part of Graviton.
 * Copyright (C) 2013-2014 Torrie Fischer <tdfischer@phrobo.net>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef __GRAVITON_INTROSPECTION_INTERFACE_H__
#define __GRAVITON_INTROSPECTION_INTERFACE_H__

#include <glib.h>
#include <glib-object.h>
#include "service-interface.h"

G_BEGIN_DECLS

#define GRAVITON_INTROSPECTION_INTERFACE_TYPE            (graviton_introspection_interface_get_type ())
#define GRAVITON_INTROSPECTION_INTERFACE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_INTROSPECTION_INTERFACE_TYPE, GravitonIntrospectionControl))
#define GRAVITON_INTROSPECTION_INTERFACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_INTROSPECTION_INTERFACE_TYPE, GravitonIntrospectionControlClass))
#define IS_GRAVITON_INTROSPECTION_INTERFACE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_INTROSPECTION_INTERFACE_TYPE))
#define IS_GRAVITON_INTROSPECTION_INTERFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_INTROSPECTION_INTERFACE_TYPE))
#define GRAVITON_INTROSPECTION_INTERFACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_INTROSPECTION_INTERFACE_TYPE, GravitonIntrospectionControlClass))

typedef struct _GravitonIntrospectionControlPrivate GravitonIntrospectionControlPrivate;

typedef struct _GravitonIntrospectionControl      GravitonIntrospectionControl;
typedef struct _GravitonIntrospectionControlClass GravitonIntrospectionControlClass;

struct _GravitonIntrospectionControlClass
{
  GravitonServiceInterfaceClass parent_class;
};

struct _GravitonIntrospectionControl
{
  GravitonServiceInterface parent;
  GravitonIntrospectionControlPrivate *priv;
};

GType graviton_introspection_interface_get_type (void);

GList *graviton_introspection_interface_list_interfaces (GravitonIntrospectionControl *self, GError **error);
GList *graviton_introspection_interface_list_properties (GravitonIntrospectionControl *self, GError **error);
GList *graviton_introspection_interface_list_streams (GravitonIntrospectionControl *self, GError **error);
GList *graviton_introspection_interface_list_methods (GravitonIntrospectionControl *self, GError **error);
GravitonIntrospectionControl *graviton_introspection_interface_new_from_interface (GravitonServiceInterface *service);
GravitonIntrospectionControl *graviton_introspection_interface_new (GravitonNode *node, const gchar *name);

G_END_DECLS

#endif
