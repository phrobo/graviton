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

#ifndef __GRAVITON_XBEE_DISCOVERY_METHOD_H__
#define __GRAVITON_XBEE_DISCOVERY_METHOD_H__

#include <glib.h>
#include <glib-object.h>

#include <graviton/client/discovery-method.h>

G_BEGIN_DECLS

#define GRAVITON_XBEE_DISCOVERY_METHOD_TYPE            (graviton_xbee_discovery_method_get_type ())
#define GRAVITON_XBEE_DISCOVERY_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_XBEE_DISCOVERY_METHOD_TYPE, GravitonXbeeDiscoveryMethod))
#define GRAVITON_XBEE_DISCOVERY_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_XBEE_DISCOVERY_METHOD_TYPE, GravitonXbeeDiscoveryMethodClass))
#define IS_GRAVITON_XBEE_DISCOVERY_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_XBEE_DISCOVERY_METHOD_TYPE))
#define IS_GRAVITON_XBEE_DISCOVERY_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_XBEE_DISCOVERY_METHOD_TYPE))
#define GRAVITON_XBEE_DISCOVERY_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_XBEE_DISCOVERY_METHOD_TYPE, GravitonXbeeDiscoveryMethodClass))

typedef struct _GravitonXbeeDiscoveryMethod      GravitonXbeeDiscoveryMethod;
typedef struct _GravitonXbeeDiscoveryMethodClass GravitonXbeeDiscoveryMethodClass;
typedef struct _GravitonXbeeDiscoveryMethodPrivate GravitonXbeeDiscoveryMethodPrivate;

struct _GravitonXbeeDiscoveryMethodClass
{
  GravitonDiscoveryMethodClass parent_class;
};

struct _GravitonXbeeDiscoveryMethod
{
  GravitonDiscoveryMethod parent;
  GravitonXbeeDiscoveryMethodPrivate *priv;
};

GType graviton_xbee_discovery_method_get_type (void);

GravitonXbeeDiscoveryMethod *graviton_xbee_discovery_method_new ();

G_END_DECLS

#endif
