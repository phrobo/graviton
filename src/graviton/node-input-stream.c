#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "node-input-stream.h"
#include "node-io-stream.h"

typedef struct _GravitonNodeInputStreamPrivate GravitonNodeInputStreamPrivate;

struct _GravitonNodeInputStreamPrivate
{
  GravitonNodeIOStream *stream;
  SoupMessage *msg;
  GList *buffers;
  GCond input_cond;
  GMutex input_lock;
};

typedef struct _Buffer
{
  void *data;
  gsize size;
} Buffer;

static gssize
read_buffer (GList *buffer_list, void *buffer, gsize count)
{
  gssize read_size = 0;
  void *ret;
  GList *cur = buffer_list;
  while (read_size < count && cur) {
    cur = cur->next;
  }
}

static GList *
add_buffer (GList *buffer_list, const void *buffer, gsize size)
{
  Buffer *buf = g_new0 (Buffer, 1);
  buf->data = g_memdup (buffer, size);
  buf->size = size;
  return g_list_append (buffer_list, buf);
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
  GravitonNodeInputStream *self = GRAVITON_NODE_INPUT_STREAM (self);
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
cb_chunk_ready (SoupMessage *msg, SoupBuffer *chunk, gpointer user_data)
{
  GravitonNodeInputStream *self = GRAVITON_NODE_INPUT_STREAM (user_data);
  //g_debug ("Read in a chunk of %d bytes", chunk->length);

  g_mutex_lock (&self->priv->input_lock);
  self->priv->buffers = add_buffer (self->priv->buffers, chunk->data, chunk->length);
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
  if (!self->priv->msg) {
    SoupMessageBody *body;
    self->priv->msg = soup_message_new_from_uri ( "GET", graviton_node_io_stream_get_uri (self->priv->stream));
    g_object_get (self->priv->msg, SOUP_MESSAGE_RESPONSE_BODY, &body, NULL);
    soup_message_body_set_accumulate (body, FALSE);
    g_signal_connect (self->priv->msg, "got-chunk", G_CALLBACK (cb_chunk_ready), self);
    soup_session_queue_message (graviton_node_io_stream_get_session (self->priv->stream), self->priv->msg, NULL, NULL);
  }

  g_cond_wait (&self->priv->input_cond, &self->priv->input_lock);

  read_size = read_buffer (self->priv->buffers, buffer, count);

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
