#ifndef __GRAVITON_NODE_CONTROL_H__
#define __GRAVITON_NODE_CONTROL_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRAVITON_NODE_CONTROL_TYPE            (graviton_node_control_get_type ())
#define GRAVITON_NODE_CONTROL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NODE_CONTROL_TYPE, GravitonNodeControl))
#define GRAVITON_NODE_CONTROL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NODE_CONTROL_TYPE, GravitonNodeControlClass))
#define IS_GRAVITON_NODE_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NODE_CONTROL_TYPE))
#define IS_GRAVITON_NODE_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NODE_CONTROL_TYPE))
#define GRAVITON_NODE_CONTROL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NODE_CONTROL_TYPE, GravitonNodeControlClass))

typedef struct _GravitonNode GravitonNode;

typedef struct _GravitonNodeControl      GravitonNodeControl;
typedef struct _GravitonNodeControlClass GravitonNodeControlClass;

typedef struct _GravitonNodeControlPrivate GravitonNodeControlPrivate;

struct _GravitonNodeControlClass
{
  GObjectClass parent_class;
};

struct _GravitonNodeControl
{
  GObject parent;
  GravitonNodeControlPrivate *priv;
};

GType graviton_node_control_get_type (void);

const gchar *graviton_node_control_get_name (GravitonNodeControl *self);
GList *graviton_node_control_list_subcontrols (GravitonNodeControl *self, GError **error);
GravitonNodeControl *graviton_node_control_get_subcontrol (GravitonNodeControl *self, const gchar *name);
GList *graviton_node_control_list_properties (GravitonNodeControl *control, GError **error);
GVariant *graviton_node_control_get_property (GravitonNodeControl *control, const gchar *prop, GError **error);
GravitonNode *graviton_node_control_get_node (GravitonNodeControl *control);

G_END_DECLS

#endif
