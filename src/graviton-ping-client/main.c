#include <graviton/cloud.h>
#include <graviton/service.h>

int main(int argc, char **argv)
{
  g_type_init ();

  GMainLoop *loop = g_main_loop_new (NULL, 0);
  GravitonCloud *cloud = graviton_cloud_new_default_cloud ();
  GList *providers = NULL;
  GError *error = NULL;

  providers = graviton_cloud_find_service_sync (cloud, "net:phrobo:graviton:ping", &error);

  GList *cur = providers;
  while (cur) {
    GravitonService *service = GRAVITON_SERVICE (cur->data);
    GravitonNode *node = graviton_service_get_node (service);
    g_printf ("Calling ping on %s:%d\n", graviton_node_get_id (node, &error), graviton_node_get_port (node));
    graviton_service_call (service, "ping", &error, NULL);
    cur = cur->next;
  }

  g_object_unref (cloud);
  g_main_loop_unref (loop);

  return 0;
}
