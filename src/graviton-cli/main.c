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
    /*GravitonNodeStream *stream = graviton_node_control_get_stream (control, (gchar*)cur->data, args);
    g_print ("\t\t%s\n", graviton_stream_get_name (stream));
    GIOStream *ioStream = graviton_node_stream_open (stream);
    GInputStream *input = g_io_stream_get_input_stream (ioStream);
    gchar buf[1024];
    GError *error = NULL;
    gssize total_size = 0;
    GTimer *timer = g_timer_new ();
    gssize read_size = g_input_stream_read (input, &buf, sizeof (buf), NULL, &error);
    while (read_size > 0) {
      read_size = g_input_stream_read (input, &buf, sizeof (buf), NULL, &error);
      total_size += read_size;
      gdouble elapsed = g_timer_elapsed (timer, NULL);
      gdouble rate = total_size / elapsed;
      g_print ("Read %db %.2lfb/s", total_size, rate);
      int i;
      for (i = 0;i < read_size % 3;i++)
        g_print (".");
      g_print("\r");
    }
    g_print ("\n");
    if (error)
      g_print ("Error: %s", error->message);*/
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

  if (argc == 3) {
    GInetSocketAddress *addr = NULL;
    GInetAddress *addrName = NULL;
    guint port = atoi (argv[2]);
    addrName = g_inet_address_new_from_string (argv[1]);
    if (addrName) {
      addr = (GInetSocketAddress*)g_inet_socket_address_new (addrName, port);
    }
    GravitonNode *node = graviton_node_new_from_address (addr);
    graviton_client_add_node (client, node);
  }

  g_main_loop_run (loop);

  return 0;
}
