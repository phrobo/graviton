#include "mpd-plugin.h"

#include <json-glib/json-glib.h>
#include <mpd/client.h>
#include <gio/gunixinputstream.h>

#include <graviton/control.h>

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

enum
{
  PROP_0,
  PROP_STATE,
  PROP_QUEUE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

struct _GravitonMPDPluginPrivate
{
  struct mpd_connection *mpd;
  GPollableInputStream *mpd_stream;
  GSource *mpd_source;
  struct mpd_status *last_status;
  GList *last_queue;
};

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

static void
update_status (GravitonMPDPlugin *self)
{
  if (self->priv->last_status)
    mpd_status_free (self->priv->last_status);
  self->priv->last_status = mpd_run_status (self->priv->mpd);
  g_object_notify_by_pspec (G_OBJECT(self), obj_properties[PROP_STATE]);
}

static void
update_queue (GravitonMPDPlugin *self)
{
  if (self->priv->last_queue) {
    GList *cur = self->priv->last_queue;
    while (cur) {
      mpd_song_free(cur->data);
    }
    g_list_free_full (self->priv->last_queue, (GDestroyNotify)mpd_song_free);
    self->priv->last_queue = NULL;
  }
  int i = 0;
  struct mpd_song *song = mpd_run_get_queue_song_pos (self->priv->mpd, i);
  while (song) {
    self->priv->last_queue = g_list_append (self->priv->last_queue, song);
    i++;
    g_debug ("Found song at %d: %s", i, mpd_song_get_tag (song, MPD_TAG_TITLE, 0));
    song = mpd_run_get_queue_song_pos (self->priv->mpd, i);
  }
  g_debug ("Playlist has %d items.", i);
  g_object_notify_by_pspec (G_OBJECT(self), obj_properties[PROP_QUEUE]);
}

static void
parse_idle(GravitonMPDPlugin *self)
{
  enum mpd_idle idle_status;
  idle_status = mpd_recv_idle (self->priv->mpd, TRUE);
  g_debug ("Got idle!");
  if (idle_status & MPD_IDLE_PLAYER) {
    g_debug ("Player update!");
    update_status (self);
  }
  if (idle_status & MPD_IDLE_DATABASE)
    g_debug ("Database update!");
  if (idle_status & MPD_IDLE_STORED_PLAYLIST)
    g_debug ("Playlist update!");
  if (idle_status & MPD_IDLE_QUEUE) {
    g_debug ("Queue update!");
    update_queue (self);
  }
  if (idle_status & MPD_IDLE_MIXER)
    g_debug ("Volume update!");
  if (idle_status & MPD_IDLE_OUTPUT)
    g_debug ("Output update!");
  if (idle_status & MPD_IDLE_OPTIONS)
    g_debug ("Option update!");
  if (idle_status & MPD_IDLE_UPDATE)
    g_debug ("Database update! Again!");
}

static void
cb_mpd_idle(GObject *stream, gpointer user_data)
{
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN (user_data);
  parse_idle (self);
  mpd_send_idle (self->priv->mpd);
}

static void
stop_mpd_idle (GravitonMPDPlugin *self)
{
  g_assert (self->priv->mpd_source);
  g_source_destroy (self->priv->mpd_source);
  self->priv->mpd_source = NULL;
  g_debug ("Sending noidle");
  mpd_send_noidle (self->priv->mpd);
  parse_idle (self);
}

static void
resume_mpd_idle (GravitonMPDPlugin *self)
{
  g_assert (self->priv->mpd_source == NULL);
  self->priv->mpd_source = g_pollable_input_stream_create_source (self->priv->mpd_stream, NULL);
  g_source_set_callback (self->priv->mpd_source, (GSourceFunc)cb_mpd_idle, self, NULL);
  g_source_attach (self->priv->mpd_source, NULL);
  g_debug ("Sending idle");
  mpd_send_idle (self->priv->mpd);
}

static enum mpd_error
connect_to_mpd(GravitonMPDPlugin *self, GError **error)
{
  if (!self->priv->mpd) {
    self->priv->mpd = mpd_connection_new ("10.2.0.6", 0, 0);
  }
  enum mpd_error err = mpd_connection_get_error (self->priv->mpd);
  if (err != MPD_ERROR_SUCCESS) {
    set_mpd_error (error, self->priv->mpd);
  } else {
    self->priv->mpd_stream = G_POLLABLE_INPUT_STREAM (g_unix_input_stream_new (mpd_connection_get_fd (self->priv->mpd), FALSE));
    if (self->priv->last_status) {
      mpd_status_free (self->priv->last_status);
      self->priv->last_status = NULL;
    }
  }

  return err;
}


static const gchar *
mpd_state_string (GravitonMPDPlugin *self)
{
  if (self->priv->last_status) {
    switch (mpd_status_get_state(self->priv->last_status)) {
      case MPD_STATE_STOP:
        return "stopped";
      case MPD_STATE_PLAY:
        return "playing";
      case MPD_STATE_PAUSE:
        return "paused";
      default:
        g_warning ("Unknown MPD state: %d", mpd_status_get_state(self->priv->last_status));
        return "unknown";
    }
  } else {
    return "disconnected";
  }
}

static void
set_property (GObject *object,
              guint property_id,
              const GValue *value,
              GParamSpec *pspec)
{
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN (object);

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static GVariant *
queue_to_variant (GravitonMPDPlugin *self)
{
  GVariantBuilder ret;
  g_variant_builder_init (&ret, (GVariantType*)"aa{s*}");
  GList *cur = self->priv->last_queue;
  while (cur) {
    GVariantBuilder song_variant;
    g_variant_builder_init (&song_variant, (GVariantType*)"a{s*}");
    int i;
    for(i = 0;i<MPD_TAG_COUNT;i++) {
      const gchar *tagValue = mpd_song_get_tag (cur->data, i, 0);
      if (tagValue) {
        g_variant_builder_add_parsed (&song_variant,
                                      "{%s, <%s>}",
                                      mpd_tag_name (i),
                                      mpd_song_get_tag (cur->data, i, 0));
      }
    }
    g_variant_builder_add_value (&ret, g_variant_builder_end (&song_variant));
    cur = g_list_next (cur);
  }

  return g_variant_builder_end (&ret);
}

static void
get_property (GObject *object,
              guint property_id,
              GValue *value,
              GParamSpec *pspec)
{
  GravitonMPDPlugin *self = GRAVITON_MPD_PLUGIN (object);
  GVariant *v;

  switch (property_id) {
    case PROP_STATE:
      g_value_set_string (value, mpd_state_string (self));
      break;
    case PROP_QUEUE:
      v = queue_to_variant (self);
      g_value_set_variant (value, v);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_mpd_plugin_class_init (GravitonMPDPluginClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;
  g_type_class_add_private (klass, sizeof (GravitonMPDPluginPrivate));

  obj_properties[PROP_STATE] =
    g_param_spec_string ("state", 
                         "playback state",
                         "Current MPD playback state",
                         "unknown",
                         G_PARAM_READABLE);
  obj_properties[PROP_QUEUE] =
    g_param_spec_variant ("queue", 
                          "playback queue",
                          "Current MPD playback queue",
                          (GVariantType*)"aa{s*}",
                          NULL,
                          G_PARAM_READABLE);
  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);
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
    stop_mpd_idle (self);
    if (!mpd_run_pause (self->priv->mpd, true))
      set_mpd_error (error, self->priv->mpd);
    resume_mpd_idle (self);
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
    stop_mpd_idle (self);
    if (!mpd_run_play (self->priv->mpd))
      set_mpd_error (error, self->priv->mpd);
    resume_mpd_idle (self);
  }
  return NULL;
}

static void
graviton_mpd_plugin_init (GravitonMPDPlugin *self)
{
  GravitonMPDPluginPrivate *priv;
  self->priv = priv = GRAVITON_MPD_PLUGIN_GET_PRIVATE (self);
  priv->mpd = NULL;
  priv->last_status = NULL;
  priv->last_queue = NULL;

  if (connect_to_mpd (self, NULL) == MPD_ERROR_SUCCESS) {
    update_status (self);
    update_queue (self);
    resume_mpd_idle (self);
  }

  GravitonControl* playback = g_object_new (GRAVITON_TYPE_CONTROL, "name", "playback", NULL);
  graviton_control_add_method (playback,
                               "play",
                               cb_play,
                               0,
                               NULL,
                               self,
                               NULL);
  graviton_control_add_method (playback,
                               "pause",
                               cb_pause,
                               0,
                               NULL,
                               self,
                               NULL);
  graviton_control_add_method (playback,
                               "next",
                               cb_next,
                               0,
                               NULL,
                               self,
                               NULL);
  graviton_control_add_method (playback,
                               "previous",
                               cb_previous,
                               0,
                               NULL,
                               self,
                               NULL);
 
  graviton_control_add_subcontrol (GRAVITON_CONTROL(self), playback);
}
