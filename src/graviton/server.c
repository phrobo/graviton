#include "server.h"
#include "internal-plugin.h"
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <graviton/plugin-manager.h>
#include <graviton/plugin.h>
#include <graviton/control.h>
#include <string.h>

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
  GravitonPluginManager *plugins;
  GList *event_listeners;
  GHashTable *plugin_names;
};

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

  int split = g_strv_length (rpc_method_name)-1;

  method_name = g_strdup (rpc_method_name[split]);
  g_free (rpc_method_name[split]);
  rpc_method_name[split] = NULL;
  control_name = g_strjoinv (".", rpc_method_name);
  g_strfreev (rpc_method_name);

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
          g_hash_table_replace (args, param->data, param_value);
          g_debug ("Setting param %s", param->data);
          param = g_list_next (param);
        }
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
  JsonNode *result;

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
  json_generator_set_root (generator, result);
  gsize length;
  gchar *data = json_generator_to_data (generator, &length);

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

static void
graviton_server_init (GravitonServer *self)
{
  GravitonServerPrivate *priv;
  SoupAddress *address = soup_address_new_any (SOUP_ADDRESS_FAMILY_IPV4, 2718);
  self->priv = priv = GRAVITON_SERVER_GET_PRIVATE (self);

  priv->plugins = graviton_plugin_manager_new ();
  g_signal_connect (priv->plugins,
                    "property-update",
                    G_CALLBACK (cb_property_update),
                    self);
  priv->event_listeners = NULL;

  priv->server = soup_server_new (
    "interface", address,
    "server-header", "Graviton/" GRAVITON_VERSION " ",
    NULL
  );

  g_object_unref (address);

  g_signal_connect (priv->server,
                    "request-aborted",
                    G_CALLBACK(cb_aborted_request),
                    self);

  soup_server_add_handler (priv->server, "/rpc", cb_handle_rpc, self, NULL);
  soup_server_add_handler (priv->server, "/events", cb_handle_events, self, NULL);

  graviton_control_add_subcontrol (GRAVITON_CONTROL (self->priv->plugins),
                                   g_object_new (GRAVITON_TYPE_INTERNAL_PLUGIN, 
                                                 "server", self,
                                                 "name", "graviton",
                                                 NULL));
}

GravitonServer *graviton_server_new ()
{
  return g_object_new (GRAVITON_TYPE_SERVER, NULL);
}
void graviton_server_run_async (GravitonServer *self)
{
  soup_server_run_async (self->priv->server);
}

void graviton_server_load_plugins (GravitonServer *self)
{
  int i;
  GArray *plugins;
  
  plugins = graviton_plugin_manager_find_plugins (self->priv->plugins);
  for (i = 0; i < plugins->len; i++) {
    GravitonPluginLoaderFunc factory = g_array_index (plugins, GravitonPluginLoaderFunc, i);
    GravitonPlugin *plugin = factory();
    graviton_control_add_subcontrol (GRAVITON_CONTROL (self->priv->plugins), GRAVITON_CONTROL (plugin));
  }
}

GravitonPluginManager *
graviton_server_get_plugin_manager (GravitonServer *self)
{
  g_object_ref (self->priv->plugins);
  return self->priv->plugins;
}
