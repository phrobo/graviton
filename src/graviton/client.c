#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "client.h"
#include "node.h"
#include <uuid/uuid.h>

typedef struct _GravitonClientPrivate GravitonClientPrivate;

struct _GravitonClientPrivate
{
  GList *discovery_methods;
  GList *discovered_nodes;
  int pending_discovery_methods;
  const gchar *cloud_id;
};

#define GRAVITON_CLIENT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_CLIENT_TYPE, GravitonClientPrivate))

static void graviton_client_class_init (GravitonClientClass *klass);
static void graviton_client_init       (GravitonClient *self);
static void graviton_client_dispose    (GObject *object);
static void graviton_client_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonClient, graviton_client, G_TYPE_OBJECT);

enum {
  SIGNAL_0,
  SIGNAL_NODE_FOUND,
  SIGNAL_NODE_LOST,
  SIGNAL_ALL_NODES_FOUND,
  N_SIGNALS
};

enum {
  PROP_0,
  PROP_CLOUD_ID,
  N_PROPERTIES
};

static int obj_signals[N_SIGNALS] = {0, };

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
set_property (GObject *object,
              guint property_id,
              const GValue *value,
              GParamSpec *pspec)
{
  GravitonClient *self = GRAVITON_CLIENT (object);

  switch (property_id) {
    case PROP_CLOUD_ID:
      self->priv->cloud_id = g_value_get_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
get_property (GObject *object,
              guint property_id,
              GValue *value,
              GParamSpec *pspec)
{
  GravitonClient *self = GRAVITON_CLIENT (object);


  switch (property_id) {
    case PROP_CLOUD_ID:
      g_value_set_string (value, self->priv->cloud_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_client_class_init (GravitonClientClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonClientPrivate));

  object_class->dispose = graviton_client_dispose;
  object_class->finalize = graviton_client_finalize;

  object_class->set_property = set_property;
  object_class->get_property = get_property;

  obj_properties[PROP_CLOUD_ID] =
    g_param_spec_string ("cloud-id",
                         "Cloud UUID",
                         "Universally Unique Cloud ID",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);

  /**
   * GravitonClient::node-found:
   * @client: The client that is reporting the found node
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
   * GravitonClient::node-lost:
   * @client: The client that is reporting the lost node
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
   * GravitonNode::all-nodes-found:
   * @client: The client
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
graviton_client_init (GravitonClient *self)
{
  GravitonClientPrivate *priv;
  self->priv = priv = GRAVITON_CLIENT_GET_PRIVATE (self);
}

static void
graviton_client_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_client_parent_class)->dispose (object);
}

static void
graviton_client_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_client_parent_class)->finalize (object);
}

static void
cb_node_found (GravitonDiscoveryMethod *method, GravitonNode *node, gpointer data)
{
  GravitonClient *self = GRAVITON_CLIENT (data);
  self->priv->discovered_nodes = g_list_append (self->priv->discovered_nodes, node);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_FOUND], 0, node);
}

static void
cb_node_lost (GravitonDiscoveryMethod *method, GravitonNode *node, gpointer data)
{
  GravitonClient *self = GRAVITON_CLIENT (data);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_LOST], 0, node);
}

static void
cb_discovery_finished (GravitonDiscoveryMethod *method, gpointer data)
{
  GravitonClient *self = GRAVITON_CLIENT (data);
  self->priv->pending_discovery_methods--;
  if (self->priv->pending_discovery_methods == 0)
    g_signal_emit (self, obj_signals[SIGNAL_ALL_NODES_FOUND], 0, NULL);
}

GravitonNode *
graviton_client_find_node_sync (GravitonClient *self, const gchar *guid, GError **error)
{
  GList *cur = self->priv->discovered_nodes;
  while (cur) {
    const gchar *node_id = graviton_node_get_id (cur->data, NULL);
    if (strcmp(node_id, guid) == 0) {

    }
    cur = cur->next;
  }
}

GList *
graviton_client_find_service_sync (GravitonClient *self, const gchar *controlName, GError **error)
{
  GList *cur = self->priv->discovered_nodes;
  while (cur) {
    const gchar *node_id = graviton_node_get_id (cur->data, NULL);
    if (strcmp(node_id, self->priv->cloud_id) == 0) {

    }
    cur = cur->next;
  }
}

/**
 * graviton_client_new: Creates a new #GravitonClient
 *
 */
GravitonClient *
graviton_client_new (const gchar *cloud_id)
{
  return g_object_new (GRAVITON_CLIENT_TYPE, "cloud-id", cloud_id, NULL);
}

/**
 * graviton_client_get_found_nodes:
 * @client: a #GravitonClient to query
 *
 * Returns: (transfer none): List of nodes that have been discovered in this
 * client's lifetime. No guarantees are made about their availability.
 */
GList *
graviton_client_get_found_nodes (GravitonClient *self)
{
  return self->priv->discovered_nodes;
}

void
graviton_client_add_discovery_method (GravitonClient *self, GravitonDiscoveryMethod *method)
{
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
graviton_client_load_discovery_plugins (GravitonClient *self)
{
  int i;
  GArray *plugins;

  plugins = graviton_client_find_discovery_plugins (self);
  for (i = 0; i < plugins->len; i++) {
    GravitonDiscoveryPluginLoaderFunc factory = g_array_index (plugins, GravitonDiscoveryPluginLoaderFunc, i);
    GravitonDiscoveryMethod *method = factory(self);
    graviton_client_add_discovery_method (self, method);
    graviton_discovery_method_start (method);
  }
  g_debug ("loaded %i plugins", i);
}

GArray*
graviton_client_find_discovery_plugins (GravitonClient *self)
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
      goto nextPlugin;
    }

    if (loader == NULL) {
      g_warning ("graviton_plugin symbol is NULL in %s: %s", entryPath, g_module_error ());
      goto nextPlugin;
    }

    g_array_append_val (pluginList, loader);

nextPlugin:
    g_free (entryPath);
    entry = g_dir_read_name (pluginDir);
  }

  g_dir_close (pluginDir);

  return pluginList;
}

GravitonClient *
graviton_client_new_default_cloud ()
{
  GKeyFile *keyfile = g_key_file_new ();
  g_key_file_load_from_data_dirs (keyfile, "gravitonrc", NULL, G_KEY_FILE_KEEP_COMMENTS, NULL);
  gchar *cloud_id = g_key_file_get_string (keyfile, "graviton", "default-cloud-id", NULL);
  if (cloud_id == NULL) {
    cloud_id = g_new0 (gchar, 37);
    uuid_t uuid;
    uuid_generate (uuid);
    uuid_unparse_upper (uuid, cloud_id);
    g_key_file_set_string (keyfile, "graviton", "default-cloud-id", cloud_id);
  }

  g_key_file_unref (keyfile);
  GravitonClient *client = graviton_client_new (cloud_id);
  g_free (cloud_id);
  return client;
}
