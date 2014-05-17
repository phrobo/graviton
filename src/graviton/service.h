#ifndef GRAVITON_SERVICE_H
#define GRAVITON_SERVICE_H

#include <glib-object.h>
#include <gio/gio.h>

#define GRAVITON_SERVICE_TYPE            (graviton_service_get_type ())
#define GRAVITON_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_SERVICE_TYPE, GravitonService))
#define GRAVITON_IS_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_SERVICE_TYPE))
#define GRAVITON_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_SERVICE_TYPE, GravitonServiceClass))
#define GRAVITON_IS_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_SERVICE_TYPE))
#define GRAVITON_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_SERVICE_TYPE, GravitonServiceClass))

#define GRAVITON_SERVICE_ERROR (graviton_service_error_quark ())

typedef enum {
  GRAVITON_SERVICE_ERROR_NO_SUCH_METHOD
} GravitonServiceError;

typedef struct _GravitonStream GravitonStream;

typedef struct _GravitonService GravitonService;
typedef struct _GravitonServiceClass GravitonServiceClass;

typedef struct _GravitonServicePrivate GravitonServicePrivate;

struct _GravitonService 
{
  GObject parent_instance;
  GravitonServicePrivate *priv;
};

struct _GravitonServiceClass
{
  GObjectClass parent_class;
};

GType graviton_service_get_type ();

typedef GVariant *(*GravitonServiceMethod)(GravitonService *self, GHashTable *args, GError **error, gpointer user_data);

void graviton_service_add_method (GravitonService *self,
                                  const gchar *name,
                                  GravitonServiceMethod func,
                                  gpointer user_data,
                                  GDestroyNotify destroy_func);

GVariant *graviton_service_call_method (GravitonService *self,
                                   const gchar *name,
                                   GHashTable *args,
                                   GError **error);

GList *graviton_service_list_methods (GravitonService *self);

gboolean graviton_service_has_method (GravitonService *self, const gchar *name);

void graviton_service_add_subservice (GravitonService *self,
                                       GravitonService *control);

GravitonService *graviton_service_get_subservice (GravitonService *self,
                                  const gchar *name);

GList *graviton_service_list_subservices (GravitonService *self);

typedef GravitonStream *(*GravitonServiceStreamGenerator)(GravitonService *self, const gchar *name, GHashTable *args, GError **error, gpointer user_data);

void graviton_service_add_stream (GravitonService *self, const gchar *name, GravitonServiceStreamGenerator func, gpointer user_data);

GList *graviton_service_list_streams (GravitonService *self);
GravitonStream *graviton_service_get_stream (GravitonService *self, const gchar *name, GHashTable *args, GError **error);

GravitonService *graviton_service_new (const gchar *serviceName);

#endif // GRAVITON_SERVICE_H
