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

#ifndef __GRAVITON_DBUS_DISCOVERY_METHOD_H__
#define __GRAVITON_DBUS_DISCOVERY_METHOD_H__

#include <glib-object.h>
#include <glib.h>

#include <graviton/client/discovery-method.h>

G_BEGIN_DECLS

#define GRAVITON_DBUS_DISCOVERY_METHOD_TYPE            ( \
    graviton_dbus_discovery_method_get_type ())
#define GRAVITON_DBUS_DISCOVERY_METHOD(obj)            ( \
    G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE, \
                                GravitonDbusDiscoveryMethod))
#define GRAVITON_DBUS_DISCOVERY_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST (( \
                                                                                   klass), \
                                                                                 GRAVITON_DBUS_DISCOVERY_METHOD_TYPE, \
                                                                                 GravitonDbusDiscoveryMethodClass))
#define IS_GRAVITON_DBUS_DISCOVERY_METHOD(obj)         ( \
    G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE))
#define IS_GRAVITON_DBUS_DISCOVERY_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE (( \
                                                                                   klass), \
                                                                                 GRAVITON_DBUS_DISCOVERY_METHOD_TYPE))
#define GRAVITON_DBUS_DISCOVERY_METHOD_GET_CLASS(obj)  ( \
    G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE, \
                               GravitonDbusDiscoveryMethodClass))

typedef struct _GravitonDbusDiscoveryMethod GravitonDbusDiscoveryMethod;
typedef struct _GravitonDbusDiscoveryMethodClass
  GravitonDbusDiscoveryMethodClass;

struct _GravitonDbusDiscoveryMethodClass
{
  GravitonDiscoveryMethodClass parent_class;
};

struct _GravitonDbusDiscoveryMethod
{
  GravitonDiscoveryMethod parent;
};

GType graviton_dbus_discovery_method_get_type (void);

GravitonDbusDiscoveryMethod *graviton_dbus_discovery_method_new ();

G_END_DECLS

#endif