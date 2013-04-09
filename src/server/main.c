#include "config.h"
#include "info-plugin.h"
#include <graviton-server/server.h>

#include <glib.h>

int main(int argc, char** argv)
{
  GMainLoop *loop = NULL;
  GravitonServer *server = NULL;

  g_type_init ();

  loop = g_main_loop_new (NULL, FALSE);

  server = graviton_server_new ();

  graviton_server_run_async (server);

  g_main_loop_run (loop);
  return 0;
}
