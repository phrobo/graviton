#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dbus-publish.h"
#include "server-interface.h"
#include <graviton/server/server.h>

typedef struct _GravitonDbusServerPublishMethodPrivate GravitonDbusServerPublishMethodPrivate;

struct _GravitonDbusServerPublishMethodPrivate
{
  GravitonDBusServer *dbus;
};

#define GRAVITON_DBUS_SERVER_PUBLISH_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_DBUS_SERVER_PUBLISH_METHOD_TYPE, GravitonDbusServerPublishMethodPrivate))

static void graviton_dbus_server_publish_method_class_init (GravitonDbusServerPublishMethodClass *klass);
static void graviton_dbus_server_publish_method_init       (GravitonDbusServerPublishMethod *self);
static void graviton_dbus_server_publish_method_dispose    (GObject *object);
static void graviton_dbus_server_publish_method_finalize   (GObject *object);
static void graviton_dbus_server_publish_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_dbus_server_publish_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonDbusServerPublishMethod, graviton_dbus_server_publish_method, GRAVITON_SERVER_PUBLISH_METHOD_TYPE);

GRAVITON_DEFINE_PUBLISH_PLUGIN (GRAVITON_DBUS_SERVER_PUBLISH_METHOD_TYPE)

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
stop_publish (GravitonServerPublishMethod *method)
{
  //GravitonDbusServerPublishMethod *self = GRAVITON_DBUS_SERVER_PUBLISH_METHOD (method);
}

static void
start_publish (GravitonServerPublishMethod *method)
{
  GravitonDbusServerPublishMethod *self = GRAVITON_DBUS_SERVER_PUBLISH_METHOD (method);
  int port = graviton_server_get_port (graviton_server_publish_method_get_server (method));

  graviton_dbus_server_set_port (self->priv->dbus, port);

  GDBusConnection *connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
  g_dbus_interface_skeleton_export ((GDBusInterfaceSkeleton*)self->priv->dbus,
                                    connection,
                                    "/",
                                    NULL);
  gchar *bus_name = g_strdup_printf ("org.aether.graviton-%d", port);
  g_bus_own_name (G_BUS_TYPE_SESSION,
                  bus_name,
                  G_BUS_NAME_OWNER_FLAGS_NONE,
                  NULL,
                  NULL,
                  NULL,
                  NULL,
                  NULL);
  g_free (bus_name);
}

static void
graviton_dbus_server_publish_method_class_init (GravitonDbusServerPublishMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonDbusServerPublishMethodPrivate));

  object_class->dispose = graviton_dbus_server_publish_method_dispose;
  object_class->finalize = graviton_dbus_server_publish_method_finalize;
  object_class->set_property =  graviton_dbus_server_publish_method_set_property;
  object_class->get_property =  graviton_dbus_server_publish_method_get_property;

  GravitonServerPublishMethodClass *method_class = GRAVITON_SERVER_PUBLISH_METHOD_CLASS (
      klass);
  method_class->start = start_publish;
  method_class->stop = stop_publish;
}

static void
graviton_dbus_server_publish_method_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonDbusServerPublishMethod *self = GRAVITON_DBUS_SERVER_PUBLISH_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_dbus_server_publish_method_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonDbusServerPublishMethod *self = GRAVITON_DBUS_SERVER_PUBLISH_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_dbus_server_publish_method_init (GravitonDbusServerPublishMethod *self)
{
  GravitonDbusServerPublishMethodPrivate *priv;
  priv = self->priv = GRAVITON_DBUS_SERVER_PUBLISH_METHOD_GET_PRIVATE (self);
  priv->dbus = graviton_dbus_server_skeleton_new ();
}

static void
graviton_dbus_server_publish_method_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_dbus_server_publish_method_parent_class)->dispose (object);
}

static void
graviton_dbus_server_publish_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_dbus_server_publish_method_parent_class)->finalize (object);
}
