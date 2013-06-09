#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "node-control.h"
#include "node.h"

typedef struct _GravitonNodeControlPrivate GravitonNodeControlPrivate;

struct _GravitonNodeControlPrivate
{
  GravitonNode *node;
  gchar *name;
};

#define GRAVITON_NODE_CONTROL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_CONTROL_TYPE, GravitonNodeControlPrivate))

static void graviton_node_control_class_init (GravitonNodeControlClass *klass);
static void graviton_node_control_init       (GravitonNodeControl *self);
static void graviton_node_control_dispose    (GObject *object);
static void graviton_node_control_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonNodeControl, graviton_node_control, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_NAME,
  PROP_NODE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
set_property (GObject *object,
                     guint property_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  GravitonNodeControl *self = GRAVITON_NODE_CONTROL (object);
  switch (property_id) {
    case PROP_NAME:
      self->priv->name = g_value_dup_string (value);
      break;
    case PROP_NODE:
      self->priv->node = GRAVITON_NODE (g_value_dup_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
get_property (GObject *object,
                     guint property_id,
                     GValue *value,
                     GParamSpec *pspec)
{
  GravitonNodeControl *self = GRAVITON_NODE_CONTROL (self);
  switch (property_id) {
    case PROP_NAME:
      g_value_set_string (value, self->priv->name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_node_control_class_init (GravitonNodeControlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNodeControlPrivate));

  object_class->dispose = graviton_node_control_dispose;
  object_class->finalize = graviton_node_control_finalize;

  object_class->set_property = set_property;
  object_class->get_property = get_property;

  obj_properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "Name of this control",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );
  obj_properties [PROP_NODE] = 
    g_param_spec_object ("node",
                         "Node",
                         "The underlying GravitonNode",
                         GRAVITON_NODE_TYPE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );
  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
graviton_node_control_init (GravitonNodeControl *self)
{
  GravitonNodeControlPrivate *priv;
  self->priv = priv = GRAVITON_NODE_CONTROL_GET_PRIVATE (self);
  priv->node = NULL;
}

static void
graviton_node_control_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_control_parent_class)->dispose (object);
}

static void
graviton_node_control_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_control_parent_class)->finalize (object);
}

const gchar*
graviton_node_control_get_name (GravitonNodeControl *self)
{
  return self->priv->name;
}

GravitonNodeControl *
graviton_node_control_get_subcontrol (GravitonNodeControl *self, const gchar *name)
{
  gchar *full_name;
  GravitonNodeControl *node = self;
  if (self->priv->node) {
    node = g_object_ref (self->priv->node);
    full_name = g_strdup_printf ("%s.%s", self->priv->name, name);
  } else {
    full_name = g_strdup (name);
  }
  GravitonNodeControl *ret = g_object_new (GRAVITON_NODE_CONTROL_TYPE, "node", node, "name", full_name, NULL);
  g_free (full_name);
  return ret;
}

GravitonNode *
graviton_node_control_get_node (GravitonNodeControl *self)
{
  if (self->priv->node)
    return self->priv->node;
  return GRAVITON_NODE (self);
}

GVariant *
graviton_node_control_get_property (GravitonNodeControl *self, const gchar *property, GError **err)
{
  GError *error = NULL;
  GVariant *ret = graviton_node_call (graviton_node_control_get_node (self),
                                      "graviton.introspection/getProperty",
                                      &error,
                                      "control",
                                      g_variant_new_string (graviton_node_control_get_name (self)),
                                      "property",
                                      g_variant_new_string (property),
                                      NULL);
  if (error) {
    g_propagate_error (err, error);
    return NULL;
  }

  return ret;
}

