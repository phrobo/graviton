/*
 * This file is part of Graviton.
 * Copyright (C) 2013-2014 Torrie Fischer <tdfischer@phrobo.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <graviton/server/quickserver.h>

#include <glib.h>

static GVariant *
cb_ping(GravitonService *control, GHashTable *args, GError **error, gpointer user_data)
{
  g_print ("Responding to ping request\n");
  graviton_service_emit_event (control, "handled-ping", NULL);
  return g_variant_new_string ("pong");
}

int main(int argc, char** argv)
{
  GravitonQuickserver *server = NULL;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  g_type_init ();
#endif

  server = graviton_quickserver_new ();
  graviton_quickserver_add_method (server,
                                   "net:phrobo:graviton:examples:ping",
                                   "ping",
                                   cb_ping,
                                   NULL,
                                   NULL);

  const gchar *cloud_id = graviton_server_get_cloud_id (server);
  const gchar *node_id = graviton_server_get_node_id (server);
  
  g_print ("Echo server running at %s:%s\n", cloud_id, node_id);

  graviton_quickserver_run (server);

  return 0;
}
