#include "control.h"

#define GRAVITON_CONTROL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRAVITON_TYPE_CONTROL, GravitonControlPrivate))

GQuark
graviton_control_error_quark ()
{
  return g_quark_from_static_string ("graviton-control-error-quark");
}

G_DEFINE_TYPE (GravitonControl, graviton_control, G_TYPE_OBJECT);

struct _GravitonControlPrivate
{
  gchar *name;
  GHashTable *methods;
  GHashTable *method_data;
  GHashTable *method_args;
};

static void
graviton_control_class_init (GravitonControlClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  g_type_class_add_private (klass, sizeof (GravitonControlPrivate));
}

static void
graviton_control_init (GravitonControl *self)
{
  GravitonControlPrivate *priv;
  self->priv = priv = GRAVITON_CONTROL_GET_PRIVATE (self);
  priv->name = 0;
  priv->methods = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);
  priv->method_data = g_hash_table_new_full (g_str_hash,
                                        g_str_equal,
                                        g_free,
                                        NULL);
  priv->method_args = g_hash_table_new_full (g_str_hash,
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
graviton_control_add_method (GravitonControl *self, const gchar *name, GravitonControlMethod func, GParamSpec **args, int arg_count, gpointer user_data)
{
  int i;
  g_hash_table_replace (self->priv->methods, g_strdup (name), func);
  g_hash_table_replace (self->priv->method_data, g_strdup (name), user_data);

  for (i = 0; i < arg_count; i++) {
    g_param_spec_ref (args[i]);
  }
  g_hash_table_replace (self->priv->method_args, g_strdup (name), args);
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
    return func (self, args, error, data);
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
