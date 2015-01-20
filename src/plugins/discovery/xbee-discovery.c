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

#include "xbee-discovery.h"

#include <geebee/xbee.h>

typedef struct _GravitonXbeeDiscoveryMethodPrivate GravitonXbeeDiscoveryMethodPrivate;

struct _GravitonXbeeDiscoveryMethodPrivate
{
  GeebeeXbee *bee;
};

#define GRAVITON_XBEE_DISCOVERY_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_XBEE_DISCOVERY_METHOD_TYPE, GravitonXbeeDiscoveryMethodPrivate))

static void graviton_xbee_discovery_method_class_init (GravitonXbeeDiscoveryMethodClass *klass);
static void graviton_xbee_discovery_method_init       (GravitonXbeeDiscoveryMethod *self);
static void graviton_xbee_discovery_method_dispose    (GObject *object);
static void graviton_xbee_discovery_method_finalize   (GObject *object);
static void graviton_xbee_discovery_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_xbee_discovery_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

GRAVITON_DEFINE_DISCOVERY_PLUGIN (GRAVITON_XBEE_DISCOVERY_METHOD_TYPE)

G_DEFINE_TYPE (GravitonXbeeDiscoveryMethod, graviton_xbee_discovery_method, GRAVITON_DISCOVERY_METHOD_TYPE);

static void
cb_frame (GeebeeXbee *xbee, GeebeePacket *reply, gpointer data)
{
  GravitonXbeeDiscoveryMethod *self = GRAVITON_XBEE_DISCOVERY_METHOD (data);
  switch (reply->api_id) {
    case RemoteAtResponse:
      g_print ("Got response from ND!\n");
      break;
    default:
      g_print ("Got some weird packet\n");
  }
}

static void
start_browse (GravitonDiscoveryMethod *method)
{
  GravitonXbeeDiscoveryMethod *self = GRAVITON_XBEE_DISCOVERY_METHOD (method);
  GError *error = NULL;

  GeebeeRemoteAtCommandRequest *req = geebee_remote_at_command_request_new (GEEBEE_BROADCAST_ADDR64, GEEBEE_BROADCAST_ADDR16, "ND", NULL, 0);

  geebee_xbee_send_async (self->priv->bee, (GeebeePacket*)req, NULL, NULL, NULL);
  geebee_packet_unref ((GeebeePacket*)req);
  g_print ("Started browsing XBEE things\n");
}

static void
stop_browse (GravitonDiscoveryMethod *method)
{
}

enum {
  PROP_ZERO,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
graviton_xbee_discovery_method_class_init (GravitonXbeeDiscoveryMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonXbeeDiscoveryMethodPrivate));

  object_class->dispose = graviton_xbee_discovery_method_dispose;
  object_class->finalize = graviton_xbee_discovery_method_finalize;
  object_class->set_property =  graviton_xbee_discovery_method_set_property;
  object_class->get_property =  graviton_xbee_discovery_method_get_property;
  /*g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);*/

  GravitonDiscoveryMethodClass *method_class = GRAVITON_DISCOVERY_METHOD_CLASS (
    klass);
  method_class->start = start_browse;
  method_class->stop = stop_browse;
}

static void
graviton_xbee_discovery_method_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonXbeeDiscoveryMethod *self = GRAVITON_XBEE_DISCOVERY_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_xbee_discovery_method_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonXbeeDiscoveryMethod *self = GRAVITON_XBEE_DISCOVERY_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}
static void
graviton_xbee_discovery_method_init (GravitonXbeeDiscoveryMethod *self)
{
  GravitonXbeeDiscoveryMethodPrivate *priv;
  priv = self->priv = GRAVITON_XBEE_DISCOVERY_METHOD_GET_PRIVATE (self);
  priv->bee = geebee_xbee_new ();
  g_signal_connect (priv->bee, "new-frame", G_CALLBACK (cb_frame), self);
}

static void
graviton_xbee_discovery_method_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_xbee_discovery_method_parent_class)->dispose (object);
}

static void
graviton_xbee_discovery_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_xbee_discovery_method_parent_class)->finalize (object);
}
