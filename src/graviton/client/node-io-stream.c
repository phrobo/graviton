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

#include "node-input-stream.h"
#include "node-io-stream.h"

typedef struct _GravitonNodeIOStreamPrivate GravitonNodeIOStreamPrivate;

struct _GravitonNodeIOStreamPrivate
{
  SoupURI *uri;
  SoupSession *session;
  GravitonNodeInputStream *input;
  GOutputStream *output;
};

#define GRAVITON_NODE_IO_STREAM_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_IO_STREAM_TYPE, \
                                GravitonNodeIOStreamPrivate))

static void graviton_node_io_stream_class_init (GravitonNodeIOStreamClass *klass);
static void graviton_node_io_stream_init       (GravitonNodeIOStream *self);
static void graviton_node_io_stream_dispose    (GObject *object);
static void graviton_node_io_stream_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonNodeIOStream, graviton_node_io_stream, G_TYPE_IO_STREAM);

enum {
  PROP_0,
  PROP_URI,
  PROP_SESSION,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
set_property (GObject *object,
              guint property_id,
              const GValue *value,
              GParamSpec *pspec)
{
  GravitonNodeIOStream *self = GRAVITON_NODE_IO_STREAM (object);
  switch (property_id) {
  case PROP_URI:
    self->priv->uri = g_value_dup_boxed (value);
    break;
  case PROP_SESSION:
    self->priv->session = SOUP_SESSION (g_value_dup_object (value));
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
  GravitonNodeIOStream *self = GRAVITON_NODE_IO_STREAM (object);
  switch (property_id) {
  case PROP_URI:
    g_value_set_boxed (value, self->priv->uri);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static GOutputStream *
get_output_stream (GIOStream *stream)
{
  GravitonNodeIOStream *self = GRAVITON_NODE_IO_STREAM (stream);
  if (self->priv->output == NULL) {
  }
  return self->priv->output;
}

static GInputStream *
get_input_stream (GIOStream *stream)
{
  GravitonNodeIOStream *self = GRAVITON_NODE_IO_STREAM (stream);
  if (self->priv->input == NULL) {
    self->priv->input = graviton_node_input_stream_new (self);
  }
  return G_INPUT_STREAM (self->priv->input);
}

static void
graviton_node_io_stream_class_init (GravitonNodeIOStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNodeIOStreamPrivate));

  object_class->dispose = graviton_node_io_stream_dispose;
  object_class->finalize = graviton_node_io_stream_finalize;

  object_class->get_property = get_property;
  object_class->set_property = set_property;

  GIOStreamClass *gio_class = G_IO_STREAM_CLASS (klass);
  gio_class->get_input_stream = get_input_stream;
  gio_class->get_output_stream = get_output_stream;

  obj_properties[PROP_URI] =
    g_param_spec_boxed ("uri",
                        "uri",
                        "URI to stream from",
                        SOUP_TYPE_URI,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );
  obj_properties [PROP_SESSION] =
    g_param_spec_object ("session",
                         "SoupSession",
                         "The underlying SoupSession (for proxies, if needed)",
                         SOUP_TYPE_SESSION,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );
  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
graviton_node_io_stream_init (GravitonNodeIOStream *self)
{
  self->priv = GRAVITON_NODE_IO_STREAM_GET_PRIVATE (self);
}

static void
graviton_node_io_stream_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_io_stream_parent_class)->dispose (object);
}

static void
graviton_node_io_stream_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_io_stream_parent_class)->finalize (object);
}

GravitonNodeIOStream *
graviton_node_io_stream_new (SoupURI *uri, SoupSession *session)
{
  return g_object_new (GRAVITON_NODE_IO_STREAM_TYPE,
                       "uri",
                       uri,
                       "session",
                       session,
                       NULL);
}

SoupURI *
graviton_node_io_stream_get_uri (GravitonNodeIOStream *self)
{
  return self->priv->uri;
}

SoupSession *
graviton_node_io_stream_get_session (GravitonNodeIOStream *self)
{
  return self->priv->session;
}