#include "plugin-manager.h"
#include "plugin.h"

#define GRAVITON_PLUGIN_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_PLUGIN_MANAGER, GravitonPluginManagerPrivate))

G_DEFINE_TYPE (GravitonPluginManager, graviton_plugin_manager, G_TYPE_OBJECT);

struct _GravitonPluginManagerPrivate
{
  GHashTable *plugins;
};

static void
graviton_plugin_manager_class_init (GravitonPluginManagerClass *klass)
{
  g_type_class_add_private (klass, sizeof (GravitonPluginManagerPrivate));
}

static void
graviton_plugin_manager_init (GravitonPluginManager *self)
{
  GravitonPluginManagerPrivate *priv;
  self->priv = priv = GRAVITON_PLUGIN_MANAGER_GET_PRIVATE (self);
  priv->plugins = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         g_object_unref);

}

static void
graviton_plugin_manager_dispose (GObject *gobject)
{
  GravitonPluginManager *self = GRAVITON_PLUGIN_MANAGER (gobject);

  g_hash_table_unref (self->priv->plugins);
}

static void
graviton_plugin_manager_finalize (GObject *gobject)
{
  GravitonPluginManager *self = GRAVITON_PLUGIN_MANAGER (gobject);
}

GravitonPluginManager *
graviton_plugin_manager_new ()
{
  return g_object_new (GRAVITON_TYPE_PLUGIN_MANAGER, NULL);
}

void
graviton_plugin_manager_mount_plugin (GravitonPluginManager *self, GravitonPlugin *plugin, const gchar *mount)
{
  g_object_ref (plugin);
  g_debug ("Mounting %s", mount);
  g_hash_table_replace (self->priv->plugins, g_strdup(mount), plugin);
}

GravitonPlugin *
graviton_plugin_manager_mounted_plugin (GravitonPluginManager *self, const gchar *mount)
{
  return g_hash_table_lookup (self->priv->plugins, mount);
}

GArray *graviton_plugin_manager_find_plugins (GravitonPluginManager *self)
{
  GArray *pluginList = g_array_new(FALSE, FALSE, sizeof (GravitonPluginInfo*));
  const gchar *pluginPath = g_getenv("GRAVITON_PLUGIN_PATH");
  g_debug ("Searching %s for plugins\n", pluginPath);
  GDir *pluginDir = g_dir_open (pluginPath, 0, NULL);
  const gchar *entry = g_dir_read_name (pluginDir);
  while (entry) {
    gchar *entryPath = g_build_path ("/", pluginPath, entry, NULL);
    if (!g_str_has_suffix (entryPath, ".so"))
      goto nextPlugin;
    g_debug ("Attempting to load plugin %s", entryPath);
    GModule *module = g_module_open (entryPath, G_MODULE_BIND_LOCAL);
    GravitonPluginInfo *plugin_info;
    if (!module) {
      g_warning ("Can't open plugin %s: %s", entryPath, g_module_error ());
      goto nextPlugin;
    }

    if (!g_module_symbol (module, "graviton_plugin", (gpointer *)&plugin_info)) {
      g_warning ("Can't find graviton_plugin symbol in %s: %s", entryPath, g_module_error ());
      goto nextPlugin;
    }

    if (plugin_info == NULL) {
      g_warning ("graviton_plugin symbol is NULL in %s: %s", entryPath, g_module_error ());
      goto nextPlugin;
    }

    g_array_append_val (pluginList, plugin_info);

nextPlugin:
    entry = g_dir_read_name (pluginDir);
  }

  g_dir_close (pluginDir);

  return pluginList;
}

GList *
graviton_plugin_manager_list_plugins (GravitonPluginManager *self)
{
  return g_hash_table_get_keys (self->priv->plugins);
}
