#ifndef __GRAVITON_NODE_TRANSPORT_H__
#define __GRAVITON_NODE_TRANSPORT_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GRAVITON_NODE_TRANSPORT_TYPE            (graviton_node_transport_get_type ())
#define GRAVITON_NODE_TRANSPORT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NODE_TRANSPORT_TYPE, GravitonNodeTransport))
#define GRAVITON_NODE_TRANSPORT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NODE_TRANSPORT_TYPE, GravitonNodeTransportClass))
#define IS_GRAVITON_NODE_TRANSPORT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NODE_TRANSPORT_TYPE))
#define IS_GRAVITON_NODE_TRANSPORT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NODE_TRANSPORT_TYPE))
#define GRAVITON_NODE_TRANSPORT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NODE_TRANSPORT_TYPE, GravitonNodeTransportClass))

typedef struct _GravitonNodeTransport      GravitonNodeTransport;
typedef struct _GravitonNodeTransportClass GravitonNodeTransportClass;
typedef struct _GravitonNodeTransportPrivate GravitonNodeTransportPrivate;

typedef struct _GravitonNode GravitonNode;

struct _GravitonNodeTransportClass
{
  GObjectClass parent_class;
  GVariant *(*call_args)(GravitonNodeTransport *self, GravitonNode *node, const gchar* method, GHashTable *args, GError **err);
  GIOStream *(*open_stream)(GravitonNodeTransport *self, GravitonNode *node, const gchar *name, GHashTable *args, GError **err);
};

struct _GravitonNodeTransport
{
  GObject parent;
  GravitonNodeTransportPrivate *priv;
};

GType graviton_node_transport_get_type (void);

GravitonNodeTransport *graviton_node_transport_new ();

GVariant *graviton_node_transport_call_args (GravitonNodeTransport *self, GravitonNode *node, const gchar *method, GHashTable *args, GError **error);
GIOStream *graviton_node_transport_open_stream (GravitonNodeTransport *self, GravitonNode *node, const gchar *name, GHashTable *args, GError **error);

G_END_DECLS

#endif
