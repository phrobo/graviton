#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gdns-publish.h"
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <graviton/server/server.h>

typedef struct _GravitonGdnsPublishMethodPrivate GravitonGdnsPublishMethodPrivate;

struct _GravitonGdnsPublishMethodPrivate
{
  SoupSession *session;
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

static void
start_publish (GravitonServerPublishMethod *method)
{
  GravitonGdnsPublishMethod *self = GRAVITON_GDNS_PUBLISH_METHOD (method);
  SoupMessage *msg;
  SoupURI *uri;
  JsonGenerator *generator;
  JsonNode *root = NULL;
  JsonBuilder *builder;
  const gchar *node_id;
  const gchar *cloud_id;
  int port;
  GravitonServer *server;

  server = graviton_server_publish_method_get_server (method);
  port = graviton_server_get_port (server);
  cloud_id = graviton_server_get_cloud_id (server);
  node_id = graviton_server_get_node_id (server);
  g_object_unref (server);

  uri = soup_uri_new ("http://gdns.phrobo.net/api");
  msg = soup_message_new_from_uri ("POST", uri);

  generator = json_generator_new ();
  builder = json_builder_new ();
  json_builder_begin_object (builder);

  json_builder_set_member_name (builder, "jsonrpc");
  json_builder_add_string_value (builder, "2.0");

  json_builder_set_member_name (builder, "method");
  json_builder_add_string_value (builder, "publish");

  json_builder_set_member_name (builder, "id");
  json_builder_add_string_value (builder, "");

  json_builder_set_member_name (builder, "params");
  json_builder_begin_object (builder);

  json_builder_set_member_name (builder, "node");
  json_builder_add_string_value (builder, node_id);

  json_builder_set_member_name (builder, "cloud");
  json_builder_add_string_value (builder, cloud_id);

  json_builder_set_member_name (builder, "port");
  json_builder_add_int_value (builder, port);

  json_builder_end_object (builder);
  json_builder_end_object (builder);
  root = json_builder_get_root (builder);
  g_object_unref (builder);
  json_generator_set_root (generator, root);
  gsize length;
  gchar *data = json_generator_to_data (generator, &length);
  g_object_unref (generator);
  json_node_free (root);

  soup_message_set_request (msg, "text/json", SOUP_MEMORY_TAKE, data, length);
  g_debug ("Poking gdns.phrobo.net API with an update");
  soup_session_queue_message (self->priv->session, msg, NULL, NULL);
}

static void
stop_publish (GravitonServerPublishMethod *method)
{
  //GravitonGdnsPublishMethod *self = GRAVITON_GDNS_PUBLISH_METHOD (method);
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
