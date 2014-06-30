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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define G_LOG_DOMAIN "GravitonServiceInterface"

#include "node-stream.h"
#include "node.h"
#include "service-interface.h"

#include <string.h>

typedef struct _GravitonServiceInterfacePrivate GravitonServiceInterfacePrivate;

struct _GravitonServiceInterfacePrivate
{
  GravitonNode *node;
  gchar *name;
};

#define GRAVITON_SERVICE_INTERFACE_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_SERVICE_INTERFACE_TYPE, \
                                GravitonServiceInterfacePrivate))

static void graviton_service_interface_class_init (
  GravitonServiceInterfaceClass *klass);
static void graviton_service_interface_init       (
  GravitonServiceInterface *self);
static void graviton_service_interface_dispose    (GObject *object);
static void graviton_service_interface_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonServiceInterface,
               graviton_service_interface,
               G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_NAME,
  PROP_NODE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

enum {
  SIGNAL_0,
  SIGNAL_EVENT,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = {0, };

static void
cb_dispatch_event (GravitonNode *node,
                   const gchar *name,
                   GVariant *value,
                   GravitonServiceInterface *self)
{
  gchar **event_name;
  event_name = g_strsplit (name, ".", 0);
  g_debug ("Dispatching %s event within %s..?", name, self->priv->name);
  if (self->priv->name != NULL && strcmp (event_name[0], self->priv->name) == 0) {
    g_debug ("Dispatching %s event", event_name[1]);
    g_signal_emit (self, obj_signals[SIGNAL_EVENT],
                   g_quark_from_string (event_name[1]), event_name[1], value);
  }
  g_strfreev (event_name);
}

static void
setup_node (GravitonServiceInterface *self, GravitonNode *node)
{
  if (self->priv->node) {
    g_signal_handlers_disconnect_matched (self->priv->node,
                                          G_SIGNAL_MATCH_DATA,
                                          0, 
                                          0,
                                          NULL,
                                          cb_dispatch_event,
                                          self);
    g_object_unref (self->priv->node);
  }

  self->priv->node = node;

  if (node) {
    g_object_ref_sink (self->priv->node);
    g_debug ("Connecting %s to service-event on node %p", self->priv->name, node);
    g_signal_connect (node,
                      "service-event",
                      G_CALLBACK (cb_dispatch_event),
                      self);
  }
}

static void
graviton_service_interface_set_property (GObject *object,
              guint property_id,
              const GValue *value,
              GParamSpec *pspec)
{
  GravitonServiceInterface *self = GRAVITON_SERVICE_INTERFACE (object);
  switch (property_id) {
  case PROP_NAME:
    self->priv->name = g_value_dup_string (value);
    break;
  case PROP_NODE:
    setup_node (self, GRAVITON_NODE (g_value_get_object (value)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
get_property (GObject *object,
              guint property_id,
              GValue *value,
              GParamSpec *pspec)
{
  GravitonServiceInterface *self = GRAVITON_SERVICE_INTERFACE (object);
  switch (property_id) {
  case PROP_NAME:
    g_value_set_string (value, self->priv->name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
graviton_service_interface_class_init (GravitonServiceInterfaceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonServiceInterfacePrivate));

  object_class->dispose = graviton_service_interface_dispose;
  object_class->finalize = graviton_service_interface_finalize;

  object_class->set_property = graviton_service_interface_set_property;
  object_class->get_property = get_property;

  obj_properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "Name of this service",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );
  obj_properties [PROP_NODE] =
    g_param_spec_object ("node",
                         "Node",
                         "The underlying GravitonNode",
                         GRAVITON_NODE_TYPE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );
  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);

  /**
   * GravitonServiceInterface::event:
   * @name: Event name
   * @data: Event data
   *
   * Emitted when the remote service emits an event.
   */
  obj_signals[SIGNAL_EVENT] =
    g_signal_new ("event",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_generic,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_STRING,
                  G_TYPE_VARIANT);
}

static void
graviton_service_interface_init (GravitonServiceInterface *self)
{
  GravitonServiceInterfacePrivate *priv;
  self->priv = priv = GRAVITON_SERVICE_INTERFACE_GET_PRIVATE (self);
  priv->node = NULL;
}

static void
graviton_service_interface_dispose (GObject *object)
{
  GravitonServiceInterface *self = GRAVITON_SERVICE_INTERFACE (object);

  if (self->priv->node) {
    g_signal_handlers_disconnect_matched (self->priv->node,
                                          G_SIGNAL_MATCH_DATA,
                                          0, 
                                          0,
                                          NULL,
                                          cb_dispatch_event,
                                          self);
    g_object_unref (self->priv->node);
  }
  self->priv->node = NULL;
  G_OBJECT_CLASS (graviton_service_interface_parent_class)->dispose (object);
}

static void
graviton_service_interface_finalize (GObject *object)
{
  GravitonServiceInterface *self = GRAVITON_SERVICE_INTERFACE (object);
  g_free (self->priv->name);
  self->priv->name = NULL;

  G_OBJECT_CLASS (graviton_service_interface_parent_class)->finalize (object);
}

const gchar*
graviton_service_interface_get_name (GravitonServiceInterface *self)
{
  return self->priv->name;
}

GravitonServiceInterface *
graviton_service_interface_get_subservice (GravitonServiceInterface *self,
                                           const gchar *name)
{
  gchar *full_name;
  if (self->priv->node) {
    full_name = g_strdup_printf ("%s/%s", self->priv->name, name);
  } else {
    full_name = g_strdup (name);
  }
  GravitonServiceInterface *ret = g_object_new (GRAVITON_SERVICE_INTERFACE_TYPE,
                                                "node",
                                                graviton_service_interface_get_node (
                                                  self),
                                                "name",
                                                full_name,
                                                NULL);
  g_free (full_name);
  return ret;
}

GravitonNode *
graviton_service_interface_get_node (GravitonServiceInterface *self)
{
  if (self->priv->node)
    return g_object_ref (self->priv->node);
  return g_object_ref (GRAVITON_NODE (self));
}

GVariant *
graviton_service_interface_get_property (GravitonServiceInterface *self,
                                         const gchar *property,
                                         GError **err)
{
  GError *error = NULL;
  GVariant *ret = graviton_node_call (graviton_service_interface_get_node (
                                        self),
                                      "net:phrobo:graviton/introspection.getProperty",
                                      &error,
                                      "service",
                                      g_variant_new_string (
                                        graviton_service_interface_get_name (
                                          self)),
                                      "property",
                                      g_variant_new_string (property),
                                      NULL);
  if (error) {
    g_propagate_error (err, error);
    return NULL;
  }

  return ret;
}

gchar *
make_method_name (GravitonServiceInterface *service, const gchar *method)
{
  return g_strdup_printf ("%s.%s", graviton_service_interface_get_name (
                            service), method);
}

void
graviton_service_interface_call_noref (GravitonServiceInterface *service,
                                       const gchar *method,
                                       GError **error, ...)
{
  va_list args;
  va_start (args, error);
  gchar *full_method = make_method_name (service, method);
  GVariant *ret =
    graviton_node_call_va (graviton_service_interface_get_node (
                             service), full_method, error, args);
  va_end (args);
  g_free (full_method);
  if (ret)
    g_variant_unref (ret);
}

GVariant *
graviton_service_interface_call (GravitonServiceInterface *service,
                                 const gchar *method,
                                 GError **error, ...)
{
  va_list args;
  va_start (args, error);
  gchar *full_method = make_method_name (service, method);
  GVariant *ret =
    graviton_node_call_va (graviton_service_interface_get_node (
                             service), full_method, error, args);
  va_end (args);
  g_free (full_method);
  return ret;
}

GravitonNodeStream *
graviton_service_interface_get_stream (GravitonServiceInterface *service,
                                       const gchar *name,
                                       GHashTable *args)
{
  GravitonNodeStream *stream = graviton_node_stream_new (service, name, args);
  return stream;
}

gboolean
graviton_service_interface_subscribe_events (GravitonServiceInterface *service,
                                             const gchar *name,
                                             GError **error)
{
  GravitonNode *node;
  gboolean ret;
  
  node = graviton_service_interface_get_node (service);
  ret = graviton_node_subscribe_events (node, name, error);
  g_object_unref (node);

  return ret;
}

gboolean
graviton_service_interface_unsubscribe_events (GravitonServiceInterface *service,
                                               const gchar *name,
                                               GError **error)
{
  GravitonNode *node;
  gboolean ret;
  
  node = graviton_service_interface_get_node (service);
  ret = graviton_node_unsubscribe_events (node, name, error);
  g_object_unref (node);

  return ret;
}
