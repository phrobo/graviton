#ifndef __GRAVITON_SERVICE_H__
#define __GRAVITON_SERVICE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRAVITON_SERVICE_TYPE            (graviton_service_get_type ())
#define GRAVITON_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_SERVICE_TYPE, GravitonService))
#define GRAVITON_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_SERVICE_TYPE, GravitonServiceClass))
#define IS_GRAVITON_SERVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_SERVICE_TYPE))
#define IS_GRAVITON_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_SERVICE_TYPE))
#define GRAVITON_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_SERVICE_TYPE, GravitonServiceClass))

typedef struct _GravitonNode GravitonNode;
typedef struct _GravitonNodeStream GravitonNodeStream;

typedef struct _GravitonService      GravitonService;
typedef struct _GravitonServiceClass GravitonServiceClass;

typedef struct _GravitonServicePrivate GravitonServicePrivate;

struct _GravitonServiceClass
{
  GObjectClass parent_class;
};

struct _GravitonService
{
  GObject parent;
  GravitonServicePrivate *priv;
};

GType graviton_service_get_type (void);

const gchar *graviton_service_get_name (GravitonService *self);
GList *graviton_service_list_subcontrols (GravitonService *self, GError **error);
GravitonService *graviton_service_get_subcontrol (GravitonService *self, const gchar *name);
GList *graviton_service_list_properties (GravitonService *control, GError **error);
GVariant *graviton_service_get_property (GravitonService *control, const gchar *prop, GError **error);
GravitonNode *graviton_service_get_node (GravitonService *control);
GVariant *graviton_service_call (GravitonService *control, const gchar *method, GError **error, ...);
GVariant *graviton_service_call_args (GravitonService *control, const gchar *method, GHashTable *args, GError **error);
GVariant *graviton_service_call_va (GravitonService *control, const gchar *method, GError **error, va_list args);

GravitonNodeStream *graviton_service_get_stream (GravitonService *control, const gchar *name, GHashTable *args);

G_END_DECLS

#endif
