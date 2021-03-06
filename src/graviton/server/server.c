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

#define G_LOG_DOMAIN "GravitonServer"

#include "server.h"

#include "introspection-service.h"
#include "root-service.h"
#include "server-publish-method.h"
#include "service.h"

#ifdef GRAVITON_ENABLE_STREAMS
#include "stream.h"
#endif // GRAVITON_ENABLE_STREAMS

#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <string.h>
#include <uuid/uuid.h>

#include <graviton/common/configuration.h>

#include "config.h"

/**
 * SECTION:server
 * @short_description: A nucleus around which services are exposed within a
 *    Graviton cloud
 * @title: GravitonServer
 * @see_also: #GravitonCloud, #GravitonNode, #GravitonService
 * @stability: Unstable
 * @include: graviton/server/server.h
 *
 * One of the core components of Graviton is the GravitonServer. It provides an
 * interface through which services may be exposed to the cloud.
 *
 * At the moment, GravitonServer is implemented as a glorified JSON-RPC server.
 * In the future, you will not have to worry about what implementation a
 * GravitonServer uses to expose services. It might not even be JSON-RPC. Try to
 * avoid thinking of anything like that when possible.
 *
 * To expose services in Graviton, the pattern is to create a service, add
 * methods to it, and attach it to a server.
 *
 * <example>
 * <title>A simple ping server</title>
 * <programlisting>
 * #include <graviton/server/server.h>
 * #include <graviton/server/service.h>
 * #include <glib.h>
 *
 * static GVariant *
 * cb_ping(GravitonService *control, GHashTable *args, GError **error, gpointer
 *user_data)
 * {
 *   return g_variant_new_string ("pong");
 * }
 *
 * int main(int argc, char** argv)
 * {
 *   GMainLoop *loop = NULL;
 *   GravitonServer *server = NULL;
 *
 * #if !GLIB_CHECK_VERSION(2, 36, 0)
 *   g_type_init ();
 * #endif
 *
 *   loop = g_main_loop_new (NULL, FALSE);
 *
 *   server = graviton_server_new ();
 *   GravitonRootService *root = graviton_server_get_root_service (server);
 *   GravitonService *ping_service = graviton_service_new
 *("net:phrobo:graviton:ping");
 *   graviton_service_add_subservice (GRAVITON_SERVICE (root), ping_service);
 *   graviton_service_add_method (ping_service, "ping", cb_ping, NULL, NULL);
 *
 *   graviton_server_run_async (server);
 *
 *   g_main_loop_run (loop);
 *
 *   return 0;
 * }
 * </programlisting>
 * </example>
 *
 * The above example does the following:
 *
 * - Creates a #GMainLoop which is essential to GLib event processing
 * - Creates a new #GravitonServer
 * - Grabs the #GravitonRootService from the server
 * - Creates a new #GravitonService that exposes the net:phrobo:graviton:ping
 *   interface
 * - Attaches the service to the root service
 * - Exposes a "ping" method that takes no arguments, and is implemented in
 *   cb_ping
 * - Starts the server within the event loop via graviton_server_run_async()
 * - Starts the event loop
 *
 * Calling graviton_server_run_async() causes the following to happen:
 *
 * - An internal-use-only #SoupServer is started up on a random TCP port, both
 *   on IPV6 and IPV4
 * - The address and TCP port are exposed on the LAN via avahi
 * - A local DBus service under the name org.aether.graviton-[port] is exported
 *
 * All of that is implementation detail and subject to change, but still useful
 * to know!
 *
 * Once the #GMainLoop is started, the internal server begins responding to
 * JSON-RPC requests. Calling the ping() method on the net:phrobo:graviton:ping
 * calls cb_ping with:
 *
 * - The net:phrobo:graviton:ping #GravitonService you attached with
 *   graviton_service_add_method()
 * - A #GHashTable that maps parameter names to #GVariant values
 * - A pointer to a #GError variable that you can set to return any errors to
 *   the client
 * - The generic pointer that was set in graviton_service_add_method()
 *
 * Methods can return either a #GVariant (of any structural complexity) or NULL.
 * The returned #GVariant is passed to g_variant_take_ref(), which allows you to
 * return a non-floating reference in case the value already exists. As
 * demonstrated, this lets you return simple values incredibly easily via
 * g_variant_new_string(), g_variant_new_int32(), g_variant_new_boolean(), and
 * even build one using #GVariantBuilder and return g_variant_builder_end().
 *
 * To restate in fewer words, don't worry about references or ownership because
 * magic.
 */

#define GRAVITON_SERVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                                       GRAVITON_SERVER_TYPE, \
                                                                       GravitonServerPrivate))

GQuark
graviton_server_error_quark ()
{
  return g_quark_from_static_string ("graviton-server-error-quark");
}

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
  GHashTable *plugin_names;
  GList *streams;
  GList *publish_methods;

  gchar *cloud_id;
  gchar *node_id;

  GMutex event_lock;
  GList *event_listeners;

  guint64 last_event_id;
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

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
  const gchar *method_name;
  gchar *service_name;
  GVariant *method_result;

  builder = json_builder_new ();

  request_id = json_object_get_string_member (request, "id");
  service_name = strdup (json_object_get_string_member (request, "method"));
  g_debug ("Looking to call %s", service_name);
  method_name = strchr (service_name, '.') + 1;
  service_name[method_name - service_name - 1] = 0;


  service =
    graviton_service_get_subservice (GRAVITON_SERVICE (self->priv->plugins),
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
          param_value =
            json_gvariant_deserialize (json_object_get_member (param_obj,
                                                               param->data), NULL,
                                       &error);
          g_hash_table_replace (args, g_strdup (param->data), param_value);
          param = g_list_next (param);
        }
        g_list_free (param_names);
      }
    }
    method_result = graviton_service_call_method (service,
                                                  method_name,
                                                  args,
                                                  &error);
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

  g_mutex_lock (&self->priv->event_lock);
  self->priv->event_listeners =
    g_list_append (self->priv->event_listeners, msg);
  g_mutex_unlock (&self->priv->event_lock);
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
cb_aborted_request (SoupServer *server,
                    SoupMessage *message,
                    SoupClientContext *client,
                    gpointer user_data)
{
  GravitonServer *self = GRAVITON_SERVER (user_data);

  g_mutex_lock (&self->priv->event_lock);
  self->priv->event_listeners = g_list_remove (self->priv->event_listeners,
                                               message);
  g_mutex_unlock (&self->priv->event_lock);

#ifdef GRAVITON_ENABLE_STREAMS
  GList *cur = self->priv->streams;
  while (cur) {
    StreamConnection *conn = (StreamConnection*)cur->data;
    if (conn->message == message) {
      g_cancellable_cancel (conn->cancellable);
      break;
    }
    cur = cur->next;
  }
#endif // GRAVITON_ENABLE_STREAMS

  g_object_unref (G_OBJECT (message));

  g_debug ("Aborted event stream");
}

static void
cb_event (GravitonService *service,
          const gchar *name,
          GVariant *data,
          gpointer user_data)
{
  GravitonServer *self = GRAVITON_SERVER (user_data);
  GList *client;
  JsonBuilder *builder;
  JsonNode *result = NULL;
  JsonGenerator *generator;

  g_debug ("Event from a service: %s", name);

  generator = json_generator_new ();

  builder = json_builder_new ();
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "id");
  json_builder_add_int_value (builder, self->priv->last_event_id++);
  json_builder_set_member_name (builder, "name");
  json_builder_add_string_value (builder, name);
  json_builder_set_member_name (builder, "data");
  if (data) {
    json_builder_add_value (builder, json_gvariant_serialize (data));
  } else {
    json_builder_add_null_value (builder);
  }

  json_builder_end_object (builder);
  result = json_builder_get_root (builder);
  g_object_unref (builder);
  json_generator_set_root (generator, result);
  gsize length;
  gchar *json = json_generator_to_data (generator, &length);
  gchar *full_data = g_strconcat (json, "\r\n", NULL);
  length = strlen(full_data);
  json_node_free (result);

  // Copy the list of event listeners which will all be unrefed immediately after
  // transmission
  g_mutex_lock (&self->priv->event_lock);
  client = self->priv->event_listeners;
  self->priv->event_listeners = NULL;
  g_mutex_unlock (&self->priv->event_lock);

  while (client) {
    SoupMessageBody *body;
    g_object_get (client->data, SOUP_MESSAGE_RESPONSE_BODY, &body, NULL);
    soup_message_body_append (body, SOUP_MEMORY_COPY, full_data, length);
    soup_message_body_complete (body);
    soup_server_unpause_message (self->priv->server, client->data);
    client = g_list_next (client);
    g_debug ("Sent event: %s", (gchar*)json);
  }

  g_free (full_data);
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

#ifdef GRAVITON_ENABLE_STREAMS
  soup_server_add_handler (server, "/stream", cb_handle_stream, self, NULL);
#endif // GRAVITON_ENABLE_STREAMS

  return server;
}

static void
graviton_server_init (GravitonServer *self)
{
  GravitonServerPrivate *priv;
  self->priv = priv = GRAVITON_SERVER_GET_PRIVATE (self);
  uuid_t uuid;

  self->priv->cloud_id = g_strdup (graviton_config_get_default_cloud_id ());

  //self->priv->cloud_id = g_new0(gchar, 37);
  //uuid_unparse_upper (uuid, self->priv->cloud_id);
  uuid_generate (uuid);
  self->priv->node_id = g_new0(gchar, 37);
  uuid_unparse_upper (uuid, self->priv->node_id);

  g_mutex_init (&priv->event_lock);

  priv->plugins = graviton_root_service_new ();
  priv->event_listeners = NULL;
  priv->streams = NULL;
  priv->server = new_server (self, SOUP_ADDRESS_FAMILY_IPV4);
  priv->server6 = new_server (self, SOUP_ADDRESS_FAMILY_IPV6);

  g_signal_connect (priv->plugins,
                    "event",
                    G_CALLBACK (cb_event),
                    self);

  graviton_service_add_subservice (GRAVITON_SERVICE (self->priv->plugins),
                                   g_object_new (
                                     GRAVITON_INTROSPECTION_CONTROL_TYPE,
                                     "server", self,
                                     "name", "net:phrobo:graviton",
                                     NULL));

}

/**
 * graviton_server_new:
 *
 * Creates a new #GravitonServer object
 *
 * Returns: A new #GravitonServer
 */
GravitonServer *
graviton_server_new ()
{
  return g_object_new (GRAVITON_SERVER_TYPE, NULL);
}

/**
 * graviton_server_run_async:
 * @server: The #GravitonServer to start
 *
 * Starts @server, causing it to listen for and process requests.
 *
 * This runs in @server's #GMainContext. It will not perform any processing
 * unless the appropriate main loop is running.
 */
void
graviton_server_run_async (GravitonServer *self)
{
  graviton_server_load_publish_plugins (self);
  soup_server_run_async (self->priv->server);
  soup_server_run_async (self->priv->server6);
}

/**
 * graviton_server_get_root_service:
 * @server: The graviton server in question
 *
 * Gets the #GravitonRootService for this server. Required for attaching
 * sub-services and exposing them to the cloud this server is a member of.
 *
 * Returns: (transfer full): The #GravitonRootService for this server. Must be
 * freed with g_object_unref() when finished.
 */
GravitonRootService *
graviton_server_get_root_service (GravitonServer *server)
{
  g_object_ref (server->priv->plugins);
  return server->priv->plugins;
}

/**
 * graviton_server_get_node_id:
 * @server: The server in question
 *
 * Gets the ID of the node this server is providing
 *
 * Returns: (transfer none): Node ID of this server
 */
const gchar *
graviton_server_get_node_id (GravitonServer *server)
{
  return server->priv->node_id;
}

/**
 * graviton_server_get_cloud_id:
 * @server: The server in question
 *
 * Gets the ID of the cloud this server is a member of
 *
 * Returns: (transfer none): Cloud ID of this server
 */
const gchar *
graviton_server_get_cloud_id (GravitonServer *server)
{
  return server->priv->cloud_id;
}

/**
 * graviton_server_get_port:
 * @server: The server in question
 *
 * Gets the TCP port number this server is listening on.
 *
 * Returns: The port this server is listening on
 */
int
graviton_server_get_port (GravitonServer *server)
{
  return soup_server_get_port (server->priv->server);
}

void
graviton_server_load_publish_plugins (GravitonServer *self)
{
  int i;
  GArray *plugins;

  plugins = graviton_server_find_publish_plugins (self);
  for (i = 0; i < plugins->len; i++) {
    GravitonServerPublishMethodLoaderFunc factory = g_array_index (plugins,
                                                                    GravitonServerPublishMethodLoaderFunc,
                                                                    i);
    GravitonServerPublishMethod *method = factory(self);
    graviton_server_add_publish_method (self, method);
    g_object_unref (method);
    graviton_server_publish_method_start (method);
  }

  g_array_unref (plugins);
  g_debug ("loaded %i publish plugins", i);
}

void
graviton_server_add_publish_method (GravitonServer *self,
                                    GravitonServerPublishMethod *method)
{
  g_object_ref_sink (method);
  self->priv->publish_methods = g_list_append (self->priv->publish_methods,
                                               method);
}

GArray*
graviton_server_find_publish_plugins (GravitonServer *self)
{
  GArray *plugin_list =
    g_array_new(FALSE, FALSE, sizeof (GravitonServerPublishMethodLoaderFunc));
  const gchar *plugin_path = g_getenv("GRAVITON_PUBLISH_PLUGIN_PATH");
  if (!plugin_path)
    plugin_path = GRAVITON_DEFAULT_PUBLISH_PLUGIN_PATH;
  g_debug ("Searching %s for plugins\n", plugin_path);
  GDir *plugin_dir = g_dir_open (plugin_path, 0, NULL);
  if (!plugin_dir) {
    g_debug ("Plugin path not found: %s", plugin_path);
    return plugin_list;
  }
  const gchar *entry = g_dir_read_name (plugin_dir);
  while (entry) {
    gchar *entry_path = g_build_path ("/", plugin_path, entry, NULL);
    if (!g_str_has_suffix (entry_path, ".so"))
      goto next_plugin;
    g_debug ("Attempting to load plugin %s", entry_path);
    GModule *module = g_module_open (entry_path, G_MODULE_BIND_LOCAL);
    GravitonServerPublishMethodLoaderFunc loader = NULL;
    if (!module) {
      g_warning ("Can't open plugin %s: %s", entry_path, g_module_error ());
      goto next_plugin;
    }

    if (!g_module_symbol (module, "make_graviton_publish_plugin",
                          (gpointer *)&loader)) {
      g_warning ("Can't find make_graviton_publish_plugin symbol in %s: %s",
                 entry_path,
                 g_module_error ());
      goto bad_plugin;
    }

    if (loader == NULL) {
      g_warning ("make_graviton_publish_plugin symbol is NULL in %s: %s",
                 entry_path,
                 g_module_error ());
      goto bad_plugin;
    }

    g_module_make_resident (module);
    g_array_append_val (plugin_list, loader);

    goto next_plugin;

bad_plugin:
    g_module_close (module);

next_plugin:
    g_free (entry_path);
    entry = g_dir_read_name (plugin_dir);
  }

  g_dir_close (plugin_dir);

  return plugin_list;
}

#ifdef GRAVITON_ENABLE_STREAMS
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
new_stream (SoupMessage *message, GravitonStream *stream,
            GravitonServer *server)
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
    SoupMessageBody *body;
    g_object_get (connection->message, SOUP_MESSAGE_RESPONSE_BODY, &body, NULL);
    soup_message_body_complete (body);
    soup_server_unpause_message (connection->server->priv->server, connection->message);
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
  connection->server->priv->streams = g_list_remove (
    connection->server->priv->streams,
    connection);
  g_object_unref (connection->server);
}

static void
cb_read_stream (GObject *source, GAsyncResult *res, gpointer user_data)
{
  GError *error = NULL;
  SoupMessageBody *body;
  StreamConnection *connection = (StreamConnection*)user_data;

  if (g_cancellable_set_error_if_cancelled (connection->cancellable, &error)) {
    free_stream (connection);
    return;
  }

  gssize read_size = g_input_stream_read_finish (G_INPUT_STREAM (
                                                   source), res, &error);

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

  g_object_get (connection->message, SOUP_MESSAGE_RESPONSE_BODY, &body, NULL);
  soup_message_body_append (body, SOUP_MEMORY_COPY, connection->buf, read_size);
  g_debug ("Read %li", read_size); //FIXME: Should use G_SSIZE_FORMAT
  soup_server_unpause_message (connection->server->priv->server,
                               connection->message);
  g_input_stream_read_async (G_INPUT_STREAM (source),
                             connection->buf,
                             connection->bufsize,
                             G_PRIORITY_DEFAULT,
                             connection->cancellable,
                             cb_read_stream,
                             connection);
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

  service =
    graviton_service_get_subservice (GRAVITON_SERVICE (self->priv->plugins),
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

    g_object_get (msg,
                  SOUP_MESSAGE_RESPONSE_HEADERS,
                  &headers,
                  SOUP_MESSAGE_RESPONSE_BODY,
                  &body,
                  NULL);
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
      g_debug ("Unsupported operation %s for %s.%s.",
               method,
               service_name,
               stream_name);
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
#endif // GRAVITON_ENABLE_STREAMS
