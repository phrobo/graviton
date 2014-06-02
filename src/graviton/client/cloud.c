#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define G_LOG_DOMAIN "GravitonCloud"

#include "cloud.h"
#include "node.h"
#include <uuid/uuid.h>

typedef struct _GravitonCloudPrivate GravitonCloudPrivate;

struct _GravitonCloudPrivate
{
  gchar *cloud_id;
  GravitonNodeBrowser *browser;
};

#define GRAVITON_CLOUD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_CLOUD_TYPE, GravitonCloudPrivate))

static void graviton_cloud_class_init (GravitonCloudClass *klass);
static void graviton_cloud_init       (GravitonCloud *self);
static void graviton_cloud_dispose    (GObject *object);
static void graviton_cloud_finalize   (GObject *object);

void setup_browser (GravitonCloud *self, GravitonNodeBrowser *browser);

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
  PROP_NODE_BROWSER,
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
    case PROP_NODE_BROWSER:
      setup_browser (self, g_value_dup_object (value));
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
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  obj_properties [PROP_NODE_BROWSER] = 
    g_param_spec_object ("browser",
                         "GravitonNodeBrowser",
                         "The underlying GravitonNodeBrowser",
                         GRAVITON_NODE_BROWSER_TYPE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

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
  priv->cloud_id = NULL;
  priv->browser = NULL;
}

static void
graviton_cloud_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_cloud_parent_class)->dispose (object);
  GravitonCloud *self = GRAVITON_CLOUD (object);
  g_object_unref (self->priv->browser);
  self->priv->browser = NULL;
}

static void
graviton_cloud_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_cloud_parent_class)->finalize (object);
  GravitonCloud *self = GRAVITON_CLOUD (object);
  if (self->priv->cloud_id) {
    g_free (self->priv->cloud_id);
    self->priv->cloud_id = NULL;
  }
}

GravitonNode *
graviton_cloud_find_node (GravitonCloud *self, const gchar *guid, GError **error)
{
  g_error ("graviton_cloud_find_node_sync is not implemented");
}

GList *
graviton_cloud_find_service (GravitonCloud *self, const gchar *serviceName, GError **error)
{
  GList *cur = graviton_node_browser_get_found_nodes (self->priv->browser, self->priv->cloud_id);
  GList *ret = NULL;
  g_debug ("Browsing %s for %s", self->priv->cloud_id, serviceName);
  while (cur) {
    GError *error = NULL;
    if (graviton_node_has_service (cur->data, serviceName, &error)) {
      g_debug ("Hit!");
      GravitonServiceInterface *service = graviton_service_interface_get_subservice (GRAVITON_SERVICE_INTERFACE (cur->data), serviceName);
      ret = g_list_append (ret, service);
    } else {
      if (error) {
        g_debug ("Error while asking about service: %s", error->message);
      } else {
        g_debug ("Miss!");
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
graviton_cloud_new (const gchar *cloud_id, GravitonNodeBrowser *browser)
{
  return g_object_new (GRAVITON_CLOUD_TYPE, "cloud-id", cloud_id, "browser", browser, NULL);
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
  return graviton_node_browser_get_found_nodes (self->priv->browser, self->priv->cloud_id);
}

GravitonCloud *
graviton_cloud_new_default_cloud ()
{
  GravitonNodeBrowser *browser;
  GravitonCloud *cloud;
  GKeyFile *keyfile = g_key_file_new ();
  g_key_file_load_from_data_dirs (keyfile, "gravitonrc", NULL, G_KEY_FILE_KEEP_COMMENTS, NULL);
  gchar *cloud_id = g_key_file_get_string (keyfile, "graviton", "default-cloud-id", NULL);
  //FIXME: Need to store/load cloud ids
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
  browser = graviton_node_browser_new ();
  graviton_node_browser_load_discovery_plugins (browser);
  cloud = graviton_cloud_new (cloud_id, browser);
  g_free (cloud_id);
  g_object_unref (browser);
  return cloud;
}

const gchar *
graviton_cloud_get_cloud_id (GravitonCloud *self)
{
  return self->priv->cloud_id;
}

static void
cb_node_found (GravitonNodeBrowser *browser, GravitonNode *node, gpointer data)
{
  GravitonCloud *self = GRAVITON_CLOUD (data);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_FOUND], 0, node);
}

static void
cb_node_lost (GravitonNodeBrowser *browser, GravitonNode *node, gpointer data)
{
  GravitonCloud *self = GRAVITON_CLOUD (data);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_LOST], 0, node);
}

static void
cb_nodes_found (GravitonNodeBrowser *browser, gpointer data)
{
  GravitonCloud *self = GRAVITON_CLOUD (data);
  g_signal_emit (self, obj_signals[SIGNAL_ALL_NODES_FOUND], 0, NULL);
}

void
setup_browser (GravitonCloud *self, GravitonNodeBrowser *browser)
{
  self->priv->browser = browser;
  g_signal_connect (browser,
                    "node-found",
                    G_CALLBACK (cb_node_found),
                    self);
  g_signal_connect (browser,
                    "node-lost",
                    G_CALLBACK (cb_node_lost),
                    self);
  g_signal_connect (browser,
                    "all-nodes-found",
                    G_CALLBACK (cb_nodes_found),
                    self);
}
