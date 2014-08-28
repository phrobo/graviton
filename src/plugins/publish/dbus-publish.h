#ifndef __GRAVITON_DBUS_SERVER_PUBLISH_METHOD_H__
#define __GRAVITON_DBUS_SERVER_PUBLISH_METHOD_H__

#include <glib.h>
#include <glib-object.h>

#include <graviton/server/server-publish-method.h>

G_BEGIN_DECLS

#define GRAVITON_DBUS_SERVER_PUBLISH_METHOD_TYPE            (graviton_dbus_server_publish_method_get_type ())
#define GRAVITON_DBUS_SERVER_PUBLISH_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_DBUS_SERVER_PUBLISH_METHOD_TYPE, GravitonDbusServerPublishMethod))
#define GRAVITON_DBUS_SERVER_PUBLISH_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_DBUS_SERVER_PUBLISH_METHOD_TYPE, GravitonDbusServerPublishMethodClass))
#define IS_GRAVITON_DBUS_SERVER_PUBLISH_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_DBUS_SERVER_PUBLISH_METHOD_TYPE))
#define IS_GRAVITON_DBUS_SERVER_PUBLISH_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_DBUS_SERVER_PUBLISH_METHOD_TYPE))
#define GRAVITON_DBUS_SERVER_PUBLISH_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_DBUS_SERVER_PUBLISH_METHOD_TYPE, GravitonDbusServerPublishMethodClass))

typedef struct _GravitonDbusServerPublishMethod      GravitonDbusServerPublishMethod;
typedef struct _GravitonDbusServerPublishMethodClass GravitonDbusServerPublishMethodClass;
typedef struct _GravitonDbusServerPublishMethodPrivate GravitonDbusServerPublishMethodPrivate;

struct _GravitonDbusServerPublishMethodClass
{
  GravitonServerPublishMethodClass parent_class;
};

struct _GravitonDbusServerPublishMethod
{
  GravitonServerPublishMethod parent;
  GravitonDbusServerPublishMethodPrivate *priv;
};

GType graviton_dbus_server_publish_method_get_type (void);

G_END_DECLS

#endif
