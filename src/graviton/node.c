#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "node.h"
#include <json-glib/json-glib.h>

typedef struct _GravitonNodePrivate GravitonNodePrivate;

struct _GravitonNodePrivate
{
  SoupSession *soup;
  SoupAddress *address;
  gchar *rpc_uri;
  gchar *event_uri;
};

#define GRAVITON_NODE_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_TYPE, GravitonNodePrivate))

static void graviton_node_class_init (GravitonNodeClass *klass);
static void graviton_node_init       (GravitonNode *self);
static void graviton_node_dispose    (GObject *object);
static void graviton_node_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonNode, graviton_node, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_ADDRESS,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
rebuild_uri (GravitonNode *self)
{
  if (self->priv->rpc_uri)
    g_free (self->priv->rpc_uri);
  if (self->priv->event_uri)
    g_free (self->priv->event_uri);
  gchar *base = g_strdup_printf ("http://%s:%d", soup_address_get_name (self->priv->address), soup_address_get_port (self->priv->address));
  self->priv->rpc_uri = g_strdup_printf ("%s/rpc", base);
  self->priv->event_uri = g_strdup_printf ("%s/events", base);
  g_free (base);
}

static void
set_property (GObject *object,
                     guint property_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  GravitonNode *self = GRAVITON_NODE (object);
  switch (property_id) {
    case PROP_ADDRESS:
      if (self->priv->address)
        g_object_unref (self->priv->address);
      self->priv->address = g_value_dup_object (value);
      rebuild_uri (self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
get_property (GObject *object,
                     guint property_id,
                     GValue *value,
                     GParamSpec *pspec)
{
  GravitonNode *self = GRAVITON_NODE (object);
  switch (property_id) {
    case PROP_ADDRESS:
      g_value_set_object (value, self->priv->address);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_node_class_init (GravitonNodeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNodePrivate));

  object_class->dispose = graviton_node_dispose;
  object_class->finalize = graviton_node_finalize;

  object_class->set_property = set_property;
  object_class->get_property = get_property;

  obj_properties[PROP_ADDRESS] =
    g_param_spec_object ("address",
                         "Address to connect to",
                         "A reachable address to connect to",
                         SOUP_TYPE_ADDRESS,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
graviton_node_init (GravitonNode *self)
{
  GravitonNodePrivate *priv;
  self->priv = priv = GRAVITON_NODE_GET_PRIVATE (self);
  self->priv->rpc_uri = 0;
  self->priv->event_uri = 0;
  self->priv->address = NULL;
  self->priv->soup = soup_session_async_new ();
  g_object_set (self->priv->soup, SOUP_SESSION_TIMEOUT, 5, NULL);
}

static void
graviton_node_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_parent_class)->dispose (object);
}

static void
graviton_node_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_parent_class)->finalize (object);
}

GravitonNode *
graviton_node_proxy_to_id (GravitonNode *node,
                           gchar *id,
                           GError **error)
{
  return NULL;
}

GravitonNode *
graviton_node_new_from_address (SoupAddress *address)
{
  return g_object_new(GRAVITON_NODE_TYPE, "address", address, NULL);
}

const gchar *
graviton_node_get_id (GravitonNode *self, GError **err)
{
  GVariant *ret = graviton_node_get_property (self, "graviton", "deviceid", err);
  if (ret) {
    gchar *r = g_variant_dup_string (ret, NULL);
    g_variant_unref (ret);
    return r;
  }
  return NULL;
}

GVariant *
graviton_node_get_property (GravitonNode *self, const gchar *control, const gchar *property, GError **err)
{
  GError *error = NULL;
  GHashTable *args = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            (GDestroyNotify)g_variant_unref);
  g_hash_table_replace (args, g_strdup ("control"), g_variant_new_string (control));
  g_hash_table_replace (args, g_strdup ("property"), g_variant_new_string (property));
  GVariant *ret = graviton_node_call (self,
                                      "graviton.introspection/getProperty",
                                      args,
                                      &error);
  g_hash_table_unref (args);
  if (error) {
    g_propagate_error (err, error);
    return NULL;
  }

  return ret;
}

GVariant *
graviton_node_call (GravitonNode *self,
                    const gchar *method,
                    GHashTable *args,
                    GError **err)
{
  SoupMessage *request = soup_message_new ("POST", self->priv->rpc_uri);
  GError *error = NULL;
  GVariant *ret = NULL;

  JsonGenerator *generator = json_generator_new ();
  JsonNode *root = NULL;
  JsonBuilder *builder = json_builder_new ();
  json_builder_begin_object (builder);

  json_builder_set_member_name (builder, "jsonrpc");
  json_builder_add_string_value (builder, "2.0");

  json_builder_set_member_name (builder, "method");
  json_builder_add_string_value (builder, method);

  json_builder_set_member_name (builder, "id");
  json_builder_add_string_value (builder, "");

  json_builder_set_member_name (builder, "params");
  json_builder_begin_object (builder);

  GList *argList = g_hash_table_get_keys (args);
  GList *cur = argList;
  while (cur) {
    json_builder_set_member_name (builder, cur->data);
    json_builder_add_value (builder, json_gvariant_serialize (g_hash_table_lookup (args, cur->data)));
    cur = cur->next;
  }
  
  json_builder_end_object (builder);
  json_builder_end_object (builder);

  root = json_builder_get_root (builder);
  g_object_unref (builder);

  json_generator_set_root (generator, root);
  gsize length;
  gchar *data = json_generator_to_data (generator, &length);

  g_debug ("Submitting to %s: %s", self->priv->rpc_uri, data);
  soup_message_set_request (request, "text/json", SOUP_MEMORY_TAKE, data, length);
  soup_session_send_message (self->priv->soup, request);
  guint code;
  g_object_get (request, SOUP_MESSAGE_STATUS_CODE, &code, NULL);
  g_debug ("Got status: %d", code);

  SoupMessageBody *responseBody;
  g_object_get (request, SOUP_MESSAGE_RESPONSE_BODY, &responseBody, NULL);

  JsonParser *parser = json_parser_new ();
  if (json_parser_load_from_data (parser, responseBody->data, responseBody->length, &error)) {
    JsonNode *responseJSON = json_parser_get_root (parser);
    if (JSON_NODE_HOLDS_OBJECT (responseJSON)) {
      JsonObject *responseObj = json_node_get_object (responseJSON);
      if (json_object_has_member (responseObj, "jsonrpc")) {
        if (json_object_has_member (responseObj, "error")) {
          JsonObject *errObj = json_object_get_object_member (responseObj, "error");
          g_set_error (err,
                       json_object_get_int_member (errObj, "data"),
                       json_object_get_int_member (errObj, "code"),
                       json_object_get_string_member (errObj, "message"));
          return NULL;
        } else {
          JsonNode *resultNode = json_object_get_member (responseObj, "result");
          ret = json_gvariant_deserialize (resultNode, NULL, &error);
        }
      } else {
        g_critical ("JSON-RPC reply from %s wasn't JSON-RPC: %s", self->priv->rpc_uri, responseBody->data);
      }
    } else {
      g_critical ("Got badly structured JSON-RPC reply from node %s: %s", self->priv->rpc_uri, responseBody->data);
    }
  } else {
    g_critical ("Got unparsable JSON from node %s: %s", self->priv->rpc_uri, error->message);
    g_error_free (error);
  }

  g_object_unref (parser);

  if (error)
    g_propagate_error (err, error);

  soup_message_body_free (responseBody);
  g_object_unref (request);
  return ret;
}
