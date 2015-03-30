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
#include <stdlib.h>

static void
cb_services (GravitonCloud *cloud, GravitonServiceEvent event, GravitonServiceInterface *iface, gpointer user_data)
{
  GError *error = NULL;
  GravitonNode *node;
  GMainLoop *loop = (GMainLoop*)user_data;//FIXME: proper cast
  switch (event) {
    case GRAVITON_SERVICE_NEW:
      node = graviton_service_interface_get_node (iface);
      g_print ("Calling ping on %s\n", graviton_node_get_id (node, &error));
      graviton_service_interface_call_noref (iface, "ping", &error, NULL);
      break;
    case GRAVITON_SERVICE_LOST:
      g_print ("Ping service has vanished!\n");
      break;
    case GRAVITON_SERVICE_ALL_FOR_NOW:
      exit (0);
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

  graviton_cloud_browse_services (cloud, "net:phrobo:graviton:examples:ping", cb_services, loop);

  g_main_loop_run (loop);

  return 0;
}
