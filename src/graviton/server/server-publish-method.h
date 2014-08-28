#ifndef __GRAVITON_SERVER_PUBLISH_METHOD_H__
#define __GRAVITON_SERVER_PUBLISH_METHOD_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRAVITON_SERVER_PUBLISH_METHOD_TYPE            (graviton_server_publish_method_get_type ())
#define GRAVITON_SERVER_PUBLISH_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_SERVER_PUBLISH_METHOD_TYPE, GravitonServerPublishMethod))
#define GRAVITON_SERVER_PUBLISH_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_SERVER_PUBLISH_METHOD_TYPE, GravitonServerPublishMethodClass))
#define IS_GRAVITON_SERVER_PUBLISH_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_SERVER_PUBLISH_METHOD_TYPE))
#define IS_GRAVITON_SERVER_PUBLISH_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_SERVER_PUBLISH_METHOD_TYPE))
#define GRAVITON_SERVER_PUBLISH_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_SERVER_PUBLISH_METHOD_TYPE, GravitonServerPublishMethodClass))

typedef struct _GravitonServerPublishMethod      GravitonServerPublishMethod;
typedef struct _GravitonServerPublishMethodClass GravitonServerPublishMethodClass;
typedef struct _GravitonServerPublishMethodPrivate GravitonServerPublishMethodPrivate;

typedef struct _GravitonServer GravitonServer;

struct _GravitonServerPublishMethodClass
{
  GObjectClass parent_class;
  void (*start) (GravitonServerPublishMethod *self);
  void (*stop) (GravitonServerPublishMethod *self);
};

struct _GravitonServerPublishMethod
{
  GObject parent;
  GravitonServerPublishMethodPrivate *priv;
};

GType graviton_server_publish_method_get_type (void);

typedef GravitonServerPublishMethod *(*GravitonServerPublishMethodLoaderFunc)(GravitonServer *server);

#define GRAVITON_DEFINE_PUBLISH_PLUGIN(type) \
  GravitonServerPublishMethod *make_graviton_publish_plugin( \
      GravitonServer *server) {return g_object_new ((type), \
                                                    "server", \
                                                    server, \
                                                    NULL); }

GravitonServerPublishMethod *graviton_server_publish_method_new ();

void graviton_server_publish_method_start (GravitonServerPublishMethod *method);
void graviton_server_publish_method_stop (GravitonServerPublishMethod *method);
GravitonServer *graviton_server_publish_method_get_server (GravitonServerPublishMethod *method);

G_END_DECLS

#endif
