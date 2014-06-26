#include "mpd-service.h"
#include <graviton/server/server.h>
#include <stdlib.h>

static void
usage(const char *progname)
{
  g_print ("Usage: %s hostname-or-address [port]\n", progname);
}

int
main(int argc, char** argv)
{
  GMainLoop *loop;
  GravitonServer *server;
  GravitonRootService *root;
  GravitonMPDService *mpd_service;
  const gchar *address;
  guint port;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  g_type_init ();
#endif

  loop = g_main_loop_new (NULL, FALSE);

  server = graviton_server_new ();
  root = graviton_server_get_root_service (server);

  if (argc > 1) {
    address = argv[2];
    port = 0; // Use default port according to libmpd
    if (argc == 2) {
      port = atoi (argv[3]);
    } else {
      usage(argv[0]);
      exit(1);
    }
  } else {
    usage(argv[0]);
    exit(1);
  }


  mpd_service = graviton_mpd_service_new (address, port);
  graviton_service_add_subservice (GRAVITON_SERVICE (root), GRAVITON_SERVICE (mpd_service));
  graviton_server_run_async (server);
  g_main_loop_run (loop);

  return 0;
}
