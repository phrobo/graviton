#ifndef GRAVITON_PLUGIN_H
#define GRAVITON_PLUGIN_H

#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <graviton-plugin/control.h>

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
};

GType graviton_plugin_get_type ();

const gchar *graviton_plugin_get_name (GravitonPlugin *plugin);

typedef GravitonPlugin *(*GravitonPluginLoaderFunc)(void);

#define GRAVITON_DEFINE_PLUGIN(type, name) \
  GravitonPlugin *make_graviton_plugin(void) { return g_object_new((type), "name", name, NULL); } \

typedef JsonNode *(*GravitonPluginPathHandler)(GravitonPlugin *self, const gchar *path, gpointer user_data);

void graviton_plugin_register_handler(GravitonPlugin *self, const gchar *path, GravitonPluginPathHandler handler, gpointer user_data);

void graviton_plugin_register_control (GravitonPlugin *self,
                                       GravitonControl *control);

GravitonControl *graviton_plugin_get_control (GravitonPlugin *self,
                                  const gchar *name);

GList *graviton_plugin_list_controls (GravitonPlugin *self);

#endif // GRAVITON_PLUGIN_H
