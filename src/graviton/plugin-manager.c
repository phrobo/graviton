#include "plugin-manager.h"
#include "plugin.h"
#include <gmodule.h>

#include "config.h"

#define GRAVITON_PLUGIN_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_PLUGIN_MANAGER, GravitonPluginManagerPrivate))

G_DEFINE_TYPE (GravitonPluginManager, graviton_plugin_manager, GRAVITON_TYPE_CONTROL);

struct _GravitonPluginManagerPrivate
{
  int dummy;
};

enum {
  SIGNAL_0,
  SIGNAL_PLUGIN_MOUNTED,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = {0, };

static void
graviton_plugin_manager_class_init (GravitonPluginManagerClass *klass)
{
  g_type_class_add_private (klass, sizeof (GravitonPluginManagerPrivate));

  obj_signals[SIGNAL_PLUGIN_MOUNTED] =
    g_signal_new ("plugin-mounted",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GRAVITON_TYPE_PLUGIN);
}

static void
graviton_plugin_manager_init (GravitonPluginManager *self)
{
  GravitonPluginManagerPrivate *priv;
  self->priv = priv = GRAVITON_PLUGIN_MANAGER_GET_PRIVATE (self);

}

static void
graviton_plugin_manager_dispose (GObject *gobject)
{
  GravitonPluginManager *self = GRAVITON_PLUGIN_MANAGER (gobject);
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

GArray *graviton_plugin_manager_find_plugins (GravitonPluginManager *self)
{
  GArray *pluginList = g_array_new(FALSE, FALSE, sizeof (GravitonPluginLoaderFunc));
  const gchar *pluginPath = g_getenv("GRAVITON_PLUGIN_PATH");
  if (!pluginPath)
    pluginPath = GRAVITON_DEFAULT_PLUGIN_PATH;
  g_debug ("Searching %s for plugins\n", pluginPath);
  GDir *pluginDir = g_dir_open (pluginPath, 0, NULL);
  if (!pluginDir) {
    g_debug ("Plugin path not found: %s", pluginPath);
    return pluginList;
  }
  const gchar *entry = g_dir_read_name (pluginDir);
  while (entry) {
    gchar *entryPath = g_build_path ("/", pluginPath, entry, NULL);
    if (!g_str_has_suffix (entryPath, ".so"))
      goto nextPlugin;
    g_debug ("Attempting to load plugin %s", entryPath);
    GModule *module = g_module_open (entryPath, G_MODULE_BIND_LOCAL);
    GravitonPluginLoaderFunc loader = NULL;
    if (!module) {
      g_warning ("Can't open plugin %s: %s", entryPath, g_module_error ());
      goto nextPlugin;
    }

    if (!g_module_symbol (module, "make_graviton_plugin", (gpointer *)&loader)) {
      g_warning ("Can't find graviton_plugin symbol in %s: %s", entryPath, g_module_error ());
      goto nextPlugin;
    }

    if (loader == NULL) {
      g_warning ("graviton_plugin symbol is NULL in %s: %s", entryPath, g_module_error ());
      goto nextPlugin;
    }

    g_array_append_val (pluginList, loader);

nextPlugin:
    g_free (entryPath);
    entry = g_dir_read_name (pluginDir);
  }

  g_dir_close (pluginDir);

  return pluginList;
}
