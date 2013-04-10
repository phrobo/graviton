#ifndef GRAVITON_SERVER_H
#define GRAVITON_SERVER_H

#include <glib-object.h>

#define GRAVITON_TYPE_SERVER            (graviton_server_get_type ())
#define GRAVITON_SERVER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_TYPE_SERVER, GravitonServer))
#define GRAVITON_IS_SERVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_TYPE_SERVER))
#define GRAVITON_SERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_TYPE_SERVER, GravitonServerClass))
#define GRAVITON_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_TYPE_SERVER))
#define GRAVITON_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_TYPE_SERVER, GravitonServerClass))

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

#endif // GRAVITON_SERVER_H
