/*
 * This file is part of Graviton.
 * Copyright (C) 2013-2014 Torrie Fischer <tdfischer@phrobo.net>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define G_LOG_DOMAIN "GravitonNodeBrowser"

#include "cloud.h"
#include "node-browser.h"

typedef struct _GravitonNodeBrowserPrivate GravitonNodeBrowserPrivate;

struct _GravitonNodeBrowserPrivate
{
  GList *discovery_methods;
  int pending_discovery_methods;
  GHashTable *discovered_nodes;
  GHashTable *cloud_nodes;
  GAsyncQueue *unprobed_nodes;
  GThread *probe_thread;
};

#define GRAVITON_NODE_BROWSER_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_BROWSER_TYPE, \
                                GravitonNodeBrowserPrivate))

static void graviton_node_browser_class_init (GravitonNodeBrowserClass *klass);
static void graviton_node_browser_init       (GravitonNodeBrowser *self);
static void graviton_node_browser_dispose    (GObject *object);
static void graviton_node_browser_finalize   (GObject *object);
static void graviton_node_browser_set_property (GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec);
static void graviton_node_browser_get_property (GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec);

static void notify_new_cloud (GravitonNodeBrowser *browser,
                              GravitonCloud *cloud);

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
  SIGNAL_NEW_CLOUD,
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
   *   N_PROPERTIES,
   *   obj_properties);*/

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
  /**
   * GravitonNodeBrowser::new-cloud:
   * @browser: The browser
   * @cloud_id: Cloud ID
   *
   * A new cloud has been discovered.
   *
   */
  obj_signals[SIGNAL_NEW_CLOUD] =
    g_signal_new ("new-cloud",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GRAVITON_CLOUD_TYPE);
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
    GravitonNode *cur =
      GRAVITON_NODE (g_async_queue_pop (self->priv->unprobed_nodes));
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
      nodes = g_hash_table_lookup (self->priv->cloud_nodes,
                                   cloud_id);
      if (!g_list_find (nodes, cur)) {
        nodes = g_list_prepend (nodes, cur);
        g_hash_table_replace (self->priv->cloud_nodes, g_strdup (
                                cloud_id), nodes);

        g_signal_emit (self, obj_signals[SIGNAL_NODE_FOUND], 0, cur);
      }

      if (g_async_queue_length (self->priv->unprobed_nodes) <= 0 &&
          g_atomic_int_get (&self->priv->pending_discovery_methods) == 0) {
        g_debug (
          "All backends are finished, and we've probed all bootstrapped nodes");
        g_signal_emit (self, obj_signals[SIGNAL_ALL_NODES_FOUND], 0, NULL);
      } else {
        g_debug ("Still waiting on %i nodes and %i methods",
                 g_async_queue_length (self->priv->unprobed_nodes),
                 g_atomic_int_get (&self->priv->pending_discovery_methods));
      }
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
  priv->cloud_nodes = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             NULL);
  priv->pending_discovery_methods = 0;

  priv->probe_thread = g_thread_new (NULL, probe_thread_loop, self);
  priv->discovered_nodes = g_hash_table_new_full (g_str_hash,
                                                  g_str_equal,
                                                  g_free,
                                                  g_object_unref);
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

  g_hash_table_foreach (self->priv->cloud_nodes,
                        free_node_list,
                        NULL);
  g_hash_table_unref (self->priv->cloud_nodes);
  self->priv->cloud_nodes = NULL;

  g_list_free_full (self->priv->discovery_methods, g_object_unref);
  self->priv->discovery_methods = NULL;

  g_hash_table_ref (self->priv->discovered_nodes);
}

static void
graviton_node_browser_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_browser_parent_class)->finalize (object);
}

static void
cb_node_lost (GravitonDiscoveryMethod *method, GravitonNode *node,
              gpointer data)
{
  GravitonNodeBrowser *self = GRAVITON_NODE_BROWSER (data);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_LOST], 0, node);
}

static void
cb_discovery_finished (GravitonDiscoveryMethod *method, gpointer data)
{
  GravitonNodeBrowser *self = GRAVITON_NODE_BROWSER (data);
  g_atomic_int_dec_and_test (&self->priv->pending_discovery_methods);
  if (g_async_queue_length (self->priv->unprobed_nodes) <= 0 &&
      g_atomic_int_get (&self->priv->pending_discovery_methods) == 0) {
    g_debug (
      "All backends are finished, and we've probed all bootstrapped nodes");
    g_signal_emit (self, obj_signals[SIGNAL_ALL_NODES_FOUND], 0, NULL);
  } else {
    g_debug ("Still waiting on %i nodes and %i methods",
             g_async_queue_length (self->priv->unprobed_nodes),
             g_atomic_int_get (&self->priv->pending_discovery_methods));
  }
}

void
graviton_node_browser_add_discovery_method (GravitonNodeBrowser *self,
                                            GravitonDiscoveryMethod *method)
{
  g_object_ref_sink (method);
  g_atomic_int_inc (&self->priv->pending_discovery_methods);
  self->priv->discovery_methods = g_list_append (self->priv->discovery_methods,
                                                 method);
  g_signal_connect (method,
                    "finished",
                    G_CALLBACK (cb_discovery_finished),
                    self);
  g_signal_connect (method,
                    "node-lost",
                    G_CALLBACK (cb_node_lost),
                    self);
}

void
graviton_node_browser_load_discovery_plugins (GravitonNodeBrowser *self)
{
  int i;
  GArray *plugins;

  plugins = graviton_node_browser_find_discovery_plugins (self);
  for (i = 0; i < plugins->len; i++) {
    GravitonDiscoveryPluginLoaderFunc factory = g_array_index (plugins,
                                                               GravitonDiscoveryPluginLoaderFunc,
                                                               i);
    GravitonDiscoveryMethod *method = factory(self);
    graviton_node_browser_add_discovery_method (self, method);
    g_object_unref (method);
    graviton_discovery_method_start (method);
  }
  g_array_unref (plugins);
  g_debug ("loaded %i discovery plugins", i);
}

GArray*
graviton_node_browser_find_discovery_plugins (GravitonNodeBrowser *self)
{
  GArray *plugin_list =
    g_array_new(FALSE, FALSE, sizeof (GravitonDiscoveryPluginLoaderFunc));
  const gchar *plugin_path = g_getenv("GRAVITON_DISCOVERY_PLUGIN_PATH");
  if (!plugin_path)
    plugin_path = GRAVITON_DEFAULT_DISCOVERY_PLUGIN_PATH;
  g_debug ("Searching %s for plugins\n", plugin_path);
  GDir *plugin_dir = g_dir_open (plugin_path, 0, NULL);
  if (!plugin_dir) {
    g_debug ("Plugin path not found: %s", plugin_path);
    return plugin_list;
  }
  const gchar *entry = g_dir_read_name (plugin_dir);
  while (entry) {
    gchar *entry_path = g_build_path ("/", plugin_path, entry, NULL);
    if (!g_str_has_suffix (entry_path, ".so"))
      goto next_plugin;
    g_debug ("Attempting to load plugin %s", entry_path);
    GModule *module = g_module_open (entry_path, G_MODULE_BIND_LOCAL);
    GravitonDiscoveryPluginLoaderFunc loader = NULL;
    if (!module) {
      g_warning ("Can't open plugin %s: %s", entry_path, g_module_error ());
      goto next_plugin;
    }

    if (!g_module_symbol (module, "make_graviton_discovery_plugin",
                          (gpointer *)&loader)) {
      g_warning ("Can't find graviton_plugin symbol in %s: %s",
                 entry_path,
                 g_module_error ());
      goto bad_plugin;
    }

    if (loader == NULL) {
      g_warning ("graviton_plugin symbol is NULL in %s: %s",
                 entry_path,
                 g_module_error ());
      goto bad_plugin;
    }

    g_module_make_resident (module);
    g_array_append_val (plugin_list, loader);

    goto next_plugin;

bad_plugin:
    g_module_close (module);

next_plugin:
    g_free (entry_path);
    entry = g_dir_read_name (plugin_dir);
  }

  g_dir_close (plugin_dir);

  return plugin_list;
}

GravitonNodeBrowser *
graviton_node_browser_new ()
{
  return g_object_new (GRAVITON_NODE_BROWSER_TYPE, NULL);
}

GList*
graviton_node_browser_get_found_nodes (GravitonNodeBrowser *self,
                                       const gchar*cloud_id)
{
  return g_hash_table_lookup (self->priv->cloud_nodes, cloud_id);
}

GList *
graviton_node_browser_get_found_cloud_ids (GravitonNodeBrowser *client)
{
  return g_hash_table_get_keys (client->priv->cloud_nodes);
}

static void
notify_new_cloud (GravitonNodeBrowser *browser, GravitonCloud *cloud)
{
  const gchar*cloud_id = graviton_cloud_get_cloud_id (cloud);

  if (!g_hash_table_contains (browser->priv->cloud_nodes, cloud_id)) {
    g_hash_table_insert (browser->priv->cloud_nodes, g_strdup (cloud_id), NULL);
    g_signal_emit (browser, obj_signals[SIGNAL_NEW_CLOUD], 0, cloud);
  }
}

/**
 * graviton_cloud_new:
 * @cloud_id: The cloud ID requested
 * @browser: A #GravitonNodeBrowser that will be used to discover nodes and
 * services
 *
 * Creates a new #GravitonCloud
 *
 * Returns: (transfer full): A new #GravitonCloud
 */
GravitonCloud *
graviton_node_browser_get_cloud (GravitonNodeBrowser *browser,
                                 const gchar *cloud_id)
{
  GravitonCloud *cloud;

  cloud = g_object_new (GRAVITON_CLOUD_TYPE,
                        "cloud-id",
                        cloud_id,
                        "browser",
                        browser,
                        NULL);
  notify_new_cloud (browser, cloud);
  return cloud;
}

static void
cb_transport_added (GravitonNode *node,
                    GravitonNodeTransport *transport,
                    GravitonNodeBrowser *self)
{
  g_debug ("Got a new node. Queueing it for bootstrap probe");
  g_async_queue_push (self->priv->unprobed_nodes, node);
}

GravitonNode *
graviton_node_browser_get_node_by_id (GravitonNodeBrowser *browser,
                                      const gchar *node_id)
{
  GravitonNode *result = NULL;
  result = g_hash_table_lookup (browser->priv->discovered_nodes, node_id);
  if (result == NULL) {
    result = g_object_new (GRAVITON_NODE_TYPE,
                           "node-id", node_id,
                           NULL);

    g_signal_connect (result,
                      "transport-added",
                      G_CALLBACK (cb_transport_added),
                      browser);

    g_hash_table_insert (browser->priv->discovered_nodes, g_strdup (
                           node_id), result);
    g_debug ("Created new node for %s", node_id);
  } else {
    g_debug ("Returning existing node for %s", node_id);
    g_object_ref (result);
  }

  return result;
}
