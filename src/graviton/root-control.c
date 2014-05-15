#include "root-control.h"
#include "control.h"
#include <gmodule.h>

#include "config.h"

#define GRAVITON_ROOT_CONTROL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_ROOT_CONTROL_TYPE, GravitonRootControlPrivate))

G_DEFINE_TYPE (GravitonRootControl, graviton_root_control, GRAVITON_CONTROL_TYPE);

struct _GravitonRootControlPrivate
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
graviton_root_control_class_init (GravitonRootControlClass *klass)
{
  g_type_class_add_private (klass, sizeof (GravitonRootControlPrivate));

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
                  GRAVITON_CONTROL_TYPE);
}

static void
graviton_root_control_init (GravitonRootControl *self)
{
  GravitonRootControlPrivate *priv;
  self->priv = priv = GRAVITON_ROOT_CONTROL_GET_PRIVATE (self);

}

static void
graviton_root_control_dispose (GObject *gobject)
{
  GravitonRootControl *self = GRAVITON_ROOT_CONTROL (gobject);
}

static void
graviton_root_control_finalize (GObject *gobject)
{
  GravitonRootControl *self = GRAVITON_ROOT_CONTROL (gobject);
}

GravitonRootControl *
graviton_root_control_new ()
{
  return g_object_new (GRAVITON_ROOT_CONTROL_TYPE, NULL);
}
