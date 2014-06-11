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

#ifndef GRAVITON_INTROSPECTION_CONTROL_H
#define GRAVITON_INTROSPECTION_CONTROL_H

#include <glib-object.h>
#include "service.h"

#define GRAVITON_INTROSPECTION_CONTROL_TYPE     (graviton_internal_plugin_get_type ())
#define GRAVITON_INTROSPECTION_CONTROL(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_INTROSPECTION_CONTROL_TYPE, GravitonIntrospectionControl))
#define GRAVITON_IS_INTERNAL_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_INTROSPECTION_CONTROL_TYPE))
#define GRAVITON_INTROSPECTION_CONTROL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_INTROSPECTION_CONTROL_TYPE, GravitonIntrospectionControlClass))
#define GRAVITON_IS_INTERNAL_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_INTROSPECTION_CONTROL_TYPE))
#define GRAVITON_INTROSPECTION_CONTROL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_INTROSPECTION_CONTROL_TYPE, GravitonIntrospectionControlClass))

#define GRAVITON_INTROSPECTION_ERROR (graviton_introspection_error_quark ())

typedef enum {
  GRAVITON_INTROSPECTION_ERROR_NO_SUCH_PLUGIN,
  GRAVITON_INTROSPECTION_ERROR_NO_SUCH_CONTROL,
  GRAVITON_INTROSPECTION_ERROR_NO_SUCH_PROPERTY
} GravitonInspectionError;

typedef struct _GravitonIntrospectionControl GravitonIntrospectionControl;
typedef struct _GravitonIntrospectionControlClass GravitonIntrospectionControlClass;

typedef struct _GravitonIntrospectionControlPrivate GravitonIntrospectionControlPrivate;

struct _GravitonIntrospectionControl
{
  GravitonService parent_instance;
  GravitonIntrospectionControlPrivate *priv;
};

struct _GravitonIntrospectionControlClass
{
  GravitonServiceClass parent_class;
};

GType graviton_internal_plugin_get_type ();

#endif // GRAVITON_INTROSPECTION_CONTROL_H
