#include "plugin.h"
#include <graviton/control.h>

#define GRAVITON_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_PLUGIN, GravitonPluginPrivate))
G_DEFINE_TYPE (GravitonPlugin, graviton_plugin, GRAVITON_TYPE_CONTROL);

enum {
  PROP_0,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

struct _GravitonPluginPrivate
{
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
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

/*static GVariant*
handle_get(GravitonPlugin *self, const gchar *path, GHashTable *args)
{
  GravitonControl *control = NULL;
  gpointer data;
  gchar **path_elements = g_strsplit(path, "/", 0);
  int element_count = g_strv_length (path_elements);
  g_debug ("handling %s", path);
  while (control == NULL && element_count > 0) {
    gchar *subpath;
    subpath = g_strjoinv ("/", path_elements);
    g_debug ("Searching for %s", subpath);
    control = g_hash_table_lookup (self->priv->controls, subpath);
    g_free (subpath);
    element_count--;
    g_free (path_elements[element_count]);
    path_elements[element_count] = NULL;
  }
  g_free (path_elements);
  if (control) {
    //FIXME: Marshall args
    return graviton_control_call_method (control, path, args);
  } else {
    g_message ("No control defined for %s", path);
    return NULL;
  }
}*/

static void
graviton_plugin_class_init (GravitonPluginClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  //g_type_class_add_private (klass, sizeof (GravitonPluginPrivate));

  gobject_class->set_property = plugin_set_property;
  gobject_class->get_property = plugin_get_property;

  /*g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);*/
}

static void
graviton_plugin_init (GravitonPlugin *self)
{
  GravitonPluginPrivate *priv;
  //self->priv = priv = GRAVITON_PLUGIN_GET_PRIVATE (self);
}

