#ifndef __GRAVITON_INTROSPECTION_INTERFACE_H__
#define __GRAVITON_INTROSPECTION_INTERFACE_H__

#include <glib.h>
#include <glib-object.h>
#include "service-interface.h"

G_BEGIN_DECLS

#define GRAVITON_INTROSPECTION_INTERFACE_TYPE            (graviton_introspection_interface_get_type ())
#define GRAVITON_INTROSPECTION_INTERFACE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_INTROSPECTION_INTERFACE_TYPE, GravitonIntrospectionControl))
#define GRAVITON_INTROSPECTION_INTERFACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_INTROSPECTION_INTERFACE_TYPE, GravitonIntrospectionControlClass))
#define IS_GRAVITON_INTROSPECTION_INTERFACE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_INTROSPECTION_INTERFACE_TYPE))
#define IS_GRAVITON_INTROSPECTION_INTERFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_INTROSPECTION_INTERFACE_TYPE))
#define GRAVITON_INTROSPECTION_INTERFACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_INTROSPECTION_INTERFACE_TYPE, GravitonIntrospectionControlClass))

typedef struct _GravitonIntrospectionControlPrivate GravitonIntrospectionControlPrivate;

typedef struct _GravitonIntrospectionControl      GravitonIntrospectionControl;
typedef struct _GravitonIntrospectionControlClass GravitonIntrospectionControlClass;

struct _GravitonIntrospectionControlClass
{
  GravitonServiceInterfaceClass parent_class;
};

struct _GravitonIntrospectionControl
{
  GravitonServiceInterface parent;
  GravitonIntrospectionControlPrivate *priv;
};

GType graviton_introspection_interface_get_type (void);

GList *graviton_introspection_interface_list_interfaces (GravitonIntrospectionControl *self, GError **error);
GList *graviton_introspection_interface_list_properties (GravitonIntrospectionControl *self, GError **error);
GList *graviton_introspection_interface_list_streams (GravitonIntrospectionControl *self, GError **error);
GravitonIntrospectionControl *graviton_introspection_interface_new_from_interface (GravitonServiceInterface *control);
GravitonIntrospectionControl *graviton_introspection_interface_new (GravitonNode *node, const gchar *name);

G_END_DECLS

#endif
