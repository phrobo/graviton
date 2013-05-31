#ifndef __GRAVITON_CLIENT_H__
#define __GRAVITON_CLIENT_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRAVITON_CLIENT_TYPE            (graviton_client_get_type ())
#define GRAVITON_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_CLIENT_TYPE, GravitonClient))
#define GRAVITON_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_CLIENT_TYPE, GravitonClientClass))
#define IS_GRAVITON_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_CLIENT_TYPE))
#define IS_GRAVITON_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_CLIENT_TYPE))
#define GRAVITON_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_CLIENT_TYPE, GravitonClientClass))

typedef struct _GravitonClient      GravitonClient;
typedef struct _GravitonClientClass GravitonClientClass;
typedef struct _GravitonClientPrivate GravitonClientPrivate;

struct _GravitonClientClass
{
  GObjectClass parent_class;
};

struct _GravitonClient
{
  GObject parent;
  GravitonClientPrivate *priv;
};

GType graviton_client_get_type (void);

GravitonClient *graviton_client_new ();

GList *
graviton_client_get_found_nodes (GravitonClient *client);

G_END_DECLS

#endif
