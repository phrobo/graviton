#include <graviton/client.h>
#include <graviton/node.h>

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
