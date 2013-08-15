#ifndef __GRAVITON_NEIGHBOR_DISCOVERY_METHOD_H__
#define __GRAVITON_NEIGHBOR_DISCOVERY_METHOD_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE            (graviton_neighbor_discovery_method_get_type ())
#define GRAVITON_NEIGHBOR_DISCOVERY_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE, GravitonNeighborDiscoveryMethod))
#define GRAVITON_NEIGHBOR_DISCOVERY_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE, GravitonNeighborDiscoveryMethodClass))
#define IS_GRAVITON_NEIGHBOR_DISCOVERY_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE))
#define IS_GRAVITON_NEIGHBOR_DISCOVERY_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE))
#define GRAVITON_NEIGHBOR_DISCOVERY_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NEIGHBOR_DISCOVERY_METHOD_TYPE, GravitonNeighborDiscoveryMethodClass))

typedef struct _GravitonNeighborDiscoveryMethod      GravitonNeighborDiscoveryMethod;
typedef struct _GravitonNeighborDiscoveryMethodClass GravitonNeighborDiscoveryMethodClass;
typedef struct _GravitonNeighborDiscoveryMethodPrivate GravitonNeighborDiscoveryMethodPrivate;

struct _GravitonNeighborDiscoveryMethodClass
{
  GravitonDiscoveryMethodClass parent_class;
};

struct _GravitonNeighborDiscoveryMethod
{
  GravitonDiscoveryMethod parent;
  GravitonNeighborDiscoveryMethodPrivate *priv;
};

GType graviton_neighbor_discovery_method_get_type (void);

G_END_DECLS

#endif
