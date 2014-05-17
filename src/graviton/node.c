#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "node.h"
#include "cloud.h"
#include "node-io-stream.h"
#include <json-glib/json-glib.h>

typedef struct _GravitonNodePrivate GravitonNodePrivate;

struct _GravitonNodePrivate
{
  SoupSession *soup;
  GInetSocketAddress *address;
  SoupURI *rpc_uri;
  SoupURI *event_uri;
  SoupURI *stream_uri;
  GravitonServiceInterface *gobj;
};

#define GRAVITON_NODE_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_TYPE, GravitonNodePrivate))

GQuark
graviton_node_error_quark ()
{
  return g_quark_from_static_string ("graviton-node-error-quark");
}

static void graviton_node_class_init (GravitonNodeClass *klass);
static void graviton_node_init       (GravitonNode *self);
static void graviton_node_dispose    (GObject *object);
static void graviton_node_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonNode, graviton_node, GRAVITON_SERVICE_INTERFACE_TYPE);

enum {
  PROP_0,
  PROP_ADDRESS,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
unref_arg (GVariant *var)
{
  if (var)
    g_variant_unref (var);
}

static void
rebuild_uri (GravitonNode *self)
{
  if (self->priv->rpc_uri)
    soup_uri_free (self->priv->rpc_uri);
  if (self->priv->event_uri)
    soup_uri_free (self->priv->event_uri);
  if (self->priv->stream_uri)
    soup_uri_free (self->priv->stream_uri);
  gchar *ip_str;
  GInetAddress *addr;
  guint port;
  g_object_get (self->priv->address, "address", &addr, "port", &port, NULL);

  ip_str = g_inet_address_to_string (addr);

  if (g_inet_address_get_family (addr) == G_SOCKET_FAMILY_IPV6) {
    gchar *tmp = ip_str;
    ip_str = g_strdup_printf ("[%s]", tmp);
    g_free (tmp);
  } 
  SoupURI *base = soup_uri_new (NULL);
  soup_uri_set_scheme (base, SOUP_URI_SCHEME_HTTP);
  soup_uri_set_host (base, ip_str);
  soup_uri_set_port (base, port);
  soup_uri_set_path (base, "/");

  self->priv->rpc_uri = soup_uri_new_with_base (base, "rpc");
  self->priv->event_uri = soup_uri_new_with_base (base, "events");
  self->priv->stream_uri = soup_uri_new_with_base (base, "stream/");
  soup_uri_free (base);

  g_free (ip_str);
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
                         G_TYPE_INET_SOCKET_ADDRESS,
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
  self->priv->stream_uri = 0;
  self->priv->address = NULL;
  self->priv->soup = soup_session_sync_new ();
  g_object_set (self->priv->soup, SOUP_SESSION_TIMEOUT, 5, NULL);
  self->priv->gobj = graviton_service_interface_get_subcontrol (GRAVITON_SERVICE_INTERFACE (self), "net:phrobo:graviton");
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
                           const gchar *id,
                           GError **error)
{
  return NULL;
}

static void
cb_resolve_id (GravitonCloud *client, GravitonNode **result)
{
  GList *nodes = graviton_cloud_get_found_nodes (client);
  GList *cur = nodes;
  while (cur) {
    cur = cur->next;
  }
}

GravitonNode *graviton_node_new_from_id (const gchar *node_id, GError **error)
{
  GravitonNode *result = NULL;
  GravitonCloud *client = graviton_cloud_new_default_cloud ();
  g_signal_connect (client,
                    "all-nodes-found",
                    G_CALLBACK (cb_resolve_id),
                    &result);
  g_object_unref (client);
  return result;
}

GravitonNode *
graviton_node_new_from_address (GInetSocketAddress *address)
{
  return g_object_new(GRAVITON_NODE_TYPE, "address", address, NULL);
}

const gchar *
graviton_node_get_id (GravitonNode *self, GError **err)
{
  GVariant *ret = graviton_service_interface_get_property (self->priv->gobj, "node-id", err);
  if (ret) {
    gchar *r = g_variant_dup_string (ret, NULL);
    g_variant_unref (ret);
    return r;
  }
  return NULL;
}

const gchar *
graviton_node_get_cloud_id (GravitonNode *self, GError **err)
{
  GVariant *ret = graviton_service_interface_get_property (self->priv->gobj, "cloud-id", err);
  if (ret) {
    gchar *r = g_variant_dup_string (ret, NULL);
    g_variant_unref (ret);
    return r;
  }
  return NULL;
}

GVariant *
graviton_node_call (GravitonNode *self,
                    const gchar *method,
                    GError **err,
                    ...)
{
  va_list argList;
  va_start (argList, err);
  GVariant *ret = graviton_node_call_va (self, method, err, argList);
  va_end (argList);
  return ret;
}

GVariant *
graviton_node_call_va (GravitonNode *self,
                       const gchar *method,
                       GError **err,
                       va_list argList)
{
  gchar *propName = NULL;
  GVariant *propValue = NULL;
  GHashTable *args = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            NULL,
                                            (GDestroyNotify)unref_arg);
  propName = va_arg (argList, gchar*);

  while (propName != NULL) {
    propValue = va_arg (argList, GVariant*);
    g_hash_table_replace (args, propName, propValue);
    if (propValue)
      g_debug ("%s = %s", propName, g_variant_print (propValue, TRUE));
    else
      g_debug ("%s = NULL", propName);
    propName = va_arg (argList, gchar*);
  }
  va_end (argList);

  GVariant *ret = graviton_node_call_args (self,
                                           method,
                                           args,
                                           err);
  g_hash_table_unref (args);
  return ret;
}

GVariant *
graviton_node_call_args (GravitonNode *self,
                    const gchar *method,
                    GHashTable *args,
                    GError **err)
{
  g_assert (self->priv->rpc_uri);
  SoupMessage *request = soup_message_new_from_uri ("POST", self->priv->rpc_uri);
  g_assert (request);
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
    GVariant *val = g_hash_table_lookup (args, cur->data);
    if (val)
      json_builder_add_value (builder, json_gvariant_serialize (val));
    else
      json_builder_add_null_value (builder);
    cur = cur->next;
  }
  
  json_builder_end_object (builder);
  json_builder_end_object (builder);

  root = json_builder_get_root (builder);
  g_object_unref (builder);

  json_generator_set_root (generator, root);
  gsize length;
  gchar *data = json_generator_to_data (generator, &length);
  g_object_unref (generator);
  json_node_free (root);

  gchar *uri_str = soup_uri_to_string (self->priv->rpc_uri, FALSE);
  g_debug ("Submitting to %s: %s", uri_str, data);
  g_free (uri_str);
  soup_message_set_request (request, "text/json", SOUP_MEMORY_TAKE, data, length);
  soup_session_send_message (self->priv->soup, request);
  guint code;
  g_object_get (request, SOUP_MESSAGE_STATUS_CODE, &code, NULL);
  g_debug ("Got status: %u", code);

  if (SOUP_STATUS_IS_TRANSPORT_ERROR (code)) {
    const gchar *msg;
    switch (code) {
      case SOUP_STATUS_CANCELLED:
        msg = "Request cancelled";
        break;
      case SOUP_STATUS_CANT_RESOLVE:
        msg = "Cannot resolve hostname";
        break;
      case SOUP_STATUS_CANT_RESOLVE_PROXY:
        msg = "Proxy cannot resolve hostname";
        break;
      case SOUP_STATUS_CANT_CONNECT:
        msg = "Could not connect";
        break;
      case SOUP_STATUS_CANT_CONNECT_PROXY:
        msg = "Proxy could not connect";
        break;
      case SOUP_STATUS_SSL_FAILED:
        msg = "SSL failure";
        break;
      case SOUP_STATUS_IO_ERROR:
        msg = "IO Error";
        break;
      case SOUP_STATUS_MALFORMED:
        msg = "Malformed request";
        break;
      case SOUP_STATUS_TRY_AGAIN:
        msg = "Try again";
        break;
      case SOUP_STATUS_TOO_MANY_REDIRECTS:
        msg = "Too many redirects";
        break;
      case SOUP_STATUS_TLS_FAILED:
        msg = "TLS failure";
        break;
    }
    g_set_error (err,
                 GRAVITON_NODE_ERROR,
                 code,
                 msg);
    g_object_unref (request);
    return NULL;
  }

  SoupMessageBody *responseBody = NULL;
  g_object_get (request, SOUP_MESSAGE_RESPONSE_BODY, &responseBody, NULL);

  g_assert (responseBody != NULL);

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
      g_critical ("Got badly structured JSON-RPC reply from node %s: <<%s>>", self->priv->rpc_uri, responseBody->data);
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

GravitonServiceInterface *
graviton_node_get_control (GravitonNode *node, const gchar *name, GError **error)
{
  return NULL;
}

GIOStream *
graviton_node_open_stream (GravitonNode *self, const gchar *name, GHashTable *args)
{
  SoupURI *stream_uri = soup_uri_new_with_base (self->priv->stream_uri, name);
  soup_uri_set_query_from_form (stream_uri, args);
  GIOStream *ret = G_IO_STREAM (graviton_node_io_stream_new (stream_uri, self->priv->soup));
  soup_uri_free (stream_uri);
  return ret;
}

gboolean
graviton_node_has_service (GravitonNode *node, const gchar *name, GError **err)
{
  return TRUE;
}

int graviton_node_get_port (GravitonNode *self)
{
  return g_inet_socket_address_get_port (self->priv->address);
}

GInetSocketAddress *graviton_node_get_address (GravitonNode *node)
{
  return g_object_ref (node->priv->address);
}
