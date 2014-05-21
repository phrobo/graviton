#ifndef __GRAVITON_NODE_BROWSER_H__
#define __GRAVITON_NODE_BROWSER_H__

#include <glib.h>
#include <glib-object.h>
#include "discovery-method.h"

G_BEGIN_DECLS

#define GRAVITON_NODE_BROWSER_TYPE            (graviton_node_browser_get_type ())
#define GRAVITON_NODE_BROWSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NODE_BROWSER_TYPE, GravitonNodeBrowser))
#define GRAVITON_NODE_BROWSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NODE_BROWSER_TYPE, GravitonNodeBrowserClass))
#define IS_GRAVITON_NODE_BROWSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NODE_BROWSER_TYPE))
#define IS_GRAVITON_NODE_BROWSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NODE_BROWSER_TYPE))
#define GRAVITON_NODE_BROWSER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NODE_BROWSER_TYPE, GravitonNodeBrowserClass))

typedef struct _GravitonNodeBrowser      GravitonNodeBrowser;
typedef struct _GravitonNodeBrowserClass GravitonNodeBrowserClass;
typedef struct _GravitonNodeBrowserPrivate GravitonNodeBrowserPrivate;

struct _GravitonNodeBrowserClass
{
  GObjectClass parent_class;
};

struct _GravitonNodeBrowser
{
  GObject parent;
  GravitonNodeBrowserPrivate *priv;
};

GType graviton_node_browser_get_type (void);

GravitonNodeBrowser *graviton_node_browser_new ();

void graviton_node_browser_add_discovery_method (GravitonNodeBrowser *client, GravitonDiscoveryMethod *method);
void graviton_node_browser_load_discovery_plugins (GravitonNodeBrowser *client);
GArray *graviton_node_browser_find_discovery_plugins (GravitonNodeBrowser *client);

GList *graviton_node_browser_get_found_nodes (GravitonNodeBrowser *client);
GList *graviton_node_browser_get_found_clouds (GravitonNodeBrowser *client);

G_END_DECLS

#endif
