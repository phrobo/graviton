#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "node.h"

#include "discovery-method.h"
#include "cloud.h"

typedef struct _GravitonDiscoveryMethodPrivate GravitonDiscoveryMethodPrivate;

struct _GravitonDiscoveryMethodPrivate
{
  GravitonNodeBrowser *browser;
  GList *discovered_nodes;
};

#define GRAVITON_DISCOVERY_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_DISCOVERY_METHOD_TYPE, GravitonDiscoveryMethodPrivate))

static void graviton_discovery_method_class_init (GravitonDiscoveryMethodClass *klass);
static void graviton_discovery_method_init       (GravitonDiscoveryMethod *self);
static void graviton_discovery_method_dispose    (GObject *object);
static void graviton_discovery_method_finalize   (GObject *object);
static void graviton_discovery_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_discovery_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonDiscoveryMethod, graviton_discovery_method, G_TYPE_OBJECT);

enum {
  PROP_ZERO,
  PROP_NODE_BROWSER,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  SIGNAL_NODE_FOUND,
  SIGNAL_NODE_LOST,
  SIGNAL_FINISHED,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
graviton_discovery_method_class_init (GravitonDiscoveryMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonDiscoveryMethodPrivate));

  object_class->dispose = graviton_discovery_method_dispose;
  object_class->finalize = graviton_discovery_method_finalize;
  object_class->set_property =  graviton_discovery_method_set_property;
  object_class->get_property =  graviton_discovery_method_get_property;

  obj_properties[PROP_NODE_BROWSER] =
    g_param_spec_object ("node-browser",
                         "node browser",
                         "GravitonNodeBrowser object",
                         GRAVITON_NODE_BROWSER_TYPE,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);

  obj_signals[SIGNAL_NODE_LOST] =
    g_signal_new ("node-lost",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GRAVITON_NODE_TYPE);
  obj_signals[SIGNAL_NODE_FOUND] =
    g_signal_new ("node-found",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GRAVITON_NODE_TYPE);
  obj_signals[SIGNAL_FINISHED] =
    g_signal_new ("finished",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0,
                  G_TYPE_NONE);
}

static void
graviton_discovery_method_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonDiscoveryMethod *self = GRAVITON_DISCOVERY_METHOD (object);
  switch (property_id) {
    case PROP_NODE_BROWSER:
      if (self->priv->browser)
        g_object_unref (self->priv->browser);
      self->priv->browser = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_discovery_method_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonDiscoveryMethod *self = GRAVITON_DISCOVERY_METHOD (object);
  switch (property_id) {
    case PROP_NODE_BROWSER:
      g_value_set_object (value, self->priv->browser);
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_discovery_method_init (GravitonDiscoveryMethod *self)
{
  GravitonDiscoveryMethodPrivate *priv;
  priv = self->priv = GRAVITON_DISCOVERY_METHOD_GET_PRIVATE (self);
}

static void
graviton_discovery_method_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_discovery_method_parent_class)->dispose (object);
  GravitonDiscoveryMethod *self = GRAVITON_DISCOVERY_METHOD (object);
  g_object_unref (self->priv->browser);
  self->priv->browser = NULL;
}

static void
graviton_discovery_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_discovery_method_parent_class)->finalize (object);
}

void
graviton_discovery_method_node_found (GravitonDiscoveryMethod *self,
                                      GravitonNode *node)
{
  self->priv->discovered_nodes = g_list_append (self->priv->discovered_nodes, node);
  g_debug ("Node found!");
  g_signal_emit (self, obj_signals[SIGNAL_NODE_FOUND], 0, node);
}

void
graviton_discovery_method_node_lost (GravitonDiscoveryMethod *self,
                                     GravitonNode *node)
{
  self->priv->discovered_nodes = g_list_remove (self->priv->discovered_nodes, node);
  g_debug ("Node lost!");
  g_signal_emit (self, obj_signals[SIGNAL_NODE_LOST], 0, node);
}

GList *
graviton_discovery_method_found_nodes (GravitonDiscoveryMethod *self)
{
  return self->priv->discovered_nodes;
}

void
graviton_discovery_method_finished (GravitonDiscoveryMethod *self)
{
  g_debug ("All nodes found!");
  g_signal_emit (self, obj_signals[SIGNAL_FINISHED], 0, NULL);
}

void
graviton_discovery_method_start (GravitonDiscoveryMethod *method)
{
  GRAVITON_DISCOVERY_METHOD_GET_CLASS (method)->start (method);
}

void
graviton_discovery_method_stop (GravitonDiscoveryMethod *method)
{
  GRAVITON_DISCOVERY_METHOD_GET_CLASS (method)->stop (method);
}

GravitonNodeBrowser *
graviton_discovery_method_get_browser (GravitonDiscoveryMethod *self)
{
  return g_object_ref (self->priv->browser);
}
