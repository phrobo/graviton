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

#include <graviton/server/server.h>
#include <graviton/server/service.h>
#include <graviton/server/static-stream.h>

#include <glib.h>

static GravitonStream *
cb_stream(GravitonService *service, const gchar *name, GHashTable *args, GError **error, gpointer user_data)
{
  return GRAVITON_STREAM (graviton_static_stream_new_contents (name));
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
  GravitonService *stream_service = graviton_service_new ("net:phrobo:graviton:examples:stream");
  graviton_service_add_subservice (GRAVITON_SERVICE (root), stream_service);

  graviton_service_add_stream (stream_service, "hello", cb_stream, NULL);

  const gchar *cloud_id = graviton_server_get_cloud_id (server);
  const gchar *node_id = graviton_server_get_node_id (server);
  
  g_print ("Example stream server running at %s:%s\n", cloud_id, node_id);

  graviton_server_run_async (server);

  g_main_loop_run (loop);

  g_object_unref (server);
  return 0;
}
