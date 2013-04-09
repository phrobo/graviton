#include "config.h"
#include "plugin-manager.h"
#include "info-plugin.h"
#include "plugin.h"

#include <glib.h>
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>

static void
cb_info (SoupServer *server,
         SoupMessage *msg,
         const char *path,
         GHashTable *query,
         SoupClientContext *client,
         gpointer user_data)
{
  GravitonPluginManager *manager = GRAVITON_PLUGIN_MANAGER (user_data);
  GravitonPlugin *plugin = graviton_plugin_manager_mounted_plugin (manager, path);
  g_printf("Requesting %s\n", path);
  if (!plugin) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
    return;
  }

  if (msg->method != SOUP_METHOD_GET) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
    return;
  }
  JsonGenerator *generator = json_generator_new ();

  JsonNode *root = GRAVITON_PLUGIN_GET_CLASS (plugin)->handle_get (plugin, path, query);
  json_generator_set_root (generator, root);
  gsize length;
  gchar *data = json_generator_to_data (generator, &length);

  json_node_free (root);
  g_object_unref (generator);

  soup_message_set_status (msg, SOUP_STATUS_OK);
  soup_message_set_response (msg,
                             "text/json",
                             SOUP_MEMORY_COPY,
                             data,
                             length);
  g_free (data);
}

int main(int argc, char** argv)
{
  GMainLoop *loop = NULL;
  GravitonPluginManager *manager = NULL;
  SoupServer *server = NULL;

  g_type_init ();

  loop = g_main_loop_new (NULL, FALSE);

  manager = graviton_plugin_manager_new ();
  GravitonInfoPlugin *info = g_object_new (GRAVITON_TYPE_INFO_PLUGIN, NULL);
  graviton_plugin_manager_mount_plugin (manager, GRAVITON_PLUGIN(info), "/info");

  server = soup_server_new (
    "port", 2718,
    "server-header", "Graviton/" GRAVITON_VERSION " ",
    NULL
  );

  soup_server_add_handler (server, NULL, cb_info, manager, NULL);

  soup_server_run_async (server);

  g_main_loop_run (loop);
  return 0;
}
