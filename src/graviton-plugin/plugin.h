#ifndef GRAVITON_PLUGIN_H
#define GRAVITON_PLUGIN_H

#include <glib-object.h>
#include <json-glib/json-glib.h>

#define GRAVITON_TYPE_PLUGIN            (graviton_plugin_get_type ())
#define GRAVITON_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_PLUGIN, GravitonPlugin))
#define GRAVITON_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_TYPE_PLUGIN))
#define GRAVITON_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_TYPE_PLUGIN, GravitonPluginClass))
#define GRAVITON_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_TYPE_PLUGIN))
#define GRAVITON_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_TYPE_PLUGIN, GravitonPluginClass))

typedef struct _GravitonPlugin GravitonPlugin;
typedef struct _GravitonPluginClass GravitonPluginClass;

typedef struct _GravitonPluginPrivate GravitonPluginPrivate;

struct _GravitonPlugin 
{
  GObject parent_instance;
  GravitonPluginPrivate *priv;
};

struct _GravitonPluginClass
{
  GObjectClass parent_class;
  JsonNode *(*handle_get) (GravitonPlugin *self, const gchar *path, GHashTable *args);
};

GType graviton_plugin_get_type ();

const gchar *graviton_plugin_get_name (GravitonPlugin *plugin);

typedef GravitonPlugin *(*GravitonPluginLoaderFunc)(void);

typedef struct _GravitonPluginInfo GravitonPluginInfo;

struct _GravitonPluginInfo
{
  GravitonPluginLoaderFunc make_plugin;
  const gchar *mount;
};

#define GRAVITON_DEFINE_PLUGIN(type, mountpoint) \
  GravitonPlugin *new_plugin(void) { return g_object_new((type), NULL); } \
  GravitonPluginInfo graviton_plugin = {\
    .make_plugin = (new_plugin), \
    .mount = (mountpoint) \
  }; \

#endif // GRAVITON_PLUGIN_H
