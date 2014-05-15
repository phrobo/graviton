#ifndef GRAVITON_INTERNAL_PLUGIN_H
#define GRAVITON_INTERNAL_PLUGIN_H

#include <glib-object.h>
#include <graviton/control.h>

#define GRAVITON_INTERNAL_PLUGIN_TYPE     (graviton_internal_plugin_get_type ())
#define GRAVITON_INTERNAL_PLUGIN(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_INTERNAL_PLUGIN_TYPE, GravitonInternalPlugin))
#define GRAVITON_IS_INTERNAL_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_INTERNAL_PLUGIN_TYPE))
#define GRAVITON_INTERNAL_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_INTERNAL_PLUGIN_TYPE, GravitonInternalPluginClass))
#define GRAVITON_IS_INTERNAL_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_INTERNAL_PLUGIN_TYPE))
#define GRAVITON_INTERNAL_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_INTERNAL_PLUGIN_TYPE, GravitonInternalPluginClass))

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
  GravitonControl parent_instance;
  GravitonInternalPluginPrivate *priv;
};

struct _GravitonInternalPluginClass
{
  GravitonControlClass parent_class;
};

GType graviton_internal_plugin_get_type ();

#endif // GRAVITON_INTERNAL_PLUGIN_H
