#ifndef __GRAVITON_NODE_H__
#define __GRAVITON_NODE_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "node-transport.h"
#include "service-interface.h"

G_BEGIN_DECLS

#define GRAVITON_NODE_TYPE            (graviton_node_get_type ())
#define GRAVITON_NODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_NODE_TYPE, GravitonNode))
#define GRAVITON_NODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_NODE_TYPE, GravitonNodeClass))
#define IS_GRAVITON_NODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_NODE_TYPE))
#define IS_GRAVITON_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_NODE_TYPE))
#define GRAVITON_NODE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_NODE_TYPE, GravitonNodeClass))

GQuark graviton_node_error_quark (void);
#define GRAVITON_NODE_ERROR (graviton_node_error_quark ())

typedef enum {
  GRAVITON_NODE_ERROR_TRANSPORT = 1
  //TODO: Fill in with values from libsoup
} GravitonNodeError;

typedef struct _GravitonNode      GravitonNode;
typedef struct _GravitonNodeClass GravitonNodeClass;
typedef struct _GravitonNodePrivate GravitonNodePrivate;

typedef struct _GravitonCloud GravitonCloud;

struct _GravitonNodeClass
{
  GravitonServiceInterfaceClass parent_class;
};

struct _GravitonNode
{
  GravitonServiceInterface parent;
  GravitonNodePrivate *priv;
};

GType graviton_node_get_type (void);

const gchar *graviton_node_get_id (GravitonNode *node, GError **err);
const gchar *graviton_node_get_cloud_id (GravitonNode *node, GError **err);

GravitonNode *graviton_node_proxy_to_id (GravitonNode *node, const gchar *id, GError **error);

GList *graviton_node_get_services (GravitonNode *node, GError **err);
gboolean graviton_node_has_service (GravitonNode *node, const gchar *name, GError **err);
GravitonServiceInterface *graviton_node_get_service_interface (GravitonNode *node, const gchar *name, GError **err);
GVariant *graviton_node_call (GravitonNode *node, const gchar *method, GError **error, ...);
GVariant *graviton_node_call_args (GravitonNode *node, const gchar *method, GHashTable *args, GError **error);
GVariant *graviton_node_call_va (GravitonNode *node, const gchar *method, GError **error, va_list args);

GIOStream *graviton_node_open_stream (GravitonNode *node, const gchar *name, GHashTable *args); //FIXME: Needs a GError

void graviton_node_add_transport (GravitonNode *node, GravitonNodeTransport *transport, int priority);
GPtrArray *graviton_node_get_transports (GravitonNode *node, int priority);
GravitonNodeTransport *graviton_node_get_default_transport (GravitonNode *node);

G_END_DECLS

#endif
