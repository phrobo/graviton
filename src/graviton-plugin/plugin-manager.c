#include "plugin-manager.h"

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
  g_hash_table_replace (self->priv->plugins, g_strdup(mount), plugin);
}

GravitonPlugin *
graviton_plugin_manager_mounted_plugin (GravitonPluginManager *self, const gchar *mount)
{
  return g_hash_table_lookup (self->priv->plugins, mount);
}
