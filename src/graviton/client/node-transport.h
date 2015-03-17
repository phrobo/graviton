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

#ifndef __GRAVITON_NODE_TRANSPORT_H__
#define __GRAVITON_NODE_TRANSPORT_H__

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define GRAVITON_NODE_TRANSPORT_TYPE            ( \
    graviton_node_transport_get_type ())
#define GRAVITON_NODE_TRANSPORT(obj)            (G_TYPE_CHECK_INSTANCE_CAST (( \
                                                                               obj), \
                                                                             GRAVITON_NODE_TRANSPORT_TYPE, \
                                                                             GravitonNodeTransport))
#define GRAVITON_NODE_TRANSPORT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                                          GRAVITON_NODE_TRANSPORT_TYPE, \
                                                                          GravitonNodeTransportClass))
#define IS_GRAVITON_NODE_TRANSPORT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (( \
                                                                               obj), \
                                                                             GRAVITON_NODE_TRANSPORT_TYPE))
#define IS_GRAVITON_NODE_TRANSPORT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                                          GRAVITON_NODE_TRANSPORT_TYPE))
#define GRAVITON_NODE_TRANSPORT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                                            GRAVITON_NODE_TRANSPORT_TYPE, \
                                                                            GravitonNodeTransportClass))

typedef struct _GravitonNodeTransport GravitonNodeTransport;
typedef struct _GravitonNodeTransportClass GravitonNodeTransportClass;

typedef struct _GravitonNode GravitonNode;

struct _GravitonNodeTransportClass
{
  GObjectClass parent_class;
  GVariant *(*call_args)(GravitonNodeTransport *self, GravitonNode *node,
                         const gchar*method, GHashTable *args, GError **err);
  gboolean (*subscribe_events)(GravitonNodeTransport *self, GravitonNode *node, const gchar *name, GError **err);
  gboolean (*unsubscribe_events)(GravitonNodeTransport *self, GravitonNode *node, const gchar *name, GError **err);

#ifdef GRAVITON_ENABLE_STREAMS
  GIOStream *(*open_stream)(GravitonNodeTransport *self, GravitonNode *node,
                            const gchar *name, GHashTable *args, GError **err);
#endif // GRAVITON_ENABLE_STREAMS
};

struct _GravitonNodeTransport
{
  GObject parent;
};

GType graviton_node_transport_get_type (void);

GravitonNodeTransport *graviton_node_transport_new ();

GVariant *graviton_node_transport_call_args (GravitonNodeTransport *self,
                                             GravitonNode *node,
                                             const gchar *method,
                                             GHashTable *args,
                                             GError **error);

void graviton_node_transport_emit_event (GravitonNodeTransport *self,
                                         const gchar *node_id,
                                         const gchar *name,
                                         guint64 event_id,
                                         GVariant *data);

gboolean graviton_node_transport_subscribe_events (GravitonNodeTransport *transport,
                                                   GravitonNode *node,
                                                   const gchar *name,
                                                   GError **error);
gboolean graviton_node_transport_unsubscribe_events (GravitonNodeTransport *transport,
                                                     GravitonNode *node,
                                                     const gchar *name,
                                                     GError **error);

#ifdef GRAVITON_ENABLE_STREAMS
GIOStream *graviton_node_transport_open_stream (GravitonNodeTransport *self,
                                                GravitonNode *node,
                                                const gchar *name,
                                                GHashTable *args,
                                                GError **error);
#endif // GRAVITON_ENABLE_STREAMS

G_END_DECLS

#endif
