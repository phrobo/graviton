#include "config.h"

#include <glib.h>
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>

static void
cb_info (SoupServer *server,
         SoupMessage *msg,
         const char *path,
         GHashTable *query,
         SoupClientContext *client,
         gpointer user_data)
{
  if (msg->method != SOUP_METHOD_GET) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
    return;
  }
  JsonBuilder *builder = json_builder_new ();
  JsonGenerator *generator = json_generator_new ();

  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "version");
  json_builder_add_string_value (builder, GRAVITON_VERSION);
  json_builder_end_object (builder);

  JsonNode *root = json_builder_get_root (builder);
  json_generator_set_root (generator, root);
  gsize length;
  gchar *data = json_generator_to_data (generator, &length);

  json_node_free (root);
  g_object_unref (generator);
  g_object_unref (builder);

  soup_message_set_status (msg, SOUP_STATUS_OK);
  soup_message_set_response (msg,
                             "text/json",
                             SOUP_MEMORY_COPY,
                             data,
                             length);
  g_free (data);
}

int main(int argc, char** argv)
{
  GMainLoop *loop = NULL;
  SoupServer *server = NULL;

  g_type_init ();

  loop = g_main_loop_new (NULL, FALSE);

  server = soup_server_new (
    "port", 2718,
    "server-header", "Graviton/" GRAVITON_VERSION " "
  );

  soup_server_add_handler (server, "/info", cb_info, NULL, NULL);

  soup_server_run_async (server);

  g_main_loop_run (loop);
  return 0;
}
