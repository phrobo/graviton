#include "server.h"
#include "internal-plugin.h"
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <graviton/root-control.h>
#include <graviton/control.h>
#include <string.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-glib/glib-watch.h>
#include "stream.h"

#include "config.h"

#define GRAVITON_SERVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_SERVER, GravitonServerPrivate))

GQuark
graviton_server_error_quark ()
{
  return g_quark_from_static_string ("graviton-server-error-quark");
}

G_DEFINE_TYPE (GravitonServer, graviton_server, G_TYPE_OBJECT);

struct _GravitonServerPrivate
{
  SoupServer *server;
  SoupServer *server6;
  GravitonRootControl *plugins;
  GList *event_listeners;
  GHashTable *plugin_names;
  GList *streams;
  AvahiClient *avahi;
  AvahiGLibPoll *avahi_poll_api;
  AvahiEntryGroup *avahi_group;
};

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
  g_debug ("Read %d", read_size);
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
                                 "Graviton",
                                 "_graviton._tcp",
                                 NULL,
                                 NULL,
                                 port,
                                 NULL);
  avahi_entry_group_commit (server->priv->avahi_group);
  g_debug ("Created avahi services for port %d", port);
}

static void
cb_avahi (AvahiClient *client, AvahiClientState state, gpointer data)
{
  if (state == AVAHI_CLIENT_S_RUNNING) {
    create_avahi_services (client, GRAVITON_SERVER(data));
  }
}

static void
graviton_server_class_init (GravitonServerClass *klass)
{
  g_type_class_add_private (klass, sizeof (GravitonServerPrivate));
}

static JsonNode *
handle_rpc (GravitonServer *self, JsonObject *request)
{
  JsonBuilder *builder;
  JsonNode *result = NULL;
  GError *error = NULL;
  const gchar *request_id;
  GravitonControl *control;
  gchar **rpc_method_name;
  gchar *method_name;
  gchar *control_name;
  GVariant *method_result;

  builder = json_builder_new ();

  request_id = json_object_get_string_member (request, "id");
  rpc_method_name = g_strsplit (json_object_get_string_member (request, "method"), ".", 0);

  control_name = g_strdup (rpc_method_name[0]);
  method_name = g_strdup (rpc_method_name[1]);
  g_strfreev (rpc_method_name);

  g_debug ("Looking for control %s", control_name);

  control = graviton_control_get_subcontrol (GRAVITON_CONTROL (self->priv->plugins),
                                             control_name);
  if (!control) {
    g_set_error (&error,
                 GRAVITON_SERVER_ERROR,
                 GRAVITON_SERVER_ERROR_NO_SUCH_METHOD,
                 "No such control: %s (looking for method %s)",
                 control_name, method_name);
    goto out;
  }

  g_debug ("Looking for method %s", method_name);

  if (graviton_control_has_method (control, method_name)) {
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
          g_debug ("Setting param %s to %s", param->data, display);
          g_free (display);
          param = g_list_next (param);
        }
        g_list_free (param_names);
      }
    }
    method_result = graviton_control_call_method (control, method_name, args, &error);
    g_hash_table_unref (args);
    if (method_result)
      g_variant_take_ref (method_result);
  } else {
    g_set_error (&error,
                 GRAVITON_SERVER_ERROR,
                 GRAVITON_SERVER_ERROR_NO_SUCH_METHOD,
                 "No such method %s on control %s",
                 method_name, control_name);
  }

out:
  g_free (method_name);
  g_free (control_name);
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
  
  if (control)
    g_object_unref (control);
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
  GravitonControl *control = NULL;
  GError *error = NULL;
  gchar **stream_path;
  gchar *control_name;
  gchar *stream_name;

  const gchar *path_start = &path[strlen("/stream/")];

  stream_path = g_strsplit (path_start, ".", 0);

  control_name = g_strdup (stream_path[0]);
  stream_name = g_strdup (stream_path[1]);
  g_strfreev (stream_path);

  control = graviton_control_get_subcontrol (GRAVITON_CONTROL (self->priv->plugins),
                                             control_name);

  if (!control) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_FOUND);
    g_debug ("Couldn't find control %s for streaming", control_name);
    return;
  }

  stream = graviton_control_get_stream (control, stream_name, query, &error);

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
      g_debug ("Unsupported operation %s for %s.%s.", method, control_name, stream_name);
      if (error) {
        g_debug ("Associated error: %s", error->message);
      }
      free_stream (connection);
    }

    g_free (method);

    g_debug ("Now streaming: %s", path_start);
  } else {
    soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
    g_debug ("Couldn't find stream %s for %s: ", stream_name, control_name);
  }

  g_free (control_name);
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

static void
broadcast_property_notify (GObject *object, GParamSpec *pspec, gpointer user_data)
{
  /*GravitonServer *self = GRAVITON_SERVER (user_data);
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
  return TRUE;*/
}

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
cb_property_update (GravitonControl *control, const gchar *name, gpointer user_data)
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
  gchar *control_name;
  int split = g_strv_length (split_property_name) - 1;
  property_name = g_strdup (split_property_name[split]);
  g_free (split_property_name[split]);
  split_property_name[split] = NULL;
  control_name = g_strjoinv (".", split_property_name);
  g_strfreev (split_property_name);

  GravitonControl *subcontrol = graviton_control_get_subcontrol (GRAVITON_CONTROL (self->priv->plugins),
                                                                 control_name);
  GParamSpec *prop = g_object_class_find_property (G_OBJECT_GET_CLASS (subcontrol), property_name);
  g_value_init (&property_value, prop->value_type);
  g_object_get_property (G_OBJECT (subcontrol), prop->name, &property_value);
  g_free (property_name);
  g_free (control_name);

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
  priv->avahi_group = NULL;

  priv->plugins = graviton_root_control_new ();
  priv->event_listeners = NULL;
  priv->streams = NULL;
  g_signal_connect (priv->plugins,
                    "property-update",
                    G_CALLBACK (cb_property_update),
                    self);

  priv->server = new_server (self, SOUP_ADDRESS_FAMILY_IPV4);
  priv->server6 = new_server (self, SOUP_ADDRESS_FAMILY_IPV6);

  graviton_control_add_subcontrol (GRAVITON_CONTROL (self->priv->plugins),
                                   g_object_new (GRAVITON_TYPE_INTERNAL_PLUGIN, 
                                                 "server", self,
                                                 "name", "graviton",
                                                 NULL));

  priv->avahi_poll_api = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
  const AvahiPoll *poll_api = avahi_glib_poll_get (priv->avahi_poll_api);
  priv->avahi = avahi_client_new (poll_api,
                                  0,
                                  cb_avahi,
                                  self,
                                  NULL);
}

GravitonServer *graviton_server_new ()
{
  return g_object_new (GRAVITON_TYPE_SERVER, NULL);
}
void graviton_server_run_async (GravitonServer *self)
{
  soup_server_run_async (self->priv->server);
  soup_server_run_async (self->priv->server6);
}

GravitonRootControl *
graviton_server_get_root_control (GravitonServer *self)
{
  g_object_ref (self->priv->plugins);
  return self->priv->plugins;
}
