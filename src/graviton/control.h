#ifndef GRAVITON_CONTROL_H
#define GRAVITON_CONTROL_H

#include <glib-object.h>
#include <gio/gio.h>

#define GRAVITON_CONTROL_TYPE            (graviton_control_get_type ())
#define GRAVITON_CONTROL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_CONTROL_TYPE, GravitonControl))
#define GRAVITON_IS_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_CONTROL_TYPE))
#define GRAVITON_CONTROL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_CONTROL_TYPE, GravitonControlClass))
#define GRAVITON_IS_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_CONTROL_TYPE))
#define GRAVITON_CONTROL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_CONTROL_TYPE, GravitonControlClass))

#define GRAVITON_CONTROL_ERROR (graviton_control_error_quark ())

typedef enum {
  GRAVITON_CONTROL_ERROR_NO_SUCH_METHOD
} GravitonControlError;

typedef struct _GravitonStream GravitonStream;

typedef struct _GravitonControl GravitonControl;
typedef struct _GravitonControlClass GravitonControlClass;

typedef struct _GravitonControlPrivate GravitonControlPrivate;

struct _GravitonControl 
{
  GObject parent_instance;
  GravitonControlPrivate *priv;
};

struct _GravitonControlClass
{
  GObjectClass parent_class;
};

GType graviton_control_get_type ();

typedef GVariant *(*GravitonControlMethod)(GravitonControl *self, GHashTable *args, GError **error, gpointer user_data);

void graviton_control_add_method (GravitonControl *self,
                                  const gchar *name,
                                  GravitonControlMethod func,
                                  gpointer user_data,
                                  GDestroyNotify destroy_func);

GVariant *graviton_control_call_method (GravitonControl *self,
                                   const gchar *name,
                                   GHashTable *args,
                                   GError **error);

GList *graviton_control_list_methods (GravitonControl *self);

gboolean graviton_control_has_method (GravitonControl *self, const gchar *name);

void graviton_control_add_subcontrol (GravitonControl *self,
                                       GravitonControl *control);

GravitonControl *graviton_control_get_subcontrol (GravitonControl *self,
                                  const gchar *name);

GList *graviton_control_list_subcontrols (GravitonControl *self);

typedef GravitonStream *(*GravitonControlStreamGenerator)(GravitonControl *self, const gchar *name, GHashTable *args, GError **error, gpointer user_data);

void graviton_control_add_stream (GravitonControl *self, const gchar *name, GravitonControlStreamGenerator func, gpointer user_data);

GList *graviton_control_list_streams (GravitonControl *self);
GravitonStream *graviton_control_get_stream (GravitonControl *self, const gchar *name, GHashTable *args, GError **error);

GravitonControl *graviton_control_new (const gchar *serviceName);

#endif // GRAVITON_CONTROL_H
