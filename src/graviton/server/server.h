#ifndef GRAVITON_SERVER_H
#define GRAVITON_SERVER_H

#include <glib-object.h>
#include "root-service.h"

#define GRAVITON_SERVER_TYPE            (graviton_server_get_type ())
#define GRAVITON_SERVER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_SERVER_TYPE, GravitonServer))
#define GRAVITON_IS_SERVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_SERVER_TYPE))
#define GRAVITON_SERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_SERVER_TYPE, GravitonServerClass))
#define GRAVITON_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_SERVER_TYPE))
#define GRAVITON_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_SERVER_TYPE, GravitonServerClass))

#define GRAVITON_SERVER_ERROR (graviton_server_error_quark ())

typedef enum {
  GRAVITON_SERVER_ERROR_INVALID_REQUEST = -32600, // These magic numbers are specified in the jsonrpc 2.0 spec
  GRAVITON_SERVER_ERROR_NO_SUCH_METHOD = -32601,
  GRAVITON_SERVER_ERROR_INVALID_PARAMS = -32602,
  GRAVITON_SERVER_ERROR_NOT_IMPLEMENTED = -32603,
  GRAVITON_SERVER_ERROR_PARSE_ERROR = -32700
} GravitonServerErrror;

typedef struct _GravitonServer GravitonServer;
typedef struct _GravitonServerClass GravitonServerClass;

typedef struct _GravitonServerPrivate GravitonServerPrivate;

struct _GravitonServer
{
  GObject parent_instance;
  GravitonServerPrivate *priv;
};

struct _GravitonServerClass
{
  GObjectClass parent_class;
};

GType graviton_server_get_type ();

GravitonServer *graviton_server_new ();
void graviton_server_run_async (GravitonServer *server);

void graviton_server_load_plugins (GravitonServer *server);

GravitonRootService *graviton_server_get_root_service (GravitonServer *server);

const gchar *graviton_server_get_cloud_id (GravitonServer *server);
const gchar *graviton_server_get_node_id (GravitonServer *server);
int graviton_server_get_port (GravitonServer *server);

#endif // GRAVITON_SERVER_H
