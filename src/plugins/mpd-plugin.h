#ifndef GRAVITON_MPD_PLUGIN_H

#include <glib-object.h>
#include <graviton-plugin/plugin.h>
#include <mpd/client.h>

#define GRAVITON_TYPE_MPD_PLUGIN      (graviton_mpd_plugin_get_type ())
#define GRAVITON_MPD_PLUGIN(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_MPD_PLUGIN, GravitonMPDPlugin))
#define GRAVITON_IS_MPD_PLUGIN(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_TYPE_MPD_PLUGIN))
#define GRAVITON_MPD_PLUGIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_TYPE_MPD_PLUGIN, GravitonMPDPluginClass))
#define GRAVITON_IS_MPD_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_TYPE_MPD_PLUGIN))
#define GRAVITON_MPD_PLUGIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_TYPE_MPD_PLUGIN, GravitonMPDPluginClass))

#define GRAVITON_MPD_ERROR (graviton_mpd_error_quark ())
#define GRAVITON_MPD_SERVER_ERROR (graviton_mpd_server_error_quark ())
#define GRAVITON_MPD_SYSTEM_ERROR (graviton_mpd_system_error_quark ())

typedef enum {
  GRAVITON_MPD_ERROR_SUCCESS = MPD_ERROR_SUCCESS,
  GRAVITON_MPD_ERROR_OOM = MPD_ERROR_OOM,
  GRAVITON_MPD_ERROR_ARGUMENT = MPD_ERROR_ARGUMENT,
  GRAVITON_MPD_ERROR_STATE = MPD_ERROR_STATE,
  GRAVITON_MPD_ERROR_TIMEOUT = MPD_ERROR_TIMEOUT,
  GRAVITON_MPD_ERROR_SYSTEM = MPD_ERROR_SYSTEM,
  GRAVITON_MPD_ERROR_RESOLVER = MPD_ERROR_RESOLVER,
  GRAVITON_MPD_ERROR_MALFORMED = MPD_ERROR_MALFORMED,
  GRAVITON_MPD_ERROR_CLOSED = MPD_ERROR_CLOSED,
  GRAVITON_MPD_ERROR_SERVER = MPD_ERROR_SERVER
} GravitonMPDError;

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
