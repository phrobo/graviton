#ifndef __GRAVITON_INTROSPECTION_CONTROL_H__
#define __GRAVITON_INTROSPECTION_CONTROL_H__

#include <glib.h>
#include <glib-object.h>
#include "node-control.h"

G_BEGIN_DECLS

#define GRAVITON_INTROSPECTION_CONTROL_TYPE            (graviton_introspection_control_get_type ())
#define GRAVITON_INTROSPECTION_CONTROL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_INTROSPECTION_CONTROL_TYPE, GravitonIntrospectionControl))
#define GRAVITON_INTROSPECTION_CONTROL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_INTROSPECTION_CONTROL_TYPE, GravitonIntrospectionControlClass))
#define IS_GRAVITON_INTROSPECTION_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_INTROSPECTION_CONTROL_TYPE))
#define IS_GRAVITON_INTROSPECTION_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_INTROSPECTION_CONTROL_TYPE))
#define GRAVITON_INTROSPECTION_CONTROL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_INTROSPECTION_CONTROL_TYPE, GravitonIntrospectionControlClass))

typedef struct _GravitonIntrospectionControlPrivate GravitonIntrospectionControlPrivate;

typedef struct _GravitonIntrospectionControl      GravitonIntrospectionControl;
typedef struct _GravitonIntrospectionControlClass GravitonIntrospectionControlClass;

struct _GravitonIntrospectionControlClass
{
  GravitonNodeControlClass parent_class;
};

struct _GravitonIntrospectionControl
{
  GravitonNodeControl parent;
  GravitonIntrospectionControlPrivate *priv;
};

GType graviton_introspection_control_get_type (void);

GList *graviton_introspection_control_list_controls (GravitonIntrospectionControl *self, GError **error);
GList *graviton_introspection_control_list_properties (GravitonIntrospectionControl *self, GError **error);
GravitonIntrospectionControl *graviton_introspection_control_new_from_control (GravitonNodeControl *control);
GravitonIntrospectionControl *graviton_introspection_control_new (GravitonNode *node, const gchar *name);

G_END_DECLS

#endif
