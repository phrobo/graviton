#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define G_LOG_DOMAIN "GravitonNodeBrowser"

#include "node-browser.h"
#include "cloud.h"

typedef struct _GravitonNodeBrowserPrivate GravitonNodeBrowserPrivate;

struct _GravitonNodeBrowserPrivate
{
  GList *discovery_methods;
  int pending_discovery_methods;
  GHashTable *discovered_nodes;
  GAsyncQueue *unprobed_nodes;
  GThread *probe_thread;
};

#define GRAVITON_NODE_BROWSER_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_BROWSER_TYPE, GravitonNodeBrowserPrivate))

static void graviton_node_browser_class_init (GravitonNodeBrowserClass *klass);
static void graviton_node_browser_init       (GravitonNodeBrowser *self);
static void graviton_node_browser_dispose    (GObject *object);
static void graviton_node_browser_finalize   (GObject *object);
static void graviton_node_browser_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_node_browser_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonNodeBrowser, graviton_node_browser, G_TYPE_OBJECT);

enum {
  PROP_ZERO,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  SIGNAL_NODE_FOUND,
  SIGNAL_NODE_LOST,
  SIGNAL_ALL_NODES_FOUND,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

//static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
graviton_node_browser_class_init (GravitonNodeBrowserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNodeBrowserPrivate));

  object_class->dispose = graviton_node_browser_dispose;
  object_class->finalize = graviton_node_browser_finalize;
  object_class->set_property =  graviton_node_browser_set_property;
  object_class->get_property =  graviton_node_browser_get_property;

  /*g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);*/

  /**
   * GravitonNodeBrowser::node-found:
   * @browser: The browser that is reporting the found node
   * @node: The node that was found
   *
   * A #GravitonNode has appeared through a discovery method. No guarantee about
   * its reachability is made, which might be the case for i.e. a
   * file-based discovery method that reports a static list of nodes
   * the user has bookmarked or recently accessed.
   */
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
  /**
   * GravitonNodeBrowser::node-lost:
   * @borwser: The browser that is reporting the lost node
   * @node: The node that was lost
   *
   * A #GravitonNode has disappeared from view. It might still be reachable, but
   * a discovery method has reported it as gone.
   */
  //FIXME: Add method argument
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
  /**
   * GravitonNodeBrowser::all-nodes-found:
   * @browser: The browser
   *
   * All current discovery methods have finished their initial enumeration of
   * available nodes. This does not mean that they won't discover more later
   * while the methods are active.
   */
  obj_signals[SIGNAL_ALL_NODES_FOUND] =
    g_signal_new ("all-nodes-found",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0,
                  G_TYPE_NONE);
}

static void
graviton_node_browser_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  //GravitonNodeBrowser *self = GRAVITON_NODE_BROWSER (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_node_browser_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  //GravitonNodeBrowser *self = GRAVITON_NODE_BROWSER (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static gpointer
probe_thread_loop (gpointer data)
{
  GravitonNodeBrowser *self = GRAVITON_NODE_BROWSER (data);
  g_object_ref (self);
  while (TRUE) {
    g_debug ("Waiting for new nodes to probe");
    GravitonNode *cur = GRAVITON_NODE (g_async_queue_pop (self->priv->unprobed_nodes));
    g_debug ("Probing new node");
    if (cur == NULL) {
      g_debug ("Exiting probe loop");
      g_object_unref (self);
      return 0;
    } else {
      GList *nodes = NULL;
      GError *error = NULL;
      const gchar *cloud_id = NULL;
      cloud_id = graviton_node_get_cloud_id (cur, &error);
      g_debug ("Node belongs to cloud %s", cloud_id);
      nodes = g_hash_table_lookup (self->priv->discovered_nodes,
                                   cloud_id);
      nodes = g_list_prepend (nodes, cur);
      g_hash_table_replace (self->priv->discovered_nodes, g_strdup (cloud_id), nodes);
      //g_assert (g_hash_table_lookup (self->priv->discovered_nodes, cloud_id) == nodes);
      //g_assert (nodes->data == cur);
    }
  }
}

static void
free_node_list (gpointer key, gpointer value, gpointer data)
{
  GList *list = (GList*)value;
  g_list_free_full (list, g_object_unref);
}

static void
graviton_node_browser_init (GravitonNodeBrowser *self)
{
  GravitonNodeBrowserPrivate *priv;
  priv = self->priv = GRAVITON_NODE_BROWSER_GET_PRIVATE (self);
  priv->unprobed_nodes = g_async_queue_new_full (g_object_unref);
  priv->discovered_nodes = g_hash_table_new_full (g_str_hash,
                                                  g_str_equal,
                                                  g_free,
                                                  NULL);

  priv->probe_thread = g_thread_new (NULL, probe_thread_loop, self);
}

static void
graviton_node_browser_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_browser_parent_class)->dispose (object);
  GravitonNodeBrowser *self = GRAVITON_NODE_BROWSER (object);

  g_async_queue_unref (self->priv->unprobed_nodes);
  g_async_queue_push (self->priv->unprobed_nodes, NULL);
  self->priv->unprobed_nodes = NULL;

  g_thread_unref (self->priv->probe_thread);
  self->priv->probe_thread = NULL;

  g_hash_table_foreach (self->priv->discovered_nodes,
                       free_node_list,
                       NULL);
  g_hash_table_unref (self->priv->discovered_nodes);
  self->priv->discovered_nodes = NULL;

  g_list_free_full (self->priv->discovery_methods, g_object_unref);
  self->priv->discovery_methods = NULL;
}

static void
graviton_node_browser_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_browser_parent_class)->finalize (object);
}

static void
cb_node_found (GravitonDiscoveryMethod *method, GravitonNode *node, gpointer data)
{
  GravitonNodeBrowser *self = GRAVITON_NODE_BROWSER (data);
  g_debug ("Got a new node. Queueing it for bootstrap probe");
  g_async_queue_push (self->priv->unprobed_nodes, node);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_FOUND], 0, node);
}

static void
cb_node_lost (GravitonDiscoveryMethod *method, GravitonNode *node, gpointer data)
{
  GravitonNodeBrowser *self = GRAVITON_NODE_BROWSER (data);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_LOST], 0, node);
}

static void
cb_discovery_finished (GravitonDiscoveryMethod *method, gpointer data)
{
  GravitonNodeBrowser *self = GRAVITON_NODE_BROWSER (data);
  self->priv->pending_discovery_methods--;
  if (self->priv->pending_discovery_methods == 0)
    g_signal_emit (self, obj_signals[SIGNAL_ALL_NODES_FOUND], 0, NULL);
}


void
graviton_node_browser_add_discovery_method (GravitonNodeBrowser *self, GravitonDiscoveryMethod *method)
{
  g_object_ref_sink (method);
  self->priv->discovery_methods = g_list_append (self->priv->discovery_methods, method);
  g_signal_connect (method,
                    "finished",
                    G_CALLBACK (cb_discovery_finished),
                    self);
  g_signal_connect (method,
                    "node-found",
                    G_CALLBACK (cb_node_found),
                    self);
  g_signal_connect (method,
                    "node-lost",
                    G_CALLBACK (cb_node_lost),
                    self);
  self->priv->pending_discovery_methods++;
}

void
graviton_node_browser_load_discovery_plugins (GravitonNodeBrowser *self)
{
  int i;
  GArray *plugins;

  plugins = graviton_node_browser_find_discovery_plugins (self);
  for (i = 0; i < plugins->len; i++) {
    GravitonDiscoveryPluginLoaderFunc factory = g_array_index (plugins, GravitonDiscoveryPluginLoaderFunc, i);
    GravitonDiscoveryMethod *method = factory(self);
    graviton_node_browser_add_discovery_method (self, method);
    g_object_unref (method);
    graviton_discovery_method_start (method);
  }
  g_array_unref (plugins);
  g_debug ("loaded %i plugins", i);
}

GArray*
graviton_node_browser_find_discovery_plugins (GravitonNodeBrowser *self)
{
  GArray *pluginList = g_array_new(FALSE, FALSE, sizeof (GravitonDiscoveryPluginLoaderFunc));
  const gchar *pluginPath = g_getenv("GRAVITON_DISCOVERY_PLUGIN_PATH");
  if (!pluginPath)
    pluginPath = GRAVITON_DEFAULT_DISCOVERY_PLUGIN_PATH;
  g_debug ("Searching %s for plugins\n", pluginPath);
  GDir *pluginDir = g_dir_open (pluginPath, 0, NULL);
  if (!pluginDir) {
    g_debug ("Plugin path not found: %s", pluginPath);
    return pluginList;
  }
  const gchar *entry = g_dir_read_name (pluginDir);
  while (entry) {
    gchar *entryPath = g_build_path ("/", pluginPath, entry, NULL);
    if (!g_str_has_suffix (entryPath, ".so"))
      goto nextPlugin;
    g_debug ("Attempting to load plugin %s", entryPath);
    GModule *module = g_module_open (entryPath, G_MODULE_BIND_LOCAL);
    GravitonDiscoveryPluginLoaderFunc loader = NULL;
    if (!module) {
      g_warning ("Can't open plugin %s: %s", entryPath, g_module_error ());
      goto nextPlugin;
    }

    if (!g_module_symbol (module, "make_graviton_discovery_plugin", (gpointer *)&loader)) {
      g_warning ("Can't find graviton_plugin symbol in %s: %s", entryPath, g_module_error ());
      goto badPlugin;
    }

    if (loader == NULL) {
      g_warning ("graviton_plugin symbol is NULL in %s: %s", entryPath, g_module_error ());
      goto badPlugin;
    }

    g_module_make_resident (module);
    g_array_append_val (pluginList, loader);

    goto nextPlugin;

badPlugin:
    g_module_close (module);

nextPlugin:
    g_free (entryPath);
    entry = g_dir_read_name (pluginDir);
  }

  g_dir_close (pluginDir);

  return pluginList;
}

GravitonNodeBrowser *
graviton_node_browser_new ()
{
  return g_object_new (GRAVITON_NODE_BROWSER_TYPE, NULL);
}

GList*
graviton_node_browser_get_found_nodes (GravitonNodeBrowser *self, const gchar* cloud_id)
{
  return g_hash_table_lookup (self->priv->discovered_nodes, cloud_id);
}

GList *
graviton_node_browser_get_found_cloud_ids (GravitonNodeBrowser *client)
{
  return g_hash_table_get_keys (client->priv->discovered_nodes);
}
