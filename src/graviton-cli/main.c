#include <graviton/client.h>
#include <graviton/node.h>
#include <graviton/node-control.h>
#include <graviton/introspection-control.h>

void
print_streams (GravitonNodeControl *control)
{
  GError *error = NULL;
  GravitonIntrospectionControl *inspect = graviton_introspection_control_new_from_control (control);
  GList *streams = graviton_introspection_control_list_streams (inspect, &error);
  GList *cur = streams;

  if (error) {
    g_print ("Error listing streams for %s: %s", graviton_node_control_get_name (control), error->message);
    g_object_unref (inspect);
    return;
  }

  while (cur) {
    g_print ("\t\t%s\n", cur->data);
    cur = cur->next;
  }

  g_object_unref (inspect);
}

void
print_properties (GravitonNodeControl *control)
{
  GError *error = NULL;
  GravitonIntrospectionControl *inspect = graviton_introspection_control_new_from_control (control);
  GList *properties = graviton_introspection_control_list_properties (inspect, &error);
  GList *cur = properties;

  if (error) {
    g_print ("Error listing properties for %s: %s", graviton_node_control_get_name (control), error->message);
    return;
  }

  while (cur) {
    GVariant *prop = graviton_node_control_get_property (control, (gchar*)cur->data, &error);
    if (error ){
      g_print ("Error getting property %s: %s", cur->data, error->message);
    } else {
      if (prop) {
        g_printf ("\t\t%s = %s\n", cur->data, g_variant_print (prop, TRUE));
        g_variant_unref (prop);
      } else {
        g_printf ("\t\t%s = (null)\n", cur->data);
      }
    }
    cur = cur->next;
  }

  g_object_unref (inspect);
}

void
print_controls (GravitonNodeControl *control)
{
  GError *error = NULL;
  if (graviton_node_control_get_name (control) != NULL) {
    g_printf("%s:\n", graviton_node_control_get_name (control));
    g_printf ("\tProperties: \n");
    print_properties (control);
    g_print ("\tStreams:\n");
    print_streams (control);
  }
  GravitonIntrospectionControl *inspect = graviton_introspection_control_new_from_control (control);
  GList *controls = graviton_introspection_control_list_controls (inspect, &error);
  GList *cur = controls;
  while (cur) {
    GravitonNodeControl *subcontrol = graviton_node_control_get_subcontrol (control, cur->data);
    print_controls (subcontrol);
    cur = cur->next;
    g_object_unref (subcontrol);
  }
  g_object_unref (inspect);

  if (error) {
    g_print("Error listing controls for %s: %s", graviton_node_control_get_name (control), error->message);
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
  g_print ("Controls:\n");
  print_controls (GRAVITON_NODE_CONTROL (node));
}

void
cb_nodes (GravitonClient *client, gpointer data)
{
  GMainLoop *loop = (GMainLoop*)(data);
  GList *nodes = graviton_client_get_found_nodes (client);
  GList *cur = nodes;

  g_print ("Discovered nodes:\n", nodes);

  while(cur) {
    GravitonNode *node = GRAVITON_NODE (cur->data);
    print_node (node);
    cur = cur->next;
  }

  g_main_loop_quit (loop);

}

int main (int argc, char** argv)
{
  g_type_init ();

  GMainLoop *loop = g_main_loop_new (NULL, 0);

  if (argc == 3) {
    GInetSocketAddress *addr = NULL;
    GInetAddress *addrName = NULL;
    guint port = atoi (argv[2]);
    addrName = g_inet_address_new_from_string (argv[1]);
    if (addrName) {
      addr = (GInetSocketAddress*)g_inet_socket_address_new (addrName, port);
    }
    GravitonNode *node = graviton_node_new_from_address (addr);
    print_node (node);
    g_object_unref (node);
  } else if (argc == 2) {
    const gchar *cloud_id;
    const gchar *node_id;
    GravitonClient *client = graviton_client_new (cloud_id);
    GravitonNode *node = graviton_client_find_node_sync (client, node_id, NULL);
    print_node (node);
    g_object_unref (node);
    g_object_unref (client);
  } else {
    GravitonClient *client = graviton_client_new_default_cloud ();
    g_signal_connect (client,
                      "all-nodes-found",
                      G_CALLBACK (cb_nodes),
                      loop);
    graviton_client_load_discovery_plugins (client);
    g_main_loop_run (loop);
  }


  return 0;
}
