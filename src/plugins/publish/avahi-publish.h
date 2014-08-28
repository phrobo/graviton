#ifndef __GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_H__
#define __GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_H__

#include <glib.h>
#include <glib-object.h>

#include <graviton/server/server-publish-method.h>

G_BEGIN_DECLS

#define GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_TYPE            (graviton_avahi_server_publish_method_get_type ())
#define GRAVITON_AVAHI_SERVER_PUBLISH_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_TYPE, GravitonAvahiServerPublishMethod))
#define GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_TYPE, GravitonAvahiServerPublishMethodClass))
#define IS_GRAVITON_AVAHI_SERVER_PUBLISH_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_TYPE))
#define IS_GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_TYPE))
#define GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_AVAHI_SERVER_PUBLISH_METHOD_TYPE, GravitonAvahiServerPublishMethodClass))

typedef struct _GravitonAvahiServerPublishMethod      GravitonAvahiServerPublishMethod;
typedef struct _GravitonAvahiServerPublishMethodClass GravitonAvahiServerPublishMethodClass;
typedef struct _GravitonAvahiServerPublishMethodPrivate GravitonAvahiServerPublishMethodPrivate;

struct _GravitonAvahiServerPublishMethodClass
{
  GravitonServerPublishMethodClass parent_class;
};

struct _GravitonAvahiServerPublishMethod
{
  GravitonServerPublishMethod parent;
  GravitonAvahiServerPublishMethodPrivate *priv;
};

GType graviton_avahi_server_publish_method_get_type (void);

G_END_DECLS

#endif
