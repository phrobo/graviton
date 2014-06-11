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

#include <graviton/client/cloud.h>
#include <graviton/client/node.h>
#include <graviton/client/service-interface.h>
#include <graviton/client/introspection-interface.h>
#include <graviton/client/jsonrpc-node-transport.h>

void
print_streams (GravitonServiceInterface *service)
{
  GError *error = NULL;
  GravitonIntrospectionControl *inspect = graviton_introspection_interface_new_from_interface (service);
  GList *streams = graviton_introspection_interface_list_streams (inspect, &error);
  GList *cur = streams;

  if (error) {
    g_print ("Error listing streams for %s: %s", graviton_service_interface_get_name (service), error->message);
    g_object_unref (inspect);
    return;
  }

  while (cur) {
    g_print ("\t\t%s\n", (gchar*)cur->data);
    cur = cur->next;
  }

  g_object_unref (inspect);
}

void
print_properties (GravitonServiceInterface *service)
{
  GError *error = NULL;
  GravitonIntrospectionControl *inspect = graviton_introspection_interface_new_from_interface (service);
  GList *properties = graviton_introspection_interface_list_properties (inspect, &error);
  GList *cur = properties;

  if (error) {
    g_print ("Error listing properties for %s: %s", graviton_service_interface_get_name (service), error->message);
    return;
  }

  while (cur) {
    GVariant *prop = graviton_service_interface_get_property (service, (gchar*)cur->data, &error);
    if (error ){
      g_print ("Error getting property %s: %s", (gchar*)cur->data, error->message);
    } else {
      if (prop) {
        g_print ("\t\t%s = %s\n", (gchar*)cur->data, g_variant_print (prop, TRUE));
        g_variant_unref (prop);
      } else {
        g_print ("\t\t%s = (null)\n", (gchar*)cur->data);
      }
    }
    cur = cur->next;
  }

  g_object_unref (inspect);
}

void
print_services (GravitonServiceInterface *service)
{
  GError *error = NULL;
  if (graviton_service_interface_get_name (service) != NULL) {
    g_print ("%s:\n", graviton_service_interface_get_name (service));
    g_print ("\tProperties: \n");
    print_properties (service);
    g_print ("\tStreams:\n");
    print_streams (service);
  }
  GravitonIntrospectionControl *inspect = graviton_introspection_interface_new_from_interface (service);
  GList *services = graviton_introspection_interface_list_interfaces (inspect, &error);
  GList *cur = services;
  while (cur) {
    GravitonServiceInterface *subservice = graviton_service_interface_get_subservice (service, cur->data);
    print_services (subservice);
    cur = cur->next;
    g_object_unref (subservice);
  }
  g_object_unref (inspect);

  if (error) {
    g_print("Error listing services for %s: %s", graviton_service_interface_get_name (service), error->message);
  }
}

void
print_node (GravitonNode *node)
{
  GError *error = NULL;
  const gchar *id = graviton_node_get_id (node, &error);
  if (error) {
    g_print ("Error: %s\n", error->message);
    return;
  } else {
    g_print ("Found node: %s\n", id);
  }
  g_print ("Services:\n");
  print_services (GRAVITON_SERVICE_INTERFACE (node));
}

void
cb_nodes (GravitonNodeBrowser *browser, gpointer data)
{
  GMainLoop *loop = (GMainLoop*)(data);
  GList *cloud_ids = graviton_node_browser_get_found_cloud_ids (browser);
  GList *cur_cloud = cloud_ids;

  while (cur_cloud) {
    GList *nodes = graviton_node_browser_get_found_nodes (browser, cur_cloud->data);
    GList *cur = nodes;

    g_print ("Discovered nodes in cloud %s:\n", (gchar*)cur_cloud->data);

    while(cur) {
      GravitonNode *node = GRAVITON_NODE (cur->data);
      print_node (node);
      cur = cur->next;
    }

    cur_cloud = cur_cloud->next;
  }

  g_main_loop_quit (loop);

}

int main (int argc, char** argv)
{

#if !GLIB_CHECK_VERSION(2, 36, 0)
  g_type_init ();
#endif

  GMainLoop *loop = g_main_loop_new (NULL, 0);

  GravitonNodeBrowser *browser = graviton_node_browser_new ();
  g_signal_connect (browser,
                    "all-nodes-found",
                    G_CALLBACK (cb_nodes),
                    loop);
  graviton_node_browser_load_discovery_plugins (browser);
  g_main_loop_run (loop);

  return 0;
}
