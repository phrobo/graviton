#ifndef GRAVITON_MPD_PLUGIN_H

#include <glib-object.h>
#include <graviton-plugin/plugin.h>

#define GRAVITON_TYPE_MPD_PLUGIN      (graviton_mpd_plugin_get_type ())
#define GRAVITON_MPD_PLUGIN(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_MPD_PLUGIN, GravitonMPDPlugin))
#define GRAVITON_IS_MPD_PLUGIN(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_TYPE_MPD_PLUGIN))
#define GRAVITON_MPD_PLUGIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_TYPE_MPD_PLUGIN, GravitonMPDPluginClass))
#define GRAVITON_IS_MPD_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_TYPE_MPD_PLUGIN))
#define GRAVITON_MPD_PLUGIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_TYPE_MPD_PLUGIN, GravitonMPDPluginClass))

typedef struct _GravitonMPDPlugin GravitonMPDPlugin;
typedef struct _GravitonMPDPluginClass GravitonMPDPluginClass;

typedef struct _GravitonMPDPluginPrivate GravitonMPDPluginPrivate;

struct _GravitonMPDPlugin
{
  GravitonPlugin parent_instance;
  GravitonMPDPluginPrivate *priv;
};

struct _GravitonMPDPluginClass
{
  GravitonPluginClass parent_class;
};

GType graviton_mpd_plugin_get_type ();

#endif // GRAVITON_MPD_PLUGIN_H
