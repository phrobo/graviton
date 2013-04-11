#include "mpd-plugin.h"

#include <json-glib/json-glib.h>
#include <mpd/client.h>

#define GRAVITON_MPD_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_MPD_PLUGIN, GravitonMPDPluginPrivate))

GRAVITON_DEFINE_PLUGIN(GRAVITON_TYPE_MPD_PLUGIN, "mpd")

G_DEFINE_TYPE (GravitonMPDPlugin, graviton_mpd_plugin, GRAVITON_TYPE_PLUGIN);

struct _GravitonMPDPluginPrivate
{
  struct mpd_connection *mpd;
};

static void
graviton_mpd_plugin_class_init (GravitonMPDPluginClass *klass)
{
  g_type_class_add_private (klass, sizeof (GravitonMPDPluginPrivate));
}

static enum mpd_error
connect_to_mpd(GravitonMPDPlugin *self)
{
  struct mpd_connection *ret;
  if (self->priv->mpd) {
    mpd_connection_free (self->priv->mpd);
  }
  ret = mpd_connection_new (NULL, 0, 0);
  self->priv->mpd = mpd_connection_new (NULL, 0, 0);
  return mpd_connection_get_error (self->priv->mpd);
}


static JsonNode *
cb_status(GravitonPlugin *plugin_self, const gchar *path, gpointer user_data)
{
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN(plugin_self);
  JsonNode *node;
  JsonBuilder *builder = json_builder_new ();

  json_builder_begin_object (builder);

  if (connect_to_mpd (self) == MPD_ERROR_SUCCESS) {
    struct mpd_status *status;
    status = mpd_run_status (self->priv->mpd);
    json_builder_set_member_name (builder, "state");
    switch (mpd_status_get_state(status)) {
      case MPD_STATE_STOP:
        json_builder_add_string_value (builder, "stopped");
        break;
      case MPD_STATE_PLAY:
        json_builder_add_string_value (builder, "playing");
        break;
      case MPD_STATE_PAUSE:
        json_builder_add_string_value (builder, "paused");
        break;
      default:
        g_warning ("Unknown MPD state: %d", mpd_status_get_state(status));
        json_builder_add_string_value (builder, "unknown");
    }

    mpd_status_free (status);
  } else {
    json_builder_set_member_name (builder, "error");
    json_builder_add_string_value (builder, mpd_connection_get_error_message (self->priv->mpd));
  }

  json_builder_end_object (builder);
  
  node = json_builder_get_root (builder);
  g_object_unref (builder);
  return node;
}

static void
graviton_mpd_plugin_init (GravitonMPDPlugin *self)
{
  GravitonMPDPluginPrivate *priv;
  self->priv = priv = GRAVITON_MPD_PLUGIN_GET_PRIVATE (self);
  priv->mpd = NULL;
  connect_to_mpd (self);

  graviton_plugin_register_handler(GRAVITON_PLUGIN(self), "status", cb_status, NULL);
}
