#include "mpd-service.h"

#include <json-glib/json-glib.h>
#include <mpd/client.h>
#include <gio/gunixinputstream.h>

#define GRAVITON_MPD_SERVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_MPD_SERVICE_TYPE, GravitonMPDServicePrivate))

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

G_DEFINE_TYPE (GravitonMPDService, graviton_mpd_service, GRAVITON_SERVICE_TYPE);

enum
{
  PROP_0,
  PROP_ADDRESS,
  PROP_PORT,
  PROP_STATE,
  PROP_QUEUE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

struct _GravitonMPDServicePrivate
{
  gchar *address;
  guint port;
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
    g_warning ("setting mpd error :(");
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
update_status (GravitonMPDService *self)
{
  if (self->priv->last_status)
    mpd_status_free (self->priv->last_status);
  self->priv->last_status = mpd_run_status (self->priv->mpd);
  g_object_notify_by_pspec (G_OBJECT(self), obj_properties[PROP_STATE]);
}

static void
update_queue (GravitonMPDService *self)
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
parse_idle(GravitonMPDService *self)
{
  enum mpd_idle idle_status;
  g_debug ("recv idle");
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
  GravitonMPDService *self = GRAVITON_MPD_SERVICE (user_data);
  g_debug ("Catching an idle event!");
  parse_idle (self);
  g_debug ("sending idle");
  mpd_send_idle (self->priv->mpd);
}

static gboolean
stop_mpd_idle (GravitonMPDService *self)
{
  g_assert (self->priv->mpd_source);
  g_source_destroy (self->priv->mpd_source);
  self->priv->mpd_source = NULL;
  g_debug ("Sending noidle");
  if (!mpd_send_noidle (self->priv->mpd)) {
    g_debug ("could not send noidle!");
    return FALSE;
  }
  parse_idle (self);
  g_debug ("Idle has been stopped.");
  return TRUE;
}

static void
resume_mpd_idle (GravitonMPDService *self)
{
  g_assert (self->priv->mpd_source == NULL);
  self->priv->mpd_source = g_pollable_input_stream_create_source (self->priv->mpd_stream, NULL);
  g_source_set_callback (self->priv->mpd_source, (GSourceFunc)cb_mpd_idle, self, NULL);
  g_source_attach (self->priv->mpd_source, NULL);
  g_debug ("Sending idle");
  mpd_send_idle (self->priv->mpd);
  g_debug ("Idle has been resumed");
}

static enum mpd_error
connect_to_mpd(GravitonMPDService *self, GError **error)
{
  if (!self->priv->mpd) {
    self->priv->mpd = mpd_connection_new (self->priv->address, self->priv->port, 0);
  }
  enum mpd_error err = mpd_connection_get_error (self->priv->mpd);
  if (err != MPD_ERROR_SUCCESS) {
    set_mpd_error (error, self->priv->mpd);
  } else {
    self->priv->mpd_stream = G_POLLABLE_INPUT_STREAM (g_unix_input_stream_new (mpd_connection_get_fd (self->priv->mpd), FALSE));
  }

  return err;
}


static const gchar *
mpd_state_string (GravitonMPDService *self)
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
  GravitonMPDService *self = GRAVITON_MPD_SERVICE (object);

  switch (property_id) {
    case PROP_ADDRESS:
      g_free (self->priv->address);
      self->priv->address = g_value_dup_string (value);
      break;
    case PROP_PORT:
      self->priv->port = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static GVariant *
queue_to_variant (GravitonMPDService *self)
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
  GravitonMPDService *self = GRAVITON_MPD_SERVICE (object);
  GVariant *v;

  switch (property_id) {
    case PROP_PORT:
      g_value_set_uint (value, self->priv->port);
      break;
    case PROP_ADDRESS:
      g_value_set_string (value, self->priv->address);
      break;
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
graviton_mpd_service_class_init (GravitonMPDServiceClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;
  g_type_class_add_private (klass, sizeof (GravitonMPDServicePrivate));

  obj_properties[PROP_ADDRESS] =
    g_param_spec_string ("address",
                         "mpd address",
                         "Address of the MPD server",
                         "localhost",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  obj_properties[PROP_PORT] =
    g_param_spec_uint ("port",
                       "mpd port",
                       "Address of the MPD server",
                       0,
                       65535,
                       0,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  obj_properties[PROP_STATE] =
    g_param_spec_string ("state", 
                         "playback state",
                         "Current MPD playback state",
                         "unknown",
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  obj_properties[PROP_QUEUE] =
    g_param_spec_variant ("queue", 
                          "playback queue",
                          "Current MPD playback queue",
                          (GVariantType*)"aa{s*}",
                          NULL,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static JsonNode *
cb_status(GravitonService *service_self, const gchar *path, GError **error, gpointer user_data)
{
  GravitonMPDService *self = GRAVITON_MPD_SERVICE(service_self);
  JsonNode *node;
  JsonBuilder *builder = json_builder_new ();

  json_builder_begin_object (builder);

  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
    stop_mpd_idle (self);
    update_status (self);
    resume_mpd_idle (self);
    json_builder_set_member_name (builder, "state");
    switch (mpd_status_get_state(self->priv->last_status)) {
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
        g_warning ("Unknown MPD state: %d", mpd_status_get_state(self->priv->last_status));
        json_builder_add_string_value (builder, "unknown");
    }
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
cb_playlist(GravitonService *service_self, const gchar *path, GError **error, gpointer user_data)
{
  GravitonMPDService *self = GRAVITON_MPD_SERVICE(service_self);
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
cb_previous (GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  GravitonMPDService *self = GRAVITON_MPD_SERVICE (service);
  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
    if (mpd_run_previous (self->priv->mpd))
      return NULL;
    else
      set_mpd_error (error, self->priv->mpd);
  }
  return NULL;
}

static GVariant *
cb_next (GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  GravitonMPDService *self = GRAVITON_MPD_SERVICE (service);
  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
    g_print ("Stopping idle\n");
    if (!stop_mpd_idle (self)) {
      set_mpd_error (error, self->priv->mpd);
      return NULL;
    }
    g_print ("Running next\n");
    if (!mpd_run_next (self->priv->mpd)) {
      set_mpd_error (error, self->priv->mpd);
      return NULL;
    }
    g_debug ("Next has been performed");
    resume_mpd_idle (self);
  }
  return NULL;
}

static GVariant *
cb_pause (GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  GravitonMPDService *self = GRAVITON_MPD_SERVICE (service);
  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
    stop_mpd_idle (self);
    if (!mpd_run_pause (self->priv->mpd, true))
      set_mpd_error (error, self->priv->mpd);
    resume_mpd_idle (self);
  }
  return NULL;
}

static GVariant *
cb_play (GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  GravitonMPDService *self = GRAVITON_MPD_SERVICE (service);
  if (connect_to_mpd (self, error) == MPD_ERROR_SUCCESS) {
    stop_mpd_idle (self);
    if (!mpd_run_play (self->priv->mpd))
      set_mpd_error (error, self->priv->mpd);
    resume_mpd_idle (self);
  }
  return NULL;
}

static void
graviton_mpd_service_init (GravitonMPDService *self)
{
  GravitonService *self_service = GRAVITON_SERVICE (self);
  GravitonMPDServicePrivate *priv;
  self->priv = priv = GRAVITON_MPD_SERVICE_GET_PRIVATE (self);
  priv->mpd = NULL;
  priv->last_status = NULL;
  priv->last_queue = NULL;

  if (connect_to_mpd (self, NULL) == MPD_ERROR_SUCCESS) {
    update_status (self);
    update_queue (self);
    resume_mpd_idle (self);
  }

  graviton_service_add_method (self_service,
                               "play",
                               cb_play,
                               NULL,
                               NULL);
  graviton_service_add_method (self_service,
                               "pause",
                               cb_pause,
                               NULL,
                               NULL);
  graviton_service_add_method (self_service,
                               "next",
                               cb_next,
                               NULL,
                               NULL);
  graviton_service_add_method (self_service,
                               "previous",
                               cb_previous,
                               NULL,
                               NULL);
}

GravitonMPDService *
graviton_mpd_service_new (const gchar *address, guint port)
{
  return g_object_new (GRAVITON_MPD_SERVICE_TYPE,
                      "name", "net:phrobo:graviton:examples:mpd",
                      "address", address,
                      "port", port,
                      NULL);
}
