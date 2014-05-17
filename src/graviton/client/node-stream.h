#ifndef __GRAVITON_NODE_STREAM_H__
#define __GRAVITON_NODE_STREAM_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRAVITON_NODE_STREAM_TYPE            (graviton_node_stream_get_type ())
#define GRAVITON_NODE_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NODE_STREAM_TYPE, GravitonNodeStream))
#define GRAVITON_NODE_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NODE_STREAM_TYPE, GravitonNodeStreamClass))
#define IS_GRAVITON_NODE_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NODE_STREAM_TYPE))
#define IS_GRAVITON_NODE_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NODE_STREAM_TYPE))
#define GRAVITON_NODE_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NODE_STREAM_TYPE, GravitonNodeStreamClass))

typedef struct _GravitonServiceInterface GravitonServiceInterface;

typedef struct _GIOStream GIOStream;

typedef struct _GravitonNodeStream      GravitonNodeStream;
typedef struct _GravitonNodeStreamClass GravitonNodeStreamClass;
typedef struct _GravitonNodeStreamPrivate      GravitonNodeStreamPrivate;

struct _GravitonNodeStreamClass
{
  GObjectClass parent_class;
};

struct _GravitonNodeStream
{
  GObject parent;
  GravitonNodeStreamPrivate *priv;
};

GType graviton_node_stream_get_type (void);

GravitonNodeStream *graviton_node_stream_new (GravitonServiceInterface *node, const gchar *name, GHashTable *args);
const gchar *graviton_node_stream_get_name (GravitonNodeStream *stream);
GIOStream *graviton_node_stream_open (GravitonNodeStream *stream);

G_END_DECLS

#endif
