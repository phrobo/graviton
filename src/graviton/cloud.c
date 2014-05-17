#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cloud.h"
#include "node.h"
#include <uuid/uuid.h>

typedef struct _GravitonCloudPrivate GravitonCloudPrivate;

struct _GravitonCloudPrivate
{
  GList *discovery_methods;
  GList *discovered_nodes;
  int pending_discovery_methods;
  const gchar *cloud_id;
};

#define GRAVITON_CLOUD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_CLOUD_TYPE, GravitonCloudPrivate))

static void graviton_cloud_class_init (GravitonCloudClass *klass);
static void graviton_cloud_init       (GravitonCloud *self);
static void graviton_cloud_dispose    (GObject *object);
static void graviton_cloud_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonCloud, graviton_cloud, G_TYPE_OBJECT);

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
  GravitonCloud *self = GRAVITON_CLOUD (object);

  switch (property_id) {
    case PROP_CLOUD_ID:
      self->priv->cloud_id = g_value_dup_string (value);
      g_debug ("Got new cloud id: %s", self->priv->cloud_id);
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
  GravitonCloud *self = GRAVITON_CLOUD (object);


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
graviton_cloud_class_init (GravitonCloudClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonCloudPrivate));

  object_class->dispose = graviton_cloud_dispose;
  object_class->finalize = graviton_cloud_finalize;

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
   * GravitonCloud::node-found:
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
   * GravitonCloud::node-lost:
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
graviton_cloud_init (GravitonCloud *self)
{
  GravitonCloudPrivate *priv;
  self->priv = priv = GRAVITON_CLOUD_GET_PRIVATE (self);
}

static void
graviton_cloud_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_cloud_parent_class)->dispose (object);
}

static void
graviton_cloud_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_cloud_parent_class)->finalize (object);
}

static void
cb_node_found (GravitonDiscoveryMethod *method, GravitonNode *node, gpointer data)
{
  GravitonCloud *self = GRAVITON_CLOUD (data);
  self->priv->discovered_nodes = g_list_append (self->priv->discovered_nodes, node);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_FOUND], 0, node);
}

static void
cb_node_lost (GravitonDiscoveryMethod *method, GravitonNode *node, gpointer data)
{
  GravitonCloud *self = GRAVITON_CLOUD (data);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_LOST], 0, node);
}

static void
cb_discovery_finished (GravitonDiscoveryMethod *method, gpointer data)
{
  GravitonCloud *self = GRAVITON_CLOUD (data);
  self->priv->pending_discovery_methods--;
  if (self->priv->pending_discovery_methods == 0)
    g_signal_emit (self, obj_signals[SIGNAL_ALL_NODES_FOUND], 0, NULL);
}

GravitonNode *
graviton_cloud_find_node_sync (GravitonCloud *self, const gchar *guid, GError **error)
{
  GList *cur = self->priv->discovered_nodes;
  while (cur) {
    const gchar *node_id = graviton_node_get_id (cur->data, NULL);
    if (strcmp(node_id, guid) == 0) {

    }
    cur = cur->next;
  }
}

static void
cb_sync_node_discovery (GravitonCloud *self, GMainLoop *loop)
{
  g_main_loop_quit (loop);
}

GList *
graviton_cloud_find_service_sync (GravitonCloud *self, const gchar *serviceName, GError **error)
{
  if (!self->priv->discovered_nodes) {
    g_debug ("Loading discovery plugins");
    graviton_cloud_load_discovery_plugins (self);
    g_debug ("Waiting for all-nodes-found...");
    GMainLoop *loop = g_main_loop_new (NULL, FALSE);
    g_signal_connect (self,
        "all-nodes-found",
        G_CALLBACK (cb_sync_node_discovery),
        loop);
    g_main_loop_run (loop);
    g_signal_handlers_disconnect_by_data (self, loop);
    g_main_loop_unref (loop);
    g_debug ("All nodes found!");
  }
  GList *cur = self->priv->discovered_nodes;
  GList *ret = NULL;
  g_debug ("Browsing %s for %s", self->priv->cloud_id, serviceName);
  while (cur) {
    GError *error = NULL;
    const gchar *cloud_id = graviton_node_get_cloud_id (cur->data, &error);
    if (error) {
      g_debug ("Error while querying for cloud id: %s", error->message);
    } else {
      const gchar *node_id = graviton_node_get_id (cur->data, &error);
      if (error) {
        g_debug ("Error while querying for node id: %s", error->message);
      } else {
        g_debug ("Found member of %s: %s", cloud_id, node_id);
        //if (strcmp(cloud_id, self->priv->cloud_id) == 0) {
        if (TRUE) {
          if (graviton_node_has_service (cur->data, serviceName, &error)) {
            g_debug ("Hit!");
            GravitonServiceInterface *service = graviton_service_interface_get_subservice (GRAVITON_SERVICE_INTERFACE (cur->data), serviceName);
            ret = g_list_append (ret, service);
          } else {
            if (error) {
              g_debug ("Error while asking about service: %s", error->message);
            }
          }
        }
      }
    }
    cur = cur->next;
  }

  return ret;
}

/**
 * graviton_cloud_new: Creates a new #GravitonCloud
 *
 */
GravitonCloud *
graviton_cloud_new (const gchar *cloud_id)
{
  return g_object_new (GRAVITON_CLOUD_TYPE, "cloud-id", cloud_id, NULL);
}

/**
 * graviton_cloud_get_found_nodes:
 * @client: a #GravitonCloud to query
 *
 * Returns: (transfer none): List of nodes that have been discovered in this
 * client's lifetime. No guarantees are made about their availability.
 */
GList *
graviton_cloud_get_found_nodes (GravitonCloud *self)
{
  return self->priv->discovered_nodes;
}

void
graviton_cloud_add_discovery_method (GravitonCloud *self, GravitonDiscoveryMethod *method)
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
graviton_cloud_load_discovery_plugins (GravitonCloud *self)
{
  int i;
  GArray *plugins;

  plugins = graviton_cloud_find_discovery_plugins (self);
  for (i = 0; i < plugins->len; i++) {
    GravitonDiscoveryPluginLoaderFunc factory = g_array_index (plugins, GravitonDiscoveryPluginLoaderFunc, i);
    GravitonDiscoveryMethod *method = factory(self);
    graviton_cloud_add_discovery_method (self, method);
    graviton_discovery_method_start (method);
  }
  g_debug ("loaded %i plugins", i);
}

GArray*
graviton_cloud_find_discovery_plugins (GravitonCloud *self)
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

GravitonCloud *
graviton_cloud_new_default_cloud ()
{
  GKeyFile *keyfile = g_key_file_new ();
  g_key_file_load_from_data_dirs (keyfile, "gravitonrc", NULL, G_KEY_FILE_KEEP_COMMENTS, NULL);
  gchar *cloud_id = g_key_file_get_string (keyfile, "graviton", "default-cloud-id", NULL);
  cloud_id = g_strdup ("3857E91C-BA9F-4CB9-B667-4BBB42C06FC3");
  if (cloud_id == NULL) {
    cloud_id = g_new0 (gchar, 37);
    uuid_t uuid;
    uuid_generate (uuid);
    uuid_unparse_upper (uuid, cloud_id);
    g_key_file_set_string (keyfile, "graviton", "default-cloud-id", cloud_id);
    g_debug ("Generated new default cloud id %s", cloud_id);
  }

  g_key_file_unref (keyfile);
  GravitonCloud *client = graviton_cloud_new (cloud_id);
  g_free (cloud_id);
  return client;
}

const gchar *
graviton_cloud_get_cloud_id (GravitonCloud *self)
{
  return self->priv->cloud_id;
}
