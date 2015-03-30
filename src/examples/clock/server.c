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

#include <glib.h>
#include <time.h>

static gboolean
cb_tick (gpointer data)
{
  GravitonService *clock_service = GRAVITON_SERVICE (data);

  graviton_service_emit_event (clock_service, "tick", g_variant_new_int32 (time(NULL)));
  return G_SOURCE_CONTINUE;
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
  GravitonService *clock_service = graviton_service_new ("net:phrobo:graviton:examples:clock");
  graviton_service_add_subservice (GRAVITON_SERVICE (root), clock_service);

  g_timeout_add_seconds (1, cb_tick, clock_service);

  graviton_server_run_async (server);

  g_main_loop_run (loop);

  g_object_unref (server);
  return 0;
}
