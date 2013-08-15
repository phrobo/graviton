#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

typedef struct _GravitonNeighborDiscoveryMethodPrivate GravitonNeighborDiscoveryMethodPrivate;

struct _GravitonNeighborDiscoveryMethodPrivate
{
};

#define GRAVITON_NEIGHBOR_DISCOVERY_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE, GravitonNeighborDiscoveryMethodPrivate))

static void graviton_neighbor_discovery_method_class_init (GravitonNeighborDiscoveryMethodClass *klass);
static void graviton_neighbor_discovery_method_init       (GravitonNeighborDiscoveryMethod *self);
static void graviton_neighbor_discovery_method_dispose    (GObject *object);
static void graviton_neighbor_discovery_method_finalize   (GObject *object);
static void graviton_neighbor_discovery_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_neighbor_discovery_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonNeighborDiscoveryMethod, graviton_neighbor_discovery_method, GRAVITON_DISCOVERY_METHOD_TYPE);

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
graviton_neighbor_discovery_method_class_init (GravitonNeighborDiscoveryMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNeighborDiscoveryMethodPrivate));

  object_class->dispose = graviton_neighbor_discovery_method_dispose;
  object_class->finalize = graviton_neighbor_discovery_method_finalize;
  object_class->set_property =  graviton_neighbor_discovery_method_set_property;
  object_class->get_property =  graviton_neighbor_discovery_method_get_property;
  g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);
}

static void
graviton_neighbor_discovery_method_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonNeighborDiscoveryMethod *self = GRAVITON_NEIGHBOR_DISCOVERY_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_neighbor_discovery_method_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonNeighborDiscoveryMethod *self = GRAVITON_NEIGHBOR_DISCOVERY_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_neighbor_discovery_method_init (GravitonNeighborDiscoveryMethod *self)
{
  GravitonNeighborDiscoveryMethodPrivate *priv;
  priv = self->priv = GRAVITON_NEIGHBOR_DISCOVERY_METHOD_GET_PRIVATE (self);
}

static void
graviton_neighbor_discovery_method_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_neighbor_discovery_method_parent_class)->dispose (object);
}

static void
graviton_neighbor_discovery_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_neighbor_discovery_method_parent_class)->finalize (object);
}

static void
start_browse (GravitonDiscoveryMethod *method)
{
  
}

static void
stop_browse (GravitonDiscoveryMethod *method)
{

}
