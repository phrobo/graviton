#include "control.h"

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

struct _GravitonControlPrivate
{
  gchar *name;
  GHashTable *controls;
  GHashTable *methods;
  GHashTable *method_data;
  GHashTable *method_destroys;
  GHashTable *method_args;
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
}

static void
graviton_control_init (GravitonControl *self)
{
  GravitonControlPrivate *priv;
  self->priv = priv = GRAVITON_CONTROL_GET_PRIVATE (self);
  priv->name = 0;
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
graviton_control_add_method (GravitonControl *self,
                             const gchar *name,
                             GravitonControlMethod func,
                             int arg_count,
                             GParamSpec **args,
                             gpointer user_data,
                             GDestroyNotify destroy_func)
{
  int i;
  g_hash_table_replace (self->priv->methods, g_strdup (name), func);
  g_hash_table_replace (self->priv->method_data, g_strdup (name), user_data);
  g_hash_table_replace (self->priv->method_destroys, g_strdup (name), destroy_func);

  GList *paramList = NULL;

  for (i = 0; i < arg_count; i++) {
    g_list_append (paramList, args[i]);
    g_param_spec_ref (args[i]);
  }
  g_hash_table_replace (self->priv->method_args, g_strdup (name), paramList);
}

GVariant *
graviton_control_call_method (GravitonControl *self,
                              const gchar *name,
                              GHashTable *args,
                              GError **error)
{
  GravitonControlMethod func;
  gpointer data;
  GList *params;
  func = g_hash_table_lookup (self->priv->methods, name);
  data = g_hash_table_lookup (self->priv->method_data, name);
  params = g_hash_table_lookup (self->priv->method_args, name);

  g_debug ("Calling %s", name);

  if (func) {
    GList *cur = params;
    GHashTable *calledParams = g_hash_table_new_full (g_str_hash,
                                                      g_str_equal,
                                                      g_free,
                                                      (GDestroyNotify)g_variant_unref);
    while (cur) {
      GParamSpec *param = cur->data;
      GVariant *paramIn = g_hash_table_lookup (args, param->name);
      GVariant *convertedVariant;
      if (paramIn == NULL) {
        /*GValue valueIn = G_VALUE_INIT;
        GValue convertedValue = G_VALUE_INIT;
        g_param_value_set_default (param, &inValue);
        g_param_value_convert (param, &inValue, &convertedValue, FALSE);*/
        convertedVariant = g_variant_new_string (NULL);
      } else {
        convertedVariant = paramIn;
      }
      g_hash_table_replace (calledParams, g_strdup (param->name), convertedVariant);
      cur = g_list_next (cur);
    }
    GVariant *ret = func (self, args, error, data);
    g_hash_table_unref (calledParams);
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
  tokens = g_strsplit (path, ".", 0);
  control = g_hash_table_lookup (self->priv->controls, tokens[0]);
  if (control) {
    if (g_strv_length (tokens) > 1) {
      gchar *subname = g_strjoinv (".", &tokens[1]);
      control = graviton_control_get_subcontrol (control, subname);
      g_free (subname);
    } else {
      g_object_ref (control);
    }
  }
  g_strfreev (tokens);
  return control;
}

void
graviton_control_add_subcontrol (GravitonControl *self,
                                      GravitonControl *control)
{
  g_object_ref (control);
  gchar *name;
  g_object_get (control, "name", &name, NULL);
  g_hash_table_replace (self->priv->controls, name, control);
}

GList *
graviton_control_list_subcontrols (GravitonControl *self)
{
  return g_hash_table_get_keys (self->priv->controls);
}
