#include "service.h"
#include <string.h>

#define GRAVITON_SERVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_SERVICE_TYPE, GravitonServicePrivate))

GQuark
graviton_service_error_quark ()
{
  return g_quark_from_static_string ("graviton-service-error-quark");
}

/**
 * GravitonService:
 *
 * Controls provide services to the graviton network by way of exposing a set of
 * properties, methods, IO channels, and child services. Controls have names
 * which are browseable via the net:phrobo:graviton introspection service.
 *
 * After a #GravitonServer is created, you can attach services to it by fetching
 * its #GravitonRootService via graviton_server_get_root_service() and calling
 * graviton_service_add_subservice().
 *
 * Properties are exposed on a #GravitonService through the normal #GObject API
 * that is used by g_object_set()/g_object_get()
 *
 * FIXME: Example of properties
 *
 * Methods are exposed by calling graviton_service_add_method() and supplying a
 * callback.
 *
 * FIXME: Example of adding a method
 *
 * IO channels use graviton_service_add_stream() with a supplied
 * #GravitonServiceStreamGenerator callback for later activation.
 *
 */
G_DEFINE_TYPE (GravitonService, graviton_service, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_NAME,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

enum {
  SIGNAL_0,
  SIGNAL_PROPERTY_UPDATE,
  N_SIGNALS
};

static int obj_signals[N_SIGNALS] = {0, };

struct _GravitonServicePrivate
{
  gchar *name;
  GHashTable *services;
  GHashTable *methods;
  GHashTable *method_data;
  GHashTable *method_destroys;
  GHashTable *streams;
  GHashTable *stream_data;
};

static void
set_property (GObject *object,
                     guint property_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  GravitonService *self = GRAVITON_SERVICE (object);
  switch (property_id) {
    case PROP_NAME:
      self->priv->name = g_value_dup_string (value);
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
  GravitonService *self = GRAVITON_SERVICE (object);
  switch (property_id) {
    case PROP_NAME:
      g_value_set_string (value, self->priv->name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_service_class_init (GravitonServiceClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  g_type_class_add_private (klass, sizeof (GravitonServicePrivate));

  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;

  obj_properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Control name",
                         "Control name",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );

  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);

  obj_signals[SIGNAL_PROPERTY_UPDATE] = 
    g_signal_new ("property-update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);
}

static void
graviton_service_init (GravitonService *self)
{
  GravitonServicePrivate *priv;
  self->priv = priv = GRAVITON_SERVICE_GET_PRIVATE (self);
  priv->name = 0;
  priv->streams = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);
  priv->stream_data = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);
  priv->services = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          g_object_unref);
  priv->methods = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);
  priv->method_data = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             NULL);
  priv->method_destroys = g_hash_table_new_full (g_str_hash,
                                                 g_str_equal,
                                                 g_free,
                                                 NULL);
}

static void
graviton_service_dispose (GravitonService *self)
{
  g_hash_table_unref (self->priv->methods);
  g_hash_table_unref (self->priv->method_data);
}

static void
graviton_service_finalize (GravitonService *self)
{
  g_free (self->priv->name);
}

/**
 * graviton_service_add_method:
 * @self: The #GravitonService
 * @name: String name of the method
 * @func: Callback with a #GravitonServiceMethod signature
 * @user_data: (closure): Data for @func
 * @destroy_func: (destroy): Destroy notifier to free @user_data
 *
 * Adds a method to the #GravitonService @self.
 */
void
graviton_service_add_method (GravitonService *self,
                             const gchar *name,
                             GravitonServiceMethod func,
                             gpointer user_data,
                             GDestroyNotify destroy_func)
{
  int i;
  g_hash_table_replace (self->priv->methods, g_strdup (name), func);
  g_hash_table_replace (self->priv->method_data, g_strdup (name), user_data);
  g_hash_table_replace (self->priv->method_destroys, g_strdup (name), destroy_func);
}

/**
 * graviton_service_call_method:
 * @self: The #GravitonService
 * @name: String name of the method
 * @args: (element-type gchar* GValue): A mapping of argument names to values
 * @error: return location for a #GError or NULL
 */
GVariant *
graviton_service_call_method (GravitonService *self,
                              const gchar *name,
                              GHashTable *args,
                              GError **error)
{
  GravitonServiceMethod func;
  gpointer data;
  func = g_hash_table_lookup (self->priv->methods, name);
  data = g_hash_table_lookup (self->priv->method_data, name);

  g_debug ("Calling %s", name);

  if (func) {
    GVariant *ret = func (self, args, error, data);
    return ret;
  } else {
    g_set_error (error,
                 GRAVITON_SERVICE_ERROR,
                 GRAVITON_SERVICE_ERROR_NO_SUCH_METHOD,
                 "No such method: %s",
                 name);
    return NULL;
  }
}

/**
 * graviton_service_list_methods:
 *
 * Get a list of methods on this service
 *
 * Returns: (element-type *gchar) (transfer none): List of method names
 */
GList *
graviton_service_list_methods (GravitonService *self)
{
  return g_hash_table_get_keys (self->priv->methods);
}

/**
 * graviton_service_has_method:
 * @self: The #GravitonService
 * @name: String name of the method
 *
 * Returns: TRUE if the method exists on this service, FALSE otherwise.
 */
gboolean
graviton_service_has_method (GravitonService *self, const gchar *name)
{
  return g_hash_table_contains (self->priv->methods, name);
}

/**
 * graviton_service_get_subservice:
 * @self: The #GravitonService
 * @path: String path to the subservice
 *
 * Gets the named service.
 *
 * Returns: The service
 */
GravitonService *
graviton_service_get_subservice (GravitonService *self,
                                 const gchar *path)
{
  GravitonService *service = NULL;
  gchar **tokens;
  g_return_val_if_fail (path != NULL, NULL);
  g_return_val_if_fail (strlen(path) > 0, NULL);
  tokens = g_strsplit (path, "/", 0);
  service = g_hash_table_lookup (self->priv->services, tokens[0]);
  if (service) {
    if (g_strv_length (tokens) > 1) {
      gchar *subname = g_strjoinv ("/", &tokens[1]);
      service = graviton_service_get_subservice (service, subname);
      g_free (subname);
    } else {
      g_object_ref (service);
    }
  }
  g_strfreev (tokens);
  return service;
}

static void
cb_subservice_notify (GravitonService *subservice, GParamSpec *pspec, gpointer user_data)
{
  GravitonService *self = GRAVITON_SERVICE(user_data);
  gchar *subname;
  g_object_get (subservice, "name", &subname, NULL);
  gchar *full_name = g_strdup_printf ("%s/%s", subname, pspec->name);

  g_signal_emit (self, obj_signals[SIGNAL_PROPERTY_UPDATE], 0, full_name);
  g_debug ("Subproperty notify: %s", full_name);
  g_free (full_name);
}

static void
cb_propigate_property_update (GravitonService *subservice, const gchar *name, gpointer user_data)
{
  GravitonService *self = GRAVITON_SERVICE(user_data);
  gchar *full_name = g_strdup_printf ("%s/%s", self->priv->name, name);

  g_signal_emit (self, obj_signals[SIGNAL_PROPERTY_UPDATE], 0, full_name);

  g_debug ("Subproperty update: %s", full_name);
  g_free (full_name);
}

/**
 * graviton_service_add_subservice:
 * @self: The #GravitonService
 * @service: The #GravitonService to add as a sub-service
 *
 * Adds a service to this one as a sub-service
 *
 */
void
graviton_service_add_subservice (GravitonService *self,
                                      GravitonService *service)
{
  g_object_ref (service);
  gchar *name;
  g_object_get (service, "name", &name, NULL);
  g_hash_table_replace (self->priv->services, name, service);

  g_signal_connect (service,
                    "notify",
                    G_CALLBACK(cb_subservice_notify),
                    self);
  g_signal_connect (service,
                    "property-update",
                    G_CALLBACK(cb_propigate_property_update),
                    self);
}

/**
 * graviton_service_list_subservices:
 *
 * Get a list of the names of available subservices.
 *
 * Returns: (element-type gchar*) (transfer full): the names of the available subservices
 */
GList *
graviton_service_list_subservices (GravitonService *self)
{
  return g_hash_table_get_keys (self->priv->services);
}

/**
 * graviton_service_add_stream:
 * @self: The #GravitonService
 * @name: String name of the stream to add
 * @func: A #GravitonServiceStreamGenerator callback to use
 * @user_data: Data that is passed to @func
 *
 * Registers a new stream on this service using the given name
 */
void
graviton_service_add_stream (GravitonService *self,
                             const gchar *name,
                             GravitonServiceStreamGenerator func,
                             gpointer user_data)
{
  g_hash_table_replace (self->priv->streams, g_strdup (name), func);
  g_hash_table_replace (self->priv->stream_data, g_strdup (name), user_data);
}

/**
 * graviton_service_list_streams:
 *
 * Get a list of the streams that are available from this service
 *
 * Returns: (element-type gchar*) (transfer full): the names of the available
 * streams
 */
GList *
graviton_service_list_streams (GravitonService *self)
{
  return g_hash_table_get_keys (self->priv->streams);
}

/**
 * graviton_service_get_stream:
 * @self: The #GravitonStream
 * @name: String name of the stream requested
 * @args: (element-type gchar* GValue): Mapping of arguments to values
 * @error: Return location for a #GError, or NULL
 */
GravitonStream *
graviton_service_get_stream (GravitonService *self, const gchar *name, GHashTable *args, GError **error)
{
  GravitonServiceStreamGenerator func = g_hash_table_lookup (self->priv->streams, name);
  gpointer func_data = g_hash_table_lookup (self->priv->stream_data, name);
  if (func)
    return func(self, name, args, error, func_data);
  return NULL;
}

/**
 * graviton_service_new:
 * @serviceName: Name of the service to use
 *
 * Creates a new #GravitonService with a service name
 *
 * Returns: A new #GravitonService that exposes the given @serviceName
 */
GravitonService *
graviton_service_new (const gchar *serviceName)
{
  return g_object_new (GRAVITON_SERVICE_TYPE, "name", serviceName, NULL);
}
