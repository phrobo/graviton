#include <graviton/client.h>
#include <graviton/node.h>
#include <graviton/node-control.h>
#include <graviton/introspection-control.h>

void
print_properties (GravitonNodeControl *control)
{
  GError *error = NULL;
  GravitonIntrospectionControl *inspect = graviton_introspection_control_new_from_control (control);
  GList *properties = graviton_introspection_control_list_properties (inspect, &error);
  GList *cur = properties;
  while (cur) {
    GVariant *prop = graviton_node_control_get_property (control, (gchar*)cur->data, &error);
    g_printf ("\t\t%s = %s\n", cur->data, g_variant_print (prop, TRUE));
    g_variant_unref (prop);
    cur = cur->next;
  }

  if (error) {
    g_print ("Error listing properties for %s: %s", graviton_node_control_get_name (control), error->message);
  }
}

void
print_controls (GravitonNodeControl *control)
{
  GError *error = NULL;
  g_printf("\t%s:\n", graviton_node_control_get_name (control));
  GravitonIntrospectionControl *inspect = graviton_introspection_control_new_from_control (control);
  GList *controls = graviton_introspection_control_list_controls (inspect, &error);
  GList *cur = controls;
  while (cur) {
    GravitonNodeControl *subcontrol = graviton_node_control_get_subcontrol (control, cur->data);
    g_printf ("\tProperties: \n");
    print_properties (subcontrol);
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
cb_nodes (GravitonClient *client, gpointer data)
{
  GMainLoop *loop = (GMainLoop*)(data);
  GList *nodes = graviton_client_get_found_nodes (client);
  GList *cur = nodes;

  g_print ("Discovered nodes:\n", nodes);

  while(cur) {
    GError *error = NULL;
    GravitonNode *node = GRAVITON_NODE (cur->data);
    const gchar *id = graviton_node_get_id (node, &error);
    if (error) {
      g_print ("Error: %s", error->message);
    } else {
      g_print ("Found node: %s\n", id);
    }
    g_print ("Controls:\n");
    print_controls (GRAVITON_NODE_CONTROL (node));
    cur = cur->next;
  }

  g_main_loop_quit (loop);

}

int main (int argc, char** argv)
{
  g_type_init ();

  GMainLoop *loop = g_main_loop_new (NULL, 0);

  GravitonClient *client = graviton_client_new ();

  g_signal_connect (client,
                    "all-nodes-found",
                    G_CALLBACK (cb_nodes),
                    loop);

  g_main_loop_run (loop);

  return 0;
}
