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

#ifndef __GRAVITON_NODE_BROWSER_H__
#define __GRAVITON_NODE_BROWSER_H__

#include "discovery-method.h"
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define GRAVITON_NODE_BROWSER_TYPE            (graviton_node_browser_get_type ())
#define GRAVITON_NODE_BROWSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                                           GRAVITON_NODE_BROWSER_TYPE, \
                                                                           GravitonNodeBrowser))
#define GRAVITON_NODE_BROWSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                                        GRAVITON_NODE_BROWSER_TYPE, \
                                                                        GravitonNodeBrowserClass))
#define IS_GRAVITON_NODE_BROWSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                                                           GRAVITON_NODE_BROWSER_TYPE))
#define IS_GRAVITON_NODE_BROWSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                                        GRAVITON_NODE_BROWSER_TYPE))
#define GRAVITON_NODE_BROWSER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                                          GRAVITON_NODE_BROWSER_TYPE, \
                                                                          GravitonNodeBrowserClass))

typedef struct _GravitonNodeBrowser GravitonNodeBrowser;
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

GravitonCloud *graviton_node_browser_get_cloud (GravitonNodeBrowser *browser,
                                                const gchar *cloud_id);
GravitonNode *graviton_node_browser_get_node_by_id (
  GravitonNodeBrowser *browser,
  const gchar *node_id);

void graviton_node_browser_add_discovery_method (GravitonNodeBrowser *client,
                                                 GravitonDiscoveryMethod *method);
void graviton_node_browser_load_discovery_plugins (GravitonNodeBrowser *client);
GArray *graviton_node_browser_find_discovery_plugins (
  GravitonNodeBrowser *client);

GravitonNode *graviton_node_browser_request_node (GravitonNodeBrowser *client,
                                                  const gchar *node_id);
GList *graviton_node_browser_get_found_nodes (GravitonNodeBrowser *client,
                                              const gchar *cloud_id);
GList *graviton_node_browser_get_found_cloud_ids (GravitonNodeBrowser *client);

G_END_DECLS

#endif