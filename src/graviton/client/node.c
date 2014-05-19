#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "node.h"
#include "cloud.h"
#include "node-io-stream.h"

typedef struct _GravitonNodePrivate GravitonNodePrivate;

struct _GravitonNodePrivate
{
  GravitonServiceInterface *gobj;
  GHashTable *transports;
};

#define GRAVITON_NODE_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_NODE_TYPE, GravitonNodePrivate))

GQuark
graviton_node_error_quark ()
{
  return g_quark_from_static_string ("graviton-node-error-quark");
}

static void graviton_node_class_init (GravitonNodeClass *klass);
static void graviton_node_init       (GravitonNode *self);
static void graviton_node_dispose    (GObject *object);
static void graviton_node_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonNode, graviton_node, GRAVITON_SERVICE_INTERFACE_TYPE);

enum {
  PROP_0,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
unref_arg (GVariant *var)
{
  if (var)
    g_variant_unref (var);
}

static void
set_property (GObject *object,
                     guint property_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  GravitonNode *self = GRAVITON_NODE (object);
  switch (property_id) {
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
  GravitonNode *self = GRAVITON_NODE (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_node_class_init (GravitonNodeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonNodePrivate));

  object_class->dispose = graviton_node_dispose;
  object_class->finalize = graviton_node_finalize;

  object_class->set_property = set_property;
  object_class->get_property = get_property;

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
graviton_node_init (GravitonNode *self)
{
  GravitonNodePrivate *priv;
  self->priv = priv = GRAVITON_NODE_GET_PRIVATE (self);
  self->priv->gobj = graviton_service_interface_get_subservice (GRAVITON_SERVICE_INTERFACE (self), "net:phrobo:graviton");
}

static void
graviton_node_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_parent_class)->dispose (object);
}

static void
graviton_node_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_node_parent_class)->finalize (object);
}

GravitonNode *
graviton_node_proxy_to_id (GravitonNode *node,
                           const gchar *id,
                           GError **error)
{
  return NULL;
}

static void
cb_resolve_id (GravitonCloud *client, GravitonNode **result)
{
  GList *nodes = graviton_cloud_get_found_nodes (client);
  GList *cur = nodes;
  while (cur) {
    cur = cur->next;
  }
}

GravitonNode *graviton_node_new_from_id (const gchar *node_id, GError **error)
{
  GravitonNode *result = NULL;
  GravitonCloud *client = graviton_cloud_new_default_cloud ();
  g_signal_connect (client,
                    "all-nodes-found",
                    G_CALLBACK (cb_resolve_id),
                    &result);
  g_object_unref (client);
  return result;
}

const gchar *
graviton_node_get_id (GravitonNode *self, GError **err)
{
  GVariant *ret = graviton_service_interface_get_property (self->priv->gobj, "node-id", err);
  if (ret) {
    gchar *r = g_variant_dup_string (ret, NULL);
    g_variant_unref (ret);
    return r;
  }
  return NULL;
}

const gchar *
graviton_node_get_cloud_id (GravitonNode *self, GError **err)
{
  GVariant *ret = graviton_service_interface_get_property (self->priv->gobj, "cloud-id", err);
  if (ret) {
    gchar *r = g_variant_dup_string (ret, NULL);
    g_variant_unref (ret);
    return r;
  }
  return NULL;
}

GVariant *
graviton_node_call (GravitonNode *self,
                    const gchar *method,
                    GError **err,
                    ...)
{
  va_list argList;
  va_start (argList, err);
  GVariant *ret = graviton_node_call_va (self, method, err, argList);
  va_end (argList);
  return ret;
}

GVariant *
graviton_node_call_va (GravitonNode *self,
                       const gchar *method,
                       GError **err,
                       va_list argList)
{
  gchar *propName = NULL;
  GVariant *propValue = NULL;
  GHashTable *args = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            NULL,
                                            (GDestroyNotify)unref_arg);
  propName = va_arg (argList, gchar*);

  while (propName != NULL) {
    propValue = va_arg (argList, GVariant*);
    g_hash_table_replace (args, propName, propValue);
    if (propValue)
      g_debug ("%s = %s", propName, g_variant_print (propValue, TRUE));
    else
      g_debug ("%s = NULL", propName);
    propName = va_arg (argList, gchar*);
  }
  va_end (argList);

  GVariant *ret = graviton_node_call_args (self,
                                           method,
                                           args,
                                           err);
  g_hash_table_unref (args);
  return ret;
}

GravitonServiceInterface *
graviton_node_get_service_interface (GravitonNode *node, const gchar *name, GError **error)
{
  return NULL;
}

gboolean
graviton_node_has_service (GravitonNode *node, const gchar *name, GError **err)
{
  return TRUE;
}

GVariant *
graviton_node_call_args (GravitonNode *self,
                         const gchar *method,
                         GHashTable *args,
                         GError **err)
{
  GravitonNodeTransport *transport = graviton_node_get_default_transport (self);
  return graviton_node_transport_call_args (transport, self, method, args, err);
}

GIOStream *
graviton_node_open_stream (GravitonNode *self,
                           const gchar *name,
                           GHashTable *args)
{
  GravitonNodeTransport *transport = graviton_node_get_default_transport (self);
  return graviton_node_transport_open_stream (transport, self, name, args, NULL);
}

//FIXME: Check refcounts for node_*_transport functions
void
graviton_node_add_transport (GravitonNode *self,
                             GravitonNodeTransport *transport,
                            int priority)
{
  //FIXME: Switch to GPtrArray
  GArray *transports = graviton_node_get_transports (self, priority);
  if (transports == NULL) {
    transports = g_array_new (FALSE, FALSE, sizeof (GravitonNodeTransport*));
    g_hash_table_insert (self->priv->transports, GINT_TO_POINTER (priority), transports);
  }
  g_array_append_val (transports, transport);
}

GArray *
graviton_node_get_transports (GravitonNode *node, int priority)
{
  return g_hash_table_lookup (node->priv->transports, GINT_TO_POINTER (priority));
}

GravitonNodeTransport *graviton_node_get_default_transport (GravitonNode *node)
{
  int priority;
  GArray *transports;
  GList *priorities = g_hash_table_get_keys (node->priv->transports);
  priority = GPOINTER_TO_INT (priorities->data);
  g_list_free (priorities);
  transports = graviton_node_get_transports (node, priority);
  return GRAVITON_NODE_TRANSPORT (transports->data);
}
