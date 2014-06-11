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

#ifndef GRAVITON_ROOT_SERVICE_H
#define GRAVITON_ROOT_SERVICE_H

#include <glib-object.h>
#include "service.h"

#define GRAVITON_ROOT_SERVICE_TYPE            (graviton_root_service_get_type ())
#define GRAVITON_ROOT_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_ROOT_SERVICE_TYPE, GravitonRootService))
#define GRAVITON_IS_ROOT_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_ROOT_SERVICE_TYPE))
#define GRAVITON_ROOT_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_ROOT_SERVICE_TYPE, GravitonRootServiceClass))
#define GRAVITON_IS_ROOT_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_ROOT_SERVICE_TYPE))
#define GRAVITON_PLUGIN_GET_MANAGER_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_ROOT_SERVICE_TYPE, GravitonRootServiceClass))

typedef struct _GravitonRootService GravitonRootService;
typedef struct _GravitonRootServiceClass GravitonRootServiceClass;

typedef struct _GravitonRootServicePrivate GravitonRootServicePrivate;

struct _GravitonRootService
{
  GravitonService parent_instance;
};

struct _GravitonRootServiceClass
{
  GObjectClass parent_class;
};

typedef struct _GravitonPlugin GravitonPlugin;

GType graviton_root_service_get_type ();
GravitonRootService *graviton_root_service_new ();

GArray *graviton_root_service_find_plugins (GravitonRootService *manager);

#endif // GRAVITON_PLUGIN_H
