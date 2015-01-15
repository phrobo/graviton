/*
 * This file is part of Graviton.
 * Copyright (C) 2013-2015 Torrie Fischer <tdfischer@phrobo.net>
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "quickserver.h"

#define GRAVITON_QUICKSERVER_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_QUICKSERVER_TYPE, GravitonQuickserverPrivate))

static void graviton_quickserver_class_init (GravitonQuickserverClass *klass);
static void graviton_quickserver_init       (GravitonQuickserver *self);
static void graviton_quickserver_dispose    (GObject *object);
static void graviton_quickserver_finalize   (GObject *object);
static void graviton_quickserver_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_quickserver_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonQuickserver, graviton_quickserver, GRAVITON_SERVER_TYPE);

static void
graviton_quickserver_class_init (GravitonQuickserverClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = graviton_quickserver_dispose;
  object_class->finalize = graviton_quickserver_finalize;
  object_class->set_property =  graviton_quickserver_set_property;
  object_class->get_property =  graviton_quickserver_get_property;
}

static void
graviton_quickserver_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_quickserver_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_quickserver_init (GravitonQuickserver *self)
{
}

static void
graviton_quickserver_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_quickserver_parent_class)->dispose (object);
}

static void
graviton_quickserver_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_quickserver_parent_class)->finalize (object);
}

GravitonQuickserver *
graviton_quickserver_new ()
{
  return g_object_new (GRAVITON_QUICKSERVER_TYPE, NULL);
}

GravitonService *
graviton_quickserver_get_service (GravitonQuickserver *self, const gchar *service_name)
{
  GravitonServer *srv = GRAVITON_SERVER (self);
  GravitonRootService *root = graviton_server_get_root_service (srv);
  GravitonService *svc = NULL;

  if (!(svc = graviton_service_get_subservice (GRAVITON_SERVICE (root), service_name))) {
    svc = graviton_service_new (service_name);
  }

  graviton_service_add_subservice (GRAVITON_SERVICE (root), svc);

  return svc;
}

void
graviton_quickserver_add_method (GravitonQuickserver *server,
                                 const gchar *service_name,
                                 const gchar *method_name,
                                 GravitonServiceMethod func,
                                 gpointer user_data,
                                 GDestroyNotify destroy_func)
{
  GravitonService *svc = graviton_quickserver_get_service (server, service_name);
  graviton_service_add_method (svc, method_name, func, user_data, destroy_func);
}

void
graviton_quickserver_run (GravitonQuickserver *server)
{
  GMainLoop *loop = g_main_loop_new (NULL, FALSE);
  graviton_server_run_async (GRAVITON_SERVER (server));
  g_main_loop_run (loop);
  g_object_unref (loop);
}
