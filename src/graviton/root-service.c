#include "root-service.h"
#include "service.h"
#include <gmodule.h>

#include "config.h"

#define GRAVITON_ROOT_SERVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_ROOT_SERVICE_TYPE, GravitonRootServicePrivate))

G_DEFINE_TYPE (GravitonRootService, graviton_root_service, GRAVITON_SERVICE_TYPE);

struct _GravitonRootServicePrivate
{
  int dummy;
};

enum {
  SIGNAL_0,
  SIGNAL_CONTROL_ADDED,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = {0, };

static void
graviton_root_service_class_init (GravitonRootServiceClass *klass)
{
  g_type_class_add_private (klass, sizeof (GravitonRootServicePrivate));

  obj_signals[SIGNAL_CONTROL_ADDED] =
    g_signal_new ("control-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GRAVITON_SERVICE_TYPE);
}

static void
graviton_root_service_init (GravitonRootService *self)
{
  GravitonRootServicePrivate *priv;
  self->priv = priv = GRAVITON_ROOT_SERVICE_GET_PRIVATE (self);

}

static void
graviton_root_service_dispose (GObject *gobject)
{
  GravitonRootService *self = GRAVITON_ROOT_SERVICE (gobject);
}

static void
graviton_root_service_finalize (GObject *gobject)
{
  GravitonRootService *self = GRAVITON_ROOT_SERVICE (gobject);
}

GravitonRootService *
graviton_root_service_new ()
{
  return g_object_new (GRAVITON_ROOT_SERVICE_TYPE, NULL);
}
