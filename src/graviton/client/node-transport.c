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

#include "node-transport.h"

static void graviton_node_transport_class_init (GravitonNodeTransportClass *klass);
static void graviton_node_transport_init       (GravitonNodeTransport *self);
static void graviton_node_transport_dispose    (GObject *object);
static void graviton_node_transport_finalize   (GObject *object);
static void graviton_node_transport_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_node_transport_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonNodeTransport, graviton_node_transport, G_TYPE_OBJECT);

enum {
  SIGNAL_0,
  SIGNAL_EVENT,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

static void
graviton_node_transport_class_init (GravitonNodeTransportClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = graviton_node_transport_dispose;
  object_class->finalize = graviton_node_transport_finalize;
  object_class->set_property =  graviton_node_transport_set_property;
  object_class->get_property =  graviton_node_transport_get_property;

  /**
   * GravitonNodeTransport::event:
   * @node_id: Node ID that this originated from
   * @name: Event name
   * @data: Event data
   *
   * Emitted when the transport receives an event from a node on the other side.
   * Detail is the event name.
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
                  3,
                  G_TYPE_STRING,
                  G_TYPE_STRING,
                  G_TYPE_VARIANT);

}

static void
graviton_node_transport_set_property (GObject *object,
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
graviton_node_transport_get_property (GObject *object,
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
graviton_node_transport_init (GravitonNodeTransport *self)
{
}

static void
graviton_node_transport_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_transport_parent_class)->dispose (object);
}

static void
graviton_node_transport_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_transport_parent_class)->finalize (object);
}

GIOStream *
graviton_node_transport_open_stream (GravitonNodeTransport *self,
                                     GravitonNode *node,
                                     const gchar *name,
                                     GHashTable *args,
                                     GError **error)
{
  GravitonNodeTransportClass *klass = GRAVITON_NODE_TRANSPORT_GET_CLASS (self);
  return klass->open_stream (self, node, name, args, error);
}

GVariant *
graviton_node_transport_call_args (GravitonNodeTransport *self,
                                   GravitonNode *node,
                                   const gchar *method,
                                   GHashTable *args,
                                   GError **error)
{
  GravitonNodeTransportClass *klass = GRAVITON_NODE_TRANSPORT_GET_CLASS (self);
  return klass->call_args (self, node, method, args, error);
}

void
graviton_node_transport_emit_event (GravitonNodeTransport *self,
                                    const gchar *node_id,
                                    const gchar *name,
                                    GVariant *data)
{
  g_debug ("Emitting a %s event for %s", name, node_id);
  g_signal_emit (self, obj_signals[SIGNAL_EVENT], g_quark_from_string (name), node_id, name, data);
}
