#ifndef GRAVITON_INTERNAL_PLUGIN_H
#define GRAVITON_INTERNAL_PLUGIN_H

#include <glib-object.h>
#include <graviton/plugin.h>

#define GRAVITON_TYPE_INTERNAL_PLUGIN     (graviton_internal_plugin_get_type ())
#define GRAVITON_INTERNAL_PLUGIN(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_INTERNAL_PLUGIN, GravitonInternalPlugin))
#define GRAVITON_IS_INTERNAL_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_TYPE_INTERNAL_PLUGIN))
#define GRAVITON_INTERNAL_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_TYPE_INTERNAL_PLUGIN, GravitonInternalPluginClass))
#define GRAVITON_IS_INTERNAL_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_TYPE_INTERNAL_PLUGIN))
#define GRAVITON_INTERNAL_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_TYPE_INTERNAL_PLUGIN, GravitonInternalPluginClass))

#define GRAVITON_INTROSPECTION_ERROR (graviton_introspection_error_quark ())

typedef enum {
  GRAVITON_INTROSPECTION_ERROR_NO_SUCH_PLUGIN,
  GRAVITON_INTROSPECTION_ERROR_NO_SUCH_CONTROL,
  GRAVITON_INTROSPECTION_ERROR_NO_SUCH_PROPERTY
} GravitonInspectionError;

typedef struct _GravitonInternalPlugin GravitonInternalPlugin;
typedef struct _GravitonInternalPluginClass GravitonInternalPluginClass;

typedef struct _GravitonInternalPluginPrivate GravitonInternalPluginPrivate;

struct _GravitonInternalPlugin
{
  GravitonPlugin parent_instance;
  GravitonInternalPluginPrivate *priv;
};

struct _GravitonInternalPluginClass
{
  GravitonPluginClass parent_class;
};

GType graviton_internal_plugin_get_type ();

#endif // GRAVITON_INTERNAL_PLUGIN_H
