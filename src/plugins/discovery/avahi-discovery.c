#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "avahi-discovery.h"

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-glib/glib-watch.h>

#include <graviton/client/cloud.h>
#include <graviton/client/jsonrpc-node-transport.h>

typedef struct _GravitonAvahiDiscoveryMethodPrivate GravitonAvahiDiscoveryMethodPrivate;

struct _GravitonAvahiDiscoveryMethodPrivate
{
  AvahiClient *avahi;
  AvahiGLibPoll *avahi_poll_api;
  guint unresolved_count;
  gboolean end_of_avahi_list;
  gboolean started;
};

#define GRAVITON_AVAHI_DISCOVERY_METHOD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_AVAHI_DISCOVERY_METHOD_TYPE, GravitonAvahiDiscoveryMethodPrivate))

static void graviton_avahi_discovery_method_class_init (GravitonAvahiDiscoveryMethodClass *klass);
static void graviton_avahi_discovery_method_init       (GravitonAvahiDiscoveryMethod *self);
static void graviton_avahi_discovery_method_dispose    (GObject *object);
static void graviton_avahi_discovery_method_finalize   (GObject *object);
static void graviton_avahi_discovery_method_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_avahi_discovery_method_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

static void start_browse (GravitonDiscoveryMethod *self);
static void stop_browse (GravitonDiscoveryMethod *self);

GRAVITON_DEFINE_DISCOVERY_PLUGIN (GRAVITON_AVAHI_DISCOVERY_METHOD_TYPE)

G_DEFINE_TYPE (GravitonAvahiDiscoveryMethod, graviton_avahi_discovery_method, GRAVITON_DISCOVERY_METHOD_TYPE);

enum {
  PROP_ZERO,
  N_PROPERTIES
};

enum {
  SIGNAL_0,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = { 0, };

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
graviton_avahi_discovery_method_class_init (GravitonAvahiDiscoveryMethodClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonAvahiDiscoveryMethodPrivate));

  object_class->dispose = graviton_avahi_discovery_method_dispose;
  object_class->finalize = graviton_avahi_discovery_method_finalize;
  object_class->set_property =  graviton_avahi_discovery_method_set_property;
  object_class->get_property =  graviton_avahi_discovery_method_get_property;
  /*g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);*/

  GravitonDiscoveryMethodClass *method_class = GRAVITON_DISCOVERY_METHOD_CLASS (klass);
  method_class->start = start_browse;
  method_class->stop = stop_browse;
}

static void
graviton_avahi_discovery_method_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonAvahiDiscoveryMethod *self = GRAVITON_AVAHI_DISCOVERY_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_avahi_discovery_method_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonAvahiDiscoveryMethod *self = GRAVITON_AVAHI_DISCOVERY_METHOD (object);
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

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
  GravitonAvahiDiscoveryMethod *self = GRAVITON_AVAHI_DISCOVERY_METHOD (user_data);
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
      GravitonJsonrpcNodeTransport *transport = graviton_jsonrpc_node_transport_new (addr);
      g_object_unref (addr);
      const gchar *node_id = graviton_jsonrpc_node_transport_get_node_id (transport);
      GravitonNode *node = graviton_node_get_by_id (node_id);
      graviton_node_add_transport (node, GRAVITON_NODE_TRANSPORT (transport), 0);

      graviton_discovery_method_node_found (GRAVITON_DISCOVERY_METHOD (self), node);
      self->priv->unresolved_count--;
      break;
  }
  if (self->priv->end_of_avahi_list && self->priv->unresolved_count == 0)
    graviton_discovery_method_finished (GRAVITON_DISCOVERY_METHOD (self));

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
  GravitonAvahiDiscoveryMethod *self = GRAVITON_AVAHI_DISCOVERY_METHOD (user_data);
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
        graviton_discovery_method_finished (GRAVITON_DISCOVERY_METHOD (self));
      break;
  }
}

static void
browse_services (AvahiClient *client, GravitonAvahiDiscoveryMethod *self)
{
  AvahiClientState state = avahi_client_get_state (client);
  if (state == AVAHI_CLIENT_S_RUNNING && self->priv->started) {
    avahi_service_browser_new (client,
                               AVAHI_IF_UNSPEC,
                               AVAHI_PROTO_UNSPEC,
                               "_graviton._tcp",
                               NULL,
                               0,
                               cb_browse,
                               self);
  }
}

static void
start_browse (GravitonDiscoveryMethod *method)
{
  GravitonAvahiDiscoveryMethod *self = GRAVITON_AVAHI_DISCOVERY_METHOD (method);
  self->priv->started = TRUE;
  g_debug ("Starting browsing");
  browse_services (self->priv->avahi, self);
}

static void
stop_browse (GravitonDiscoveryMethod *method)
{
  GravitonAvahiDiscoveryMethod *self = GRAVITON_AVAHI_DISCOVERY_METHOD (method);
  self->priv->started = FALSE;
  //FIXME: Remove discovered nodes
}

static void
cb_avahi (AvahiClient *client, AvahiClientState state, gpointer data)
{
  browse_services (client, GRAVITON_AVAHI_DISCOVERY_METHOD (data));
}

static void
graviton_avahi_discovery_method_init (GravitonAvahiDiscoveryMethod *self)
{
  GravitonAvahiDiscoveryMethodPrivate *priv;
  priv = self->priv = GRAVITON_AVAHI_DISCOVERY_METHOD_GET_PRIVATE (self);
  priv->started = FALSE;

  priv->avahi_poll_api = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
  const AvahiPoll *poll_api = avahi_glib_poll_get (priv->avahi_poll_api);
  priv->avahi = avahi_client_new (poll_api,
                                  0,
                                  cb_avahi,
                                  self,
                                  NULL);
}

static void
graviton_avahi_discovery_method_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_avahi_discovery_method_parent_class)->dispose (object);
}

static void
graviton_avahi_discovery_method_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_avahi_discovery_method_parent_class)->finalize (object);
}
