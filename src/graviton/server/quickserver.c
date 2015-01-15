#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "quickserver.h"

typedef struct _GravitonQuickserverPrivate GravitonQuickserverPrivate;

struct _GravitonQuickserverPrivate
{
};

#define GRAVITON_QUICKSERVER_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_QUICKSERVER_TYPE, GravitonQuickserverPrivate))

static void graviton_quickserver_class_init (GravitonQuickserverClass *klass);
static void graviton_quickserver_init       (GravitonQuickserver *self);
static void graviton_quickserver_dispose    (GObject *object);
static void graviton_quickserver_finalize   (GObject *object);
static void graviton_quickserver_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_quickserver_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonQuickserver, graviton_quickserver, GRAVITON_SERVER_TYPE);

enum {
  PROP_ZERO,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
graviton_quickserver_class_init (GravitonQuickserverClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonQuickserverPrivate));

  object_class->dispose = graviton_quickserver_dispose;
  object_class->finalize = graviton_quickserver_finalize;
  object_class->set_property =  graviton_quickserver_set_property;
  object_class->get_property =  graviton_quickserver_get_property;
  g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);
}

static void
graviton_quickserver_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonQuickserver *self = GRAVITON_QUICKSERVER (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_quickserver_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonQuickserver *self = GRAVITON_QUICKSERVER (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_quickserver_init (GravitonQuickserver *self)
{
  GravitonQuickserverPrivate *priv;
  priv = self->priv = GRAVITON_QUICKSERVER_GET_PRIVATE (self);
}

static void
graviton_quickserver_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_quickserver_parent_class)->dispose (object);
}

static void
graviton_quickserver_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_quickserver_parent_class)->finalize (object);
}
