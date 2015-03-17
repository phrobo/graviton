/*
 * This file is part of Graviton.
 * Copyright (C) 2013-2014 Torrie Fischer <tdfischer@phrobo.net>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef GRAVITON_SERVICE_H
#define GRAVITON_SERVICE_H

#include <gio/gio.h>
#include <glib-object.h>

#define GRAVITON_SERVICE_TYPE            (graviton_service_get_type ())
#define GRAVITON_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                                      GRAVITON_SERVICE_TYPE, \
                                                                      GravitonService))
#define GRAVITON_IS_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                                                      GRAVITON_SERVICE_TYPE))
#define GRAVITON_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                                   GRAVITON_SERVICE_TYPE, \
                                                                   GravitonServiceClass))
#define GRAVITON_IS_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                                   GRAVITON_SERVICE_TYPE))
#define GRAVITON_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                                     GRAVITON_SERVICE_TYPE, \
                                                                     GravitonServiceClass))

#define GRAVITON_SERVICE_ERROR (graviton_service_error_quark ())

typedef enum {
  GRAVITON_SERVICE_ERROR_NO_SUCH_METHOD
} GravitonServiceError;

typedef struct _GravitonStream GravitonStream;

typedef struct _GravitonService GravitonService;
typedef struct _GravitonServiceClass GravitonServiceClass;

typedef struct _GravitonServicePrivate GravitonServicePrivate;

/**
 * GravitonService:
 *
 * Controls provide services to the graviton network by way of exposing a set of
 * properties, methods, IO channels, and child services. Controls have names
 * which are browseable via the net:phrobo:graviton introspection service.
 *
 * After a #GravitonServer is created, you can attach services to it by fetching
 * its #GravitonRootService via graviton_server_get_root_service() and calling
 * graviton_service_add_subservice().
 *
 * Properties are exposed on a #GravitonService through the normal #GObject API
 * that is used by g_object_set()/g_object_get()
 *
 * FIXME: Example of properties
 *
 * Methods are exposed by calling graviton_service_add_method() and supplying a
 * callback.
 *
 * FIXME: Example of adding a method
 *
 * IO channels use graviton_service_add_stream() with a supplied
 * #GravitonServiceStreamGenerator callback for later activation.
 *
 */
struct _GravitonService
{
  GObject parent_instance;
  GravitonServicePrivate *priv;
};

struct _GravitonServiceClass
{
  GObjectClass parent_class;
};

GType graviton_service_get_type ();

typedef GVariant *(*GravitonServiceMethod)(GravitonService *self,
                                           GHashTable *args, GError **error,
                                           gpointer user_data);

void graviton_service_add_method (GravitonService *self,
                                  const gchar *name,
                                  GravitonServiceMethod func,
                                  gpointer user_data,
                                  GDestroyNotify destroy_func);

GVariant *graviton_service_call_method (GravitonService *self,
                                        const gchar *name,
                                        GHashTable *args,
                                        GError **error);

GList *graviton_service_list_methods (GravitonService *self);

gboolean graviton_service_has_method (GravitonService *self, const gchar *name);

void graviton_service_add_subservice (GravitonService *self,
                                      GravitonService *service);

GravitonService *graviton_service_get_subservice (GravitonService *self,
                                                  const gchar *name);

GList *graviton_service_list_subservices (GravitonService *self);

typedef GravitonStream *(*GravitonServiceStreamGenerator)(GravitonService *self,
                                                          const gchar *name,
                                                          GHashTable *args,
                                                          GError **error,
gpointer user_data);


GList *graviton_service_list_streams (GravitonService *self);
GravitonStream *graviton_service_get_stream (GravitonService *self,
                                             const gchar *name,
                                             GHashTable *args,
                                             GError **error);

GravitonService *graviton_service_new (const gchar *service_name);

void graviton_service_emit_event (GravitonService *self,
                                  const gchar *name,
                                  GVariant *data);

#ifdef GRAVITON_ENABLE_STREAMS
void graviton_service_add_stream (GravitonService *self,
                                  const gchar *name,
                                  GravitonServiceStreamGenerator func,
                                  gpointer user_data);
#endif // GRAVITON_ENABLE_STREAMS

#endif // GRAVITON_SERVICE_H
