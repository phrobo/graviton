#include "control.h"
#include <string.h>

#define GRAVITON_CONTROL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_CONTROL, GravitonControlPrivate))

GQuark
graviton_control_error_quark ()
{
  return g_quark_from_static_string ("graviton-control-error-quark");
}

G_DEFINE_TYPE (GravitonControl, graviton_control, G_TYPE_OBJECT);

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

struct _GravitonControlPrivate
{
  gchar *name;
  GHashTable *controls;
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
  GravitonControl *self = GRAVITON_CONTROL (object);
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
  GravitonControl *self = GRAVITON_CONTROL (object);
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
graviton_control_class_init (GravitonControlClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  g_type_class_add_private (klass, sizeof (GravitonControlPrivate));

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
graviton_control_init (GravitonControl *self)
{
  GravitonControlPrivate *priv;
  self->priv = priv = GRAVITON_CONTROL_GET_PRIVATE (self);
  priv->name = 0;
  priv->streams = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);
  priv->stream_data = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);
  priv->controls = g_hash_table_new_full (g_str_hash,
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
graviton_control_dispose (GravitonControl *self)
{
  g_hash_table_unref (self->priv->methods);
  g_hash_table_unref (self->priv->method_data);
}

static void
graviton_control_finalize (GravitonControl *self)
{
  g_free (self->priv->name);
}

void
graviton_control_add_method (GravitonControl *self,
                             const gchar *name,
                             GravitonControlMethod func,
                             gpointer user_data,
                             GDestroyNotify destroy_func)
{
  int i;
  g_hash_table_replace (self->priv->methods, g_strdup (name), func);
  g_hash_table_replace (self->priv->method_data, g_strdup (name), user_data);
  g_hash_table_replace (self->priv->method_destroys, g_strdup (name), destroy_func);
}

GVariant *
graviton_control_call_method (GravitonControl *self,
                              const gchar *name,
                              GHashTable *args,
                              GError **error)
{
  GravitonControlMethod func;
  gpointer data;
  func = g_hash_table_lookup (self->priv->methods, name);
  data = g_hash_table_lookup (self->priv->method_data, name);

  g_debug ("Calling %s", name);

  if (func) {
    GVariant *ret = func (self, args, error, data);
    return ret;
  } else {
    g_set_error (error,
                 GRAVITON_CONTROL_ERROR,
                 GRAVITON_CONTROL_ERROR_NO_SUCH_METHOD,
                 "No such method: %s",
                 name);
    return NULL;
  }
}

GList *
graviton_control_list_methods (GravitonControl *self)
{
  return g_hash_table_get_keys (self->priv->methods);
}

gboolean
graviton_control_has_method (GravitonControl *self, const gchar *name)
{
  return g_hash_table_contains (self->priv->methods, name);
}

GravitonControl *
graviton_control_get_subcontrol (GravitonControl *self,
                                 const gchar *path)
{
  GravitonControl *control = NULL;
  gchar **tokens;
  g_return_val_if_fail (path != NULL, NULL);
  g_return_val_if_fail (strlen(path) > 0, NULL);
  tokens = g_strsplit (path, "/", 0);
  control = g_hash_table_lookup (self->priv->controls, tokens[0]);
  if (control) {
    if (g_strv_length (tokens) > 1) {
      gchar *subname = g_strjoinv ("/", &tokens[1]);
      control = graviton_control_get_subcontrol (control, subname);
      g_free (subname);
    } else {
      g_object_ref (control);
    }
  }
  g_strfreev (tokens);
  return control;
}

static void
cb_subcontrol_notify (GravitonControl *subcontrol, GParamSpec *pspec, gpointer user_data)
{
  GravitonControl *self = GRAVITON_CONTROL(user_data);
  gchar *subname;
  g_object_get (subcontrol, "name", &subname, NULL);
  gchar *full_name = g_strdup_printf ("%s/%s", subname, pspec->name);

  g_signal_emit (self, obj_signals[SIGNAL_PROPERTY_UPDATE], 0, full_name);
  g_debug ("Subproperty notify: %s", full_name);
  g_free (full_name);
}

static void
cb_propigate_property_update (GravitonControl *subcontrol, const gchar *name, gpointer user_data)
{
  GravitonControl *self = GRAVITON_CONTROL(user_data);
  gchar *full_name = g_strdup_printf ("%s/%s", self->priv->name, name);

  g_signal_emit (self, obj_signals[SIGNAL_PROPERTY_UPDATE], 0, full_name);

  g_debug ("Subproperty update: %s", full_name);
  g_free (full_name);
}

void
graviton_control_add_subcontrol (GravitonControl *self,
                                      GravitonControl *control)
{
  g_object_ref (control);
  gchar *name;
  g_object_get (control, "name", &name, NULL);
  g_hash_table_replace (self->priv->controls, name, control);

  g_signal_connect (control,
                    "notify",
                    G_CALLBACK(cb_subcontrol_notify),
                    self);
  g_signal_connect (control,
                    "property-update",
                    G_CALLBACK(cb_propigate_property_update),
                    self);
}

GList *
graviton_control_list_subcontrols (GravitonControl *self)
{
  return g_hash_table_get_keys (self->priv->controls);
}

void
graviton_control_add_stream (GravitonControl *self,
                             const gchar *name,
                             GravitonControlStreamGenerator func,
                             gpointer user_data)
{
  g_hash_table_replace (self->priv->streams, g_strdup (name), func);
  g_hash_table_replace (self->priv->stream_data, g_strdup (name), user_data);
}

GList *
graviton_control_list_streams (GravitonControl *self)
{
  return g_hash_table_get_keys (self->priv->streams);
}

GravitonStream *
graviton_control_get_stream (GravitonControl *self, const gchar *name, GHashTable *args, GError **error)
{
  GravitonControlStreamGenerator func = g_hash_table_lookup (self->priv->streams, name);
  gpointer func_data = g_hash_table_lookup (self->priv->stream_data, name);
  if (func)
    return func(self, name, args, error, func_data);
  return NULL;
}

GravitonControl *
graviton_control_new (const gchar *serviceName)
{
  return g_object_new (GRAVITON_TYPE_CONTROL, "name", serviceName, NULL);
}
