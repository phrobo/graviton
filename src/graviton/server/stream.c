#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "stream.h"

typedef struct _GravitonStreamPrivate GravitonStreamPrivate;

struct _GravitonStreamPrivate
{
  gchar *name;
};

#define GRAVITON_STREAM_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_STREAM_TYPE, GravitonStreamPrivate))

static void graviton_stream_class_init (GravitonStreamClass *klass);
static void graviton_stream_init       (GravitonStream *self);
static void graviton_stream_dispose    (GObject *object);
static void graviton_stream_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonStream, graviton_stream, G_TYPE_OBJECT);

static void
graviton_stream_class_init (GravitonStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonStreamPrivate));

  object_class->dispose = graviton_stream_dispose;
  object_class->finalize = graviton_stream_finalize;
}

static void
graviton_stream_init (GravitonStream *self)
{
  GravitonStreamPrivate *priv;
  priv = self->priv = GRAVITON_STREAM_GET_PRIVATE (self);
}

static void
graviton_stream_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_stream_parent_class)->dispose (object);
}

static void
graviton_stream_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_stream_parent_class)->finalize (object);
}

GravitonStream *
graviton_stream_new (const gchar *name)
{
  return g_object_new (GRAVITON_STREAM_TYPE, "name", name, NULL);
}

const gchar *
graviton_stream_get_name (GravitonStream *self)
{
  return self->priv->name;
}

GInputStream *
graviton_stream_open_read (GravitonStream *self, GError **error)
{
  return GRAVITON_STREAM_GET_CLASS(self)->open_read (self, error);
}

GOutputStream *
graviton_stream_open_write (GravitonStream *self, GError **error)
{
  return GRAVITON_STREAM_GET_CLASS(self)->open_write (self, error);
}
