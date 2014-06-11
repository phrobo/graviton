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

#ifndef __GRAVITON_NODE_IO_STREAM_H__
#define __GRAVITON_NODE_IO_STREAM_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <libsoup/soup.h>

G_BEGIN_DECLS

#define GRAVITON_NODE_IO_STREAM_TYPE            (graviton_node_io_stream_get_type ())
#define GRAVITON_NODE_IO_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NODE_IO_STREAM_TYPE, GravitonNodeIOStream))
#define GRAVITON_NODE_IO_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NODE_IO_STREAM_TYPE, GravitonNodeIOStreamClass))
#define IS_GRAVITON_NODE_IO_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NODE_IO_STREAM_TYPE))
#define IS_GRAVITON_NODE_IO_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NODE_IO_STREAM_TYPE))
#define GRAVITON_NODE_IO_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NODE_IO_STREAM_TYPE, GravitonNodeIOStreamClass))

typedef struct _GravitonNodeIOStream      GravitonNodeIOStream;
typedef struct _GravitonNodeIOStreamClass GravitonNodeIOStreamClass;
typedef struct _GravitonNodeIOStreamPrivate      GravitonNodeIOStreamPrivate;

struct _GravitonNodeIOStreamClass
{
  GIOStreamClass parent_class;
};

struct _GravitonNodeIOStream
{
  GIOStream parent;
  GravitonNodeIOStreamPrivate *priv;
};

GType graviton_node_io_stream_get_type (void);

GravitonNodeIOStream *graviton_node_io_stream_new (SoupURI *uri, SoupSession *session);

SoupURI *graviton_node_io_stream_get_uri (GravitonNodeIOStream *stream);
SoupSession *graviton_node_io_stream_get_session (GravitonNodeIOStream *stream);

G_END_DECLS

#endif
