#ifndef GRAVITON_PLUGIN_H
#define GRAVITON_PLUGIN_H

#include <glib-object.h>

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

typedef GravitonPlugin *(*GravitonPluginLoaderFunc)(void);

#define GRAVITON_DEFINE_PLUGIN(type, name) \
  GravitonPlugin *make_graviton_plugin(void) { return g_object_new((type), "name", name, NULL); }

#endif // GRAVITON_PLUGIN_H
