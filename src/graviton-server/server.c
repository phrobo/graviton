#include "server.h"
#include "internal-plugin.h"
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <graviton-plugin/plugin-manager.h>
#include <graviton-plugin/plugin.h>
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
  gchar *request_id;
  GravitonPlugin *plugin;
  GravitonControl *control;
  gchar **method_name;
  GVariant *method_result;

  builder = json_builder_new ();

  request_id = g_strdup (json_object_get_string_member (request, "id"));
  method_name = g_strsplit (json_object_get_string_member (request, "method"), ".", 3);

  if (g_strv_length (method_name) != 3) {
    g_set_error (&error,
                 GRAVITON_SERVER_ERROR,
                 GRAVITON_SERVER_ERROR_NO_SUCH_METHOD,
                 "No such method: %s",
                 json_object_get_string_member (request, "method"));
    goto out;
  }

  plugin = graviton_plugin_manager_mounted_plugin (self->priv->plugins, method_name[0]);
  if (!plugin) {
    g_set_error (&error,
                 GRAVITON_SERVER_ERROR,
                 GRAVITON_SERVER_ERROR_NO_SUCH_METHOD,
                 "No such plugin: %s",
                 method_name[0]);
    goto out;
  }

  control = graviton_plugin_get_control (plugin, method_name[1]);
  if (!control) {
    g_set_error (&error,
                 GRAVITON_SERVER_ERROR,
                 GRAVITON_SERVER_ERROR_NO_SUCH_METHOD,
                 "No such control: %s.%s",
                 method_name[0], method_name[1]);
    goto out;
  }

  if (graviton_control_has_method (control, method_name[2])) {
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
    method_result = graviton_control_call_method (control, method_name[2], args, &error);
    g_hash_table_unref (args);
    if (method_result)
      g_variant_ref_sink (method_result);
  } else {
    g_set_error (&error,
                 GRAVITON_SERVER_ERROR,
                 GRAVITON_SERVER_ERROR_NO_SUCH_METHOD,
                 "No such method %s on control %s.%s",
                 method_name[2], method_name[0], method_name[1]);
  }

out:
  builder = json_builder_new ();
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "jsonrpc");
  json_builder_add_string_value (builder, "2.0");

  json_builder_set_member_name (builder, "id");
  if (request_id) {
    json_builder_add_string_value (builder, request_id);
    g_free (request_id);
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
  g_strfreev (method_name);
  g_object_unref (builder);
  
  if (plugin)
    g_object_unref (plugin);
  return result;
}

static void
cb_handle_soup (SoupServer *server,
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
graviton_server_init (GravitonServer *self)
{
  GravitonServerPrivate *priv;
  SoupAddress *address = soup_address_new_any (SOUP_ADDRESS_FAMILY_IPV4, 2718);
  self->priv = priv = GRAVITON_SERVER_GET_PRIVATE (self);

  priv->plugins = graviton_plugin_manager_new ();

  priv->server = soup_server_new (
    "interface", address,
    "server-header", "Graviton/" GRAVITON_VERSION " ",
    NULL
  );

  g_object_unref (address);

  soup_server_add_handler (priv->server, NULL, cb_handle_soup, self, NULL);

  graviton_plugin_manager_mount_plugin (self->priv->plugins,
                                        g_object_new (GRAVITON_TYPE_INTERNAL_PLUGIN, 
                                                      "server", self,
                                                      NULL),
                                        "graviton");
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
    GravitonPluginInfo *info = g_array_index (plugins, GravitonPluginInfo*, i);
    g_debug ("Mounting plugin on %s", info->mount);
    graviton_plugin_manager_mount_plugin (self->priv->plugins, info->make_plugin(), info->mount);
  }
}

GravitonPluginManager *
graviton_server_get_plugin_manager (GravitonServer *self)
{
  g_object_ref (self->priv->plugins);
  return self->priv->plugins;
}
