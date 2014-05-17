#ifndef __GRAVITON_SERVICE_INTERFACE_H__
#define __GRAVITON_SERVICE_INTERFACE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRAVITON_SERVICE_INTERFACE_TYPE            (graviton_service_interface_get_type ())
#define GRAVITON_SERVICE_INTERFACE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_SERVICE_INTERFACE_TYPE, GravitonServiceInterface))
#define GRAVITON_SERVICE_INTERFACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_SERVICE_INTERFACE_TYPE, GravitonServiceInterfaceClass))
#define IS_GRAVITON_SERVICE_INTERFACE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_SERVICE_INTERFACE_TYPE))
#define IS_GRAVITON_SERVICE_INTERFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_SERVICE_INTERFACE_TYPE))
#define GRAVITON_SERVICE_INTERFACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_SERVICE_INTERFACE_TYPE, GravitonServiceInterfaceClass))

typedef struct _GravitonNode GravitonNode;
typedef struct _GravitonNodeStream GravitonNodeStream;

typedef struct _GravitonServiceInterface      GravitonServiceInterface;
typedef struct _GravitonServiceInterfaceClass GravitonServiceInterfaceClass;

typedef struct _GravitonServiceInterfacePrivate GravitonServiceInterfacePrivate;

struct _GravitonServiceInterfaceClass
{
  GObjectClass parent_class;
};

struct _GravitonServiceInterface
{
  GObject parent;
  GravitonServiceInterfacePrivate *priv;
};

GType graviton_service_interface_get_type (void);

const gchar *graviton_service_interface_get_name (GravitonServiceInterface *self);
GList *graviton_service_interface_list_subservices (GravitonServiceInterface *self, GError **error);
GravitonServiceInterface *graviton_service_interface_get_subservice (GravitonServiceInterface *self, const gchar *name);
GList *graviton_service_interface_list_properties (GravitonServiceInterface *control, GError **error);
GVariant *graviton_service_interface_get_property (GravitonServiceInterface *control, const gchar *prop, GError **error);
GravitonNode *graviton_service_interface_get_node (GravitonServiceInterface *control);
GVariant *graviton_service_interface_call (GravitonServiceInterface *control, const gchar *method, GError **error, ...);
GVariant *graviton_service_interface_call_args (GravitonServiceInterface *control, const gchar *method, GHashTable *args, GError **error);
GVariant *graviton_service_interface_call_va (GravitonServiceInterface *control, const gchar *method, GError **error, va_list args);

GravitonNodeStream *graviton_service_interface_get_stream (GravitonServiceInterface *control, const gchar *name, GHashTable *args);

G_END_DECLS

#endif
