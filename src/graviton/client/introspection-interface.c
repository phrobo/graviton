#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "introspection-interface.h"
#include "service-interface.h"
#include "node.h"

typedef struct _GravitonIntrospectionControlPrivate GravitonIntrospectionControlPrivate;

struct _GravitonIntrospectionControlPrivate
{
  gchar *target;
};

#define GRAVITON_INTROSPECTION_INTERFACE_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_INTROSPECTION_INTERFACE_TYPE, GravitonIntrospectionControlPrivate))

static void graviton_introspection_interface_class_init (GravitonIntrospectionControlClass *klass);
static void graviton_introspection_interface_init       (GravitonIntrospectionControl *self);
static void graviton_introspection_interface_dispose    (GObject *object);
static void graviton_introspection_interface_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonIntrospectionControl, graviton_introspection_interface, GRAVITON_SERVICE_INTERFACE_TYPE);

enum {
  PROP_0,
  PROP_TARGET,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
set_property (GObject *object,
                     guint property_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  GravitonIntrospectionControl *self = GRAVITON_INTROSPECTION_INTERFACE (object);
  switch (property_id) {
    case PROP_TARGET:
      if (self->priv->target)
        g_free (self->priv->target);
      self->priv->target = g_value_dup_string (value);
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
  GravitonIntrospectionControl *self = GRAVITON_INTROSPECTION_INTERFACE (object);
  switch (property_id) {
    case PROP_TARGET:
      g_value_set_string (value, self->priv->target);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_introspection_interface_class_init (GravitonIntrospectionControlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonIntrospectionControlPrivate));

  object_class->dispose = graviton_introspection_interface_dispose;
  object_class->finalize = graviton_introspection_interface_finalize;

  object_class->set_property = set_property;
  object_class->get_property = get_property;

  obj_properties[PROP_TARGET] =
    g_param_spec_string ("target",
                         "Target service",
                         "Target service",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
graviton_introspection_interface_init (GravitonIntrospectionControl *self)
{
  GravitonIntrospectionControlPrivate *priv;
  priv = self->priv = GRAVITON_INTROSPECTION_INTERFACE_GET_PRIVATE (self);
}

static void
graviton_introspection_interface_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_introspection_interface_parent_class)->dispose (object);
}

static void
graviton_introspection_interface_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_introspection_interface_parent_class)->finalize (object);
}

GravitonIntrospectionControl *
graviton_introspection_interface_new_from_interface (GravitonServiceInterface *service)
{
  return graviton_introspection_interface_new (graviton_service_interface_get_node (service), graviton_service_interface_get_name (service));
}

GravitonIntrospectionControl *
graviton_introspection_interface_new (GravitonNode *node, const gchar *name)
{
  return g_object_new (GRAVITON_INTROSPECTION_INTERFACE_TYPE, "node", node, "name", "net:phrobo:graviton/introspection", "target", name, NULL);
}

static GList *
call_string_list_method (GravitonIntrospectionControl *self, const gchar *method, GError **err, ...)
{
  va_list args;
  va_start (args, err);

  GVariant *result = graviton_node_call_va (graviton_service_interface_get_node (GRAVITON_SERVICE_INTERFACE (self)),
                                            method,
                                            err,
                                            args);
  GList *ret = NULL;
  int i;
  if (result) {
    for (i = 0;i < g_variant_n_children (result);i++) {
      GVariant *idx = g_variant_get_child_value (result, i);
      GVariant *strIdx = g_variant_get_variant (idx);
      ret = g_list_prepend (ret, g_variant_dup_string (strIdx, NULL));
    }
    g_variant_unref (result);
  }
  return ret;
}

GList *graviton_introspection_interface_list_interfaces (GravitonIntrospectionControl *self, GError **err)
{
  GError *error = NULL;
  GList *ret = NULL;
  GVariant *name = NULL;
  if (self->priv->target)
    name = g_variant_new_string (self->priv->target);
  return call_string_list_method (self,
                                  "net:phrobo:graviton/introspection.listControls",
                                  err,
                                  "service",
                                  name,
                                  NULL);
}

GList *graviton_introspection_interface_list_properties (GravitonIntrospectionControl *self, GError **err)
{
  GError *error = NULL;
  GList *ret = NULL;
  GVariant *name = NULL;
  if (self->priv->target)
    name = g_variant_new_string (self->priv->target);
  return call_string_list_method (self,
                                  "net:phrobo:graviton/introspection.listProperties",
                                  err,
                                  "service",
                                  name,
                                  NULL);
}

GList *graviton_introspection_interface_list_streams (GravitonIntrospectionControl *self, GError **err)
{
  GError *error = NULL;
  GList *ret = NULL;
  GVariant *name = NULL;
  if (self->priv->target)
    name = g_variant_new_string (self->priv->target);
  return call_string_list_method (self,
                                  "net:phrobo:graviton/introspection.listStreams",
                                  err,
                                  "service",
                                  name,
                                  NULL);
}
