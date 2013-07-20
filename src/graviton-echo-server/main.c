#include <graviton/server.h>
#include <graviton/control.h>

#include <glib.h>

static GVariant *
cb_ping(GravitonControl *control, GHashTable *args, GError **error, gpointer user_data)
{
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

  graviton_control_add_method (GRAVITON_CONTROL (root), "ping", cb_ping, NULL, NULL);

  graviton_server_run_async (server);

  g_main_loop_run (loop);

  g_object_unref (server);
  return 0;
}
