#ifndef GRAVITON_PLUGIN_MANAGER_H
#define GRAVITON_PLUGIN_MANAGER_H

#include <glib-object.h>

#define GRAVITON_TYPE_PLUGIN_MANAGER            (graviton_plugin_manager_get_type ())
#define GRAVITON_PLUGIN_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_PLUGIN_MANAGER, GravitonPluginManager))
#define GRAVITON_IS_PLUGIN_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_TYPE_PLUGIN_MANAGER))
#define GRAVITON_PLUGIN_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_TYPE_PLUGIN_MANAGER, GravitonPluginManagerClass))
#define GRAVITON_IS_PLUGIN_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_TYPE_PLUGIN_MANAGER))
#define GRAVITON_PLUGIN_GET_MANAGER_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_TYPE_PLUGIN_MANAGER, GravitonPluginManagerClass))

typedef struct _GravitonPluginManager GravitonPluginManager;
typedef struct _GravitonPluginManagerClass GravitonPluginManagerClass;

typedef struct _GravitonPluginManagerPrivate GravitonPluginManagerPrivate;

struct _GravitonPluginManager
{
  GObject parent_instance;
  GravitonPluginManagerPrivate *priv;
};

struct _GravitonPluginManagerClass
{
  GObjectClass parent_class;
};

typedef struct _GravitonPlugin GravitonPlugin;

GType graviton_plugin_manager_get_type ();
GravitonPluginManager *graviton_plugin_manager_new ();
void graviton_plugin_manager_mount_plugin (GravitonPluginManager *manager, GravitonPlugin *plugin);
GravitonPlugin *graviton_plugin_manager_mounted_plugin (GravitonPluginManager *manager, const gchar *mount);
GArray *graviton_plugin_manager_find_plugins (GravitonPluginManager *manager);
GList *graviton_plugin_manager_list_plugins (GravitonPluginManager *self);

#endif // GRAVITON_PLUGIN_H
