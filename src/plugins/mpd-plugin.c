#include "mpd-plugin.h"

#include <json-glib/json-glib.h>
#include <mpd/client.h>

#include <graviton-plugin/control.h>

#define GRAVITON_MPD_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_MPD_PLUGIN, GravitonMPDPluginPrivate))

GQuark
graviton_mpd_error_quark ()
{
  return g_quark_from_static_string ("graviton-mpd-error-quark");
}

GQuark
graviton_mpd_server_error_quark ()
{
  return g_quark_from_static_string ("graviton-mpd-server-error-quark");
}

GQuark
graviton_mpd_system_error_quark ()
{
  return g_quark_from_static_string ("graviton-mpd-system-error-quark");
}

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

static void
set_mpd_error (GError **error, struct mpd_connection *connection)
{
  enum mpd_error err = mpd_connection_get_error (connection);
  if (err != MPD_ERROR_SUCCESS) {
    if (err == MPD_ERROR_SERVER) {
      g_set_error (error,
                   GRAVITON_MPD_SERVER_ERROR,
                   mpd_connection_get_server_error (connection),
                   mpd_connection_get_error_message (connection));
    } else if (err == MPD_ERROR_SYSTEM) {
      g_set_error (error,
                   GRAVITON_MPD_SYSTEM_ERROR,
                   mpd_connection_get_system_error (connection),
                   mpd_connection_get_error_message (connection));
    } else {
      g_set_error (error,
                   GRAVITON_MPD_ERROR,
                   err,
                   mpd_connection_get_error_message (connection));
    }
  }
}

static enum mpd_error
connect_to_mpd(GravitonMPDPlugin *self, GError **error)
{
  struct mpd_connection *ret;
  if (self->priv->mpd) {
    mpd_connection_free (self->priv->mpd);
  }
  self->priv->mpd = mpd_connection_new ("10.2.0.6", 0, 0);
  enum mpd_error err = mpd_connection_get_error (self->priv->mpd);
  if (err != MPD_ERROR_SUCCESS)
    set_mpd_error (error, self->priv->mpd);

  return err;
}


static JsonNode *
cb_status(GravitonPlugin *plugin_self, const gchar *path, GError **error, gpointer user_data)
{
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN(plugin_self);
  JsonNode *node;
  JsonBuilder *builder = json_builder_new ();

  json_builder_begin_object (builder);

  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
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

static JsonNode*
cb_playlist(GravitonPlugin *plugin_self, const gchar *path, GError **error, gpointer user_data)
{
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN(plugin_self);
  JsonNode *node;
  JsonBuilder *builder = json_builder_new ();

  json_builder_begin_object (builder);
  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
  }
  json_builder_set_member_name (builder, "error");
  json_builder_add_string_value (builder, mpd_connection_get_error_message (self->priv->mpd));
  json_builder_end_object (builder);

  node = json_builder_get_root (builder);
  g_object_unref (builder);
  return node;
}

static GVariant *
cb_previous (GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN (user_data);
  gboolean success;
  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
    if (mpd_run_previous (self->priv->mpd))
      return NULL;
    else
      set_mpd_error (error, self->priv->mpd);
  }
  return NULL;
}

static GVariant *
cb_next (GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN (user_data);
  gboolean success;
  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
    if (mpd_run_next (self->priv->mpd))
      return NULL;
    else
      set_mpd_error (error, self->priv->mpd);
  }
  return NULL;
}

static GVariant *
cb_pause (GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN (user_data);
  gboolean success;
  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
    if (mpd_run_pause (self->priv->mpd, true))
      return NULL;
    else
      set_mpd_error (error, self->priv->mpd);
  }
  return NULL;
}

static GVariant *
cb_play (GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN (user_data);
  gboolean success;
  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
    if (mpd_run_play (self->priv->mpd))
      return NULL;
    else
      set_mpd_error (error, self->priv->mpd);
  }
  return NULL;
}

static void
graviton_mpd_plugin_init (GravitonMPDPlugin *self)
{
  GravitonMPDPluginPrivate *priv;
  self->priv = priv = GRAVITON_MPD_PLUGIN_GET_PRIVATE (self);
  priv->mpd = NULL;

  GravitonControl* playback = g_object_new (GRAVITON_TYPE_CONTROL, NULL);
  graviton_control_add_method (playback,
                               "play",
                               cb_play,
                               NULL,
                               0,
                               self);
  graviton_control_add_method (playback,
                               "pause",
                               cb_pause,
                               NULL,
                               0,
                               self);
  graviton_control_add_method (playback,
                               "next",
                               cb_next,
                               NULL,
                               0,
                               self);
  graviton_control_add_method (playback,
                               "previous",
                               cb_previous,
                               NULL,
                               0,
                               self);
 
  graviton_plugin_register_control (GRAVITON_PLUGIN(self), "playback", playback);
}
