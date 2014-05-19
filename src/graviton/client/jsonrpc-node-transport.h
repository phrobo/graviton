#ifndef __GRAVITON_JSONRPC_NODE_TRANSPORT_H__
#define __GRAVITON_JSONRPC_NODE_TRANSPORT_H__

#include <glib.h>
#include <glib-object.h>
#include "node-transport.h"

G_BEGIN_DECLS

#define GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE            (graviton_jsonrpc_node_transport_get_type ())
#define GRAVITON_JSONRPC_NODE_TRANSPORT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE, GravitonJsonrpcNodeTransport))
#define GRAVITON_JSONRPC_NODE_TRANSPORT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE, GravitonJsonrpcNodeTransportClass))
#define GRAVITON_IS_JSONRPC_NODE_TRANSPORT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE))
#define GRAVITON_IS_JSONRPC_NODE_TRANSPORT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE))
#define GRAVITON_JSONRPC_NODE_TRANSPORT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE, GravitonJsonrpcNodeTransportClass))

typedef struct _GravitonJsonrpcNodeTransport      GravitonJsonrpcNodeTransport;
typedef struct _GravitonJsonrpcNodeTransportClass GravitonJsonrpcNodeTransportClass;
typedef struct _GravitonJsonrpcNodeTransportPrivate GravitonJsonrpcNodeTransportPrivate;

struct _GravitonJsonrpcNodeTransportClass
{
  GravitonNodeTransportClass parent_class;
};

struct _GravitonJsonrpcNodeTransport
{
  GravitonNodeTransport parent;
  GravitonJsonrpcNodeTransportPrivate *priv;
};

GType graviton_jsonrpc_node_transport_get_type (void);

GravitonJsonrpcNodeTransport *graviton_jsonrpc_node_transport_new ();
GravitonJsonrpcNodeTransport *graviton_jsonrpc_node_new_from_address (GInetSocketAddress *address);

G_END_DECLS

#endif
