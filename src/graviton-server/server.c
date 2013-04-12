#include "server.h"
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <graviton-plugin/plugin-manager.h>
#include <graviton-plugin/plugin.h>
#include <string.h>

#include "config.h"

#define GRAVITON_SERVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_SERVER, GravitonServerPrivate))

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

static void
cb_handle_soup (SoupServer *server,
         SoupMessage *msg,
         const char *path,
         GHashTable *query,
         SoupClientContext *client,
         gpointer user_data)
{
  GravitonServer *self = GRAVITON_SERVER (user_data);
  gchar *plugin_path;
  gchar **path_elements = g_strsplit(path, "/", 3);

  g_debug ("Requesting %s (%s)", path_elements[1], path);
  GravitonPlugin *plugin = graviton_plugin_manager_mounted_plugin (self->priv->plugins, path_elements[1]);

  if (strlen(path_elements[1]) == 0) {
    //TODO: list plugins
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
    goto out;
  }

  plugin_path = g_strjoinv("/", &path_elements[2]);
  if (!plugin) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
    goto out;
  }

  if (msg->method != SOUP_METHOD_GET) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
    goto out;
  }
  JsonGenerator *generator = json_generator_new ();

  JsonNode *root = GRAVITON_PLUGIN_GET_CLASS (plugin)->handle_get (plugin, plugin_path, query);
  if (!root) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
    g_object_unref (generator);
    goto out;
  }
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
out:
  g_strfreev (path_elements);
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
