#include <graviton/server/server.h>
#include <graviton/server/service.h>

#include <glib.h>

static GVariant *
cb_ping(GravitonService *control, GHashTable *args, GError **error, gpointer user_data)
{
  g_print ("Responding to ping request\n");
  return g_variant_new_string ("pong");
}

int main(int argc, char** argv)
{
  GMainLoop *loop = NULL;
  GravitonServer *server = NULL;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  g_type_init ();
#endif

  loop = g_main_loop_new (NULL, FALSE);

  server = graviton_server_new ();
  GravitonRootService *root = graviton_server_get_root_service (server);
  GravitonService *pingService = graviton_service_new ("net:phrobo:graviton:ping");
  graviton_service_add_subservice (GRAVITON_SERVICE (root), pingService);

  graviton_service_add_method (pingService, "ping", cb_ping, NULL, NULL);

  const gchar *cloud_id = graviton_server_get_cloud_id (server);
  const gchar *node_id = graviton_server_get_node_id (server);
  
  g_print ("Echo server running at %s:%s\n", cloud_id, node_id);

  graviton_server_run_async (server);

  g_main_loop_run (loop);

  g_object_unref (server);
  return 0;
}
