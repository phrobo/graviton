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

#ifndef __GRAVITON_CLOUD_H__
#define __GRAVITON_CLOUD_H__

#include "node-browser.h"
#include "node.h"
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define GRAVITON_CLOUD_TYPE            (graviton_cloud_get_type ())
#define GRAVITON_CLOUD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                                    GRAVITON_CLOUD_TYPE, \
                                                                    GravitonCloud))
#define GRAVITON_CLOUD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                                 GRAVITON_CLOUD_TYPE, \
                                                                 GravitonCloudClass))
#define IS_GRAVITON_CLOUD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                                                    GRAVITON_CLOUD_TYPE))
#define IS_GRAVITON_CLOUD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                                 GRAVITON_CLOUD_TYPE))
#define GRAVITON_CLOUD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                                   GRAVITON_CLOUD_TYPE, \
                                                                   GravitonCloudClass))

typedef struct _GravitonCloud GravitonCloud;
typedef struct _GravitonCloudClass GravitonCloudClass;
typedef struct _GravitonCloudPrivate GravitonCloudPrivate;

/**
 * GravitonCloudClass:
 * @parent_class: Parent GObjectClass
 */
struct _GravitonCloudClass
{
  GObjectClass parent_class;
};

struct _GravitonCloud
{
  GObject parent;

  /*< private >*/
  GravitonCloudPrivate *priv;
};

/**
 * GravitonServiceEvent:
 * @GRAVITON_SERVICE_NEW: A new #GravitonServiceInterface has been located
 *within the cloud
 * @GRAVITON_SERVICE_ALL_FOR_NOW: All #GravitonDiscoveryMethod objects have
 *indicated that they have
 *   identified their initial list of services in the cloud
 * @GRAVITON_SERVICE_LOST: A #GravitonNode has been confirmed as unreachable
 */
typedef enum {
  GRAVITON_SERVICE_NEW,
  GRAVITON_SERVICE_ALL_FOR_NOW,
  GRAVITON_SERVICE_LOST
} GravitonServiceEvent;

/**
 * GravitonServiceBrowseCallback:
 * @cloud: The #GravitonCloud this callback was registered with
 * @event: The particular event that triggered the callback
 * @iface: The #GravitonServiceInterface that triggered the callback
 * @user_data: User data
 *
 * A callback for use with #graviton_cloud_find_service_interfaces.
 *
 * A registered callback can be triggered by a set of conditions:
 *
 * - A #GravitonNode that exposes the requested service is discovered
 * - A #GravitonNode that exposes the requested service is confirmed to be
 *unreachable
 * - The initial set of #GravitonDiscoveryMethod objects loaded with the cloud's
 *   #GravitonNodeBrowser has finished the initial discovery and predicts that
 *   no future services will be discovered for now.
 */
typedef void (*GravitonServiceBrowseCallback)(GravitonCloud *cloud,
                                              GravitonServiceEvent event,
                                              GravitonServiceInterface *iface,
                                              gpointer user_data);

typedef void (*GravitonForeachServiceCallback)(GravitonServiceInterface *iface,
                                               gpointer user_data);

GType graviton_cloud_get_type (void);

GravitonCloud *graviton_cloud_new_default_cloud ();

GList *graviton_cloud_get_found_nodes (GravitonCloud *client);

GravitonNode *graviton_cloud_find_node (GravitonCloud *cloud,
                                        const gchar *guid,
                                        GError **error);

typedef struct _GravitonServiceBrowser GravitonServiceBrowser;
GravitonServiceBrowser*graviton_cloud_browse_services (GravitonCloud *cloud,
                                                       const gchar *service_name,
                                                       GravitonServiceBrowseCallback callback,
                                                       gpointer user_data);
void graviton_cloud_destroy_service_browser (GravitonCloud *cloud,
                                             GravitonServiceBrowser*browser);

const gchar *graviton_cloud_get_cloud_id (GravitonCloud *client);
const gchar *graviton_cloud_get_cloud_name (GravitonCloud *client);

void graviton_cloud_foreach_service (GravitonCloud *cloud,
                                     const gchar *service_name,
                                     GravitonForeachServiceCallback callback,
                                     gpointer user_data);

G_END_DECLS

#endif
