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

#define G_LOG_DOMAIN "GravitonService"

#include "service.h"
#include <string.h>

/**
 * SECTION:service
 * @short_description: An interface to expose services within a Graviton cloud
 * @title: GravitonService
 * @see_also: #GravitonServer, #GravitonServiceInterface
 * @stability: Unstable
 * @include: graviton/server/service.h
 *
 * In Graviton, services are the things in a cloud that one interacts with. To
 * expose a service on a Graviton cloud, one must create a service, attach
 * methods, properties, and streams, then publish it on a #GravitonServer.
 *
 * For a good example of how to do that, try the example code found with the
 * #GravitonServer documentation.
 *
 * To construct a service, use graviton_service_new(), and give it a relevant
 * interface name such as net:phrobo:graviton:ping.
 *
 * Attaching methods is done with graviton_service_add_method(). These methods
 * can be called via graviton_service_call_method().
 *
 * Services may be nested within other services, allowing for a hiearchy of
 * methods, properties, and streams. Following the chain of parent services will
 * lead you to the #GravitonRootService which is attached to a #GravitonServer.
 * To attach a subservice to a service, use graviton_service_add_subservice().
 *
 * There are other functions for introspection of a service:
 * - graviton_service_list_methods()
 * - graviton_service_has_method()
 * - graviton_service_list_subservices()
 * - graviton_service_list_streams()
 *
 */

#define GRAVITON_SERVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                                        GRAVITON_SERVICE_TYPE, \
                                                                        GravitonServicePrivate))

GQuark
graviton_service_error_quark ()
{
  return g_quark_from_static_string ("graviton-service-error-quark");
}

G_DEFINE_TYPE (GravitonService, graviton_service, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_NAME,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

enum {
  SIGNAL_0,
  SIGNAL_EVENT,
  SIGNAL_PROPERTY_UPDATE,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = {0, };

struct _GravitonServicePrivate
{
  gchar *name;
  GHashTable *services;
  GHashTable *methods;
  GHashTable *method_data;
  GHashTable *method_destroys;
  GHashTable *streams;
  GHashTable *stream_data;
};

static void graviton_service_dispose (GObject *object);
static void graviton_service_finalize (GObject *object);
static void cb_event_from_notify (GravitonService *self,
                                  GParamSpec *pspec,
                                  gpointer user_data);

static void
set_property (GObject *object,
              guint property_id,
              const GValue *value,
              GParamSpec *pspec)
{
  GravitonService *self = GRAVITON_SERVICE (object);
  switch (property_id) {
  case PROP_NAME:
    self->priv->name = g_value_dup_string (value);
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
  GravitonService *self = GRAVITON_SERVICE (object);
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
graviton_service_class_init (GravitonServiceClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  g_type_class_add_private (klass, sizeof (GravitonServicePrivate));

  gobject_class->dispose = graviton_service_dispose;
  gobject_class->finalize = graviton_service_finalize;
  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;

  obj_properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Control name",
                         "Control name",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );

  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);

  /**
   * GravitonService::event:
   * @name: The event name
   * @data: The event data
   *
   * Emitted when graviton_service_emit_event() is called.
   *
   * In normal usage, this is handled by GravitonServer to dispatch events to
   * the cloud.
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
graviton_service_init (GravitonService *self)
{
  GravitonServicePrivate *priv;
  self->priv = priv = GRAVITON_SERVICE_GET_PRIVATE (self);
  priv->name = 0;
  //FIXME: These should be tuples that map callback and data to each other
  priv->streams = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);
  priv->stream_data = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             NULL);
  priv->services = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          g_object_unref);
  priv->methods = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);
  priv->method_data = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             NULL);
  priv->method_destroys = g_hash_table_new_full (g_str_hash,
                                                 g_str_equal,
                                                 g_free,
                                                 NULL);

  //FIXME: Shouldn't use gobject properties since they use GValue. I guess we
  //need our own mechanism :(
  g_signal_connect (self,
                    "notify",
                    G_CALLBACK (cb_event_from_notify),
                    NULL);
}

static void
graviton_service_dispose (GObject *object)
{
  GravitonService *self = GRAVITON_SERVICE (object);
  G_OBJECT_CLASS (graviton_service_parent_class)->dispose (object);
  g_hash_table_unref (self->priv->methods);
  g_hash_table_unref (self->priv->method_data);
  g_hash_table_unref (self->priv->method_destroys);
  g_hash_table_unref (self->priv->streams);
  g_hash_table_unref (self->priv->stream_data);
  g_hash_table_unref (self->priv->services);
}

static void
graviton_service_finalize (GObject *object)
{
  GravitonService *self = GRAVITON_SERVICE (object);
  G_OBJECT_CLASS (graviton_service_parent_class)->finalize (object);
  g_free (self->priv->name);
}

/**
 * graviton_service_add_method:
 * @self: The #GravitonService
 * @name: String name of the method
 * @func: Callback with a #GravitonServiceMethod signature
 * @user_data: (closure): Data for @func
 * @destroy_func: (destroy): Destroy notifier to free @user_data
 *
 * Adds a method to the #GravitonService @self.
 */
void
graviton_service_add_method (GravitonService *self,
                             const gchar *name,
                             GravitonServiceMethod func,
                             gpointer user_data,
                             GDestroyNotify destroy_func)
{
  g_hash_table_replace (self->priv->methods, g_strdup (name), func);
  g_hash_table_replace (self->priv->method_data, g_strdup (name), user_data);
  g_hash_table_replace (self->priv->method_destroys, g_strdup (
                          name), destroy_func);
}

/**
 * graviton_service_call_method:
 * @self: The #GravitonService
 * @name: String name of the method
 * @args: (element-type gchar* GValue): A mapping of argument names to values
 * @error: return location for a #GError or NULL
 */
GVariant *
graviton_service_call_method (GravitonService *self,
                              const gchar *name,
                              GHashTable *args,
                              GError **error)
{
  GravitonServiceMethod func;
  gpointer data;
  func = g_hash_table_lookup (self->priv->methods, name);
  data = g_hash_table_lookup (self->priv->method_data, name);

  g_debug ("Calling %s", name);

  if (func) {
    GVariant *ret = func (self, args, error, data);
    return ret;
  } else {
    g_set_error (error,
                 GRAVITON_SERVICE_ERROR,
                 GRAVITON_SERVICE_ERROR_NO_SUCH_METHOD,
                 "No such method: %s",
                 name);
    return NULL;
  }
}

/**
 * graviton_service_list_methods:
 *
 * Get a list of methods on this service
 *
 * Returns: (element-type *gchar) (transfer none): List of method names
 */
GList *
graviton_service_list_methods (GravitonService *self)
{
  return g_hash_table_get_keys (self->priv->methods);
}

/**
 * graviton_service_has_method:
 * @self: The #GravitonService
 * @name: String name of the method
 *
 * Returns: TRUE if the method exists on this service, FALSE otherwise.
 */
gboolean
graviton_service_has_method (GravitonService *self, const gchar *name)
{
  return g_hash_table_contains (self->priv->methods, name);
}

/**
 * graviton_service_get_subservice:
 * @self: The #GravitonService
 * @path: String path to the subservice
 *
 * Gets the named service.
 *
 * Returns: The service
 */
GravitonService *
graviton_service_get_subservice (GravitonService *self,
                                 const gchar *path)
{
  GravitonService *service = NULL;
  gchar **tokens;
  g_return_val_if_fail (path != NULL, NULL);
  g_return_val_if_fail (strlen(path) > 0, NULL);
  tokens = g_strsplit (path, "/", 0);
  service = g_hash_table_lookup (self->priv->services, tokens[0]);
  if (service) {
    if (g_strv_length (tokens) > 1) {
      gchar *subname = g_strjoinv ("/", &tokens[1]);
      service = graviton_service_get_subservice (service, subname);
      g_free (subname);
    } else {
      g_object_ref (service);
    }
  }
  g_strfreev (tokens);
  return service;
}

static void
cb_event_from_notify (GravitonService *self,
                      GParamSpec *pspec,
                      gpointer user_data)
{
  GVariant *property_data = NULL;
  GValue property_value = G_VALUE_INIT;

  g_value_init (&property_value, pspec->value_type);
  g_object_get_property (G_OBJECT (self), pspec->name, &property_value);

  if (G_VALUE_HOLDS_STRING (&property_value)) {
    const gchar *v = g_value_get_string (&property_value);
    if (v)
      property_data = g_variant_new_string (v);
  } else if (G_VALUE_HOLDS_UINT (&property_value))
    property_data = g_variant_new_uint32 (g_value_get_uint (&property_value));

  if (property_data) {
    graviton_service_emit_event (self, "property", property_data);
  } else {
    g_debug ("Could not convert %s.%s to a GVariant!",
             self->priv->name,
             pspec->name);
  }
}

static void
cb_propagate_event (GravitonService *subservice,
                    const gchar *name,
                    GVariant *data,
                    gpointer user_data)
{
  GravitonService *self = GRAVITON_SERVICE (user_data);
  gchar *full_name;
  if (strlen (self->priv->name)) {
    full_name = g_strdup_printf ("%s/%s", self->priv->name, name);
  } else {
    full_name = g_strdup (name);
  }
  g_signal_emit (self, obj_signals[SIGNAL_EVENT], g_quark_from_string (
                   full_name), full_name, data);

  g_debug ("Propagating event: %s", full_name);
  g_free (full_name);
}

/**
 * graviton_service_add_subservice:
 * @self: The #GravitonService
 * @service: The #GravitonService to add as a sub-service
 *
 * Adds a service to this one as a sub-service
 *
 */
void
graviton_service_add_subservice (GravitonService *self,
                                 GravitonService *service)
{
  g_object_ref (service);
  gchar *name;
  g_object_get (service, "name", &name, NULL);
  g_hash_table_replace (self->priv->services, name, service);

  g_signal_connect (service,
                    "event",
                    G_CALLBACK(cb_propagate_event),
                    self);
}

/**
 * graviton_service_list_subservices:
 *
 * Get a list of the names of available subservices.
 *
 * Returns: (element-type gchar*) (transfer full): the names of the available
 *subservices
 */
GList *
graviton_service_list_subservices (GravitonService *self)
{
  return g_hash_table_get_keys (self->priv->services);
}

/**
 * graviton_service_new:
 * @service_name: Name of the service to use
 *
 * Creates a new #GravitonService with a service name
 *
 * Returns: A new #GravitonService that exposes the given @service_name
 */
GravitonService *
graviton_service_new (const gchar *service_name)
{
  return g_object_new (GRAVITON_SERVICE_TYPE, "name", service_name, NULL);
}

void
graviton_service_emit_event (GravitonService *self,
                             const gchar *name,
                             GVariant *data)
{
  gchar *full_name;

  full_name = g_strdup_printf ("%s.%s", self->priv->name, name);
  g_signal_emit (self, obj_signals[SIGNAL_EVENT], g_quark_from_string (
                   full_name), full_name, data);
  g_debug ("Dispatch event: %s", full_name);
  g_free (full_name);
}

#ifdef GRAVITON_ENABLE_STREAMS
/**
 * graviton_service_add_stream:
 * @self: The #GravitonService
 * @name: String name of the stream to add
 * @func: A #GravitonServiceStreamGenerator callback to use
 * @user_data: Data that is passed to @func
 *
 * Registers a new stream on this service using the given name
 */
void
graviton_service_add_stream (GravitonService *self,
                             const gchar *name,
                             GravitonServiceStreamGenerator func,
                             gpointer user_data)
{
  g_hash_table_replace (self->priv->streams, g_strdup (name), func);
  g_hash_table_replace (self->priv->stream_data, g_strdup (name), user_data);
}

/**
 * graviton_service_list_streams:
 *
 * Get a list of the streams that are available from this service
 *
 * Returns: (element-type gchar*) (transfer full): the names of the available
 * streams
 */
GList *
graviton_service_list_streams (GravitonService *self)
{
  return g_hash_table_get_keys (self->priv->streams);
}

/**
 * graviton_service_get_stream:
 * @self: The #GravitonStream
 * @name: String name of the stream requested
 * @args: (element-type gchar* GValue): Mapping of arguments to values
 * @error: Return location for a #GError, or NULL
 */
GravitonStream *
graviton_service_get_stream (GravitonService *self,
                             const gchar *name,
                             GHashTable *args,
                             GError **error)
{
  GravitonServiceStreamGenerator func = g_hash_table_lookup (
    self->priv->streams,
    name);
  gpointer func_data = g_hash_table_lookup (self->priv->stream_data, name);
  if (func)
    return func(self, name, args, error, func_data);
  return NULL;
}
#endif // GRAVITON_ENABLE_STREAMS
