#ifndef __GRAVITON_STATIC_STREAM_H__
#define __GRAVITON_STATIC_STREAM_H__

#include <glib.h>
#include <glib-object.h>

#include "stream.h"

G_BEGIN_DECLS

#define GRAVITON_STATIC_STREAM_TYPE            (graviton_static_stream_get_type ())
#define GRAVITON_STATIC_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_STATIC_STREAM_TYPE, GravitonStaticStream))
#define GRAVITON_STATIC_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_STATIC_STREAM_TYPE, GravitonStaticStreamClass))
#define IS_GRAVITON_STATIC_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_STATIC_STREAM_TYPE))
#define IS_GRAVITON_STATIC_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_STATIC_STREAM_TYPE))
#define GRAVITON_STATIC_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_STATIC_STREAM_TYPE, GravitonStaticStreamClass))

typedef struct _GravitonStaticStream      GravitonStaticStream;
typedef struct _GravitonStaticStreamClass GravitonStaticStreamClass;
typedef struct _GravitonStaticStreamPrivate GravitonStaticStreamPrivate;

struct _GravitonStaticStreamClass
{
  GravitonStreamClass parent_class;
};

struct _GravitonStaticStream
{
  GravitonStream parent;
  GravitonStaticStreamPrivate *priv;
};

GType graviton_static_stream_get_type (void);

GravitonStaticStream *graviton_static_stream_new ();
GravitonStaticStream *graviton_static_stream_new_contents (const gchar *contents);

G_END_DECLS

#endif
