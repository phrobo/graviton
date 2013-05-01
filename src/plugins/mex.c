#include <graviton/control.h>
#include <graviton/plugin.h>
#include <gio/gio.h>

typedef struct _GravitonMEXPlugin GravitonMEXPlugin;
typedef struct _GravitonMEXPluginClass GravitonMEXPluginClass;

struct _GravitonMEXPluginClass
{
  GravitonPluginClass parent_class;
};

struct _GravitonMEXPlugin
{
  GravitonPlugin parent_instance;
  GDBusProxy *player;
};

#define GRAVITON_TYPE_MEX_PLUGIN (graviton_mex_plugin_get_type ())
#define GRAVITON_MEX_PLUGIN (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_MEX_PLUGIN, GravitonMEXPlugin))

G_DEFINE_TYPE (GravitonMEXPlugin, graviton_mex_plugin, GRAVITON_TYPE_PLUGIN);
GRAVITON_DEFINE_PLUGIN(GRAVITON_TYPE_MEX_PLUGIN, "mex")

static GVariant *
cb_play (GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GravitonMEXPlugin *self = GRAVITON_MEX_PLUGIN (control);
  gchar *uri = g_hash_table_lookup (args, "uri");

  GVariant *playerArgs = g_variant_new ("(s)", g_variant_get_string (g_hash_table_lookup (args, "uri"), NULL));

  g_dbus_proxy_call_sync (self->player,
                          "SetUri",
                          playerArgs,
                          G_DBUS_CALL_FLAGS_NONE,
                          -1,
                          NULL,
                          error);
  return NULL;
}

static void
graviton_mex_plugin_class_init (GravitonMEXPluginClass *klass)
{
}

static void
graviton_mex_plugin_init (GravitonMEXPlugin *self)
{
  self->player = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                NULL,
                                                "org.media-explorer.MediaExplorer",
                                                "/org/MediaExplorer/Player",
                                                "org.MediaExplorer.MediaPlayer",
                                                NULL,
                                                error);
  graviton_control_add_method (self,
                               "play",
                               cb_play,
                               0,
                               NULL,
                               self,
                               NULL);
}
