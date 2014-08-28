#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "server-publish-method.h"
#include "server.h"

typedef struct _GravitonServerPublishMethodPrivate GravitonServerPublishMethodPrivate;

struct _GravitonServerPublishMethodPrivate
{
  GravitonServer *server;
};

#define GRAVITON_SERVER_PUBLISH_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_SERVER_PUBLISH_METHOD_TYPE, GravitonServerPublishMethodPrivate))

static void graviton_server_publish_method_class_init (GravitonServerPublishMethodClass *klass);
static void graviton_server_publish_method_init       (GravitonServerPublishMethod *self);
static void graviton_server_publish_method_dispose    (GObject *object);
static void graviton_server_publish_method_finalize   (GObject *object);
static void graviton_server_publish_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_server_publish_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonServerPublishMethod, graviton_server_publish_method, G_TYPE_OBJECT);

enum {
  PROP_ZERO,
  PROP_SERVER,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  SIGNAL_PUBLISHED,
  SIGNAL_UNPUBLISHED,
  N_SIGNALS
};

//static int obj_signals[N_SIGNALS] = { 0, };

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
graviton_server_publish_method_class_init (GravitonServerPublishMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonServerPublishMethodPrivate));

  object_class->dispose = graviton_server_publish_method_dispose;
  object_class->finalize = graviton_server_publish_method_finalize;
  object_class->set_property =  graviton_server_publish_method_set_property;
  object_class->get_property =  graviton_server_publish_method_get_property;

  obj_properties[PROP_SERVER] =
    g_param_spec_object ("server",
                         "server",
                         "GravitonServer object",
                         GRAVITON_SERVER_TYPE,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);
}

static void
graviton_server_publish_method_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonServerPublishMethod *self = GRAVITON_SERVER_PUBLISH_METHOD (object);
  switch (property_id) {
    case PROP_SERVER:
      self->priv->server = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_server_publish_method_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonServerPublishMethod *self = GRAVITON_SERVER_PUBLISH_METHOD (object);
  switch (property_id) {
    case PROP_SERVER:
      g_value_set_object (value, self->priv->server);
      g_assert (self->priv->server != NULL);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_server_publish_method_init (GravitonServerPublishMethod *self)
{
  GravitonServerPublishMethodPrivate *priv;
  priv = self->priv = GRAVITON_SERVER_PUBLISH_METHOD_GET_PRIVATE (self);
  priv->server = NULL;
}

static void
graviton_server_publish_method_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_server_publish_method_parent_class)->dispose (object);
}

static void
graviton_server_publish_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_server_publish_method_parent_class)->finalize (object);
}

void
graviton_server_publish_method_start (GravitonServerPublishMethod *method)
{
  GRAVITON_SERVER_PUBLISH_METHOD_GET_CLASS (method)->start (method);
}

void
graviton_server_publish_method_stop (GravitonServerPublishMethod *method)
{
  GRAVITON_SERVER_PUBLISH_METHOD_GET_CLASS (method)->stop (method);
}

GravitonServer *
graviton_server_publish_method_get_server (GravitonServerPublishMethod *method)
{
  return g_object_ref (method->priv->server);
}
