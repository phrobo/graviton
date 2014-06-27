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

#include "jsonrpc-node-transport.h"
#include "jsonrpc-io-stream.h"
#include "node.h"

#include <json-glib/json-glib.h>

typedef struct _GravitonJsonrpcNodeTransportPrivate
  GravitonJsonrpcNodeTransportPrivate;

struct _GravitonJsonrpcNodeTransportPrivate
{
  SoupSession *soup;
  SoupSession *event_session;
  GInetSocketAddress *address;
  SoupURI *rpc_uri;
  SoupURI *event_uri;
  SoupURI *stream_uri;

  gchar *endpoint_node_id;

  SoupMessage *event_request;
  gchar *event_buffer;
  GString *cur_event;
};

#define GRAVITON_JSONRPC_NODE_TRANSPORT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE, \
                                GravitonJsonrpcNodeTransportPrivate))

static void graviton_jsonrpc_node_transport_class_init (
  GravitonJsonrpcNodeTransportClass *klass);
static void graviton_jsonrpc_node_transport_init       (
  GravitonJsonrpcNodeTransport *self);
static void graviton_jsonrpc_node_transport_dispose    (GObject *object);
static void graviton_jsonrpc_node_transport_finalize   (GObject *object);
static void graviton_jsonrpc_node_transport_set_property (GObject *object,
                                                          guint property_id,
                                                          const GValue *value,
                                                          GParamSpec *pspec);
static void graviton_jsonrpc_node_transport_get_property (GObject *object,
                                                          guint property_id,
                                                          GValue *value,
                                                          GParamSpec *pspec);
static GIOStream *open_stream (GravitonNodeTransport *trans_self,
                               GravitonNode *node,
                               const gchar *name,
                               GHashTable *args,
                               GError **error);
static GVariant*call_args (GravitonNodeTransport *trans_self,
                           GravitonNode *node,
                           const gchar *method,
                           GHashTable *args,
                           GError **err);

static void open_event_stream (GravitonJsonrpcNodeTransport *self);

G_DEFINE_TYPE (GravitonJsonrpcNodeTransport,
               graviton_jsonrpc_node_transport,
               GRAVITON_NODE_TRANSPORT_TYPE);

enum {
  PROP_ZERO,
  PROP_ADDRESS,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
cb_read_event (SoupSession *session, SoupMessage *msg, gpointer data)
{
  GravitonJsonrpcNodeTransport *self;
  GError *error = NULL;
  SoupMessageBody *response = NULL;
  JsonParser *parser;
  GVariant *event_data = NULL;
  const gchar *event_name;

  self = GRAVITON_JSONRPC_NODE_TRANSPORT (data);
  g_object_get (msg, SOUP_MESSAGE_RESPONSE_BODY, &response, NULL);

  self->priv->event_request = NULL;
  open_event_stream (self);

  if (response->length) {
    parser = json_parser_new ();
    if (json_parser_load_from_data (parser, response->data, response->length,
                                    &error)) {
      JsonNode *response_json = json_parser_get_root (parser);
      if (JSON_NODE_HOLDS_OBJECT (response_json)) {
        JsonObject *response_obj = json_node_get_object (response_json);
        event_name = json_object_get_string_member (response_obj, "name");
        JsonNode *result_node = json_object_get_member (response_obj, "data");
        event_data = json_gvariant_deserialize (result_node, NULL, &error);
      } else {
        g_critical ("Got a badly structured json event: %s", response->data);
      }
    } else {
      g_critical ("Got unparsable JSON: %s", error->message);
      g_error_free (error);
    }

    if (event_data) {
      graviton_node_transport_emit_event (GRAVITON_NODE_TRANSPORT (
                                            self), graviton_jsonrpc_node_transport_get_node_id (
                                            self), event_name, event_data);
    }

    g_object_unref (parser);
  } else {
    g_debug ("cb_read_event called but there was no data to read!");
  }
  soup_message_body_free (response);
}

static void
open_event_stream (GravitonJsonrpcNodeTransport *self)
{
  if (!self->priv->event_request) {
    self->priv->event_request = soup_message_new_from_uri ("GET", self->priv->event_uri);
    soup_session_queue_message (self->priv->event_session,
                                self->priv->event_request,
                                cb_read_event,
                                self);
  }
}

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

  open_event_stream (self);
}

static void
graviton_jsonrpc_node_transport_class_init (
  GravitonJsonrpcNodeTransportClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass,
                            sizeof (GravitonJsonrpcNodeTransportPrivate));

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

  GravitonNodeTransportClass *transport_class = GRAVITON_NODE_TRANSPORT_CLASS (
    klass);
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
  priv->rpc_uri = 0;
  priv->event_uri = 0;
  priv->stream_uri = 0;
  priv->address = NULL;
  priv->soup = soup_session_sync_new ();
  g_object_set (priv->soup, SOUP_SESSION_TIMEOUT, 5, NULL);

  priv->event_request = NULL;
  priv->event_session = soup_session_async_new ();
  g_object_set (priv->event_session,
                SOUP_SESSION_TIMEOUT, 5,
                SOUP_SESSION_IDLE_TIMEOUT, 0,
                NULL);

  priv->endpoint_node_id = NULL;
}

static void
graviton_jsonrpc_node_transport_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_jsonrpc_node_transport_parent_class)->dispose (object);
  GravitonJsonrpcNodeTransport *self = GRAVITON_JSONRPC_NODE_TRANSPORT (object);
  g_object_unref (self->priv->address);
  g_object_unref (self->priv->soup);
}

static void
graviton_jsonrpc_node_transport_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_jsonrpc_node_transport_parent_class)->finalize (
    object);
  GravitonJsonrpcNodeTransport *self = GRAVITON_JSONRPC_NODE_TRANSPORT (object);
  if (self->priv->endpoint_node_id)
    g_free (self->priv->endpoint_node_id);
  if (self->priv->rpc_uri)
    g_free (self->priv->rpc_uri);
  if (self->priv->event_uri)
    g_free (self->priv->event_uri);
}

static GIOStream *
open_stream (GravitonNodeTransport *trans_self,
             GravitonNode *node,
             const gchar *name,
             GHashTable *args,
             GError **error)
{
  GravitonJsonrpcNodeTransport *self = GRAVITON_JSONRPC_NODE_TRANSPORT (
    trans_self);
  gchar *new_name;
  SoupURI *stream_uri;

  // This needs done because otherwise foo:bar:service would look like an
  // absolute URI to soup_uri_new_with_base
  new_name = g_strdup_printf ("./%s", name);
  stream_uri = soup_uri_new_with_base (self->priv->stream_uri, new_name);
  g_debug ("Preparing jsonrpc stream for %s",
           soup_uri_to_string (stream_uri, FALSE));
  g_free (new_name);

  if (args)
    soup_uri_set_query_from_form (stream_uri, args);
  GIOStream *ret =
    G_IO_STREAM (graviton_jsonrpc_io_stream_new (stream_uri, self->priv->soup));
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
  GravitonJsonrpcNodeTransport *self = GRAVITON_JSONRPC_NODE_TRANSPORT (
    trans_self);
  g_assert (self->priv->rpc_uri);
  SoupMessage *request =
    soup_message_new_from_uri ("POST", self->priv->rpc_uri);
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

  GList *arg_list = g_hash_table_get_keys (args);
  GList *cur = arg_list;
  while (cur) {
    json_builder_set_member_name (builder, cur->data);
    GVariant *val = g_hash_table_lookup (args, cur->data);
    if (val)
      json_builder_add_value (builder, json_gvariant_serialize (val));
    else
      json_builder_add_null_value (builder);
    cur = cur->next;
  }

  g_list_free (arg_list);

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
  soup_message_set_request (request, "text/json", SOUP_MEMORY_TAKE, data,
                            length);
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

  SoupMessageBody *response_body = NULL;
  g_object_get (request, SOUP_MESSAGE_RESPONSE_BODY, &response_body, NULL);

  g_assert (response_body != NULL);

  JsonParser *parser = json_parser_new ();
  if (json_parser_load_from_data (parser, response_body->data,
                                  response_body->length, &error)) {
    JsonNode *response_json = json_parser_get_root (parser);
    if (JSON_NODE_HOLDS_OBJECT (response_json)) {
      JsonObject *response_obj = json_node_get_object (response_json);
      if (json_object_has_member (response_obj, "jsonrpc")) {
        if (json_object_has_member (response_obj, "error")) {
          JsonObject *err_obj = json_object_get_object_member (response_obj,
                                                              "error");
          g_set_error (err,
                       json_object_get_int_member (err_obj, "data"),
                       json_object_get_int_member (err_obj, "code"),
                       json_object_get_string_member (err_obj, "message"));
          g_object_unref (request);
          return NULL;
        } else {
          JsonNode *result_node = json_object_get_member (response_obj, "result");
          ret = json_gvariant_deserialize (result_node, NULL, &error);
          //g_object_unref (result_node);
        }
      } else {
        g_critical ("JSON-RPC reply from %s wasn't JSON-RPC: %s",
                    soup_uri_get_host (self->priv->rpc_uri),
                    response_body->data);
      }
    } else {
      g_critical ("Got badly structured JSON-RPC reply from node %s: <<%s>>",
                  soup_uri_get_host (self->priv->rpc_uri),
                  response_body->data);
    }
  } else {
    g_critical ("Got unparsable JSON from node %s: %s",
                soup_uri_get_host (self->priv->rpc_uri), error->message);
    g_error_free (error);
  }

  g_object_unref (parser);

  if (error)
    g_propagate_error (err, error);

  soup_message_body_free (response_body);
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

const gchar *
graviton_jsonrpc_node_transport_get_node_id (
  GravitonJsonrpcNodeTransport *transport)
{
  if (transport->priv->endpoint_node_id == NULL) {
    GHashTable *args = g_hash_table_new_full (g_str_hash,
                                              g_str_equal,
                                              NULL,
                                              (GDestroyNotify)g_variant_unref);
    g_hash_table_insert (args, "service",
                         g_variant_new_string ("net:phrobo:graviton"));
    g_hash_table_insert (args, "property", g_variant_new_string ("node-id"));

    GVariant *ret = call_args (GRAVITON_NODE_TRANSPORT (transport),
                               NULL,
                               "net:phrobo:graviton/introspection.getProperty",
                               args,
                               NULL);
    g_hash_table_unref (args);
    if (ret) {
      transport->priv->endpoint_node_id = g_variant_dup_string (ret, NULL);
      g_variant_unref (ret);
    }
  }
  return transport->priv->endpoint_node_id;
}
