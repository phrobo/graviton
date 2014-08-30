#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gdns-publish.h"
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <graviton/server/server.h>
#include <graviton/common/configuration.h>
#include <libgupnp-igd/gupnp-simple-igd.h>

#include <ifaddrs.h>
#include <arpa/inet.h>

typedef struct _GravitonGdnsPublishMethodPrivate GravitonGdnsPublishMethodPrivate;

struct _GravitonGdnsPublishMethodPrivate
{
  SoupSession *session;
  SoupSession *probeSession;
  GUPnPSimpleIgd *igd;
  guint external_port;
};

#define GRAVITON_GDNS_PUBLISH_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_GDNS_PUBLISH_METHOD_TYPE, GravitonGdnsPublishMethodPrivate))

static void graviton_gdns_publish_method_class_init (GravitonGdnsPublishMethodClass *klass);
static void graviton_gdns_publish_method_init       (GravitonGdnsPublishMethod *self);
static void graviton_gdns_publish_method_dispose    (GObject *object);
static void graviton_gdns_publish_method_finalize   (GObject *object);
static void graviton_gdns_publish_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_gdns_publish_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonGdnsPublishMethod, graviton_gdns_publish_method, GRAVITON_SERVER_PUBLISH_METHOD_TYPE);

GRAVITON_DEFINE_PUBLISH_PLUGIN (GRAVITON_GDNS_PUBLISH_METHOD_TYPE)

static SoupURI *
get_uri ()
{
  gchar *uri_str;
  SoupURI *uri = NULL;
  GKeyFile *config = graviton_config_load_default ();

  uri_str = g_key_file_get_string (config, "spitzer", "uri", NULL);
  if (uri_str == NULL) {
    goto out;
  }

  uri = soup_uri_new (uri_str);
  g_free (uri_str);

out:
  g_key_file_free (config);
  return uri;
}

static JsonBuilder *
new_jsonrpc_call (const gchar *method)
{
  JsonBuilder *builder;

  builder = json_builder_new ();
  json_builder_begin_object (builder);

  json_builder_set_member_name (builder, "jsonrpc");
  json_builder_add_string_value (builder, "2.0");

  json_builder_set_member_name (builder, "method");
  json_builder_add_string_value (builder, method);

  json_builder_set_member_name (builder, "id");
  json_builder_add_string_value (builder, "");

  json_builder_set_member_name (builder, "params");
  json_builder_begin_object (builder);

  return builder;
}

static void
cb_build_probe (SoupSession *session, SoupMessage *msg, SoupSocket *socket, GravitonGdnsPublishMethod *self)
{
  const gchar *node_id;
  JsonBuilder *builder;
  const gchar *cloud_id;
  GravitonServer *server;
  JsonNode *root = NULL;
  JsonGenerator *generator;
  SoupAddress *addr = NULL;

  guint port;
  g_object_get (socket, "local-address", &addr, NULL);
  g_object_get (addr, "port", &port, NULL);
  g_object_unref (addr);

  server = graviton_server_publish_method_get_server (GRAVITON_SERVER_PUBLISH_METHOD (self));
  cloud_id = graviton_server_get_cloud_id (server);
  node_id = graviton_server_get_node_id (server);
  g_object_unref (server);

  builder = new_jsonrpc_call ("natProbe");

  json_builder_set_member_name (builder, "node");
  json_builder_add_string_value (builder, node_id);

  json_builder_set_member_name (builder, "cloud");
  json_builder_add_string_value (builder, cloud_id);

  json_builder_set_member_name (builder, "innerPort");
  json_builder_add_int_value (builder, port);

  json_builder_end_object (builder);
  json_builder_end_object (builder);

  root = json_builder_get_root (builder);
  generator = json_generator_new ();
  json_generator_set_root (generator, root);
  gsize length;
  gchar *data = json_generator_to_data (generator, &length);
  g_object_unref (generator);
  json_node_free (root);

  g_debug ("Setting message: %s", data);
  soup_message_set_request (msg, "text/json", SOUP_MEMORY_TAKE, data, length);
  g_object_unref (builder);
}

static void
send_json (GravitonGdnsPublishMethod *self, SoupURI *uri, JsonBuilder *builder)
{
  SoupMessage *msg;
  JsonGenerator *generator;
  JsonNode *root = NULL;

  msg = soup_message_new_from_uri ("POST", uri);
  generator = json_generator_new ();

  json_builder_end_object (builder); // Close params
  json_builder_end_object (builder); // Close json

  root = json_builder_get_root (builder);
  json_generator_set_root (generator, root);
  gsize length;
  gchar *data = json_generator_to_data (generator, &length);
  g_object_unref (generator);
  json_node_free (root);

  soup_message_set_request (msg, "text/json", SOUP_MEMORY_TAKE, data, length);
  soup_session_queue_message (self->priv->session, msg, NULL, NULL);
}

static gboolean
queue_spitzer_probe (gpointer user_data)
{
  GravitonGdnsPublishMethod *self = GRAVITON_GDNS_PUBLISH_METHOD (user_data);
  SoupMessage *msg;
  SoupURI *uri;

  uri = get_uri ();
  if (uri == NULL) {
    g_debug ("No spitzer URI configured. Not sending NAT punch probes.");
    return FALSE;
  }

  msg = soup_message_new_from_uri ("POST", uri);

  gchar *uriStr = soup_uri_to_string (uri, FALSE);
  g_debug ("Sending a TCP nat punch probe to %s", uriStr);

  soup_session_queue_message (self->priv->probeSession, msg, NULL, NULL);
  return TRUE;
}

static void
publish_to_gdns (GravitonGdnsPublishMethod *self, guint port)
{
  JsonBuilder *builder;
  const gchar *node_id;
  const gchar *cloud_id;
  GravitonServer *server;
  SoupURI *uri;

  server = graviton_server_publish_method_get_server (GRAVITON_SERVER_PUBLISH_METHOD (self));
  cloud_id = graviton_server_get_cloud_id (server);
  node_id = graviton_server_get_node_id (server);
  g_object_unref (server);

  uri = get_uri ();
  if (uri == NULL) {
    g_debug ("No spizter URI set. Not publishing.");
    return;
  }

  builder = new_jsonrpc_call ("publish");

  json_builder_set_member_name (builder, "node");
  json_builder_add_string_value (builder, node_id);

  json_builder_set_member_name (builder, "cloud");
  json_builder_add_string_value (builder, cloud_id);

  json_builder_set_member_name (builder, "port");
  json_builder_add_int_value (builder, (int)port);

  g_debug ("Poking gdns.phrobo.net API with an update");
  send_json (self, uri, builder);
}

static void
cb_port_mapped (GUPnPSimpleIgd *igd,
                gchar *proto,
                gchar *external_ip,
                gchar *replaces_external_ip,
                guint external_port,
                gchar *local_ip,
                guint local_port,
                gchar *description,
                gpointer user_data)
{
  GravitonGdnsPublishMethod *self = GRAVITON_GDNS_PUBLISH_METHOD (user_data);
  g_debug ("Got port mapped!");

  //FIXME: We might end up with multiple ports!!!
  self->priv->external_port = external_port;

  //FIXME: Run publish_to_gdns periodically, whenever our external address
  //changes or any remap
  publish_to_gdns (self, self->priv->external_port);

  //TODO: Publish with a port of -1 after a short period to indicate TCP nat
  //punching is required
  //TODO: Implement TCP nat punching in a publish backend
}

static void
start_publish (GravitonServerPublishMethod *method)
{
  GravitonGdnsPublishMethod *self = GRAVITON_GDNS_PUBLISH_METHOD (method);
  GravitonServer *server;
  guint port;
  struct ifaddrs *ifAddrStruct;
  struct ifaddrs *ifa;

  server = graviton_server_publish_method_get_server (method);
  port = graviton_server_get_port (server);
  g_object_unref (server);

  // FIXME: For some reason there isn't a straightforward way to do this in
  // glib. We probably want to pull in NetworkManager and handle this better.
  getifaddrs (&ifAddrStruct);
  
  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr->sa_family == AF_INET) {
      void *tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
      char addr[INET_ADDRSTRLEN];

      inet_ntop (AF_INET, tmpAddrPtr, addr, INET_ADDRSTRLEN);
      gupnp_simple_igd_add_port (self->priv->igd,
                                 "TCP",
                                 0,
                                 addr,
                                 port,
                                 60,
                                 "Graviton");
    }
  }

  if (ifAddrStruct != NULL)
    freeifaddrs (ifAddrStruct);

  g_timeout_add_full (G_PRIORITY_DEFAULT, 100, queue_spitzer_probe, self, NULL);
}

static void
stop_publish (GravitonServerPublishMethod *method)
{
  GravitonGdnsPublishMethod *self = GRAVITON_GDNS_PUBLISH_METHOD (method);
  gupnp_simple_igd_remove_port (self->priv->igd, "TCP", self->priv->external_port);
}

static void
graviton_gdns_publish_method_class_init (GravitonGdnsPublishMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonGdnsPublishMethodPrivate));

  object_class->dispose = graviton_gdns_publish_method_dispose;
  object_class->finalize = graviton_gdns_publish_method_finalize;
  object_class->set_property =  graviton_gdns_publish_method_set_property;
  object_class->get_property =  graviton_gdns_publish_method_get_property;

  GravitonServerPublishMethodClass *method_class = GRAVITON_SERVER_PUBLISH_METHOD_CLASS (
      klass);
  method_class->start = start_publish;
  method_class->stop = stop_publish;
}

static void
graviton_gdns_publish_method_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  //GravitonGdnsPublishMethod *self = GRAVITON_GDNS_PUBLISH_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_gdns_publish_method_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  //GravitonGdnsPublishMethod *self = GRAVITON_GDNS_PUBLISH_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_gdns_publish_method_init (GravitonGdnsPublishMethod *self)
{
  GravitonGdnsPublishMethodPrivate *priv;
  priv = self->priv = GRAVITON_GDNS_PUBLISH_METHOD_GET_PRIVATE (self);
  priv->session = soup_session_new ();
  priv->probeSession = soup_session_new ();
  priv->igd = gupnp_simple_igd_new ();
  g_signal_connect (priv->igd,
                    "mapped-external-port",
                    G_CALLBACK (cb_port_mapped),
                    self);

  g_signal_connect (priv->probeSession,
                    "request-started",
                    G_CALLBACK (cb_build_probe),
                    self);
}

static void
graviton_gdns_publish_method_dispose (GObject *object)
{
  GravitonGdnsPublishMethod *self = GRAVITON_GDNS_PUBLISH_METHOD (object);
  G_OBJECT_CLASS (graviton_gdns_publish_method_parent_class)->dispose (object);

  g_object_unref (self->priv->session);
}

static void
graviton_gdns_publish_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_gdns_publish_method_parent_class)->finalize (object);
}
