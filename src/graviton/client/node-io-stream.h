#ifndef __GRAVITON_NODE_IO_STREAM_H__
#define __GRAVITON_NODE_IO_STREAM_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <libsoup/soup.h>

G_BEGIN_DECLS

#define GRAVITON_NODE_IO_STREAM_TYPE            (graviton_node_io_stream_get_type ())
#define GRAVITON_NODE_IO_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NODE_IO_STREAM_TYPE, GravitonNodeIOStream))
#define GRAVITON_NODE_IO_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NODE_IO_STREAM_TYPE, GravitonNodeIOStreamClass))
#define IS_GRAVITON_NODE_IO_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NODE_IO_STREAM_TYPE))
#define IS_GRAVITON_NODE_IO_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NODE_IO_STREAM_TYPE))
#define GRAVITON_NODE_IO_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NODE_IO_STREAM_TYPE, GravitonNodeIOStreamClass))

typedef struct _GravitonNodeIOStream      GravitonNodeIOStream;
typedef struct _GravitonNodeIOStreamClass GravitonNodeIOStreamClass;
typedef struct _GravitonNodeIOStreamPrivate      GravitonNodeIOStreamPrivate;

struct _GravitonNodeIOStreamClass
{
  GIOStreamClass parent_class;
};

struct _GravitonNodeIOStream
{
  GIOStream parent;
  GravitonNodeIOStreamPrivate *priv;
};

GType graviton_node_io_stream_get_type (void);

GravitonNodeIOStream *graviton_node_io_stream_new (SoupURI *uri, SoupSession *session);

SoupURI *graviton_node_io_stream_get_uri (GravitonNodeIOStream *stream);
SoupSession *graviton_node_io_stream_get_session (GravitonNodeIOStream *stream);

G_END_DECLS

#endif
