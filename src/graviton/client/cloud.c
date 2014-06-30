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

#define G_LOG_DOMAIN "GravitonCloud"

#include "cloud.h"
#include "node-browser.h"
#include "node.h"
#include <uuid/uuid.h>

#include <graviton/common/configuration.h>

/**
 * SECTION:cloud
 * @short_description: Core interface to discover services and nodes within a
 *    Graviton cloud
 * @title: GravitonCloud
 * @see_also: #GravitonServiceInterface, #GravitonServer, #GravitonNodeBrowser
 * @stability: Unstable
 * @include: graviton/client/cloud.h
 *
 * The GravitonCloud class is the central interface to discovering services and,
 * to a lesser extent, individual nodes within a Graviton cloud.
 *
 * Conceptually speaking, a Graviton cloud is a set of all #GravitonNode objects
 *that
 * share the same cloud ID.
 *
 * The fastest way to get started is to grab the default cloud as configured by
 * the user, set up a callback, and start poking services:
 *
 * <example>
 *   <title>Grabbing the default cloud and browsing for services</title>
 *   <programlisting>
 * #include <stdlib.h>
 * #include <graviton/client/cloud.h>
 * #include <graviton/client/service-interface.h>
 *
 * static void cb_browse (GravitonCloud *cloud,
 *                        GravitonServiceEvent event,
 *                        GravitonServiceInterface *iface,
 *                        gpointer user_data)
 * {
 *   switch (event) {
 *     case GRAVITON_SERVICE_NEW:
 *       graviton_service_interface_call_noref (iface, "ping", NULL, NULL);
 *       break;
 *     case GRAVITON_SERVICE_ALL_FOR_NOW:
 *       exit(0);
 *   }
 * }
 *
 * int main(int argc, char** argv)
 * {
 *   GMainLoop *loop;
 *   GravitonCloud *cloud;
 *
 *   loop = g_main_loop_new (NULL, 0);
 *   cloud = graviton_cloud_new_default_cloud ();
 *
 *   graviton_cloud_browse_services (cloud, "net:phrobo:graviton:ping",
 *cb_browse, NULL);
 *   g_main_loop_run (loop);
 * }
 *   </programlisting>
 * </example>
 *
 * This example does the following:
 *
 * - Creates a #GMainLoop which is essential to GLib event processing
 * - Fetches the default #GravitonCloud
 * - Registers the cb_browse callback to be triggered while searching for
 *   net:phrobo:graviton:ping services in the cloud
 * - Runs the #GMainLoop
 *
 * Once the loop is running, the following steps happen behind the scenes:
 *
 * - The cloud's #GravitonNodeBrowser begins loading #GravitonDiscoveryMethod
 *   plugins, which are then started
 * - As nodes are discovered from the discovery plugins, the callback is
 *   triggered with the #GravitonServiceEvent.GRAVITON_SERVICE_NEW event
 * - Each net:phrobo:graviton:ping service on the network has its "ping" method
 *   called
 * - Once all of the plugins have determined that there are no more
 *   undiscovered nodes, the callback is triggered with
 *   #GravitonServiceEvent.GRAVITON_SERVICE_ALL_FOR_NOW
 */
typedef struct _GravitonCloudPrivate GravitonCloudPrivate;

typedef struct _GravitonServiceBrowser {
  GravitonServiceBrowseCallback callback;
  gpointer data;
  gchar *name;
} GravitonServiceBrowser;

struct _GravitonCloudPrivate
{
  gchar *cloud_id;
  GravitonNodeBrowser *browser;
  gboolean all_nodes_found_for_now;
  GHashTable *browse_callbacks;
  GMutex browse_lock;
};

#define GRAVITON_CLOUD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_CLOUD_TYPE, GravitonCloudPrivate))

static void graviton_cloud_class_init (GravitonCloudClass *klass);
static void graviton_cloud_init       (GravitonCloud *self);
static void graviton_cloud_dispose    (GObject *object);
static void graviton_cloud_finalize   (GObject *object);

static void notify_all_service_callbacks_by_iface (GravitonCloud *cloud,
                                                   GravitonServiceEvent event,
                                                   GravitonServiceInterface *iface);
static void notify_all_service_callbacks_by_node (GravitonCloud *cloud,
                                                  GravitonServiceEvent event,
                                                  GravitonNode *node);
static void notify_all_service_callbacks (GravitonCloud *cloud,
                                          GravitonServiceEvent event);
static void notify_service_browser (GravitonCloud *cloud,
                                    GravitonServiceBrowser *browser,
                                    GravitonServiceEvent event,
                                    GravitonServiceInterface *iface);
static void free_browser (GravitonServiceBrowser *data);

static void setup_browser (GravitonCloud *self, GravitonNodeBrowser *browser);

G_DEFINE_TYPE (GravitonCloud, graviton_cloud, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_CLOUD_ID,
  PROP_NODE_BROWSER,
  N_PROPERTIES
};

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
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                         G_PARAM_STATIC_STRINGS);

  obj_properties [PROP_NODE_BROWSER] =
    g_param_spec_object ("browser",
                         "GravitonNodeBrowser",
                         "The underlying GravitonNodeBrowser",
                         GRAVITON_NODE_BROWSER_TYPE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
graviton_cloud_init (GravitonCloud *self)
{
  GravitonCloudPrivate *priv;
  self->priv = priv = GRAVITON_CLOUD_GET_PRIVATE (self);
  g_mutex_init (&priv->browse_lock);
  priv->cloud_id = NULL;
  priv->browser = NULL;
  priv->browse_callbacks = g_hash_table_new_full(g_str_hash,
                                                 g_str_equal,
                                                 g_free,
                                                 NULL);
  priv->all_nodes_found_for_now = FALSE;
}

static void
graviton_cloud_dispose (GObject *object)
{
  GHashTableIter iter;
  GravitonCloud *self;
  GList *cur;
  self = GRAVITON_CLOUD (object);
  g_object_unref (self->priv->browser);
  self->priv->browser = NULL;

  g_mutex_lock (&self->priv->browse_lock);
  g_hash_table_iter_init (&iter, self->priv->browse_callbacks);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer*)&cur)) {
    g_list_free_full (cur, (GDestroyNotify)free_browser);
  }
  g_hash_table_unref (self->priv->browse_callbacks);
  g_mutex_unlock (&self->priv->browse_lock);

  G_OBJECT_CLASS (graviton_cloud_parent_class)->dispose (object);
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
  g_mutex_clear (&self->priv->browse_lock);
}

GravitonNode *
graviton_cloud_find_node (GravitonCloud *self, const gchar *guid,
                          GError **error)
{
  g_error ("graviton_cloud_find_node_sync is not implemented");
}

static void
notify_all_service_callbacks (GravitonCloud *self, GravitonServiceEvent event)
{
  GList *cur;
  GHashTableIter iter;

  g_mutex_lock (&self->priv->browse_lock);
  g_hash_table_iter_init (&iter, self->priv->browse_callbacks);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer*)&cur)) {
    while (cur) {
      notify_service_browser (self, cur->data, event, NULL);
      cur = cur->next;
    }
  }
  g_mutex_unlock (&self->priv->browse_lock);
}

static void
notify_all_service_callbacks_by_iface (GravitonCloud *cloud,
                                       GravitonServiceEvent event,
                                       GravitonServiceInterface *iface)
{
  GList *cur;
  const gchar *service_name;

  service_name = graviton_service_interface_get_name (iface);
  g_mutex_lock (&cloud->priv->browse_lock);
  cur = g_hash_table_lookup (cloud->priv->browse_callbacks, service_name);
  g_debug ("Notifying callbacks for %s: %i", service_name, event);
  if (cur) {
    while (cur) {
      notify_service_browser (cloud, cur->data, event, iface);
      cur = cur->next;
    }
  }
  g_mutex_unlock (&cloud->priv->browse_lock);
}

static void
notify_all_service_callbacks_by_node (GravitonCloud *self,
                                      GravitonServiceEvent event,
                                      GravitonNode *node)
{
  GError *error = NULL;
  GList *services = graviton_node_get_services (node, &error);
  GList *cur;

  cur = services;
  if (cur) {
    g_debug ("Notifing callbacks about a node: %i", event);
    while (cur) {
      GravitonServiceInterface *iface = graviton_node_get_service_interface (
        node,
        (gchar*)cur->data,
        NULL);
      notify_all_service_callbacks_by_iface (self, event, iface);
      g_object_unref (iface);
      cur = cur->next;
    }
  }
}

static void
notify_service_browser (GravitonCloud *cloud,
                        GravitonServiceBrowser *browser,
                        GravitonServiceEvent event,
                        GravitonServiceInterface *iface)
{
  g_debug ("FIRE");
  browser->callback (cloud, event, iface, browser->data);
}

static void
free_browser (GravitonServiceBrowser *browser)
{
  g_free (browser->data);
  g_free (browser->name);
  browser->data = NULL;
  browser->name = NULL;
  g_free (browser);
}

/**
 * graviton_cloud_browse_services:
 * @cloud: A #GravitonCloud to query
 * @service_name: A name of the service to locate, eg
 *net:phrobo:graviton:introspection
 * @callback: Callback that will be called at certain times
 * @user_data: User data
 *
 * This sets up a callback that is called for each node that supports a
 * particular service within a cloud. Check #GravitonServiceBrowseCallback for
 * more details.
 */
GravitonServiceBrowser*
graviton_cloud_browse_services (GravitonCloud *cloud,
                                const gchar *service_name,
                                GravitonServiceBrowseCallback callback,
                                gpointer user_data)
{
  GravitonServiceBrowser *browser;
  GList *current_callbacks;

  browser = g_new0 (GravitonServiceBrowser, 1);
  browser->callback = callback;
  browser->data = user_data;
  browser->name = g_strdup (service_name);

  g_mutex_lock (&cloud->priv->browse_lock);
  current_callbacks = g_hash_table_lookup (cloud->priv->browse_callbacks,
                                           service_name);
  current_callbacks = g_list_prepend (current_callbacks, browser);

  g_hash_table_replace (cloud->priv->browse_callbacks, g_strdup (
                          service_name), current_callbacks);

  g_debug ("Setting up browser for %s in %s", service_name,
           cloud->priv->cloud_id);

  GList *cur = graviton_node_browser_get_found_nodes (cloud->priv->browser,
                                                      cloud->priv->cloud_id);
  g_mutex_unlock (&cloud->priv->browse_lock);

  while (cur) {
    GError *error = NULL;
    if (graviton_node_has_service (cur->data, service_name, &error)) {
      g_debug ("Hit!");
      GravitonServiceInterface *service =
        graviton_service_interface_get_subservice (GRAVITON_SERVICE_INTERFACE (
                                                     cur
                                                     ->
                                                     data), service_name);
      notify_service_browser (cloud, browser, GRAVITON_SERVICE_NEW, service);
      g_object_unref (service);
    } else {
      if (error) {
        g_debug ("Error while asking about service: %s", error->message);
      } else {
        g_debug ("Miss!");
      }
    }
    cur = cur->next;
  }

  //FIXME: Only notify if we've gotten the signal from the discovery methods
  if (cloud->priv->all_nodes_found_for_now) {
    g_debug ("Previously received all-nodes-found, bubbling to browsers");
    notify_service_browser (cloud, browser, GRAVITON_SERVICE_ALL_FOR_NOW, NULL);
  }

  return browser;
}

void
graviton_cloud_destroy_service_browser (GravitonCloud *cloud,
                                        GravitonServiceBrowser *browser)
{
  GList *current_callbacks;

  g_mutex_lock (&cloud->priv->browse_lock);
  current_callbacks = g_hash_table_lookup (cloud->priv->browse_callbacks,
                                           browser->name);
  current_callbacks = g_list_remove (current_callbacks, browser);
  g_hash_table_replace (cloud->priv->browse_callbacks, g_strdup (
                          browser->name), current_callbacks);
  g_mutex_unlock (&cloud->priv->browse_lock);

  free_browser( browser);
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
  return graviton_node_browser_get_found_nodes (self->priv->browser,
                                                self->priv->cloud_id);
}

/**
 * graviton_cloud_new_default_cloud:
 *
 * The prefered way to create a #GravitonCloud. It returns one that uses the
 * user's configured cloud ID and discovery plugins.
 *
 * Returns: (transfer none): A cloud with the default cloud ID and configured
 * set of plugins
 */
GravitonCloud *
graviton_cloud_new_default_cloud ()
{
  GravitonNodeBrowser *browser;
  GravitonCloud *cloud;
  GKeyFile *keyfile = g_key_file_new ();
  g_key_file_load_from_data_dirs (keyfile,
                                  "gravitonrc",
                                  NULL,
                                  G_KEY_FILE_KEEP_COMMENTS,
                                  NULL);
  gchar *cloud_id = g_key_file_get_string (keyfile,
                                           "graviton",
                                           "default-cloud-id",
                                           NULL);
  //FIXME: Need to store/load cloud ids
  cloud_id = g_strdup (graviton_config_get_default_cloud_id ());
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
  cloud = graviton_node_browser_get_cloud (browser, cloud_id);
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
  GravitonCloud *self;

  self = GRAVITON_CLOUD (data);

  notify_all_service_callbacks_by_node (self, GRAVITON_SERVICE_NEW, node);
}

static void
cb_node_lost (GravitonNodeBrowser *browser, GravitonNode *node, gpointer data)
{
  GravitonCloud *self = GRAVITON_CLOUD (data);

  notify_all_service_callbacks_by_node (self, GRAVITON_SERVICE_LOST, node);
}

static void
cb_nodes_found (GravitonNodeBrowser *browser, gpointer data)
{
  GravitonCloud *self;

  self = GRAVITON_CLOUD (data);

  self->priv->all_nodes_found_for_now = TRUE;
  g_debug ("Got all-nodes-found signal.");
  notify_all_service_callbacks (self, GRAVITON_SERVICE_ALL_FOR_NOW);
}

static void
setup_browser (GravitonCloud *self, GravitonNodeBrowser *browser)
{
  g_debug ("Connecting to browser");
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
