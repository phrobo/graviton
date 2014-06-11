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

#include "config.h"
#include "introspection-service.h"
#include "server.h"
#include "service.h"
#include "file-stream.h"
#include <json-glib/json-glib.h>

#define GRAVITON_INTROSPECTION_CONTROL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_INTROSPECTION_CONTROL_TYPE, GravitonIntrospectionControlPrivate))

//GRAVITON_DEFINE_PLUGIN(GRAVITON_INTROSPECTION_CONTROL_TYPE, "graviton")

GQuark
graviton_introspection_error_quark()
{
  return g_quark_from_static_string ("graviton-introspection-error-quark");
}

G_DEFINE_TYPE (GravitonIntrospectionControl, graviton_internal_plugin, GRAVITON_SERVICE_TYPE);

enum
{
  PROP_0,
  PROP_SERVER,
  PROP_HOSTNAME,
  PROP_NODE_ID,
  PROP_CLOUD_ID,
  N_PROPERTIES
};

struct _GravitonIntrospectionControlPrivate
{
  GravitonServer *server;
  gchar *hostname;
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static GravitonService *
grab_service_arg (GravitonIntrospectionControl *self, GHashTable *args, GError **error)
{
  GravitonService *subservice;
  const gchar *service_name = NULL;
  GravitonRootService *plugins = graviton_server_get_root_service (self->priv->server);

  if (g_hash_table_lookup (args, "service")) {
    GVariant *serviceArg = g_hash_table_lookup (args, "service");
    if (g_variant_is_of_type (serviceArg, G_VARIANT_TYPE_STRING))
      service_name = g_variant_get_string (serviceArg, NULL);
  }

  if (service_name) {
    g_debug ("Querying service %s", service_name);
    subservice = graviton_service_get_subservice (GRAVITON_SERVICE (plugins), service_name);
    g_object_unref (plugins);
  } else {
    g_debug ("Querying plugin manager");
    subservice = GRAVITON_SERVICE (plugins);
  }

  if (!subservice) {
    g_set_error (error,
                 GRAVITON_INTROSPECTION_ERROR,
                 GRAVITON_INTROSPECTION_ERROR_NO_SUCH_CONTROL,
                 "No such service %s", service_name);
  }

  return subservice;
}


static void
set_property (GObject *object,
              guint property_id,
              const GValue *value,
              GParamSpec *pspec)
{
  GravitonIntrospectionControl *self = GRAVITON_INTROSPECTION_CONTROL (object);

  switch (property_id) {
    case PROP_SERVER:
      if (self->priv->server)
        g_object_unref (self->priv->server);
      self->priv->server = g_value_get_object (value);
      break;
    case PROP_HOSTNAME:
      if (self->priv->hostname)
        g_free (self->priv->hostname);
      self->priv->hostname = g_value_dup_string (value);
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
  GravitonIntrospectionControl *self = GRAVITON_INTROSPECTION_CONTROL (object);
  
  switch (property_id) {
    case PROP_SERVER:
      g_value_set_object (value, self->priv->server);
      break;
    case PROP_HOSTNAME:
      g_value_set_string (value, self->priv->hostname);
      break;
    case PROP_NODE_ID:
      g_value_set_string (value, graviton_server_get_node_id (self->priv->server));
      break;
    case PROP_CLOUD_ID:
      g_value_set_string (value, graviton_server_get_cloud_id (self->priv->server));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_internal_plugin_class_init (GravitonIntrospectionControlClass *klass)
{
  g_type_class_add_private (klass, sizeof (GravitonIntrospectionControlPrivate));

  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;

  obj_properties[PROP_SERVER] =
    g_param_spec_object ("server",
                         "Server",
                         "GravitonServer object",
                         GRAVITON_SERVER_TYPE,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  obj_properties[PROP_HOSTNAME] =
    g_param_spec_string ("hostname",
                         "Hostname",
                         "Hostname",
                         "",
                         G_PARAM_READWRITE);
  obj_properties[PROP_NODE_ID] =
    g_param_spec_string ("node-id",
                         "Node UUID",
                         "Universally Unique Node ID",
                         "",
                         G_PARAM_READABLE);
  obj_properties[PROP_CLOUD_ID] =
    g_param_spec_string ("cloud-id",
                         "Cloud UUID",
                         "Universally Unique Cloud ID",
                         "",
                         G_PARAM_READABLE);
  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static GravitonStream *
cb_stream_zero (GravitonService *self, const gchar *name, GHashTable *args, GError **error, gpointer user_data)
{
  GFile *dev = g_file_new_for_path ("/dev/zero");
  GravitonStream *ret = GRAVITON_STREAM (graviton_file_stream_new (dev));
  g_object_unref (dev);
  return ret;
}

static GVariant *
cb_streams(GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonService *subservice;
  GravitonIntrospectionControl *self = GRAVITON_INTROSPECTION_CONTROL (user_data);

  subservice = grab_service_arg (self, args, error);
  
  if (!subservice)
    return NULL;

  g_variant_builder_init (&ret, G_VARIANT_TYPE_STRING_ARRAY);
  GList *streams = graviton_service_list_streams (subservice);
  GList *cur = streams;
  while (cur) {
    g_variant_builder_add (&ret, "s", cur->data);
    cur = cur->next;
  }
  g_object_unref (subservice);

  return g_variant_builder_end (&ret);
}


static GVariant *
cb_properties(GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonService *subservice;
  GravitonIntrospectionControl *self = GRAVITON_INTROSPECTION_CONTROL (user_data);

  subservice = grab_service_arg (self, args, error);
  
  if (!subservice)
    return NULL;

  GParamSpec **properties;
  guint property_count;
  guint i;
  properties = g_object_class_list_properties (G_OBJECT_GET_CLASS (subservice), &property_count);
  g_variant_builder_init (&ret, G_VARIANT_TYPE_STRING_ARRAY);
  for (i = 0; i<property_count; i++) {
    g_variant_builder_add (&ret, "s", properties[i]->name);
  }

  g_free (properties);

  g_object_unref (subservice);

  return g_variant_builder_end (&ret);
}

static GVariant *
cb_get_property (GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  GravitonService *subservice;
  GravitonIntrospectionControl *self = GRAVITON_INTROSPECTION_CONTROL (user_data);

  subservice = grab_service_arg (self, args, error);

  if (!subservice)
    return NULL;

  const gchar *property_name;
  property_name = g_variant_get_string (g_hash_table_lookup (args, "property"), NULL);
  GParamSpec *property = g_object_class_find_property (G_OBJECT_GET_CLASS (subservice), property_name);
  if (property) {
    GValue property_value = G_VALUE_INIT;
    GVariant *converted_variant = NULL;
    g_value_init (&property_value, property->value_type);
    g_object_get_property (G_OBJECT(subservice), property->name, &property_value);
    if (G_VALUE_HOLDS_STRING (&property_value)) {
      converted_variant = g_variant_new_string (g_value_get_string (&property_value));
    } else if (G_VALUE_HOLDS_VARIANT (&property_value)) {
      converted_variant = g_value_get_variant (&property_value);
      g_variant_ref (converted_variant);
    } else {
      g_debug ("Unsupported value type: %s", G_VALUE_TYPE_NAME (&property_value));
    }
    g_value_unset (&property_value);
    return converted_variant;
  } else {
    g_set_error (error,
                 GRAVITON_INTROSPECTION_ERROR,
                 GRAVITON_INTROSPECTION_ERROR_NO_SUCH_PROPERTY,
                 "No such property %s", property_name);
  }
  return NULL;
}

static GVariant *
cb_nodes(GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  return NULL;
}

static GVariant *
cb_clouds(GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  return NULL;
}

static GVariant *
cb_services(GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonService *subservice;
  GravitonIntrospectionControl *self = GRAVITON_INTROSPECTION_CONTROL (user_data);

  subservice = grab_service_arg (self, args, error);

  if (!subservice) {
    return NULL;
  }

  GList *services = graviton_service_list_subservices (subservice);
  g_variant_builder_init (&ret, G_VARIANT_TYPE_STRING_ARRAY);
  GList *cur = services;
  while (cur) { 
    g_debug ("Found service %s", (gchar*)cur->data);
    g_variant_builder_add (&ret, "s", cur->data);
    cur = g_list_next (cur);
  }

  g_object_unref (subservice);

  return g_variant_builder_end (&ret);
}

static GVariant *
cb_methods(GravitonService *service, GHashTable *args, GError **error, gpointer user_data)
{
  GVariantBuilder ret;
  GravitonService *subservice;
  GravitonIntrospectionControl *self = GRAVITON_INTROSPECTION_CONTROL (user_data);

  subservice = grab_service_arg (self, args, error);

  if (!subservice)
    return NULL;

  GList *methods = graviton_service_list_methods (subservice);
  g_variant_builder_init (&ret, G_VARIANT_TYPE_STRING_ARRAY);

  GList *cur = methods;
  while (cur) { 
    g_variant_builder_add (&ret, "s", cur->data);
    cur = g_list_next (cur);
  }

  g_object_unref (subservice);

  return g_variant_builder_end (&ret);
}

static void
graviton_internal_plugin_init (GravitonIntrospectionControl *self)
{
  GravitonIntrospectionControlPrivate *priv;
  self->priv = priv = GRAVITON_INTROSPECTION_CONTROL_GET_PRIVATE (self);
  self->priv->hostname = g_strdup (g_get_host_name ());

  GravitonService *introspection = g_object_new (GRAVITON_SERVICE_TYPE, "name", "introspection", NULL);
  graviton_service_add_method (introspection,
                               "listControls",
                               cb_services,
                               self,
                               NULL);
  graviton_service_add_method (introspection,
                               "listMethods",
                               cb_methods,
                               self,
                               NULL);
  graviton_service_add_method (introspection,
                               "listProperties",
                               cb_properties,
                               self,
                               NULL);
  graviton_service_add_method (introspection,
                               "listStreams",
                               cb_streams,
                               self,
                               NULL);
  graviton_service_add_method (introspection,
                               "listNodes",
                               cb_nodes,
                               self,
                               NULL);
  graviton_service_add_method (introspection,
                               "listClouds",
                               cb_clouds,
                               self,
                               NULL);
  graviton_service_add_method (introspection,
                               "getProperty",
                               cb_get_property,
                               self,
                               NULL);
  graviton_service_add_stream (introspection,
                               "zero",
                               cb_stream_zero,
                               self);

  graviton_service_add_subservice (GRAVITON_SERVICE (self), introspection);
}
