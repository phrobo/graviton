#ifndef __GRAVITON_STREAM_H__
#define __GRAVITON_STREAM_H__

#include <glib.h>
#include <glib-object.h>
#include "node.h"
#include "control.h"

G_BEGIN_DECLS

#define GRAVITON_STREAM_TYPE            (graviton_stream_get_type ())
#define GRAVITON_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_STREAM_TYPE, GravitonStream))
#define GRAVITON_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_STREAM_TYPE, GravitonStreamClass))
#define IS_GRAVITON_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_STREAM_TYPE))
#define IS_GRAVITON_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_STREAM_TYPE))
#define GRAVITON_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_STREAM_TYPE, GravitonStreamClass))

typedef struct _GravitonStream      GravitonStream;
typedef struct _GravitonStreamClass GravitonStreamClass;
typedef struct _GravitonStreamPrivate GravitonStreamPrivate;

struct _GravitonStreamClass
{
  GObjectClass parent_class;
  GInputStream *(*open_read)(GravitonStream *self, GError **error);
  GOutputStream *(*open_write)(GravitonStream *self, GError **error);
};

struct _GravitonStream
{
  GObject parent;
  GravitonStreamPrivate *priv;
};

GType graviton_stream_get_type (void);

GravitonStream *graviton_stream_new (const gchar *name);

GInputStream *graviton_stream_open_read (GravitonStream *self, GError **error);
GOutputStream *graviton_stream_open_write (GravitonStream *self, GError **error);
GVariant *graviton_stream_get_metadata (GravitonStream *stream, const gchar *name);
void graviton_stream_set_metadata (GravitonStream *stream, const gchar *name, GVariant value);
const gchar *graviton_stream_get_name (GravitonStream *self);

G_END_DECLS

#endif
