#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "node.h"

#include "discovery-method.h"
#include "cloud.h"

/**
 * SECTION:discovery-method
 * @short_description: The magical discovery and routing component of Graviton
 * @title: GravitonDiscoveryMethod
 * @see_also: #GravitonCloud
 * @stability: Unstable
 * @include: graviton/client/discovery-method.h
 *
 * A GravitonDiscoveryMethod instance is a  method of discovering nodes and clouds from
 * various sources
 *
 * Two key features of Graviton are being able to discover other nodes and services in a cloud
 * and to transparently connect to other nodes regardless of where they're at.
 *
 * A #GravitonDiscoveryMethod handles both of those.
 *
 * Discovery methods are commonly used by a #GravitonNodeBrowser which handles
 * the loading, starting, and stopping of each #GravitonDiscoveryMethod. The
 * lifecycle of a discovery method is usually as follows:
 *
 * - A #GravitonNodeBrowser is constructed, often transparently via
 *   #graviton_cloud_new_default_cloud()
 * - #graviton_node_browser_load_discovery_plugins() is called, which in turn
 *   calls #graviton_node_browser_add_discovery_method() on each found plugin
 * - #graviton_discovery_method_start() starts the plugin, which means that the
 *   #GravitonDiscoveryMethodClass.start() method is called
 * - The #GravitonDiscoveryMethod starts any internal threads or asyncronous
 *   operations nessicary to find as many #GravitonNode objects as it can.
 * - Each discovered node is passed to #graviton_discovery_method_node_found()
 * - Once all nodes have been found, #graviton_discovery_method_finished()
 *   notifies listeners that it has finished its initial list of nodes. More
 *   nodes may show up later.
 * - If a node is later confirmed as unreachable,
 *   #graviton_discovery_method_node_lost() is called to notify listeners
 * - Once the #GravitonNodeBrowser has been disposed,
 *   #graviton_discovery_method_stop() is called on each plugin and is unref'd.
 *
 * Discovery methods are responsible for constructing a #GravitonNodeTransport
 * subclass and mapping it to a node ID. To do so, one fetches the node using
 * graviton_node_get_by_id() and then calling graviton_node_add_transport():
 *
 *  <example>
 *  <title>Mapping a GravitonNodeTransport to a GravitonNode</title>
 *  <programlisting>
 * #include <graviton/client/jsonrpc-node-transport.h>
 * #include <graviton/client/discovery-method.h>
 *
 * #define GRAVITON_STATIC_DISCOVERY_METHOD_TYPE (graviton_static_discovery_method_get_type ())
 * GType graviton_static_discovery_method_get_type (void);
 *
 * struct _GravitonStaticDiscoveryMethodClass
 * {
 *   GravitonDiscoveryMethodClass parent_class;
 * } GravitonStaticDiscoveryMethodClass;
 *
 * struct _GravitonstaticDiscoveryMethod
 * {
 *   GravitonDiscoveryMethod parent;
 * } GravitonStaticDiscoveryMethod;
 *
 * static void start_browse (GravitonDiscoveryMethod *method)
 * {
 *   GravitonStaticDiscoveryMethod *self;
 *   GravitonJsonrpcNodetransport *transport;
 *   GInetAddress *addrName;
 *   GInetSocketAddress *addr;
 *   GravitonNode *node;
 *
 *   addrName = g_inet_address_new_from_string ("127.0.0.1");
 *   addr = (GInetSocketAddress*)g_inet_socket_address_new (addrName, 8000);
 *
 *   self = (GravitonStaticDiscoveryMethod*)method;
 *   transport = graviton_jsonrpc_node_transport_new (addr);
 *
 *   node = graviton_node_get_by_id ("3857E91C-BA9F-4CB9-B667-4BBB42C06FC3");
 *   graviton_node_add_transport (node, GRAVITON_NODE_TRANSPORT (transport), 0);
 *   g_object_unref (addr);
 *   graviton_discovery_method_node_found (GRAVITON_DISCOVERY_METHOD (self), node);
 *   graviton_discovery_method_finished (GRAVITON_DISCOVERY_METHOD (self));
 * }
 *
 * static void stop_browse (GravitonDiscoveryMethod *method)
 * {
 * }
 * 
 * static void graviton_discovery_method_init (GravitonDiscoveryMethod *self)
 * {
 * }
 *
 * static void graviton_discovery_method_class_init (GravitonDiscoveryMethodClass *klass)
 * {
 *   klass->start = start_browse;
 *   klass->stop = stop_browse;
 * }
 *
 * G_DEFINE_TYPE (GravitonStaticDiscoveryMethod, graviton_static_discovery_method, GRAVITON_DISCOVERY_METHOD_TYPE);
 *
 * int main(int argc, char** argv)
 * {
 *   GravitonCloud *cloud;
 *   GravitonNodeBrowser *browser;
 *   GravitonStaticDiscoveryMethod *method;
 *
 *   cloud = graviton_cloud_new_default_cloud ();
 *   g_object_get (cloud, "node-browser", &browser, NULL);
 *
 *   method = g_object_new (GRAVITON_STATIC_DISCOVERY_METHOD_TYPE, "node-browser", browser, NULL);
 *
 *   graviton_node_browser_add_discovery_method (browser, method);
 * }
 *
 *  </programlisting>
 *  </example>
 *
 * For convienence, Graviton provides a #GravitonJsonrpcNodeTransport class that
 * implements Graviton over JSON-RPC via HTTP, as demonstrated above. This
 * example does the following:
 *
 * - Grabs the default #GravitonCloud via graviton_cloud_new_default_cloud()
 * - Grabs the cloud's node browser and adds a custom
 *   GravitonStaticDiscoveryMethod to the list of methods used
 * 
 * Once the method is started via graviton_discovery_method_start(), it:
 *
 * - Creates a #GravitonJsonrpcNodeTransport that connects to localhost:8000
 * - Maps the transport to the node with ID 3857E91C-BA9F-4CB9-B667-4BBB42C06FC3
 * - Notifies Graviton that the node has been discovered via
 *   graviton_discovery_method_node_found()
 * - Notifies Graviton that the method has completed its discovery algorithm via
 *   graviton_discovery_method_finished()
 *
 */

typedef struct _GravitonDiscoveryMethodPrivate GravitonDiscoveryMethodPrivate;

struct _GravitonDiscoveryMethodPrivate
{
  GravitonNodeBrowser *browser;
  GList *discovered_nodes;
};

#define GRAVITON_DISCOVERY_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_DISCOVERY_METHOD_TYPE, GravitonDiscoveryMethodPrivate))

static void graviton_discovery_method_class_init (GravitonDiscoveryMethodClass *klass);
static void graviton_discovery_method_init       (GravitonDiscoveryMethod *self);
static void graviton_discovery_method_dispose    (GObject *object);
static void graviton_discovery_method_finalize   (GObject *object);
static void graviton_discovery_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_discovery_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonDiscoveryMethod, graviton_discovery_method, G_TYPE_OBJECT);

enum {
  PROP_ZERO,
  PROP_NODE_BROWSER,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  SIGNAL_NODE_FOUND,
  SIGNAL_NODE_LOST,
  SIGNAL_FINISHED,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
graviton_discovery_method_class_init (GravitonDiscoveryMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonDiscoveryMethodPrivate));

  object_class->dispose = graviton_discovery_method_dispose;
  object_class->finalize = graviton_discovery_method_finalize;
  object_class->set_property =  graviton_discovery_method_set_property;
  object_class->get_property =  graviton_discovery_method_get_property;

  obj_properties[PROP_NODE_BROWSER] =
    g_param_spec_object ("node-browser",
                         "node browser",
                         "GravitonNodeBrowser object",
                         GRAVITON_NODE_BROWSER_TYPE,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);

  obj_signals[SIGNAL_NODE_LOST] =
    g_signal_new ("node-lost",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GRAVITON_NODE_TYPE);
  obj_signals[SIGNAL_NODE_FOUND] =
    g_signal_new ("node-found",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GRAVITON_NODE_TYPE);
  obj_signals[SIGNAL_FINISHED] =
    g_signal_new ("finished",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0,
                  G_TYPE_NONE);

  klass->start = NULL;
  klass->stop = NULL;
  klass->setup_transport = NULL;
  klass->browse_cloud = NULL;
}

static void
cb_new_cloud (GravitonNodeBrowser *browser, GravitonCloud *cloud, GravitonDiscoveryMethod *method)
{
  if (GRAVITON_DISCOVERY_METHOD_GET_CLASS (method)->browse_cloud)
    GRAVITON_DISCOVERY_METHOD_GET_CLASS (method)->browse_cloud (method, cloud);
}

static void
cb_new_node (GravitonNodeBrowser *browser, GravitonNode *node, GravitonDiscoveryMethod *method)
{
  if (GRAVITON_DISCOVERY_METHOD_GET_CLASS (method)->setup_transport)
    GRAVITON_DISCOVERY_METHOD_GET_CLASS (method)->setup_transport (method, node);
}

static void
setup_browser (GravitonDiscoveryMethod *self, GravitonNodeBrowser *browser)
{
  if (self->priv->browser)
    g_object_unref (self->priv->browser);
  self->priv->browser = browser;
  g_signal_connect (browser,
                    "new-cloud",
                    G_CALLBACK (cb_new_cloud),
                    self);
  g_signal_connect (browser,
                    "node-found",
                    G_CALLBACK (cb_new_node),
                    self);
}

static void
graviton_discovery_method_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonDiscoveryMethod *self = GRAVITON_DISCOVERY_METHOD (object);
  switch (property_id) {
    case PROP_NODE_BROWSER:
      setup_browser (self, g_value_dup_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_discovery_method_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonDiscoveryMethod *self = GRAVITON_DISCOVERY_METHOD (object);
  switch (property_id) {
    case PROP_NODE_BROWSER:
      g_value_set_object (value, self->priv->browser);
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_discovery_method_init (GravitonDiscoveryMethod *self)
{
  self->priv = GRAVITON_DISCOVERY_METHOD_GET_PRIVATE (self);
}

static void
graviton_discovery_method_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_discovery_method_parent_class)->dispose (object);
  GravitonDiscoveryMethod *self = GRAVITON_DISCOVERY_METHOD (object);
  g_object_unref (self->priv->browser);
  self->priv->browser = NULL;
}

static void
graviton_discovery_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_discovery_method_parent_class)->finalize (object);
}

void
graviton_discovery_method_node_found (GravitonDiscoveryMethod *self,
                                      GravitonNode *node)
{
  self->priv->discovered_nodes = g_list_append (self->priv->discovered_nodes, node);
  g_debug ("Node found!");
  g_signal_emit (self, obj_signals[SIGNAL_NODE_FOUND], 0, node);
}

void
graviton_discovery_method_node_lost (GravitonDiscoveryMethod *self,
                                     GravitonNode *node)
{
  self->priv->discovered_nodes = g_list_remove (self->priv->discovered_nodes, node);
  g_debug ("Node lost!");
  g_signal_emit (self, obj_signals[SIGNAL_NODE_LOST], 0, node);
}

GList *
graviton_discovery_method_found_nodes (GravitonDiscoveryMethod *self)
{
  return self->priv->discovered_nodes;
}

void
graviton_discovery_method_finished (GravitonDiscoveryMethod *self)
{
  g_debug ("All nodes found!");
  g_signal_emit (self, obj_signals[SIGNAL_FINISHED], 0, NULL);
}

void
graviton_discovery_method_start (GravitonDiscoveryMethod *method)
{
  GRAVITON_DISCOVERY_METHOD_GET_CLASS (method)->start (method);
}

void
graviton_discovery_method_stop (GravitonDiscoveryMethod *method)
{
  GRAVITON_DISCOVERY_METHOD_GET_CLASS (method)->stop (method);
}

GravitonNodeBrowser *
graviton_discovery_method_get_browser (GravitonDiscoveryMethod *self)
{
  return g_object_ref (self->priv->browser);
}

/**
 * graviton_discovery_method_get_node_from_browser:
 * @method: The method
 * @node_id:  The node to fetch
 *
 * A convienence function that calls graviton_node_browser_get_node_by_id() on
 * this method's browser.
 */
GravitonNode *
graviton_discovery_method_get_node_from_browser (GravitonDiscoveryMethod *method, const gchar *node_id)
{
  return graviton_node_browser_get_node_by_id (method->priv->browser, node_id);
}
