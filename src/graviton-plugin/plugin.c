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
  GHashTable *handlers;
  GHashTable *handler_data;
  GravitonPluginPathHandler nullHandler;
  gpointer nullHandlerData; 
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

static JsonNode*
handle_get(GravitonPlugin *self, const gchar *path, GHashTable *args)
{
  GravitonPluginPathHandler handler = NULL;
  gpointer data;
  gchar **path_elements = g_strsplit(path, "/", 0);
  int element_count = g_strv_length (path_elements);
  g_debug ("handling %s", path);
  while (handler == NULL && element_count > 0) {
    gchar *subpath;
    subpath = g_strjoinv ("/", path_elements);
    g_debug ("Searching for %s", subpath);
    handler = g_hash_table_lookup (self->priv->handlers, subpath);
    data = g_hash_table_lookup (self->priv->handler_data, subpath);
    g_free (subpath);
    element_count--;
    g_free (path_elements[element_count]);
    path_elements[element_count] = NULL;
  }
  g_free (path_elements);
  if (!handler) {
    handler = self->priv->nullHandler;
    data = self->priv->nullHandlerData;
  }
  if (handler) {
    return handler(self, path, data);
  } else {
    g_message ("No handler defined for %s", path);
    return NULL;
  }
}

void
graviton_plugin_register_handler(GravitonPlugin *self, const gchar *path, GravitonPluginPathHandler handler, gpointer user_data)
{
  if (path) {
    g_hash_table_replace (self->priv->handlers, g_strdup (path), handler);
    g_hash_table_replace (self->priv->handler_data, g_strdup (path), user_data);
  } else {
    self->priv->nullHandler = handler;
    self->priv->nullHandlerData = user_data;
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
  klass->handle_get = handle_get;
}

static void
graviton_plugin_init (GravitonPlugin *self)
{
  GravitonPluginPrivate *priv;
  self->priv = priv = GRAVITON_PLUGIN_GET_PRIVATE (self);

  priv->mount = 0;
  priv->handlers = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          NULL);
  priv->handler_data = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          NULL);
}

const gchar *graviton_plugin_get_name (GravitonPlugin *self)
{
  return "Plugin";
}
