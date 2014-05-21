#ifndef __GRAVITON_DISCOVERY_METHOD_H__
#define __GRAVITON_DISCOVERY_METHOD_H__

#include <glib.h>
#include <glib-object.h>
#include "node.h"

G_BEGIN_DECLS

#define GRAVITON_DISCOVERY_METHOD_TYPE            (graviton_discovery_method_get_type ())
#define GRAVITON_DISCOVERY_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_DISCOVERY_METHOD_TYPE, GravitonDiscoveryMethod))
#define GRAVITON_DISCOVERY_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_DISCOVERY_METHOD_TYPE, GravitonDiscoveryMethodClass))
#define IS_GRAVITON_DISCOVERY_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_DISCOVERY_METHOD_TYPE))
#define IS_GRAVITON_DISCOVERY_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_DISCOVERY_METHOD_TYPE))
#define GRAVITON_DISCOVERY_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_DISCOVERY_METHOD_TYPE, GravitonDiscoveryMethodClass))

typedef struct _GravitonDiscoveryMethod      GravitonDiscoveryMethod;
typedef struct _GravitonDiscoveryMethodClass GravitonDiscoveryMethodClass;
typedef struct _GravitonDiscoveryMethodPrivate GravitonDiscoveryMethodPrivate;

typedef struct _GravitonNodeBrowser GravitonNodeBrowser;

struct _GravitonDiscoveryMethodClass
{
  GObjectClass parent_class;
  void (*start) (GravitonDiscoveryMethod *self);
  void (*stop) (GravitonDiscoveryMethod *self);
};

struct _GravitonDiscoveryMethod
{
  GObject parent;
  GravitonDiscoveryMethodPrivate *priv;
};

GType graviton_discovery_method_get_type (void);

typedef GravitonDiscoveryMethod *(*GravitonDiscoveryPluginLoaderFunc)(GravitonNodeBrowser *browser);

#define GRAVITON_DEFINE_DISCOVERY_PLUGIN(type) \
  GravitonDiscoveryMethod *make_graviton_discovery_plugin(GravitonNodeBrowser *browser) { return g_object_new ((type), "node-browser", browser, NULL); }

void graviton_discovery_method_start (GravitonDiscoveryMethod *method);
void graviton_discovery_method_stop (GravitonDiscoveryMethod *method);

void graviton_discovery_method_node_found (GravitonDiscoveryMethod *method, GravitonNode *node);
void graviton_discovery_method_finished (GravitonDiscoveryMethod *method);
void graviton_discovery_method_node_lost (GravitonDiscoveryMethod *method, GravitonNode *node);

GravitonCloud *graviton_discovery_method_get_cloud (GravitonDiscoveryMethod *method);

GList *graviton_discovery_method_found_nodes (GravitonDiscoveryMethod *method);

G_END_DECLS

#endif
