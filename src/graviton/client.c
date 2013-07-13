#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "client.h"
#include "node.h"
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-glib/glib-watch.h>

typedef struct _GravitonClientPrivate GravitonClientPrivate;

struct _GravitonClientPrivate
{
  AvahiClient *avahi;
  AvahiGLibPoll *avahi_poll_api;
  GList *discovered_nodes;
  guint unresolved_count;
  gboolean end_of_avahi_list;
};

#define GRAVITON_CLIENT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_CLIENT_TYPE, GravitonClientPrivate))

static void graviton_client_class_init (GravitonClientClass *klass);
static void graviton_client_init       (GravitonClient *self);
static void graviton_client_dispose    (GObject *object);
static void graviton_client_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonClient, graviton_client, G_TYPE_OBJECT);

enum {
  SIGNAL_0,
  SIGNAL_NODE_FOUND,
  SIGNAL_ALL_NODES_FOUND,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = {0, };

static void
cb_resolve (AvahiServiceResolver *resolver,
            AvahiIfIndex interface,
            AvahiProtocol protocol,
            AvahiResolverEvent event,
            const char *name,
            const char *type,
            const char *domain,
            const char *host_name,
            const AvahiAddress *address,
            uint16_t port,
            AvahiStringList *txt,
            AvahiLookupResultFlags flags,
            void *user_data)
{
  GravitonClient *self = GRAVITON_CLIENT (user_data);
  gchar *ip_str;

  switch (event) {
    case AVAHI_RESOLVER_FAILURE:
      self->priv->unresolved_count--;
      break;
    case AVAHI_RESOLVER_FOUND:
      ip_str = g_new0(gchar, AVAHI_ADDRESS_STR_MAX);
      avahi_address_snprint (ip_str, AVAHI_ADDRESS_STR_MAX, address);

      GInetSocketAddress *addr = NULL;
      GInetAddress *addrName = NULL;

      addrName = g_inet_address_new_from_string (ip_str);

      if (addrName)
        addr = (GInetSocketAddress*)g_inet_socket_address_new (addrName, port);

      g_debug ("Found %s: %s:%d", type, ip_str, port);
      g_free (ip_str);
      GravitonNode *node = graviton_node_new_from_address (addr);
      g_object_unref (addr);
      graviton_client_add_node (self, node);
      self->priv->unresolved_count--;
      break;
  }
  if (self->priv->end_of_avahi_list && self->priv->unresolved_count == 0)
    g_signal_emit (self, obj_signals[SIGNAL_ALL_NODES_FOUND], 0, NULL);

  avahi_service_resolver_free (resolver);
}

static void
cb_browse (AvahiServiceBrowser *browser,
           AvahiIfIndex interface,
           AvahiProtocol protocol,
           AvahiBrowserEvent event,
           const gchar *name,
           const gchar *type,
           const gchar *domain,
           AvahiLookupResultFlags flags,
           void *user_data)
{
  GravitonClient *self = GRAVITON_CLIENT (user_data);
  switch (event) {
    case AVAHI_BROWSER_NEW:
      avahi_service_resolver_new (self->priv->avahi,
                                  interface,
                                  protocol,
                                  name,
                                  type,
                                  domain,
                                  AVAHI_PROTO_INET,
                                  0,
                                  cb_resolve,
                                  self);
      self->priv->unresolved_count++;
      break;
    case AVAHI_BROWSER_ALL_FOR_NOW:
      self->priv->end_of_avahi_list = TRUE;
      if (self->priv->unresolved_count == 0)
        g_signal_emit (self, obj_signals[SIGNAL_ALL_NODES_FOUND], 0, NULL);
      break;
  }
}

static void
browse_services (AvahiClient *client, GravitonClient *self)
{
  avahi_service_browser_new (client,
                             AVAHI_IF_UNSPEC,
                             AVAHI_PROTO_UNSPEC,
                             "_graviton._tcp",
                             NULL,
                             0,
                             cb_browse,
                             self);
}

static void
cb_avahi (AvahiClient *client, AvahiClientState state, gpointer data)
{
  if (state == AVAHI_CLIENT_S_RUNNING) {
    browse_services (client, GRAVITON_CLIENT(data));
  }
}

static void
graviton_client_class_init (GravitonClientClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonClientPrivate));

  object_class->dispose = graviton_client_dispose;
  object_class->finalize = graviton_client_finalize;

  obj_signals[SIGNAL_NODE_FOUND] =
    g_signal_new ("node-found",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GRAVITON_NODE_TYPE);
  obj_signals[SIGNAL_ALL_NODES_FOUND] =
    g_signal_new ("all-nodes-found",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0,
                  G_TYPE_NONE);
}

static void
graviton_client_init (GravitonClient *self)
{
  GravitonClientPrivate *priv;
  self->priv = priv = GRAVITON_CLIENT_GET_PRIVATE (self);

  priv->avahi_poll_api = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
  const AvahiPoll *poll_api = avahi_glib_poll_get (priv->avahi_poll_api);
  priv->avahi = avahi_client_new (poll_api,
                                  0,
                                  cb_avahi,
                                  self,
                                  NULL);
}

static void
graviton_client_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_client_parent_class)->dispose (object);
}

static void
graviton_client_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_client_parent_class)->finalize (object);
}

GravitonClient *
graviton_client_new ()
{
  return g_object_new (GRAVITON_CLIENT_TYPE, NULL);
}

GList *
graviton_client_get_found_nodes (GravitonClient *self)
{
  return self->priv->discovered_nodes;
}

void
graviton_client_add_node (GravitonClient *self, GravitonNode *node)
{
  self->priv->discovered_nodes = g_list_append (self->priv->discovered_nodes, node);
  g_signal_emit (self, obj_signals[SIGNAL_NODE_FOUND], 0, node);
}
