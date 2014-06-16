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

#ifndef __GRAVITON_NODE_INPUT_STREAM_H__
#define __GRAVITON_NODE_INPUT_STREAM_H__

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define GRAVITON_NODE_INPUT_STREAM_TYPE            ( \
    graviton_node_input_stream_get_type ())
#define GRAVITON_NODE_INPUT_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST (( \
                                                                                  obj), \
                                                                                GRAVITON_NODE_INPUT_STREAM_TYPE, \
                                                                                GravitonNodeInputStream))
#define GRAVITON_NODE_INPUT_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST (( \
                                                                               klass), \
                                                                             GRAVITON_NODE_INPUT_STREAM_TYPE, \
                                                                             GravitonNodeInputStreamClass))
#define IS_GRAVITON_NODE_INPUT_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (( \
                                                                                  obj), \
                                                                                GRAVITON_NODE_INPUT_STREAM_TYPE))
#define IS_GRAVITON_NODE_INPUT_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE (( \
                                                                               klass), \
                                                                             GRAVITON_NODE_INPUT_STREAM_TYPE))
#define GRAVITON_NODE_INPUT_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS (( \
                                                                                 obj), \
                                                                               GRAVITON_NODE_INPUT_STREAM_TYPE, \
                                                                               GravitonNodeInputStreamClass))

typedef struct _GravitonNodeIOStream GravitonNodeIOStream;

typedef struct _GravitonNodeInputStream GravitonNodeInputStream;
typedef struct _GravitonNodeInputStreamClass GravitonNodeInputStreamClass;
typedef struct _GravitonNodeInputStreamPrivate GravitonNodeInputStreamPrivate;

struct _GravitonNodeInputStreamClass
{
  GInputStreamClass parent_class;
};

struct _GravitonNodeInputStream
{
  GInputStream parent;
  GravitonNodeInputStreamPrivate *priv;
};

GType graviton_node_input_stream_get_type (void);

GravitonNodeInputStream *graviton_node_input_stream_new (
  GravitonNodeIOStream *stream);

G_END_DECLS

#endif