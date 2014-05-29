#include "server.h"

#include "server-interface.h"
#include "introspection-service.h"
#include "root-service.h"
#include "service.h"
#include "stream.h"

#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <string.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-glib/glib-watch.h>
#include <uuid/uuid.h>

#include "config.h"

#define GRAVITON_SERVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_SERVER_TYPE, GravitonServerPrivate))

GQuark
graviton_server_error_quark ()
{
  return g_quark_from_static_string ("graviton-server-error-quark");
}

/**
 * GravitonServer:
 *
 * A core concept of Graviton is nodes that provide services to a cloud of other
 * servers and clients.
 *
 * The #GravitonServer exposes a list #GravitonService objects to a cloud. It
 * may be addressed by a tuple of cloud-id and node-id.
 *
 * #GravitonServer supports routing of messages to other nodes in the network
 * simply by receiving a request that is addressed to another node. The server
 * will attempt to contact the destination node, or the next hop in the route
 * and forward the message verbatim.
 *
 * To begin, create a server using graviton_server_new(). You can add a service
 * that implements a given interface by attaching it to the
 * #GravitonRootService exposed via graviton_server_get_root_service().
 *
 * Servers have two unique properties: #GravitonServer:node-id and
 * #GravitonServer:cloud-id. The cloud ID is shared by all members of the cloud
 * and is used to identify the cloud. Each server has a unique node ID that is
 * generated at startup. These properties can also be read with
 * graviton_server_get_node_id() and graviton_server_get_cloud_id().
 *
 * All servers come equiped with an introspection service available as
 * net:phrobo:graviton.
 *
 * FIXME: Document net:phrobo:graviton API
 *
 */
G_DEFINE_TYPE (GravitonServer, graviton_server, G_TYPE_OBJECT);

enum
{
  PROP_0,
  PROP_CLOUD_ID,
  PROP_NODE_ID,
  N_PROPERTIES
};

struct _GravitonServerPrivate
{
  SoupServer *server;
  SoupServer *server6;
  GravitonRootService *plugins;
  GList *event_listeners;
  GHashTable *plugin_names;
  GList *streams;
  AvahiClient *avahi;
  AvahiGLibPoll *avahi_poll_api;
  AvahiEntryGroup *avahi_group;
  GravitonDBusServer *dbus;

  gchar *cloud_id;
  gchar *node_id;
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

typedef struct _StreamConnection
{
  GravitonStream *stream;
  SoupMessage *message;
  gchar *buf;
  gsize bufsize;
  GCancellable *cancellable;
  GravitonServer *server;
  gint refcount;
} StreamConnection;

static StreamConnection *
new_stream (SoupMessage *message, GravitonStream *stream, GravitonServer *server)
{
  StreamConnection *connection = g_new0 (StreamConnection, 1);
  connection->message = g_object_ref (message);
  connection->stream = g_object_ref (stream);
  connection->server = g_object_ref (server);
  connection->cancellable = g_cancellable_new ();

  connection->bufsize = 8192;
  connection->buf = g_new0 (gchar, connection->bufsize);

  return connection;
}

static void
free_stream (StreamConnection *connection)
{
  g_object_unref (connection->cancellable);
  if (connection->message) {
    //g_object_unref (connection->message);
    connection->message = 0;
  }
  if (connection->stream) {
    g_object_unref (connection->stream);
    connection->stream = 0;
  }
  if (connection->buf) {
    g_free (connection->buf);
    connection->buf = 0;
  }
  connection->server->priv->streams = g_list_remove (connection->server->priv->streams, connection);
  g_object_unref (connection->server);
}

static void
cb_read_stream (GObject *source, GAsyncResult *res, gpointer user_data)
{
  GError *error = NULL;
  StreamConnection *connection = (StreamConnection*)user_data;
  if (g_cancellable_set_error_if_cancelled (connection->cancellable, &error)) {
    free_stream (connection);
    return;
  }
  gssize read_size = g_input_stream_read_finish (G_INPUT_STREAM (source), res, &error);
  if (error) {
    g_debug ("Error while streaming: %s", error->message);
    free_stream (connection);
    return;
  }

  if (read_size == 0) {
    g_debug ("End of stream.");
    free_stream (connection);
    return;
  }
  SoupMessageBody *body;
  g_object_get (connection->message, SOUP_MESSAGE_RESPONSE_BODY, &body, NULL);
  soup_message_body_append (body, SOUP_MEMORY_COPY, connection->buf, read_size);
  g_debug ("Read %li", read_size); //FIXME: Should use G_SSIZE_FORMAT
  soup_server_unpause_message (connection->server->priv->server, connection->message);
  g_input_stream_read_async (G_INPUT_STREAM (source),
                             connection->buf,
                             connection->bufsize,
                             G_PRIORITY_DEFAULT,
                             connection->cancellable,
                             cb_read_stream,
                             connection);
}

static void
cb_avahi_group (AvahiEntryGroup *g, AvahiEntryGroupState state, gpointer data)
{
}

static void
create_avahi_services (AvahiClient *client, GravitonServer *server)
{
  if (!server->priv->avahi_group)
    server->priv->avahi_group = avahi_entry_group_new (client, cb_avahi_group, NULL);

  avahi_entry_group_reset (server->priv->avahi_group);

  SoupSocket *socket = soup_server_get_listener (server->priv->server);
  SoupAddress *address = soup_socket_get_local_address (socket);
  int port = soup_address_get_port (address);

  avahi_entry_group_add_service (server->priv->avahi_group,
                                 AVAHI_IF_UNSPEC,
                                 AVAHI_PROTO_INET,
                                 0,
                                 server->priv->node_id,
                                 "_graviton._tcp",
                                 NULL,
                                 NULL,
                                 port,
                                 NULL);
  avahi_entry_group_commit (server->priv->avahi_group);
  g_debug ("Created avahi services for port %d", port);
  graviton_dbus_server_set_port (server->priv->dbus, port);

  GDBusConnection *connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
  g_dbus_interface_skeleton_export ((GDBusInterfaceSkeleton*)server->priv->dbus,
                                    connection,
                                    "/",
                                    NULL);
  gchar *busName = g_strdup_printf ("org.aether.graviton-%d", port);
  g_bus_own_name (G_BUS_TYPE_SESSION,
                  busName, 
                  G_BUS_NAME_OWNER_FLAGS_NONE,
                  NULL,
                  NULL,
                  NULL,
                  NULL,
                  NULL);
  g_free (busName);
}

static void
cb_avahi (AvahiClient *client, AvahiClientState state, gpointer data)
{
  if (state == AVAHI_CLIENT_S_RUNNING) {
    create_avahi_services (client, GRAVITON_SERVER(data));
  }
}

static void
set_property (GObject *object,
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
get_property (GObject *object,
              guint property_id,
              GValue *value,
              GParamSpec *pspec)
{
  GravitonServer *self = GRAVITON_SERVER (object);
  
  switch (property_id) {
    case PROP_NODE_ID:
      g_value_set_string (value, self->priv->node_id);
      break;
    case PROP_CLOUD_ID:
      g_value_set_string (value, self->priv->cloud_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_server_class_init (GravitonServerClass *klass)
{
  g_type_class_add_private (klass, sizeof (GravitonServerPrivate));

  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;

  obj_properties[PROP_NODE_ID] =
    g_param_spec_string ("node-id",
                         "Node UUID",
                         "Universally Unique Node ID",
                         "",
                         G_PARAM_READABLE);
  obj_properties[PROP_CLOUD_ID] =
    g_param_spec_string ("cloud-id",
                         "Cloud UUID",
                         "Universally Unique Cloud ID",
                         "",
                         G_PARAM_READABLE);

  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static JsonNode *
handle_rpc (GravitonServer *self, JsonObject *request)
{
  JsonBuilder *builder;
  JsonNode *result = NULL;
  GError *error = NULL;
  const gchar *request_id;
  GravitonService *service;
  gchar **rpc_method_name;
  gchar *method_name;
  gchar *service_name;
  GVariant *method_result;

  builder = json_builder_new ();

  request_id = json_object_get_string_member (request, "id");
  rpc_method_name = g_strsplit (json_object_get_string_member (request, "method"), ".", 0);

  service_name = g_strdup (rpc_method_name[0]);
  method_name = g_strdup (rpc_method_name[1]);
  g_strfreev (rpc_method_name);

  g_debug ("Looking for service %s", service_name);

  service = graviton_service_get_subservice (GRAVITON_SERVICE (self->priv->plugins),
                                             service_name);
  if (!service) {
    g_set_error (&error,
                 GRAVITON_SERVER_ERROR,
                 GRAVITON_SERVER_ERROR_NO_SUCH_METHOD,
                 "No such service: %s (looking for method %s)",
                 service_name, method_name);
    goto out;
  }

  g_debug ("Looking for method %s", method_name);

  if (graviton_service_has_method (service, method_name)) {
    GHashTable *args = g_hash_table_new_full (g_str_hash,
                                              g_str_equal,
                                              g_free,
                                              (GDestroyNotify)g_variant_unref);
    JsonNode *param_node = json_object_get_member (request, "params");
    if (param_node) {
      if (JSON_NODE_HOLDS_OBJECT (param_node)) {
        JsonObject *param_obj = json_node_get_object (param_node);
        GList *param_names = json_object_get_members (param_obj);
        GList *param = param_names;
        while (param) {
          GVariant *param_value;
          param_value = json_gvariant_deserialize (json_object_get_member (param_obj, param->data), NULL, &error);
          g_hash_table_replace (args, g_strdup (param->data), param_value);
          gchar *display = g_variant_print (param_value, TRUE);
          g_debug ("Setting param %s to %s", (gchar*)param->data, display);
          g_free (display);
          param = g_list_next (param);
        }
        g_list_free (param_names);
      }
    }
    method_result = graviton_service_call_method (service, method_name, args, &error);
    g_hash_table_unref (args);
    if (method_result)
      g_variant_take_ref (method_result);
  } else {
    g_set_error (&error,
                 GRAVITON_SERVER_ERROR,
                 GRAVITON_SERVER_ERROR_NO_SUCH_METHOD,
                 "No such method %s on service %s",
                 method_name, service_name);
  }

out:
  g_free (method_name);
  g_free (service_name);
  builder = json_builder_new ();
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "jsonrpc");
  json_builder_add_string_value (builder, "2.0");

  json_builder_set_member_name (builder, "id");
  if (request_id) {
    json_builder_add_string_value (builder, request_id);
  } else {
    json_builder_add_null_value (builder);
  }

  if (!error) {
    json_builder_set_member_name (builder, "result");
    if (method_result) {
      json_builder_add_value (builder, json_gvariant_serialize (method_result));
      g_variant_unref (method_result);
    } else {
      json_builder_add_null_value (builder);
    }
  } else {
    json_builder_set_member_name (builder, "error");
    json_builder_begin_object (builder);
    json_builder_set_member_name (builder, "code");
    json_builder_add_int_value (builder, error->code);
    json_builder_set_member_name (builder, "message");
    json_builder_add_string_value (builder, error->message);
    json_builder_set_member_name (builder, "data");
    json_builder_add_int_value (builder, error->domain);
    json_builder_end_object (builder);
  }
  json_builder_end_object (builder);
  result = json_builder_get_root (builder);
  g_object_unref (builder);
  
  if (service)
    g_object_unref (service);
  return result;
}

static void
cb_handle_stream (SoupServer *server,
         SoupMessage *msg,
         const char *path,
         GHashTable *query,
         SoupClientContext *client,
         gpointer user_data)
{
  GravitonServer *self = GRAVITON_SERVER (user_data);
  GravitonStream *stream = NULL;
  GravitonService *service = NULL;
  GError *error = NULL;
  gchar **stream_path;
  gchar *service_name;
  gchar *stream_name;

  const gchar *path_start = &path[strlen("/stream/")];

  stream_path = g_strsplit (path_start, ".", 0);

  service_name = g_strdup (stream_path[0]);
  stream_name = g_strdup (stream_path[1]);
  g_strfreev (stream_path);

  service = graviton_service_get_subservice (GRAVITON_SERVICE (self->priv->plugins),
                                             service_name);

  if (!service) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_FOUND);
    g_debug ("Couldn't find service %s for streaming", service_name);
    return;
  }

  stream = graviton_service_get_stream (service, stream_name, query, &error);

  if (error) {
    g_debug ("Error getting stream: %s", error->message);
    return;
  }

  if (stream) {
    SoupMessageHeaders *headers;
    StreamConnection *connection;
    SoupMessageBody *body;

    g_object_get (msg, SOUP_MESSAGE_RESPONSE_HEADERS, &headers, SOUP_MESSAGE_RESPONSE_BODY, &body, NULL);
    soup_message_body_set_accumulate (body, FALSE);

    connection = new_stream (msg, stream, self);

    self->priv->streams = g_list_append (self->priv->streams, connection);

    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_headers_set_encoding (headers, SOUP_ENCODING_CHUNKED);
    soup_message_headers_append (headers, "Connection", "close");

    gboolean success = FALSE;
    gchar *method;
    g_object_get (msg, SOUP_MESSAGE_METHOD, &method, NULL);

    if (strcmp (method, "GET") == 0) {
      GInputStream *input = graviton_stream_open_read (stream, &error);
      if (input) {
        success = TRUE;
        g_input_stream_read_async (input,
                                   connection->buf,
                                   connection->bufsize,
                                   G_PRIORITY_DEFAULT,
                                   connection->cancellable,
                                   cb_read_stream,
                                   connection);
      }
    }

    if (!success) {
      soup_message_set_status (msg, SOUP_STATUS_METHOD_NOT_ALLOWED);
      g_debug ("Unsupported operation %s for %s.%s.", method, service_name, stream_name);
      if (error) {
        g_debug ("Associated error: %s", error->message);
      }
      free_stream (connection);
    }

    g_free (method);

    g_debug ("Now streaming: %s", path_start);
  } else {
    soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
    g_debug ("Couldn't find stream %s for %s: ", stream_name, service_name);
  }

  g_free (service_name);
  g_free (stream_name);
}

static void
cb_handle_events (SoupServer *server,
         SoupMessage *msg,
         const char *path,
         GHashTable *query,
         SoupClientContext *client,
         gpointer user_data)
{
  GravitonServer *self = GRAVITON_SERVER (user_data);
  SoupMessageHeaders *headers;

  g_object_ref (msg);

  soup_message_set_status (msg, SOUP_STATUS_OK);

  g_object_get (msg, SOUP_MESSAGE_RESPONSE_HEADERS, &headers, NULL);
  soup_message_headers_set_encoding (headers, SOUP_ENCODING_CHUNKED);
  soup_message_headers_set_content_type (headers, "text/json", NULL);
  soup_message_headers_append (headers, "Connection", "close");

  self->priv->event_listeners = g_list_append (self->priv->event_listeners, msg);
  g_debug ("New event listener");
}

static void
cb_handle_rpc (SoupServer *server,
         SoupMessage *msg,
         const char *path,
         GHashTable *query,
         SoupClientContext *client,
         gpointer user_data)
{
  GError *error = NULL;
  GravitonServer *self = GRAVITON_SERVER (user_data);
  JsonNode *result = NULL;

  JsonGenerator *generator = json_generator_new ();

  JsonParser *parser = json_parser_new ();
  SoupMessageBody *body;
  g_object_get (msg, SOUP_MESSAGE_REQUEST_BODY, &body, NULL);
  if (json_parser_load_from_data (parser, body->data, body->length, &error)) {
    JsonNode *rpc_body = json_parser_get_root (parser);
    if (!rpc_body) {
      g_set_error (&error,
                   GRAVITON_SERVER_ERROR,
                   GRAVITON_SERVER_ERROR_INVALID_REQUEST,
                   "Request is not valid JSON-RPC 2.0");
      goto out;
    }
    if (JSON_NODE_HOLDS_OBJECT (rpc_body)) {
      JsonObject *rpc_request = json_node_get_object (rpc_body);
      result = handle_rpc (self, rpc_request);
    } else if (JSON_NODE_HOLDS_ARRAY (rpc_body)) {
      JsonBuilder *builder = json_builder_new ();
      JsonArray *rpc_batch = json_node_get_array (rpc_body);
      GList *valid_requests = NULL;
      int i;
      for (i = 0; i < json_array_get_length (rpc_batch); i++) {
        JsonNode *batch_item = json_array_get_element (rpc_batch,i);
        if (JSON_NODE_HOLDS_OBJECT (batch_item)) {
          JsonObject *single_request = json_node_get_object (batch_item);
          valid_requests = g_list_append (valid_requests, single_request);
        }
      }
      GList *cur = valid_requests;
      json_builder_begin_array (builder);
      while (cur) {
        json_builder_add_value (builder, handle_rpc (self, cur->data));
        cur = g_list_next (cur);
      }
      json_builder_end_array (builder);
      result = json_builder_get_root (builder);
      g_object_unref (builder);
    } else {
      g_set_error (&error,
                   GRAVITON_SERVER_ERROR,
                   GRAVITON_SERVER_ERROR_INVALID_REQUEST,
                   "Request is not valid JSON-RPC 2.0");
      goto out;
    }
  }

out:
  g_object_unref (parser);
  json_generator_set_root (generator, result);
  gsize length;
  gchar *data = json_generator_to_data (generator, &length);

  if (result)
    json_node_free (result);
  soup_message_set_status (msg, SOUP_STATUS_OK);
  soup_message_set_response (msg,
                             "text/json",
                             SOUP_MEMORY_COPY,
                             data,
                             length);
  g_debug ("response: %s", data);
  g_free (data);
  g_object_unref (generator);
}

//FIXME: What was this here for?
/*static void
broadcast_property_notify (GObject *object, GParamSpec *pspec, gpointer user_data)
{
  GravitonServer *self = GRAVITON_SERVER (user_data);
  JsonBuilder *builder;
  JsonNode *result = NULL;
  GList *client = self->priv->event_listeners;
  builder = json_builder_new ();
  while (client) {
    SoupMessageBody *body;
    g_object_get (client->data, SOUP_MESSAGE_RESPONSE_BODY, &body, NULL);
    soup_message_body_append (body, SOUP_MEMORY_STATIC, "ping", strlen("ping"));
    soup_server_unpause_message (self->priv->server, client->data);
    client = g_list_next (client);
    g_debug ("Sent ping");
  }
  return TRUE;
}*/

static void
cb_aborted_request (SoupServer *server, SoupMessage *message, SoupClientContext *client, gpointer user_data)
{
  GravitonServer *self = GRAVITON_SERVER (user_data);

  self->priv->event_listeners = g_list_remove (self->priv->event_listeners, message);

  GList *cur = self->priv->streams;
  while (cur) {
    StreamConnection *conn = (StreamConnection*)cur->data;
    if (conn->message == message) {
      g_cancellable_cancel (conn->cancellable);
      break;
    }
    cur = cur->next;
  }

  g_object_unref (G_OBJECT (message));
  
  g_debug ("Aborted event stream");
}

static void
cb_property_update (GravitonService *service, const gchar *name, gpointer user_data)
{
  GravitonServer *self = GRAVITON_SERVER(user_data);

  g_debug ("Property was updated on a plugin: %s", name);
  JsonBuilder *builder;
  JsonNode *result = NULL;
  JsonGenerator *generator;

  GValue property_value = G_VALUE_INIT;
  GVariant *converted_variant = NULL;
  gchar **split_property_name = g_strsplit (name, ".", 0);
  gchar *property_name;
  gchar *service_name;
  int split = g_strv_length (split_property_name) - 1;
  property_name = g_strdup (split_property_name[split]);
  g_free (split_property_name[split]);
  split_property_name[split] = NULL;
  service_name = g_strjoinv (".", split_property_name);
  g_strfreev (split_property_name);

  GravitonService *subservice = graviton_service_get_subservice (GRAVITON_SERVICE (self->priv->plugins),
                                                                 service_name);
  GParamSpec *prop = g_object_class_find_property (G_OBJECT_GET_CLASS (subservice), property_name);
  g_value_init (&property_value, prop->value_type);
  g_object_get_property (G_OBJECT (subservice), prop->name, &property_value);
  g_free (property_name);
  g_free (service_name);

  if (G_VALUE_HOLDS_STRING (&property_value))
    converted_variant = g_variant_new_string (g_value_get_string (&property_value));

  if (converted_variant) {
    generator = json_generator_new ();

    GList *client = self->priv->event_listeners;
    builder = json_builder_new ();
    json_builder_begin_object (builder);
    json_builder_set_member_name (builder, "type");
    json_builder_add_string_value (builder, "property");
    json_builder_set_member_name (builder, "property");
    json_builder_add_string_value (builder, name);
    json_builder_set_member_name (builder, "value");

    json_builder_add_value (builder, json_gvariant_serialize (converted_variant));
    json_builder_end_object (builder);
    result = json_builder_get_root (builder);
    g_object_unref (builder);
    json_generator_set_root (generator, result);

    gsize length;
    gchar *data = json_generator_to_data (generator, &length);
    gchar *fullData = g_strconcat (data, "\r\n", NULL);
    length = strlen(fullData);
    json_node_free(result);

    while (client) {
      SoupMessageBody *body;
      g_object_get (client->data, SOUP_MESSAGE_RESPONSE_BODY, &body, NULL);
      g_object_ref (G_OBJECT (body));
      soup_message_body_append (body, SOUP_MEMORY_COPY, fullData, length);
      soup_server_unpause_message (self->priv->server, client->data);
      client = g_list_next (client);
      g_debug ("Sent event: %s", data);
    }

    g_free (fullData);
    g_free (data);
  }
}

SoupServer *
new_server (GravitonServer *self, SoupAddressFamily family)
{
  SoupAddress *address = soup_address_new_any (family, SOUP_ADDRESS_ANY_PORT);
  SoupServer *server;
  server = soup_server_new (
    "interface", address,
    "server-header", "Graviton/" GRAVITON_VERSION " ",
    NULL
  );

  g_object_unref (address);

  g_signal_connect (server,
                    "request-aborted",
                    G_CALLBACK(cb_aborted_request),
                    self);

  soup_server_add_handler (server, "/rpc", cb_handle_rpc, self, NULL);
  soup_server_add_handler (server, "/events", cb_handle_events, self, NULL);
  soup_server_add_handler (server, "/stream", cb_handle_stream, self, NULL);
  return server;
}

static void
graviton_server_init (GravitonServer *self)
{
  GravitonServerPrivate *priv;
  self->priv = priv = GRAVITON_SERVER_GET_PRIVATE (self);

  self->priv->node_id = g_new0(gchar, 37);
  //FIXME: Need to store/load cloud ids
  self->priv->cloud_id = g_strdup ("3857E91C-BA9F-4CB9-B667-4BBB42C06FC3");
  //self->priv->cloud_id = g_new0(gchar, 37);
  uuid_t uuid;
  uuid_generate (uuid);
  //uuid_unparse_upper (uuid, self->priv->cloud_id);
  uuid_generate (uuid);
  uuid_unparse_upper (uuid, self->priv->node_id);

  priv->avahi_group = NULL;

  priv->plugins = graviton_root_service_new ();
  priv->event_listeners = NULL;
  priv->streams = NULL;
  priv->server = new_server (self, SOUP_ADDRESS_FAMILY_IPV4);
  priv->server6 = new_server (self, SOUP_ADDRESS_FAMILY_IPV6);


  priv->dbus = graviton_dbus_server_skeleton_new ();

  g_signal_connect (priv->plugins,
                    "property-update",
                    G_CALLBACK (cb_property_update),
                    self);

  graviton_service_add_subservice (GRAVITON_SERVICE (self->priv->plugins),
                                   g_object_new (GRAVITON_INTROSPECTION_CONTROL_TYPE, 
                                                 "server", self,
                                                 "name", "net:phrobo:graviton",
                                                 NULL));

  priv->avahi_poll_api = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
  const AvahiPoll *poll_api = avahi_glib_poll_get (priv->avahi_poll_api);
  priv->avahi = avahi_client_new (poll_api,
                                  0,
                                  cb_avahi,
                                  self,
                                  NULL);

}

/**
 * graviton_server_new:
 *
 * Creates a new #GravitonServer object
 *
 * Returns: A new #GravitonServer
 */
GravitonServer *graviton_server_new ()
{
  return g_object_new (GRAVITON_SERVER_TYPE, NULL);
}

/**
 * graviton_server_run_async:
 *
 * Starts @server, causing it to listen for and process requests.
 *
 * This runs in @server's #GMainContext. It will not perform any processing
 * unless the appropriate main loop is running.
 */
void graviton_server_run_async (GravitonServer *self)
{
  soup_server_run_async (self->priv->server);
  soup_server_run_async (self->priv->server6);
}

/**
 * graviton_server_get_root_service:
 *
 * Gets the #GravitonRootService for this server. Required for attaching
 * sub-services and exposing them to the cloud this server is a member of.
 *
 * Returns: The #GravitonRootService for this server
 */
GravitonRootService *
graviton_server_get_root_service (GravitonServer *self)
{
  g_object_ref (self->priv->plugins);
  return self->priv->plugins;
}

/**
 * graviton_server_get_node_id:
 *
 * Gets the ID of the node this server is providing
 *
 * Returns: Node ID of this server
 */
const gchar *
graviton_server_get_node_id (GravitonServer *self)
{
  return self->priv->node_id;
}

/**
 * graviton_server_get_cloud_id:
 *
 * Gets the ID of the cloud this server is a member of
 *
 * Returns: Cloud ID of this server
 */
const gchar *
graviton_server_get_cloud_id (GravitonServer *self)
{
  return self->priv->cloud_id;
}

/**
 * graviton_server_get_port:
 *
 * Gets the TCP port number this server is listening on.
 *
 * Returns: The port this server is listening on
 */
int
graviton_server_get_port (GravitonServer *self)
{
  SoupSocket *socket = soup_server_get_listener (self->priv->server);
  SoupAddress *address = soup_socket_get_local_address (socket);
  int port = soup_address_get_port (address);
  g_object_unref (address);
  g_object_unref (socket);
  return port;
}
