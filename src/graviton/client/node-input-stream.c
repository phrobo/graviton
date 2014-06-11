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
#include <string.h>

typedef struct _GravitonNodeInputStreamPrivate GravitonNodeInputStreamPrivate;

struct _GravitonNodeInputStreamPrivate
{
  GravitonNodeIOStream *stream;
  SoupMessage *msg;
  GQueue *buffers;
  GCond input_cond;
  GMutex input_lock;
};

typedef struct _Buffer
{
  char *data;
  gsize size;
} Buffer;

static gssize
read_buffer (GQueue *queue, void *buffer, gsize count)
{
  g_debug ("Requesting to read %" G_GSIZE_FORMAT " bytes, we have %u buffers", count, g_queue_get_length (queue));
  gssize read_size = 0;
  while (read_size < count && !g_queue_is_empty (queue)) {
    Buffer *curBuf = (Buffer*) g_queue_pop_head (queue);
    g_debug ("Looking to read %" G_GSSIZE_FORMAT " more out of %" G_GSIZE_FORMAT ", current buffer is %" G_GSIZE_FORMAT, count-read_size, count, curBuf->size);
    if (curBuf->size == 0 && curBuf->data == NULL) {
      g_debug ("Got empty buffer, must mean we are done.");
      return 0;
    }
    if (count-read_size > curBuf->size) {
      memcpy (buffer + read_size, curBuf->data, curBuf->size);
      g_debug ("Consuming first buffer of size %" G_GSIZE_FORMAT, curBuf->size);
      g_free (curBuf->data);
      g_free (curBuf);
      read_size += curBuf->size;
    } else {
      memcpy (buffer + read_size, curBuf->data, count-read_size);
      char *newSegment = g_new (char, curBuf->size-count-read_size);
      memcpy (newSegment, curBuf->data + (count-read_size), curBuf->size-count-read_size);
      g_debug ("Consuming first %" G_GSSIZE_FORMAT " bytes of first buffer", count-read_size);
      g_free (curBuf->data);
      curBuf->data = newSegment;
      curBuf->size = curBuf->size-count-read_size;
      read_size += count-read_size;
      g_queue_push_head (queue, curBuf);
    }
  }
  return read_size;
}

static void
add_buffer (GQueue *queue, const void *buffer, gsize size)
{
  Buffer *buf = g_new0 (Buffer, 1);
  buf->data = g_memdup (buffer, size);
  buf->size = size;
  g_debug ("Added a buffer of size %" G_GSIZE_FORMAT, size);
  g_queue_push_tail (queue, buf);
}

#define GRAVITON_NODE_INPUT_STREAM_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_INPUT_STREAM_TYPE, GravitonNodeInputStreamPrivate))

static void graviton_node_input_stream_class_init (GravitonNodeInputStreamClass *klass);
static void graviton_node_input_stream_init       (GravitonNodeInputStream *self);
static void graviton_node_input_stream_dispose    (GObject *object);
static void graviton_node_input_stream_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonNodeInputStream, graviton_node_input_stream, G_TYPE_INPUT_STREAM);

enum {
  PROP_0,
  PROP_IO_STREAM,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
set_property (GObject *object,
                     guint property_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  GravitonNodeInputStream *self = GRAVITON_NODE_INPUT_STREAM (object);
  switch (property_id) {
    case PROP_IO_STREAM:
      self->priv->stream = g_value_dup_object (value);
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
  GravitonNodeInputStream *self = GRAVITON_NODE_INPUT_STREAM (object);
  switch (property_id) {
    case PROP_IO_STREAM:
      g_value_set_object (value, self->priv->stream);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
cb_finished (SoupMessage *msg, gpointer user_data)
{
  GravitonNodeInputStream *self = GRAVITON_NODE_INPUT_STREAM (user_data);
  g_debug ("Finished with transmission.");
  g_mutex_lock (&self->priv->input_lock);
  add_buffer (self->priv->buffers, NULL, 0);
  g_cond_signal (&self->priv->input_cond);
  g_mutex_unlock (&self->priv->input_lock);
}

static void
cb_chunk_ready (SoupMessage *msg, SoupBuffer *chunk, gpointer user_data)
{
  GravitonNodeInputStream *self = GRAVITON_NODE_INPUT_STREAM (user_data);
  //g_debug ("Read in a chunk of %d bytes", chunk->length);

  g_mutex_lock (&self->priv->input_lock);
  add_buffer (self->priv->buffers, chunk->data, chunk->length);
  g_cond_signal (&self->priv->input_cond);
  g_mutex_unlock (&self->priv->input_lock);
}

static gssize
stream_read (GInputStream *stream,
             void *buffer,
             gsize count,
             GCancellable *cancellable,
             GError **err)
{
  GravitonNodeInputStream *self = GRAVITON_NODE_INPUT_STREAM (stream);
  gssize read_size;
  g_assert (self->priv->stream);
  g_debug ("Reading stream");
  if (!self->priv->msg) {
    SoupMessageBody *body;
    SoupURI *uri = graviton_node_io_stream_get_uri (self->priv->stream);
    g_debug ("Opening node input stream to %s", soup_uri_to_string (uri, FALSE));
    self->priv->msg = soup_message_new_from_uri ( "GET", uri);
    g_object_get (self->priv->msg, SOUP_MESSAGE_RESPONSE_BODY, &body, NULL);
    soup_message_body_set_accumulate (body, FALSE);
    g_signal_connect (self->priv->msg, "got-chunk", G_CALLBACK (cb_chunk_ready), self);
    g_signal_connect (self->priv->msg, "finished", G_CALLBACK (cb_finished), self);
    soup_session_queue_message (graviton_node_io_stream_get_session (self->priv->stream), self->priv->msg, NULL, NULL);
  }

  if (g_queue_get_length (self->priv->buffers) == 0) {
    g_debug ("Waiting for new buffers to arrive...");
    g_cond_wait (&self->priv->input_cond, &self->priv->input_lock);
  }

  read_size = read_buffer (self->priv->buffers, buffer, count);
  g_debug ("Got a buffer of size %" G_GSSIZE_FORMAT " after asking for %" G_GSIZE_FORMAT, read_size, count);

  return read_size;
}

static gssize
stream_skip (GInputStream *stream,
             gsize count,
             GCancellable *cancellable,
             GError **err)
{
  GravitonNodeInputStream *self = GRAVITON_NODE_INPUT_STREAM (stream);
  g_assert (self->priv->stream);

  //FIXME: Resubmit the underlying HTTP request with proper ranges
  return 0;
}

static gboolean
stream_close (GInputStream *stream,
              GCancellable *cancellable,
              GError **err)
{
  GravitonNodeInputStream *self = GRAVITON_NODE_INPUT_STREAM (stream);
  g_assert (self->priv->stream);

  if (self->priv->msg)
    soup_session_cancel_message (graviton_node_io_stream_get_session (self->priv->stream), self->priv->msg, SOUP_STATUS_CANCELLED);

  g_debug ("Closed!");

  return TRUE;
}

static void
graviton_node_input_stream_class_init (GravitonNodeInputStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNodeInputStreamPrivate));

  object_class->dispose = graviton_node_input_stream_dispose;
  object_class->finalize = graviton_node_input_stream_finalize;

  GInputStreamClass *input_class = G_INPUT_STREAM_CLASS (klass);
  input_class->read_fn = stream_read;
  input_class->skip = stream_skip;
  input_class->close_fn = stream_close;

  object_class->set_property = set_property;
  object_class->get_property = get_property;

  obj_properties [PROP_IO_STREAM] = 
    g_param_spec_object ("io-stream",
                         "GravitonNodeIOStream",
                         "The underlying GravitonNodeIOStream",
                         GRAVITON_NODE_IO_STREAM_TYPE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );
  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
graviton_node_input_stream_init (GravitonNodeInputStream *self)
{
  GravitonNodeInputStreamPrivate *priv;
  priv = self->priv = GRAVITON_NODE_INPUT_STREAM_GET_PRIVATE (self);
  priv->buffers = g_queue_new ();
  g_mutex_init (&priv->input_lock);
  g_cond_init (&priv->input_cond);
}

static void
graviton_node_input_stream_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_input_stream_parent_class)->dispose (object);
}

static void
graviton_node_input_stream_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_input_stream_parent_class)->finalize (object);
}

GravitonNodeInputStream *
graviton_node_input_stream_new (GravitonNodeIOStream *stream)
{
  return g_object_new (GRAVITON_NODE_INPUT_STREAM_TYPE, "io-stream", stream, NULL);
}
