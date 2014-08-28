#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "avahi-publish.h"
#include <graviton/server/server.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-glib/glib-watch.h>

typedef struct _GravitonAvahiServerPublishMethodPrivate GravitonAvahiServerPublishMethodPrivate;

struct _GravitonAvahiServerPublishMethodPrivate
{
  AvahiClient *avahi;
  AvahiGLibPoll *avahi_poll_api;
  AvahiEntryGroup *avahi_group;
};

#define GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_TYPE, GravitonAvahiServerPublishMethodPrivate))

static void graviton_avahi_server_publish_method_class_init (GravitonAvahiServerPublishMethodClass *klass);
static void graviton_avahi_server_publish_method_init       (GravitonAvahiServerPublishMethod *self);
static void graviton_avahi_server_publish_method_dispose    (GObject *object);
static void graviton_avahi_server_publish_method_finalize   (GObject *object);
static void graviton_avahi_server_publish_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_avahi_server_publish_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonAvahiServerPublishMethod, graviton_avahi_server_publish_method, GRAVITON_SERVER_PUBLISH_METHOD_TYPE);

GRAVITON_DEFINE_PUBLISH_PLUGIN (GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_TYPE)

enum {
  PROP_ZERO,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
cb_avahi_group (AvahiEntryGroup *g, AvahiEntryGroupState state, gpointer data)
{
}

static void
create_avahi_services (AvahiClient *client, GravitonAvahiServerPublishMethod *self)
{
  if (!self->priv->avahi_group)
    self->priv->avahi_group = avahi_entry_group_new (client,
                                                       cb_avahi_group,
                                                       NULL);

  avahi_entry_group_reset (self->priv->avahi_group);

  GravitonServer *server = graviton_server_publish_method_get_server (GRAVITON_SERVER_PUBLISH_METHOD (self));
  int port = graviton_server_get_port (server);
  const gchar *node_id = graviton_server_get_node_id (server);
  g_object_unref (server);

  avahi_entry_group_add_service (self->priv->avahi_group,
                                 AVAHI_IF_UNSPEC,
                                 AVAHI_PROTO_INET,
                                 0,
                                 node_id,
                                 "_graviton._tcp",
                                 NULL,
                                 NULL,
                                 port,
                                 NULL);
  avahi_entry_group_commit (self->priv->avahi_group);
  g_debug ("Created avahi services for port %d", port);
}

static void
cb_avahi (AvahiClient *client, AvahiClientState state, gpointer data)
{
  if (state == AVAHI_CLIENT_S_RUNNING) {
    create_avahi_services (client, GRAVITON_AVAHI_SERVER_PUBLISH_METHOD(data));
  }
}


static void
start_publish (GravitonServerPublishMethod *method)
{
  GravitonAvahiServerPublishMethod *self = GRAVITON_AVAHI_SERVER_PUBLISH_METHOD (method);
  const AvahiPoll *poll_api = avahi_glib_poll_get (self->priv->avahi_poll_api);
  self->priv->avahi = avahi_client_new (poll_api,
                                  0,
                                  cb_avahi,
                                  self,
                                  NULL);
}

static void
stop_publish (GravitonServerPublishMethod *method)
{
  //FIXME: Stop and delete avahi client
}

static void
graviton_avahi_server_publish_method_class_init (GravitonAvahiServerPublishMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonAvahiServerPublishMethodPrivate));

  object_class->dispose = graviton_avahi_server_publish_method_dispose;
  object_class->finalize = graviton_avahi_server_publish_method_finalize;
  object_class->set_property =  graviton_avahi_server_publish_method_set_property;
  object_class->get_property =  graviton_avahi_server_publish_method_get_property;

  GravitonServerPublishMethodClass *method_class = GRAVITON_SERVER_PUBLISH_METHOD_CLASS (
      klass);
  method_class->start = start_publish;
  method_class->stop = stop_publish;
}

static void
graviton_avahi_server_publish_method_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonAvahiServerPublishMethod *self = GRAVITON_AVAHI_SERVER_PUBLISH_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_avahi_server_publish_method_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonAvahiServerPublishMethod *self = GRAVITON_AVAHI_SERVER_PUBLISH_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_avahi_server_publish_method_init (GravitonAvahiServerPublishMethod *self)
{
  GravitonAvahiServerPublishMethodPrivate *priv;
  priv = self->priv = GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_GET_PRIVATE (self);

  priv->avahi_group = NULL;

  priv->avahi_poll_api = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
}

static void
graviton_avahi_server_publish_method_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_avahi_server_publish_method_parent_class)->dispose (object);
}

static void
graviton_avahi_server_publish_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_avahi_server_publish_method_parent_class)->finalize (object);
}
