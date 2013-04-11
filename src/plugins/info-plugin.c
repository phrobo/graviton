#include "config.h"
#include "info-plugin.h"
#include <json-glib/json-glib.h>

#define GRAVITON_INFO_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_INFO_PLUGIN, GravitonInfoPluginPrivate))

GRAVITON_DEFINE_PLUGIN(GRAVITON_TYPE_INFO_PLUGIN, "/info")

G_DEFINE_TYPE (GravitonInfoPlugin, graviton_info_plugin, GRAVITON_TYPE_PLUGIN);

struct _GravitonInfoPluginPrivate
{
};

static void
graviton_info_plugin_class_init (GravitonInfoPluginClass *klass)
{
  //g_type_class_add_private (klass, sizeof (GravitonInfoPluginPrivate));

  GravitonPluginClass *plugin_class = GRAVITON_PLUGIN_CLASS (klass);
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

static void
graviton_info_plugin_init (GravitonInfoPlugin *self)
{
  GravitonInfoPluginPrivate *priv;
  //self->priv = priv = GRAVITON_INFO_PLUGIN_GET_PRIVATE (self);
  graviton_plugin_register_handler(GRAVITON_PLUGIN(self), NULL, cb_root, NULL);
}
