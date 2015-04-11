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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dbus-discovery.h"
#include <graviton/client/jsonrpc-node-transport.h>

#define GRAVITON_DBUS_DISCOVERY_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE, \
                                GravitonDbusDiscoveryMethodPrivate))

static void graviton_dbus_discovery_method_class_init (
  GravitonDbusDiscoveryMethodClass *klass);
static void graviton_dbus_discovery_method_init       (
  GravitonDbusDiscoveryMethod *self);
static void graviton_dbus_discovery_method_dispose    (GObject *object);
static void graviton_dbus_discovery_method_finalize   (GObject *object);
static void graviton_dbus_discovery_method_set_property (GObject *object,
                                                         guint property_id,
                                                         const GValue *value,
                                                         GParamSpec *pspec);
static void graviton_dbus_discovery_method_get_property (GObject *object,
                                                         guint property_id,
                                                         GValue *value,
                                                         GParamSpec *pspec);

GRAVITON_DEFINE_DISCOVERY_PLUGIN (GRAVITON_DBUS_DISCOVERY_METHOD_TYPE)

G_DEFINE_TYPE (GravitonDbusDiscoveryMethod,
               graviton_dbus_discovery_method,
               GRAVITON_DISCOVERY_METHOD_TYPE);

static void
start_browse (GravitonDiscoveryMethod *method)
{
  GravitonDbusDiscoveryMethod *self = GRAVITON_DBUS_DISCOVERY_METHOD (method);
  g_debug ("Starting browsing dbus");

  GDBusConnection *bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
  if (!bus) {
    graviton_discovery_method_finished (GRAVITON_DISCOVERY_METHOD (self));
    return;
  }
  GVariant *bus_name_list;
  GVariant *bus_name_list_reply = g_dbus_connection_call_sync (bus,
                                                               "org.freedesktop.DBus",
                                                               "/",
                                                               "org.freedesktop.DBus",
                                                               "ListNames",
                                                               NULL,
                                                               NULL,
                                                               G_DBUS_CALL_FLAGS_NONE,
                                                               -1,
                                                               NULL,
                                                               NULL);
  bus_name_list = g_variant_get_child_value (bus_name_list_reply, 0);
  const gchar **bus_names = g_variant_get_strv (bus_name_list, NULL);
  int i = 0;
  while (bus_names[i]) {
    if (g_str_has_prefix (bus_names[i], "org.aether.graviton-")) {
      g_debug ("Found server at %s", bus_names[i]);
      int port = 0;
      GInetSocketAddress *addr = NULL;
      GInetAddress *addr_name = NULL;
      GVariant *port_result_reply = g_dbus_connection_call_sync (bus,
                                                                 bus_names[i],
                                                                 "/",
                                                                 "org.freedesktop.DBus.Properties",
                                                                 "Get",
                                                                 g_variant_new (
                                                                   "(ss)",
                                                                   "org.aether.graviton.Server",
                                                                   "port"),
                                                                 NULL,
                                                                 G_DBUS_CALL_FLAGS_NONE,
                                                                 -1,
                                                                 NULL,
                                                                 NULL);
      GVariant *port_result_variant = g_variant_get_child_value (
        port_result_reply,
        0);
      GVariant *port_result = g_variant_get_variant (port_result_variant);
      port = g_variant_get_int32 (port_result);
      //g_debug ("Type: %s", g_variant_print (port_result, TRUE));
      g_variant_unref (port_result);
      g_variant_unref (port_result_variant);
      g_variant_unref (port_result_reply);
      addr_name = g_inet_address_new_from_string ("127.0.0.1");
      addr = (GInetSocketAddress*)g_inet_socket_address_new (addr_name, port);

      GravitonJsonrpcNodeTransport *transport =
        graviton_jsonrpc_node_transport_new (addr);
      const gchar *node_id = graviton_jsonrpc_node_transport_get_node_id (
        transport);
      GravitonNode *node = graviton_discovery_method_get_node_from_browser (GRAVITON_DISCOVERY_METHOD (
                                                                              self),
                                                                            node_id);
      graviton_node_add_transport (node, GRAVITON_NODE_TRANSPORT (transport),
                                   0);
      g_object_unref (addr);
    }
    i++;
  }
  g_free (bus_names);
  g_variant_unref (bus_name_list);
  g_variant_unref (bus_name_list_reply);

  g_object_unref (bus);

  graviton_discovery_method_finished (GRAVITON_DISCOVERY_METHOD (self));
}

static void
stop_browse (GravitonDiscoveryMethod *method)
{
}

static void
graviton_dbus_discovery_method_class_init (
  GravitonDbusDiscoveryMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = graviton_dbus_discovery_method_dispose;
  object_class->finalize = graviton_dbus_discovery_method_finalize;
  object_class->set_property =  graviton_dbus_discovery_method_set_property;
  object_class->get_property =  graviton_dbus_discovery_method_get_property;

  GravitonDiscoveryMethodClass *method_class = GRAVITON_DISCOVERY_METHOD_CLASS (
    klass);
  method_class->start = start_browse;
  method_class->stop = stop_browse;
}

static void
graviton_dbus_discovery_method_set_property (GObject *object,
                                             guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
graviton_dbus_discovery_method_get_property (GObject *object,
                                             guint property_id,
                                             GValue *value,
                                             GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
graviton_dbus_discovery_method_init (GravitonDbusDiscoveryMethod *self)
{
}

static void
graviton_dbus_discovery_method_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_dbus_discovery_method_parent_class)->dispose (object);
}

static void
graviton_dbus_discovery_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_dbus_discovery_method_parent_class)->finalize (object);
}
