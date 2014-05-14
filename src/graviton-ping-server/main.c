#include <graviton/server.h>
#include <graviton/control.h>

#include <glib.h>

static GVariant *
cb_ping(GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
  g_printf ("Responding to ping request\n");
  return g_variant_new_string ("pong");
}

int main(int argc, char** argv)
{
  GMainLoop *loop = NULL;
  GravitonServer *server = NULL;

  g_type_init ();

  loop = g_main_loop_new (NULL, FALSE);

  server = graviton_server_new ();
  GravitonRootControl *root = graviton_server_get_root_control (server);
  GravitonControl *pingService = graviton_control_new ("net:phrobo:graviton:ping");
  graviton_control_add_subcontrol (GRAVITON_CONTROL (root), pingService);

  graviton_control_add_method (pingService, "ping", cb_ping, NULL, NULL);

  const gchar *cloud_id = graviton_server_get_cloud_id (server);
  const gchar *node_id = graviton_server_get_node_id (server);
  
  g_printf ("Echo server running at %s:%s\n", cloud_id, node_id);

  graviton_server_run_async (server);

  g_main_loop_run (loop);

  g_object_unref (server);
  return 0;
}
