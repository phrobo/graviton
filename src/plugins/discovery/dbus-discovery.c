#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dbus-discovery.h"
#include <graviton/client/jsonrpc-node-transport.h>

#define GRAVITON_DBUS_DISCOVERY_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE, GravitonDbusDiscoveryMethodPrivate))

static void graviton_dbus_discovery_method_class_init (GravitonDbusDiscoveryMethodClass *klass);
static void graviton_dbus_discovery_method_init       (GravitonDbusDiscoveryMethod *self);
static void graviton_dbus_discovery_method_dispose    (GObject *object);
static void graviton_dbus_discovery_method_finalize   (GObject *object);
static void graviton_dbus_discovery_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_dbus_discovery_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

GRAVITON_DEFINE_DISCOVERY_PLUGIN (GRAVITON_DBUS_DISCOVERY_METHOD_TYPE)

G_DEFINE_TYPE (GravitonDbusDiscoveryMethod, graviton_dbus_discovery_method, GRAVITON_DISCOVERY_METHOD_TYPE);

static void
start_browse (GravitonDiscoveryMethod *method)
{
  GravitonDbusDiscoveryMethod *self = GRAVITON_DBUS_DISCOVERY_METHOD (method);
  g_debug ("Starting browsing dbus");

  GDBusConnection *bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
  GVariant *busNameList;
  GVariant *busNameListReply = g_dbus_connection_call_sync (bus,
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
  busNameList = g_variant_get_child_value (busNameListReply, 0);
  const gchar **busNames = g_variant_get_strv (busNameList, NULL);
  int i = 0;
  while (busNames[i]) {
    if (g_str_has_prefix (busNames[i], "org.aether.graviton-")) {
      g_debug ("Found server at %s", busNames[i]);
      int port = 0;
      GInetSocketAddress *addr = NULL;
      GInetAddress *addrName = NULL;
      GVariant *portResultReply = g_dbus_connection_call_sync (bus,
                                                          busNames[i],
                                                          "/",
                                                          "org.freedesktop.DBus.Properties",
                                                          "Get",
                                                          g_variant_new ("(ss)", 
                                                            "org.aether.graviton.Server",
                                                            "port"),
                                                          NULL,
                                                          G_DBUS_CALL_FLAGS_NONE,
                                                          -1,
                                                          NULL,
                                                          NULL);
      GVariant *portResultVariant = g_variant_get_child_value (portResultReply, 0);
      GVariant *portResult = g_variant_get_variant (portResultVariant);
      port = g_variant_get_int32 (portResult);
      //g_debug ("Type: %s", g_variant_print (portResult, TRUE));
      g_variant_unref (portResult);
      g_variant_unref (portResultVariant);
      g_variant_unref (portResultReply);
      addrName = g_inet_address_new_from_string ("127.0.0.1");
      addr = (GInetSocketAddress*)g_inet_socket_address_new (addrName, port);

      GravitonJsonrpcNodeTransport *transport = graviton_jsonrpc_node_transport_new (addr);
      const gchar *node_id = graviton_jsonrpc_node_transport_get_node_id (transport);
      GravitonNode *node = graviton_node_get_by_id (node_id);
      graviton_node_add_transport (node, GRAVITON_NODE_TRANSPORT (transport), 0);
      g_object_unref (addr);
      graviton_discovery_method_node_found (GRAVITON_DISCOVERY_METHOD (self), node);
    }
    i++;
  }
  g_free (busNames);
  g_variant_unref (busNameList);
  g_variant_unref (busNameListReply);

  g_object_unref (bus);

  graviton_discovery_method_finished (GRAVITON_DISCOVERY_METHOD (self));
}

static void
stop_browse (GravitonDiscoveryMethod *method)
{
}

static void
graviton_dbus_discovery_method_class_init (GravitonDbusDiscoveryMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = graviton_dbus_discovery_method_dispose;
  object_class->finalize = graviton_dbus_discovery_method_finalize;
  object_class->set_property =  graviton_dbus_discovery_method_set_property;
  object_class->get_property =  graviton_dbus_discovery_method_get_property;

  GravitonDiscoveryMethodClass *method_class = GRAVITON_DISCOVERY_METHOD_CLASS (klass);
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
