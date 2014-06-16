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

#ifndef __GRAVITON_SERVICE_INTERFACE_H__
#define __GRAVITON_SERVICE_INTERFACE_H__

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define GRAVITON_SERVICE_INTERFACE_TYPE            ( \
    graviton_service_interface_get_type ())
#define GRAVITON_SERVICE_INTERFACE(obj)            (G_TYPE_CHECK_INSTANCE_CAST (( \
                                                                                  obj), \
                                                                                GRAVITON_SERVICE_INTERFACE_TYPE, \
                                                                                GravitonServiceInterface))
#define GRAVITON_SERVICE_INTERFACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST (( \
                                                                               klass), \
                                                                             GRAVITON_SERVICE_INTERFACE_TYPE, \
                                                                             GravitonServiceInterfaceClass))
#define IS_GRAVITON_SERVICE_INTERFACE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (( \
                                                                                  obj), \
                                                                                GRAVITON_SERVICE_INTERFACE_TYPE))
#define IS_GRAVITON_SERVICE_INTERFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE (( \
                                                                               klass), \
                                                                             GRAVITON_SERVICE_INTERFACE_TYPE))
#define GRAVITON_SERVICE_INTERFACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS (( \
                                                                                 obj), \
                                                                               GRAVITON_SERVICE_INTERFACE_TYPE, \
                                                                               GravitonServiceInterfaceClass))

typedef struct _GravitonNode GravitonNode;
typedef struct _GravitonNodeStream GravitonNodeStream;

typedef struct _GravitonServiceInterface GravitonServiceInterface;
typedef struct _GravitonServiceInterfaceClass GravitonServiceInterfaceClass;

typedef struct _GravitonServiceInterfacePrivate GravitonServiceInterfacePrivate;

struct _GravitonServiceInterfaceClass
{
  GObjectClass parent_class;
};

struct _GravitonServiceInterface
{
  GObject parent;
  GravitonServiceInterfacePrivate *priv;
};

GType graviton_service_interface_get_type (void);

const gchar *graviton_service_interface_get_name (GravitonServiceInterface *self);
GList *graviton_service_interface_list_subservices (
  GravitonServiceInterface *self,
  GError **error);
GravitonServiceInterface *graviton_service_interface_get_subservice (
  GravitonServiceInterface *self,
  const gchar *name);
GList *graviton_service_interface_list_properties (
  GravitonServiceInterface *service,
  GError **error);
GVariant *graviton_service_interface_get_property (
  GravitonServiceInterface *service,
  const gchar *prop,
  GError **error);
GravitonNode *graviton_service_interface_get_node (
  GravitonServiceInterface *service);
void graviton_service_interface_call_noref (GravitonServiceInterface *service,
                                            const gchar *method,
                                            GError **error,
                                            ...);
GVariant *graviton_service_interface_call (GravitonServiceInterface *service,
                                           const gchar *method,
                                           GError **error,
                                           ...);
GVariant *graviton_service_interface_call_args (
  GravitonServiceInterface *service,
  const gchar *method,
  GHashTable *args,
  GError **error);
GVariant *graviton_service_interface_call_va (GravitonServiceInterface *service,
                                              const gchar *method,
                                              GError **error,
                                              va_list args);

GravitonNodeStream *graviton_service_interface_get_stream (
  GravitonServiceInterface *service,
  const gchar *name,
  GHashTable *args);

G_END_DECLS

#endif