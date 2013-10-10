#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "service.h"
#include "node.h"
#include "node-stream.h"

typedef struct _GravitonServicePrivate GravitonServicePrivate;

struct _GravitonServicePrivate
{
  GravitonNode *node;
  gchar *name;
};

#define GRAVITON_SERVICE_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_SERVICE_TYPE, GravitonServicePrivate))

static void graviton_service_class_init (GravitonServiceClass *klass);
static void graviton_service_init       (GravitonService *self);
static void graviton_service_dispose    (GObject *object);
static void graviton_service_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonService, graviton_service, G_TYPE_OBJECT);

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
  GravitonService *self = GRAVITON_SERVICE (object);
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
  GravitonService *self = GRAVITON_SERVICE (self);
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
graviton_service_class_init (GravitonServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonServicePrivate));

  object_class->dispose = graviton_service_dispose;
  object_class->finalize = graviton_service_finalize;

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
graviton_service_init (GravitonService *self)
{
  GravitonServicePrivate *priv;
  self->priv = priv = GRAVITON_SERVICE_GET_PRIVATE (self);
  priv->node = NULL;
}

static void
graviton_service_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_service_parent_class)->dispose (object);
}

static void
graviton_service_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_service_parent_class)->finalize (object);
}

const gchar*
graviton_service_get_name (GravitonService *self)
{
  return self->priv->name;
}

GravitonService *
graviton_service_get_subcontrol (GravitonService *self, const gchar *name)
{
  gchar *full_name;
  GravitonService *node = self;
  if (self->priv->node) {
    node = g_object_ref (self->priv->node);
    full_name = g_strdup_printf ("%s/%s", self->priv->name, name);
  } else {
    full_name = g_strdup (name);
  }
  GravitonService *ret = g_object_new (GRAVITON_SERVICE_TYPE, "node", node, "name", full_name, NULL);
  g_free (full_name);
  return ret;
}

GravitonNode *
graviton_service_get_node (GravitonService *self)
{
  if (self->priv->node)
    return self->priv->node;
  return GRAVITON_NODE (self);
}

GVariant *
graviton_service_get_property (GravitonService *self, const gchar *property, GError **err)
{
  GError *error = NULL;
  GVariant *ret = graviton_node_call (graviton_service_get_node (self),
                                      "net:phrobo:graviton/introspection.getProperty",
                                      &error,
                                      "control",
                                      g_variant_new_string (graviton_service_get_name (self)),
                                      "property",
                                      g_variant_new_string (property),
                                      NULL);
  if (error) {
    g_propagate_error (err, error);
    return NULL;
  }

  return ret;
}

gchar *make_method_name (GravitonService *control, const gchar *method)
{
  return g_strdup_printf ("%s.%s", graviton_service_get_name (control), method);
}

GVariant *
graviton_service_call (GravitonService *control,
    const gchar *method,
    GError **error, ...)
{
  va_list args;
  va_start (args, error);
  gchar *full_method = make_method_name (control, method);
  GVariant *ret = graviton_node_call_va (graviton_service_get_node (control), full_method, error, args);
  va_end (args);
  return ret;
}

GravitonNodeStream *
graviton_service_get_stream (GravitonService *control, const gchar *name, GHashTable *args)
{
  GravitonNodeStream *stream = graviton_node_stream_new (control, name, args);
  return stream;
}