#ifndef GRAVITON_ROOT_CONTROL_H
#define GRAVITON_ROOT_CONTROL_H

#include <glib-object.h>
#include <graviton/control.h>

#define GRAVITON_TYPE_ROOT_CONTROL            (graviton_root_control_get_type ())
#define GRAVITON_ROOT_CONTROL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_ROOT_CONTROL, GravitonRootControl))
#define GRAVITON_IS_ROOT_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_TYPE_ROOT_CONTROL))
#define GRAVITON_ROOT_CONTROL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_TYPE_ROOT_CONTROL, GravitonRootControlClass))
#define GRAVITON_IS_ROOT_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_TYPE_ROOT_CONTROL))
#define GRAVITON_PLUGIN_GET_MANAGER_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_TYPE_ROOT_CONTROL, GravitonRootControlClass))

typedef struct _GravitonRootControl GravitonRootControl;
typedef struct _GravitonRootControlClass GravitonRootControlClass;

typedef struct _GravitonRootControlPrivate GravitonRootControlPrivate;

struct _GravitonRootControl
{
  GravitonControl parent_instance;
  GravitonRootControlPrivate *priv;
};

struct _GravitonRootControlClass
{
  GObjectClass parent_class;
};

typedef struct _GravitonPlugin GravitonPlugin;

GType graviton_root_control_get_type ();
GravitonRootControl *graviton_root_control_new ();

GArray *graviton_root_control_find_plugins (GravitonRootControl *manager);

#endif // GRAVITON_PLUGIN_H
