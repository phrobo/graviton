#include "config.h"
#include "internal-plugin.h"
#include "server.h"
#include <json-glib/json-glib.h>
#include <graviton/control.h>

#define GRAVITON_INTERNAL_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_INTERNAL_PLUGIN, GravitonInternalPluginPrivate))

//GRAVITON_DEFINE_PLUGIN(GRAVITON_TYPE_INTERNAL_PLUGIN, "graviton")

GQuark
graviton_introspection_error_quark()
{
  return g_quark_from_static_string ("graviton-introspection-error-quark");
}

G_DEFINE_TYPE (GravitonInternalPlugin, graviton_internal_plugin, GRAVITON_TYPE_PLUGIN);

enum
{
  PROP_0,
  PROP_SERVER,
  N_PROPERTIES
};

struct _GravitonInternalPluginPrivate
{
  GravitonServer *server;
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static GravitonControl *
grab_control_arg (GravitonInternalPlugin *self, GHashTable *args, GError **error)
{
  GravitonControl *subcontrol;
  const gchar *control_name = NULL;
  GravitonPluginManager *plugins = graviton_server_get_plugin_manager (self->priv->server);

  if (g_hash_table_lookup (args, "control")) {
    control_name = g_variant_get_string (g_hash_table_lookup (args, "control"), NULL);
  }

  if (control_name) {
    g_debug ("Querying control %s", control_name);
    subcontrol = graviton_control_get_subcontrol (GRAVITON_CONTROL (plugins), control_name);
    g_object_unref (plugins);
  } else {
    g_debug ("Querying plugin manager");
    subcontrol = GRAVITON_CONTROL (plugins);
  }

  if (!subcontrol) {
    g_set_error (error,
                 GRAVITON_INTROSPECTION_ERROR,
                 GRAVITON_INTROSPECTION_ERROR_NO_SUCH_CONTROL,
                 "No such control %s", control_name);
  }

  return subcontrol;
}


static void
set_property (GObject *object,
              guint property_id,
              const GValue *value,
              GParamSpec *pspec)
{
  GravitonInternalPlugin *self = GRAVITON_INTERNAL_PLUGIN (object);

  switch (property_id) {
    case PROP_SERVER:
      self->priv->server = g_value_get_object (value);
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
  GravitonInternalPlugin *self = GRAVITON_INTERNAL_PLUGIN (object);
  
  switch (property_id) {
    case PROP_SERVER:
      g_value_set_object (value, self->priv->server);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_internal_plugin_class_init (GravitonInternalPluginClass *klass)
{
  g_type_class_add_private (klass, sizeof (GravitonInternalPluginPrivate));

  GravitonPluginClass *plugin_class = GRAVITON_PLUGIN_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;

  obj_properties[PROP_SERVER] =
    g_param_spec_object ("server",
                         "Server",
                         "GravitonServer object",
                         GRAVITON_TYPE_SERVER,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static GVariant *
cb_properties(GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonControl *subcontrol;
  GravitonInternalPlugin *self = GRAVITON_INTERNAL_PLUGIN (user_data);

  subcontrol = grab_control_arg (self, args, error);
  
  if (!subcontrol)
    return NULL;

  GParamSpec **properties;
  int property_count;
  int i;
  properties = g_object_class_list_properties (G_OBJECT_GET_CLASS (subcontrol), &property_count);
  g_variant_builder_init (&ret, G_VARIANT_TYPE_STRING_ARRAY);
  for (i = 0; i<property_count; i++) {
    g_variant_builder_add (&ret, "s", properties[i]->name);
  }

  g_object_unref (subcontrol);

  return g_variant_builder_end (&ret);
}

static GVariant *
cb_get_property (GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonControl *subcontrol;
  GravitonInternalPlugin *self = GRAVITON_INTERNAL_PLUGIN (user_data);

  subcontrol = grab_control_arg (self, args, error);

  if (!subcontrol)
    return NULL;

  const gchar *property_name;
  property_name = g_variant_get_string (g_hash_table_lookup (args, "property"), NULL);
  GParamSpec *property = g_object_class_find_property (G_OBJECT_GET_CLASS (subcontrol), property_name);
  g_assert (property);
  if (property) {
    GValue property_value = G_VALUE_INIT;
    GVariant *converted_variant = NULL;
    g_value_init (&property_value, property->value_type);
    g_object_get_property (G_OBJECT(subcontrol), property->name, &property_value);
    if (G_VALUE_HOLDS_STRING (&property_value))
      converted_variant = g_variant_new_string (g_value_get_string (&property_value));
    return converted_variant;
  } else {
    g_set_error (error,
                 GRAVITON_INTROSPECTION_ERROR,
                 GRAVITON_INTROSPECTION_ERROR_NO_SUCH_PROPERTY,
                 "No such property %s", property_name);
  }
  return NULL;
}

static GVariant *
cb_controls(GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonControl *subcontrol;
  GravitonInternalPlugin *self = GRAVITON_INTERNAL_PLUGIN (user_data);
  GravitonPluginManager *plugins = graviton_server_get_plugin_manager (self->priv->server);

  subcontrol = grab_control_arg (self, args, error);

  if (!subcontrol) {
    return NULL;
  }

  GList *controls = graviton_control_list_subcontrols (subcontrol);
  g_variant_builder_init (&ret, G_VARIANT_TYPE_STRING_ARRAY);
  GList *cur = controls;
  while (cur) { 
    g_debug ("Found control %s", cur->data);
    g_variant_builder_add (&ret, "s", cur->data);
    cur = g_list_next (cur);
  }

  g_object_unref (subcontrol);

  return g_variant_builder_end (&ret);
}

static GVariant *
cb_methods(GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonControl *subcontrol;
  GravitonInternalPlugin *self = GRAVITON_INTERNAL_PLUGIN (user_data);

  subcontrol = grab_control_arg (self, args, error);

  if (!subcontrol)
    return NULL;

  GList *methods = graviton_control_list_methods (subcontrol);
  g_variant_builder_init (&ret, G_VARIANT_TYPE_STRING_ARRAY);

  GList *cur = methods;
  while (cur) { 
    g_variant_builder_add (&ret, "s", cur->data);
    cur = g_list_next (cur);
  }

  g_object_unref (subcontrol);

  return g_variant_builder_end (&ret);
}

static void
graviton_internal_plugin_init (GravitonInternalPlugin *self)
{
  GravitonInternalPluginPrivate *priv;
  self->priv = priv = GRAVITON_INTERNAL_PLUGIN_GET_PRIVATE (self);
  GravitonControl *introspection = g_object_new (GRAVITON_TYPE_CONTROL, "name", "introspection", NULL);
  graviton_control_add_method (introspection,
                               "listControls",
                               cb_controls,
                               0,
                               NULL,
                               self,
                               NULL);
  graviton_control_add_method (introspection,
                               "listMethods",
                               cb_methods,
                               0,
                               NULL,
                               self,
                               NULL);
  graviton_control_add_method (introspection,
                               "listProperties",
                               cb_properties,
                               0,
                               NULL,
                               self,
                               NULL);
  graviton_control_add_method (introspection,
                               "getProperty",
                               cb_get_property,
                               0,
                               NULL,
                               self,
                               NULL);
  graviton_control_add_subcontrol (GRAVITON_CONTROL (self), introspection);
}
