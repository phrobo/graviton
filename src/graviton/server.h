#ifndef GRAVITON_SERVER_H
#define GRAVITON_SERVER_H

#include <glib-object.h>
#include <graviton/root-control.h>

#define GRAVITON_TYPE_SERVER            (graviton_server_get_type ())
#define GRAVITON_SERVER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_SERVER, GravitonServer))
#define GRAVITON_IS_SERVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_TYPE_SERVER))
#define GRAVITON_SERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_TYPE_SERVER, GravitonServerClass))
#define GRAVITON_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_TYPE_SERVER))
#define GRAVITON_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_TYPE_SERVER, GravitonServerClass))

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

GravitonRootControl *graviton_server_get_root_control (GravitonServer *server);

#endif // GRAVITON_SERVER_H
