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

#ifndef __GRAVITON_JSONRPC_NODE_TRANSPORT_H__
#define __GRAVITON_JSONRPC_NODE_TRANSPORT_H__

#include <glib.h>
#include <glib-object.h>
#include "node-transport.h"

G_BEGIN_DECLS

#define GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE            (graviton_jsonrpc_node_transport_get_type ())
#define GRAVITON_JSONRPC_NODE_TRANSPORT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE, GravitonJsonrpcNodeTransport))
#define GRAVITON_JSONRPC_NODE_TRANSPORT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE, GravitonJsonrpcNodeTransportClass))
#define GRAVITON_IS_JSONRPC_NODE_TRANSPORT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE))
#define GRAVITON_IS_JSONRPC_NODE_TRANSPORT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE))
#define GRAVITON_JSONRPC_NODE_TRANSPORT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_JSONRPC_NODE_TRANSPORT_TYPE, GravitonJsonrpcNodeTransportClass))

typedef struct _GravitonJsonrpcNodeTransport      GravitonJsonrpcNodeTransport;
typedef struct _GravitonJsonrpcNodeTransportClass GravitonJsonrpcNodeTransportClass;
typedef struct _GravitonJsonrpcNodeTransportPrivate GravitonJsonrpcNodeTransportPrivate;

struct _GravitonJsonrpcNodeTransportClass
{
  GravitonNodeTransportClass parent_class;
};

struct _GravitonJsonrpcNodeTransport
{
  GravitonNodeTransport parent;
  GravitonJsonrpcNodeTransportPrivate *priv;
};

GType graviton_jsonrpc_node_transport_get_type (void);

GravitonJsonrpcNodeTransport *graviton_jsonrpc_node_transport_new (GInetSocketAddress *address);
const gchar *graviton_jsonrpc_node_transport_get_node_id (GravitonJsonrpcNodeTransport *transport);

G_END_DECLS

#endif
