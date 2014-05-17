#ifndef __GRAVITON_AVAHI_DISCOVERY_METHOD_H__
#define __GRAVITON_AVAHI_DISCOVERY_METHOD_H__

#include <glib.h>
#include <glib-object.h>

#include <graviton/client/discovery-method.h>

G_BEGIN_DECLS

#define GRAVITON_AVAHI_DISCOVERY_METHOD_TYPE            (graviton_avahi_discovery_method_get_type ())
#define GRAVITON_AVAHI_DISCOVERY_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_AVAHI_DISCOVERY_METHOD_TYPE, GravitonAvahiDiscoveryMethod))
#define GRAVITON_AVAHI_DISCOVERY_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_AVAHI_DISCOVERY_METHOD_TYPE, GravitonAvahiDiscoveryMethodClass))
#define IS_GRAVITON_AVAHI_DISCOVERY_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_AVAHI_DISCOVERY_METHOD_TYPE))
#define IS_GRAVITON_AVAHI_DISCOVERY_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_AVAHI_DISCOVERY_METHOD_TYPE))
#define GRAVITON_AVAHI_DISCOVERY_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_AVAHI_DISCOVERY_METHOD_TYPE, GravitonAvahiDiscoveryMethodClass))

typedef struct _GravitonAvahiDiscoveryMethod      GravitonAvahiDiscoveryMethod;
typedef struct _GravitonAvahiDiscoveryMethodClass GravitonAvahiDiscoveryMethodClass;
typedef struct _GravitonAvahiDiscoveryMethodPrivate GravitonAvahiDiscoveryMethodPrivate;

struct _GravitonAvahiDiscoveryMethodClass
{
  GravitonDiscoveryMethodClass parent_class;
};

struct _GravitonAvahiDiscoveryMethod
{
  GravitonDiscoveryMethod parent;
  GravitonAvahiDiscoveryMethodPrivate *priv;
};

GType graviton_avahi_discovery_method_get_type (void);

G_END_DECLS

#endif
