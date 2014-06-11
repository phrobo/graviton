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

#ifndef __GRAVITON_NEIGHBOR_DISCOVERY_METHOD_H__
#define __GRAVITON_NEIGHBOR_DISCOVERY_METHOD_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE            (graviton_neighbor_discovery_method_get_type ())
#define GRAVITON_NEIGHBOR_DISCOVERY_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE, GravitonNeighborDiscoveryMethod))
#define GRAVITON_NEIGHBOR_DISCOVERY_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE, GravitonNeighborDiscoveryMethodClass))
#define IS_GRAVITON_NEIGHBOR_DISCOVERY_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE))
#define IS_GRAVITON_NEIGHBOR_DISCOVERY_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE))
#define GRAVITON_NEIGHBOR_DISCOVERY_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE, GravitonNeighborDiscoveryMethodClass))

typedef struct _GravitonNeighborDiscoveryMethod      GravitonNeighborDiscoveryMethod;
typedef struct _GravitonNeighborDiscoveryMethodClass GravitonNeighborDiscoveryMethodClass;
typedef struct _GravitonNeighborDiscoveryMethodPrivate GravitonNeighborDiscoveryMethodPrivate;

struct _GravitonNeighborDiscoveryMethodClass
{
  GravitonDiscoveryMethodClass parent_class;
};

struct _GravitonNeighborDiscoveryMethod
{
  GravitonDiscoveryMethod parent;
  GravitonNeighborDiscoveryMethodPrivate *priv;
};

GType graviton_neighbor_discovery_method_get_type (void);

G_END_DECLS

#endif
