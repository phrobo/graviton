#ifndef __GRAVITON_NODE_INPUT_STREAM_H__
#define __GRAVITON_NODE_INPUT_STREAM_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GRAVITON_NODE_INPUT_STREAM_TYPE            (graviton_node_input_stream_get_type ())
#define GRAVITON_NODE_INPUT_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NODE_INPUT_STREAM_TYPE, GravitonNodeInputStream))
#define GRAVITON_NODE_INPUT_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NODE_INPUT_STREAM_TYPE, GravitonNodeInputStreamClass))
#define IS_GRAVITON_NODE_INPUT_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NODE_INPUT_STREAM_TYPE))
#define IS_GRAVITON_NODE_INPUT_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NODE_INPUT_STREAM_TYPE))
#define GRAVITON_NODE_INPUT_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NODE_INPUT_STREAM_TYPE, GravitonNodeInputStreamClass))

typedef struct _GravitonNodeIOStream GravitonNodeIOStream;

typedef struct _GravitonNodeInputStream      GravitonNodeInputStream;
typedef struct _GravitonNodeInputStreamClass GravitonNodeInputStreamClass;
typedef struct _GravitonNodeInputStreamPrivate      GravitonNodeInputStreamPrivate;

struct _GravitonNodeInputStreamClass
{
  GInputStreamClass parent_class;
};

struct _GravitonNodeInputStream
{
  GInputStream parent;
  GravitonNodeInputStreamPrivate *priv;
};

GType graviton_node_input_stream_get_type (void);

GravitonNodeInputStream *graviton_node_input_stream_new (GravitonNodeIOStream *stream);

G_END_DECLS

#endif
