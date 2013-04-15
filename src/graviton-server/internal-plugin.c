#include "config.h"
#include "internal-plugin.h"
#include "server.h"
#include <json-glib/json-glib.h>

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

static JsonNode *
cb_root(GravitonPlugin *self, const gchar *path, gpointer user_data)
{
  JsonNode *node;
  JsonBuilder *builder = json_builder_new ();
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "version");
  json_builder_add_string_value (builder, GRAVITON_VERSION);
  json_builder_end_object (builder);

  node = json_builder_get_root (builder);
  g_object_unref (builder);
  return node;
}

static GVariant *
cb_controls(GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  gchar *plugin_name;
  GravitonInternalPlugin *self = GRAVITON_INTERNAL_PLUGIN (user_data);
  GravitonPluginManager *plugins = graviton_server_get_plugin_manager (self->priv->server);
  plugin_name = g_strdup (g_variant_get_string (g_hash_table_lookup (args, "plugin"), NULL));
  GravitonPlugin *plugin = graviton_plugin_manager_mounted_plugin (plugins, plugin_name);
  g_object_unref (plugins);

  if (plugin) {
    GVariantBuilder ret;
    GList *controls = graviton_plugin_list_controls (plugin);
    g_variant_builder_init (&ret, G_VARIANT_TYPE_ARRAY);

    GList *cur = controls;
    while (cur) { 
      g_variant_builder_add (&ret, "s", cur->data);
      cur = g_list_next (cur);
    }

    g_object_unref (plugin);
    g_free (plugin_name);

    return g_variant_builder_end (&ret);
  } else {
    g_set_error (error,
                 GRAVITON_INTROSPECTION_ERROR,
                 GRAVITON_INTROSPECTION_ERROR_NO_SUCH_PLUGIN,
                 "No such plugin: %s", plugin_name);
    g_free (plugin_name);
    return NULL;
  }
}

static GVariant *
cb_methods(GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GravitonInternalPlugin *self = GRAVITON_INTERNAL_PLUGIN (user_data);
  GravitonPluginManager *plugins = graviton_server_get_plugin_manager (self->priv->server);
  gchar *plugin_name, *control_name;
  plugin_name = g_strdup (g_variant_get_string (g_hash_table_lookup (args, "plugin"), NULL));
  control_name = g_strdup (g_variant_get_string (g_hash_table_lookup (args, "control"), NULL));
  GravitonPlugin *plugin = graviton_plugin_manager_mounted_plugin (plugins, plugin_name);
  g_object_unref (plugins);

  if (plugin) {
    GravitonControl *control = graviton_plugin_get_control (plugin, control_name);
    g_object_unref (plugin);
    if (control) {
      GVariantBuilder ret;
      GList *methods = graviton_control_list_methods (control);
      g_variant_builder_init (&ret, G_VARIANT_TYPE_ARRAY);

      GList *cur = methods;
      while (cur) { 
        g_variant_builder_add (&ret, "s", cur->data);
        cur = g_list_next (cur);
      }

      g_object_unref (control);
      g_free (plugin_name);
      g_free (control_name);

      return g_variant_builder_end (&ret);
    } else {
      g_set_error (error,
                   GRAVITON_INTROSPECTION_ERROR,
                   GRAVITON_INTROSPECTION_ERROR_NO_SUCH_CONTROL,
                   "No such control %s on plugin %s", control_name, plugin_name);
      g_free (plugin_name);
      g_free (control_name);
    }
  } else {
    g_set_error (error,
                 GRAVITON_INTROSPECTION_ERROR,
                 GRAVITON_INTROSPECTION_ERROR_NO_SUCH_PLUGIN,
                 "No such plugin: %s", plugin_name);
    g_free (plugin_name);
    return NULL;
  }
  return NULL;
}

static GVariant *
cb_plugins(GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonInternalPlugin *self = GRAVITON_INTERNAL_PLUGIN (user_data);
  g_variant_builder_init (&ret, G_VARIANT_TYPE_ARRAY);
  GravitonPluginManager *plugins = graviton_server_get_plugin_manager (self->priv->server);
  GList *pluginList = graviton_plugin_manager_list_plugins (plugins);
  GList *cur = pluginList;
  while (cur) {
    g_variant_builder_add (&ret, "s", cur->data);
    g_debug ("Found plugin %s", cur->data);
    cur = g_list_next (cur);
  }

  g_object_unref (plugins);

  return g_variant_builder_end (&ret);
}

static void
graviton_internal_plugin_init (GravitonInternalPlugin *self)
{
  GravitonInternalPluginPrivate *priv;
  self->priv = priv = GRAVITON_INTERNAL_PLUGIN_GET_PRIVATE (self);
  GravitonControl *introspection = g_object_new (GRAVITON_TYPE_CONTROL, NULL);
  graviton_control_add_method (introspection,
                               "listPlugins",
                               cb_plugins,
                               NULL,
                               0,
                               self);
  graviton_control_add_method (introspection,
                               "listControls",
                               cb_controls,
                               NULL,
                               0,
                               self);
  graviton_control_add_method (introspection,
                               "listMethods",
                               cb_methods,
                               NULL,
                               0,
                               self);
  graviton_plugin_register_control (GRAVITON_PLUGIN (self), "introspection", introspection);
}

  /*if (path_elements[2] == NULL) {
  }

  GravitonControl *control = graviton_plugin_get_control (plugin, path_elements[2]);
  g_debug ("Requested control at %s", path_elements[2]);

  if (!control) {
    goto out;
  }*/
