#include <graviton/client.h>



int main(int argc, char **argv)
{
  g_type_init ();

  GMainLoop *loop = g_main_loop_new (NULL, 0);
  GravitonClient *client = graviton_client_new_default_cloud ();
  GList *providers = NULL;
  GError *error = NULL;

  providers = graviton_cloud_find_service_sync (cloud, "net:phrobo:graviton:ping", &error);

  GList *cur = providers;
  while (cur) {
    graviton_node_call (GravitonNode *node, "net:phrobo:graviton:ping"
    cur = cur->next;
  }

  g_main_loop_run (loop);

  return 0;
}
