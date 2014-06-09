#include <graviton/client/cloud.h>
#include <graviton/client/service-interface.h>
#include <unistd.h>

static void
cb_services (GravitonCloud *cloud, GravitonServiceEvent event, GravitonServiceInterface *iface, gpointer user_data)
{
  GError *error = NULL;
  GravitonNode *node;
  GMainLoop *loop = (GMainLoop*)user_data;//FIXME: proper cast
  switch (event) {
    case GRAVITON_SERVICE_NEW:
      node = graviton_service_interface_get_node (iface);
      g_print ("Calling ping on %s\n", graviton_node_get_id (node, &error));
      graviton_service_interface_call_noref (iface, "ping", &error, NULL);
      break;
    case GRAVITON_SERVICE_ALL_FOR_NOW:
      exit(0);
      g_main_loop_quit (loop);
      break;
  }
}

int main(int argc, char **argv)
{

#if !GLIB_CHECK_VERSION(2, 36, 0)
  g_type_init ();
#endif

  GMainLoop *loop = g_main_loop_new (NULL, 0);
  GravitonCloud *cloud = graviton_cloud_new_default_cloud ();

  graviton_cloud_browse_services (cloud, "net:phrobo:graviton:ping", cb_services, loop);

  g_main_loop_run (loop);

  return 0;
}
