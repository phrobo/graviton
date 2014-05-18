#ifndef __GRAVITON_JSONRPC_NODE_H__
#define __GRAVITON_JSONRPC_NODE_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <libsoup/soup.h>

#include "node.h"

G_BEGIN_DECLS

#define GRAVITON_JSONRPC_NODE_TYPE            (graviton_jsonrpc_node_get_type ())
#define GRAVITON_JSONRPC_NODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_JSONRPC_NODE_TYPE, GravitonJsonrpcNode))
#define GRAVITON_JSONRPC_NODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_JSONRPC_NODE_TYPE, GravitonJsonrpcNodeClass))
#define IS_GRAVITON_JSONRPC_NODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_JSONRPC_NODE_TYPE))
#define IS_GRAVITON_JSONRPC_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_JSONRPC_NODE_TYPE))
#define GRAVITON_JSONRPC_NODE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_JSONRPC_NODE_TYPE, GravitonJsonrpcNodeClass))

typedef struct _GravitonJsonrpcNode      GravitonJsonrpcNode;
typedef struct _GravitonJsonrpcNodeClass GravitonJsonrpcNodeClass;
typedef struct _GravitonJsonrpcNodePrivate GravitonJsonrpcNodePrivate;

struct _GravitonJsonrpcNodeClass
{
  GravitonNodeClass parent_class;
};

struct _GravitonJsonrpcNode
{
  GravitonNode parent;
  GravitonJsonrpcNodePrivate *priv;
};

GType graviton_jsonrpc_node_get_type (void);

GravitonJsonrpcNode *graviton_jsonrpc_node_new ();
GravitonJsonrpcNode *graviton_jsonrpc_node_new_from_address (GInetSocketAddress *address);

int graviton_jsonrpc_node_get_port (GravitonJsonrpcNode *node);
GInetSocketAddress *graviton_jsonrpc_node_get_address (GravitonJsonrpcNode *node);

G_END_DECLS

#endif
