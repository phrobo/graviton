#ifndef GRAVITON_INFO_PLUGIN_H
#define GRAVITON_INFO_PLUGIN_H

#include <glib-object.h>
#include <graviton-plugin/plugin.h>

#define GRAVITON_TYPE_INFO_PLUGIN     (graviton_info_plugin_get_type ())
#define GRAVITON_INFO_PLUGIN          (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_INFO_PLUGIN, GravitonInfoPlugin))
#define GRAVITON_IS_INFO_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_TYPE_INFO_PLUGIN))
#define GRAVITON_INFO_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_TYPE_INFO_PLUGIN, GravitonInfoPluginClass))
#define GRAVITON_IS_INFO_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_TYPE_INFO_PLUGIN))
#define GRAVITON_INFO_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_TYPE_INFO_PLUGIN, GravitonInfoPluginClass))

typedef struct _GravitonInfoPlugin GravitonInfoPlugin;
typedef struct _GravitonInfoPluginClass GravitonInfoPluginClass;

typedef struct _GravitonInfoPluginPrivate GravitonInfoPluginPrivate;

struct _GravitonInfoPlugin
{
  GravitonPlugin parent_instance;
  GravitonInfoPluginPrivate *priv;
};

struct _GravitonInfoPluginClass
{
  GravitonPluginClass parent_class;
};

GType graviton_info_plugin_get_type ();

#endif // GRAVITON_INFO_PLUGIN_H
