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

#include <graviton/client/cloud.h>
#include <graviton/client/service-interface.h>
#include <graviton/client/node-stream.h>
#include <unistd.h>
#include <stdlib.h>

static void
read_stream (GravitonServiceInterface *iface)
{
  GravitonNodeStream *node_stream;
  GIOStream *io_stream;
  GInputStream *input;
  GError *error = NULL;
  gchar buf[1024];
  GString *read_data;
  gchar *read_string;
  gssize count = 0;
  gssize total_count = 0;

  node_stream = graviton_service_interface_get_stream (iface, "hello", NULL);
  io_stream = graviton_node_stream_open (node_stream);
  input = g_io_stream_get_input_stream (io_stream);

  read_data = g_string_new (NULL);

  while ((count = g_input_stream_read (input, &buf, sizeof(buf), NULL, &error)) && !error) {
    g_string_append_len (read_data, buf, count);
    total_count += count;
  }

  read_string = g_string_free (read_data, FALSE);

  g_print ("Read %" G_GSSIZE_FORMAT " bytes:\n%s\n", total_count, read_string);

  g_free (read_string);
}

static void
cb_services (GravitonCloud *cloud, GravitonServiceEvent event, GravitonServiceInterface *iface, gpointer user_data)
{
  GMainLoop *loop = (GMainLoop*)user_data;//FIXME: proper cast
  switch (event) {
    case GRAVITON_SERVICE_NEW:
      read_stream (iface);
      break;
    case GRAVITON_SERVICE_LOST:
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

  graviton_cloud_browse_services (cloud, "net:phrobo:graviton:examples:stream", cb_services, loop);

  g_main_loop_run (loop);

  return 0;
}
