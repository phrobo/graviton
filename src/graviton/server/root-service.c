#include "root-service.h"
#include "service.h"
#include <gmodule.h>

#include "config.h"

#define GRAVITON_ROOT_SERVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_ROOT_SERVICE_TYPE, GravitonRootServicePrivate))

G_DEFINE_TYPE (GravitonRootService, graviton_root_service, GRAVITON_SERVICE_TYPE);

enum {
  SIGNAL_0,
  SIGNAL_CONTROL_ADDED,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = {0, };

static void graviton_root_service_finalize (GObject *object);
static void graviton_root_service_dispose (GObject *object);

static void
graviton_root_service_class_init (GravitonRootServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = graviton_root_service_dispose;
  object_class->finalize = graviton_root_service_finalize;

  obj_signals[SIGNAL_CONTROL_ADDED] =
    g_signal_new ("service-added",
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
}

static void
graviton_root_service_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_root_service_parent_class)->dispose (object);
}

static void
graviton_root_service_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_root_service_parent_class)->finalize (object);
}

GravitonRootService *
graviton_root_service_new ()
{
  return g_object_new (GRAVITON_ROOT_SERVICE_TYPE, NULL);
}
