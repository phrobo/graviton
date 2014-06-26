#include <graviton/client/cloud.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  CMD_STATUS,
  CMD_PLAY,
  CMD_NEXT
} MPDCommandType;

typedef struct _MPDCommand {
 MPDCommandType type; // What we're going to do
 GMainLoop *loop;
} MPDCommand;

static void
show_state (GravitonServiceInterface *iface)
{
  GError *error = NULL;
  GVariant *state;

  state = graviton_service_interface_get_property (iface, "state", &error);

  g_print ("Current state: %s\n", g_variant_get_string (state, NULL));
}

static void
do_next (GravitonServiceInterface *iface)
{
  GError *error = NULL;

  graviton_service_interface_call_noref (iface, "next", &error, NULL);
}

static void
run_command (GravitonServiceInterface *iface, MPDCommand *command)
{
  switch (command->type) {
    case CMD_STATUS:
      show_state (iface);
      break;
    case CMD_NEXT:
      do_next (iface);
      break;
    default:
      g_print ("Unknown command :(\n");
  }
}

static void
cb_services (GravitonCloud *cloud, GravitonServiceEvent event, GravitonServiceInterface *iface, gpointer user_data)
{
  MPDCommand *command = (MPDCommand*)user_data;
  switch (event) {
    case GRAVITON_SERVICE_NEW:
      run_command (iface, command);
      break;
    case GRAVITON_SERVICE_ALL_FOR_NOW:
      g_main_loop_quit (command->loop);
      break;
    case GRAVITON_SERVICE_LOST:
      break;
  }
}

int
main (int argc, char** argv)
{
  GravitonCloud *cloud;
  gchar *str_command;
  MPDCommand command;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  g_type_init ();
#endif

  command.type = CMD_STATUS;
  command.loop = g_main_loop_new (NULL, 0);

  if (argc > 1) {
    str_command = argv[1];
    if (strcmp (str_command, "next") == 0) {
      command.type = CMD_NEXT;
    }
  }

  cloud = graviton_cloud_new_default_cloud ();
  graviton_cloud_browse_services (cloud, "net:phrobo:graviton:examples:mpd", cb_services, &command);
  g_main_loop_run (command.loop);

  return 0;
}
