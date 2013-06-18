#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "node-stream.h"
#include "node.h"

typedef struct _GravitonNodeStreamPrivate GravitonNodeStreamPrivate;

struct _GravitonNodeStreamPrivate
{
  gchar *name;
  GravitonNodeControl *control;
};

#define GRAVITON_NODE_STREAM_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_STREAM_TYPE, GravitonNodeStreamPrivate))

static void graviton_node_stream_class_init (GravitonNodeStreamClass *klass);
static void graviton_node_stream_init       (GravitonNodeStream *self);
static void graviton_node_stream_dispose    (GObject *object);
static void graviton_node_stream_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonNodeStream, graviton_node_stream, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_NAME,
  PROP_CONTROL,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
set_property (GObject *object,
                     guint property_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  GravitonNodeStream *self = GRAVITON_NODE_STREAM (object);
  switch (property_id) {
    case PROP_NAME:
      self->priv->name = g_value_dup_string (value);
      break;
    case PROP_CONTROL:
      self->priv->control = GRAVITON_NODE_CONTROL (g_value_dup_object (value));
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
  GravitonNodeStream *self = GRAVITON_NODE_STREAM (self);
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
graviton_node_stream_class_init (GravitonNodeStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNodeStreamPrivate));

  object_class->dispose = graviton_node_stream_dispose;
  object_class->finalize = graviton_node_stream_finalize;

  object_class->set_property = set_property;
  object_class->get_property = get_property;

  obj_properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "Name of this control",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );
  obj_properties [PROP_CONTROL] = 
    g_param_spec_object ("control",
                         "GravitonNodeControl",
                         "The underlying GravitonNodeControl",
                         GRAVITON_NODE_CONTROL_TYPE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );
  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
graviton_node_stream_init (GravitonNodeStream *self)
{
  GravitonNodeStream *priv;
  priv = self->priv = GRAVITON_NODE_STREAM_GET_PRIVATE (self);
}

static void
graviton_node_stream_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_stream_parent_class)->dispose (object);
}

static void
graviton_node_stream_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_stream_parent_class)->finalize (object);
}

GravitonNodeStream *
graviton_node_stream_new (GravitonNodeControl *control, const gchar *name)
{
  return g_object_new (GRAVITON_NODE_STREAM_TYPE, "control", control, "name", name);
}

GIOStream *
graviton_node_stream_open (GravitonNodeStream *self)
{
  gchar *full_name = g_strdup_printf ("%s.%s", graviton_node_control_get_name (self->priv->control), self->priv->name);
  GIOStream *ret = graviton_node_open_stream (graviton_node_control_get_node (self->priv->control), full_name);
  g_free (full_name);
  return ret;
}
