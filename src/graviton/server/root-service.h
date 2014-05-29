#ifndef GRAVITON_ROOT_SERVICE_H
#define GRAVITON_ROOT_SERVICE_H

#include <glib-object.h>
#include "service.h"

#define GRAVITON_ROOT_SERVICE_TYPE            (graviton_root_service_get_type ())
#define GRAVITON_ROOT_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_ROOT_SERVICE_TYPE, GravitonRootService))
#define GRAVITON_IS_ROOT_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_ROOT_SERVICE_TYPE))
#define GRAVITON_ROOT_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_ROOT_SERVICE_TYPE, GravitonRootServiceClass))
#define GRAVITON_IS_ROOT_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_ROOT_SERVICE_TYPE))
#define GRAVITON_PLUGIN_GET_MANAGER_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_ROOT_SERVICE_TYPE, GravitonRootServiceClass))

typedef struct _GravitonRootService GravitonRootService;
typedef struct _GravitonRootServiceClass GravitonRootServiceClass;

typedef struct _GravitonRootServicePrivate GravitonRootServicePrivate;

struct _GravitonRootService
{
  GravitonService parent_instance;
};

struct _GravitonRootServiceClass
{
  GObjectClass parent_class;
};

typedef struct _GravitonPlugin GravitonPlugin;

GType graviton_root_service_get_type ();
GravitonRootService *graviton_root_service_new ();

GArray *graviton_root_service_find_plugins (GravitonRootService *manager);

#endif // GRAVITON_PLUGIN_H
