#include "plugin.h"

#define GRAVITON_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_PLUGIN, GravitonPluginPrivate))
G_DEFINE_TYPE (GravitonPlugin, graviton_plugin, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_NAME,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

struct _GravitonPluginPrivate
{
  gchar *mount;
};

static void
plugin_set_property (GObject *object,
                     guint property_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  GravitonPlugin *self = GRAVITON_PLUGIN (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
plugin_get_property (GObject *object,
                     guint property_id,
                     GValue *value,
                     GParamSpec *pspec)
{
  GravitonPlugin *self = GRAVITON_PLUGIN (object);
  switch (property_id) {
    case PROP_NAME:
      g_value_set_string (value, graviton_plugin_get_name (self));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}


static void
graviton_plugin_class_init (GravitonPluginClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  g_type_class_add_private (klass, sizeof (GravitonPluginPrivate));

  gobject_class->set_property = plugin_set_property;
  gobject_class->get_property = plugin_get_property;

  obj_properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Plugin name",
                         "Plugin name",
                         "",
                         G_PARAM_READWRITE);
  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
graviton_plugin_init (GravitonPlugin *self)
{
  GravitonPluginPrivate *priv;
  self->priv = priv = GRAVITON_PLUGIN_GET_PRIVATE (self);

  priv->mount = 0;
}

const gchar *graviton_plugin_get_name (GravitonPlugin *self)
{
  return "Plugin";
}
