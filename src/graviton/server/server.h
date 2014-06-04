#ifndef GRAVITON_SERVER_H
#define GRAVITON_SERVER_H

#include <glib-object.h>
#include "root-service.h"

/**
 * GRAVITON_SERVER_TYPE:
 *
 * GType for a #GravitonServer
 */
#define GRAVITON_SERVER_TYPE            (graviton_server_get_type ())
#define GRAVITON_SERVER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_SERVER_TYPE, GravitonServer))
#define GRAVITON_IS_SERVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_SERVER_TYPE))
#define GRAVITON_SERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_SERVER_TYPE, GravitonServerClass))
#define GRAVITON_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_SERVER_TYPE))
#define GRAVITON_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_SERVER_TYPE, GravitonServerClass))

/**
 * GRAVITON_SERVER_ERROR:
 *
 * Error domain for #GravitonServer operations
 */
#define GRAVITON_SERVER_ERROR (graviton_server_error_quark ())

/**
 * GravitonServerError:
 * @GRAVITON_SERVER_ERROR_INVALID_REQUEST: An invalid request was recieved
 * @GRAVITON_SERVER_ERROR_NO_SUCH_METHOD: The requested method was not recieved
 * @GRAVITON_SERVER_ERROR_INVALID_PARAMS: There were some invalid parameters
 * supplied
 * @GRAVITON_SERVER_ERROR_NOT_IMPLEMENTED: The requested operation is not
 * implemented
 * @GRAVITON_SERVER_ERROR_PARSE_ERROR: There was a JSON parse error while
 * handling the request
 *
 * Error type for #GravitonServer operations.
 */
typedef enum {
  GRAVITON_SERVER_ERROR_INVALID_REQUEST = -32600, // These magic numbers are specified in the jsonrpc 2.0 spec
  GRAVITON_SERVER_ERROR_NO_SUCH_METHOD = -32601,
  GRAVITON_SERVER_ERROR_INVALID_PARAMS = -32602,
  GRAVITON_SERVER_ERROR_NOT_IMPLEMENTED = -32603,
  GRAVITON_SERVER_ERROR_PARSE_ERROR = -32700
} GravitonServerError;

typedef struct _GravitonServer GravitonServer;
typedef struct _GravitonServerClass GravitonServerClass;

typedef struct _GravitonServerPrivate GravitonServerPrivate;

/**
 * GravitonServer:
 * @parent_instance: The parent GObject type
 *
 * A core concept of Graviton is nodes that provide services to a cloud of other
 * servers and clients.
 *
 * The #GravitonServer exposes a list #GravitonService objects to a cloud. It
 * may be addressed by a tuple of cloud-id and node-id.
 *
 * #GravitonServer supports routing of messages to other nodes in the network
 * simply by receiving a request that is addressed to another node. The server
 * will attempt to contact the destination node, or the next hop in the route
 * and forward the message verbatim.
 *
 * To begin, create a server using graviton_server_new(). You can add a service
 * that implements a given interface by attaching it to the
 * #GravitonRootService exposed via graviton_server_get_root_service().
 *
 * Servers have two unique properties: #GravitonServer:node-id and
 * #GravitonServer:cloud-id. The cloud ID is shared by all members of the cloud
 * and is used to identify the cloud. Each server has a unique node ID that is
 * generated at startup. These properties can also be read with
 * graviton_server_get_node_id() and graviton_server_get_cloud_id().
 *
 * All servers come equiped with an introspection service available as
 * net:phrobo:graviton.
 *
 * FIXME: Document net:phrobo:graviton API
 *
 */
struct _GravitonServer
{
  GObject parent_instance;
  /*< private >*/
  GravitonServerPrivate *priv;
};

/**
 * GravitonServerClass:
 * @parent_class: The parent GObjectClass
 *
 * The GravitonServer class
 */
struct _GravitonServerClass
{
  GObjectClass parent_class;
};

GType graviton_server_get_type ();

GravitonServer *graviton_server_new ();
void graviton_server_run_async (GravitonServer *server);

GravitonRootService *graviton_server_get_root_service (GravitonServer *server);

const gchar *graviton_server_get_cloud_id (GravitonServer *server);
const gchar *graviton_server_get_node_id (GravitonServer *server);
int graviton_server_get_port (GravitonServer *server);

#endif // GRAVITON_SERVER_H
