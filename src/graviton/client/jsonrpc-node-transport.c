#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "jsonrpc-node-transport.h"
#include "node-io-stream.h"
#include "node.h"

#include <json-glib/json-glib.h>

typedef struct _GravitonJsonrpcNodeTransportPrivate GravitonJsonrpcNodeTransportPrivate;

struct _GravitonJsonrpcNodeTransportPrivate
{
  SoupSession *soup;
  GInetSocketAddress *address;
  SoupURI *rpc_uri;
  SoupURI *event_uri;
  SoupURI *stream_uri;
};

#define GRAVITON_JSONRPC_NODE_TRANSPORT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE, GravitonJsonrpcNodeTransportPrivate))

static void graviton_jsonrpc_node_transport_class_init (GravitonJsonrpcNodeTransportClass *klass);
static void graviton_jsonrpc_node_transport_init       (GravitonJsonrpcNodeTransport *self);
static void graviton_jsonrpc_node_transport_dispose    (GObject *object);
static void graviton_jsonrpc_node_transport_finalize   (GObject *object);
static void graviton_jsonrpc_node_transport_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_jsonrpc_node_transport_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static GIOStream * open_stream (GravitonNodeTransport *trans_self,
                                GravitonNode *node,
                                const gchar *name,
                                GHashTable *args,
                                GError **error);
static GVariant* call_args (GravitonNodeTransport *trans_self,
                            GravitonNode *node,
                            const gchar *method,
                            GHashTable *args,
                            GError **err);

G_DEFINE_TYPE (GravitonJsonrpcNodeTransport, graviton_jsonrpc_node_transport, GRAVITON_NODE_TRANSPORT_TYPE);

enum {
  PROP_ZERO,
  PROP_ADDRESS,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
rebuild_uri (GravitonJsonrpcNodeTransport *self)
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
graviton_jsonrpc_node_transport_class_init (GravitonJsonrpcNodeTransportClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonJsonrpcNodeTransportPrivate));

  object_class->dispose = graviton_jsonrpc_node_transport_dispose;
  object_class->finalize = graviton_jsonrpc_node_transport_finalize;
  object_class->set_property =  graviton_jsonrpc_node_transport_set_property;
  object_class->get_property =  graviton_jsonrpc_node_transport_get_property;

  obj_properties[PROP_ADDRESS] =
    g_param_spec_object ("address",
                         "Address to connect to",
                         "A reachable address to connect to",
                         G_TYPE_INET_SOCKET_ADDRESS,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);

  GravitonNodeTransportClass *transport_class = GRAVITON_NODE_TRANSPORT_CLASS (klass);
  transport_class->call_args = call_args;
  transport_class->open_stream = open_stream;
}

static void
graviton_jsonrpc_node_transport_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonJsonrpcNodeTransport *self = GRAVITON_JSONRPC_NODE_TRANSPORT (object);
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
graviton_jsonrpc_node_transport_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonJsonrpcNodeTransport *self = GRAVITON_JSONRPC_NODE_TRANSPORT (object);
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
graviton_jsonrpc_node_transport_init (GravitonJsonrpcNodeTransport *self)
{
  GravitonJsonrpcNodeTransportPrivate *priv;
  priv = self->priv = GRAVITON_JSONRPC_NODE_TRANSPORT_GET_PRIVATE (self);
  self->priv->rpc_uri = 0;
  self->priv->event_uri = 0;
  self->priv->stream_uri = 0;
  self->priv->address = NULL;
  self->priv->soup = soup_session_sync_new ();
  g_object_set (self->priv->soup, SOUP_SESSION_TIMEOUT, 5, NULL);
}

static void
graviton_jsonrpc_node_transport_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_jsonrpc_node_transport_parent_class)->dispose (object);
}

static void
graviton_jsonrpc_node_transport_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_jsonrpc_node_transport_parent_class)->finalize (object);
}

static GIOStream *
open_stream (GravitonNodeTransport *trans_self,
             GravitonNode *node,
             const gchar *name,
             GHashTable *args,
             GError **error)
{
  GravitonJsonrpcNodeTransport *self = GRAVITON_JSONRPC_NODE_TRANSPORT (trans_self);
  SoupURI *stream_uri = soup_uri_new_with_base (self->priv->stream_uri, name);
  soup_uri_set_query_from_form (stream_uri, args);
  GIOStream *ret = G_IO_STREAM (graviton_node_io_stream_new (stream_uri, self->priv->soup));
  soup_uri_free (stream_uri);
  return ret;
}

static GVariant*
call_args (GravitonNodeTransport *trans_self,
           GravitonNode *node,
           const gchar *method,
           GHashTable *args,
           GError **err)
{
  GravitonJsonrpcNodeTransport *self = GRAVITON_JSONRPC_NODE_TRANSPORT (trans_self);
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

  g_list_free (argList);
  
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
          //g_object_unref (resultNode);
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

GravitonJsonrpcNodeTransport *
graviton_jsonrpc_node_transport_new (GInetSocketAddress *address)
{
  return g_object_new (GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE,
                       "address", address,
                       NULL);
}

gchar *
graviton_jsonrpc_node_transport_get_node_id (GravitonJsonrpcNodeTransport *transport)
{
  GHashTable *args = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            NULL,
                                            g_variant_unref);
  g_hash_table_insert (args, "service", g_variant_new_string ("net:phrobo:graviton"));
  g_hash_table_insert (args, "property", g_variant_new_string ("node-id"));

  GVariant *ret = call_args (transport,
                             NULL,
                             "net:phrobo:graviton/introspection.getProperty",
                             args,
                             NULL);
  g_hash_table_unref (args);
  if (ret) {
    gchar *r = g_variant_dup_string (ret, NULL);
    g_variant_unref (ret);
    return r;
  }
  return NULL;
}
