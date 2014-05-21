#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "node-transport.h"

typedef struct _GravitonNodeTransportPrivate GravitonNodeTransportPrivate;

struct _GravitonNodeTransportPrivate
{
  int foo;
};

#define GRAVITON_NODE_TRANSPORT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_TRANSPORT_TYPE, GravitonNodeTransportPrivate))

static void graviton_node_transport_class_init (GravitonNodeTransportClass *klass);
static void graviton_node_transport_init       (GravitonNodeTransport *self);
static void graviton_node_transport_dispose    (GObject *object);
static void graviton_node_transport_finalize   (GObject *object);
static void graviton_node_transport_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_node_transport_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonNodeTransport, graviton_node_transport, G_TYPE_OBJECT);

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
graviton_node_transport_class_init (GravitonNodeTransportClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNodeTransportPrivate));

  object_class->dispose = graviton_node_transport_dispose;
  object_class->finalize = graviton_node_transport_finalize;
  object_class->set_property =  graviton_node_transport_set_property;
  object_class->get_property =  graviton_node_transport_get_property;
  /*g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);*/
}

static void
graviton_node_transport_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonNodeTransport *self = GRAVITON_NODE_TRANSPORT (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_node_transport_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonNodeTransport *self = GRAVITON_NODE_TRANSPORT (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_node_transport_init (GravitonNodeTransport *self)
{
  GravitonNodeTransportPrivate *priv;
  priv = self->priv = GRAVITON_NODE_TRANSPORT_GET_PRIVATE (self);
}

static void
graviton_node_transport_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_transport_parent_class)->dispose (object);
}

static void
graviton_node_transport_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_transport_parent_class)->finalize (object);
}

GIOStream *
graviton_node_transport_open_stream (GravitonNodeTransport *self,
                                     GravitonNode *node,
                                     const gchar *name,
                                     GHashTable *args,
                                     GError **error)
{
  GravitonNodeTransportClass *klass = GRAVITON_NODE_TRANSPORT_GET_CLASS (self);
  return klass->open_stream (self, node, name, args, error);
}

GVariant *
graviton_node_transport_call_args (GravitonNodeTransport *self,
                                   GravitonNode *node,
                                   const gchar *method,
                                   GHashTable *args,
                                   GError **error)
{
  GravitonNodeTransportClass *klass = GRAVITON_NODE_TRANSPORT_GET_CLASS (self);
  return klass->call_args (self, node, method, args, error);
}
