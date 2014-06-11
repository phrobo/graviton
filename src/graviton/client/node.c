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

#include "node.h"
#include "cloud.h"
#include "node-io-stream.h"
#include "introspection-interface.h"

#include <string.h>

typedef struct _GravitonNodePrivate GravitonNodePrivate;

struct _GravitonNodePrivate
{
  GravitonServiceInterface *gobj;
  //FIXME: Replace with a hash table
  GPtrArray *transports;
  gchar *node_id;
};

#define GRAVITON_NODE_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_TYPE, GravitonNodePrivate))

GQuark
graviton_node_error_quark ()
{
  return g_quark_from_static_string ("graviton-node-error-quark");
}

static void graviton_node_class_init (GravitonNodeClass *klass);
static void graviton_node_init       (GravitonNode *self);
static void graviton_node_dispose    (GObject *object);
static void graviton_node_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonNode, graviton_node, GRAVITON_SERVICE_INTERFACE_TYPE);

enum {
  PROP_0,
  PROP_NODE_ID,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  SIGNAL_TRANSPORT_ADDED,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
unref_arg (GVariant *var)
{
  if (var)
    g_variant_unref (var);
}

static void
set_property (GObject *object,
                     guint property_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  GravitonNode *self = GRAVITON_NODE (object);
  switch (property_id) {
    case PROP_NODE_ID:
      self->priv->node_id = g_value_dup_string (value);
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
  GravitonNode *self = GRAVITON_NODE (object);
  switch (property_id) {
    case PROP_NODE_ID:
      g_value_set_string (value, self->priv->node_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_node_class_init (GravitonNodeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNodePrivate));

  object_class->dispose = graviton_node_dispose;
  object_class->finalize = graviton_node_finalize;

  object_class->set_property = set_property;
  object_class->get_property = get_property;

  obj_properties[PROP_NODE_ID] = 
    g_param_spec_string ("node-id",
                         "Node UUID",
                         "Universally Unique Node ID",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);

  /**
   * GravitonNode::transport-added:
   * @node: The node that has a new transport
   * @transport: The transport that was added
   *
   * A #GravitonNode has had a new transport added. Consumers should consider
   * the priority of the new transport when possible.
   */
  obj_signals[SIGNAL_TRANSPORT_ADDED] =
    g_signal_new ("transport-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GRAVITON_NODE_TRANSPORT_TYPE);
}

static void
graviton_node_init (GravitonNode *self)
{
  GravitonNodePrivate *priv;
  self->priv = priv = GRAVITON_NODE_GET_PRIVATE (self);
  self->priv->transports = g_ptr_array_new_with_free_func (g_object_unref);
  self->priv->gobj = graviton_service_interface_get_subservice (GRAVITON_SERVICE_INTERFACE (self), "net:phrobo:graviton");
  g_assert (graviton_service_interface_get_node (self->priv->gobj) == self);
}

static void
graviton_node_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_parent_class)->dispose (object);
  GravitonNode *self = GRAVITON_NODE (object);
  g_object_unref (self->priv->gobj);
  g_ptr_array_free (self->priv->transports, TRUE);

  self->priv->gobj = NULL;
  self->priv->transports = NULL;
}

static void
graviton_node_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_parent_class)->finalize (object);
  GravitonNode *self = GRAVITON_NODE (object);
  if (self->priv->node_id)
    g_free (self->priv->node_id);
  self->priv->node_id = NULL;
}

GravitonNode *
graviton_node_proxy_to_id (GravitonNode *node,
                           const gchar *id,
                           GError **error)
{
  return NULL;
}

const gchar *
graviton_node_get_id (GravitonNode *self, GError **err)
{
  if (self->priv->node_id == NULL) {
    GVariant *ret = graviton_service_interface_get_property (self->priv->gobj, "node-id", err);
    if (ret) {
      self->priv->node_id = g_variant_dup_string (ret, NULL);
      g_variant_unref (ret);
    }
  }
  return self->priv->node_id;
}

const gchar *
graviton_node_get_cloud_id (GravitonNode *self, GError **err)
{
  GVariant *ret = graviton_service_interface_get_property (self->priv->gobj, "cloud-id", err);
  if (ret) {
    gchar *r = g_variant_dup_string (ret, NULL);
    g_variant_unref (ret);
    return r;
  }
  return NULL;
}

GVariant *
graviton_node_call (GravitonNode *self,
                    const gchar *method,
                    GError **err,
                    ...)
{
  va_list argList;
  va_start (argList, err);
  GVariant *ret = graviton_node_call_va (self, method, err, argList);
  va_end (argList);
  return ret;
}

GVariant *
graviton_node_call_va (GravitonNode *self,
                       const gchar *method,
                       GError **err,
                       va_list argList)
{
  gchar *propName = NULL;
  GVariant *propValue = NULL;
  GHashTable *args = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            NULL,
                                            (GDestroyNotify)unref_arg);
  propName = va_arg (argList, gchar*);

  while (propName != NULL) {
    propValue = va_arg (argList, GVariant*);
    g_hash_table_replace (args, propName, propValue);
    if (propValue) {
      gchar *v = g_variant_print (propValue, TRUE);
      g_debug ("%s = %s", propName, v);
      g_free (v);
    } else {
      g_debug ("%s = NULL", propName);
    }
    propName = va_arg (argList, gchar*);
  }
  va_end (argList);

  GVariant *ret = graviton_node_call_args (self,
                                           method,
                                           args,
                                           err);
  g_hash_table_unref (args);
  return ret;
}

GList *
graviton_node_get_services (GravitonNode *node, GError **error)
{
  //FIXME: handle errors
  GravitonIntrospectionControl *introspection = graviton_introspection_interface_new_from_interface (GRAVITON_SERVICE_INTERFACE (node));
  GList *ret;
  ret = graviton_introspection_interface_list_interfaces (introspection, NULL);
  g_object_unref (introspection);
  return ret;
}

GravitonServiceInterface *
graviton_node_get_service_interface (GravitonNode *node, const gchar *name, GError **error)
{
  return graviton_service_interface_get_subservice (GRAVITON_SERVICE_INTERFACE (node), name);
}

gboolean
graviton_node_has_service (GravitonNode *node, const gchar *name, GError **err)
{
  gboolean ret = FALSE;
  GError *error = NULL;
  GList *services;
  GList *cur;
  GravitonIntrospectionControl *introspection;

  introspection = graviton_introspection_interface_new_from_interface (GRAVITON_SERVICE_INTERFACE (node));
  services = graviton_introspection_interface_list_interfaces (introspection, &error);
  g_object_unref (introspection);
  cur = services;
  while (cur) {
    if (strcmp (cur->data, name) == 0) {
      ret = TRUE;
      break;
    }
    cur = cur->next;
  }
  g_list_free_full (services, g_free);

  return ret;
}

GVariant *
graviton_node_call_args (GravitonNode *self,
                         const gchar *method,
                         GHashTable *args,
                         GError **err)
{
  GravitonNodeTransport *transport = graviton_node_get_default_transport (self);
  return graviton_node_transport_call_args (transport, self, method, args, err);
}

GIOStream *
graviton_node_open_stream (GravitonNode *self,
                           const gchar *name,
                           GHashTable *args)
{
  GravitonNodeTransport *transport = graviton_node_get_default_transport (self);
  GIOStream *ret = graviton_node_transport_open_stream (transport, self, name, args, NULL);
  g_object_unref (transport);
  return ret;
}

//FIXME: Check refcounts for node_*_transport functions
void
graviton_node_add_transport (GravitonNode *self,
                             GravitonNodeTransport *transport,
                            int priority)
{
  g_object_ref_sink (transport);
  g_ptr_array_add (self->priv->transports, transport);
  g_signal_emit (self, obj_signals[SIGNAL_TRANSPORT_ADDED], 0, transport);
}

GPtrArray *
graviton_node_get_transports (GravitonNode *node, int priority)
{
  return node->priv->transports;
}

GravitonNodeTransport *graviton_node_get_default_transport (GravitonNode *node)
{
  GPtrArray *transports;
  GravitonNodeTransport *transport;

  g_debug ("Fetching default transport");
  transports = graviton_node_get_transports (node, 0);
  transport = GRAVITON_NODE_TRANSPORT (g_ptr_array_index (transports, 0));
  g_object_ref (G_OBJECT (transport));

  g_assert (transport);

  return transport;
}
