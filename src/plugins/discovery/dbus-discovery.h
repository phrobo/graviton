#ifndef __GRAVITON_DBUS_DISCOVERY_METHOD_H__
#define __GRAVITON_DBUS_DISCOVERY_METHOD_H__

#include <glib.h>
#include <glib-object.h>

#include <graviton/client/discovery-method.h>

G_BEGIN_DECLS

#define GRAVITON_DBUS_DISCOVERY_METHOD_TYPE            (graviton_dbus_discovery_method_get_type ())
#define GRAVITON_DBUS_DISCOVERY_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE, GravitonDbusDiscoveryMethod))
#define GRAVITON_DBUS_DISCOVERY_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE, GravitonDbusDiscoveryMethodClass))
#define IS_GRAVITON_DBUS_DISCOVERY_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE))
#define IS_GRAVITON_DBUS_DISCOVERY_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE))
#define GRAVITON_DBUS_DISCOVERY_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_DBUS_DISCOVERY_METHOD_TYPE, GravitonDbusDiscoveryMethodClass))

typedef struct _GravitonDbusDiscoveryMethod      GravitonDbusDiscoveryMethod;
typedef struct _GravitonDbusDiscoveryMethodClass GravitonDbusDiscoveryMethodClass;

struct _GravitonDbusDiscoveryMethodClass
{
  GravitonDiscoveryMethodClass parent_class;
};

struct _GravitonDbusDiscoveryMethod
{
  GravitonDiscoveryMethod parent;
};

GType graviton_dbus_discovery_method_get_type (void);

GravitonDbusDiscoveryMethod *graviton_dbus_discovery_method_new ();

G_END_DECLS

#endif
