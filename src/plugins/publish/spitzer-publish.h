#ifndef __GRAVITON_SPITZER_PUBLISH_METHOD_H__
#define __GRAVITON_SPITZER_PUBLISH_METHOD_H__

#include <glib.h>
#include <glib-object.h>

#include <graviton/server/server-publish-method.h>

G_BEGIN_DECLS

#define GRAVITON_SPITZER_PUBLISH_METHOD_TYPE            (graviton_spitzer_publish_method_get_type ())
#define GRAVITON_SPITZER_PUBLISH_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_SPITZER_PUBLISH_METHOD_TYPE, GravitonSpitzerPublishMethod))
#define GRAVITON_SPITZER_PUBLISH_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_SPITZER_PUBLISH_METHOD_TYPE, GravitonSpitzerPublishMethodClass))
#define IS_GRAVITON_SPITZER_PUBLISH_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_SPITZER_PUBLISH_METHOD_TYPE))
#define IS_GRAVITON_SPITZER_PUBLISH_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_SPITZER_PUBLISH_METHOD_TYPE))
#define GRAVITON_SPITZER_PUBLISH_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_SPITZER_PUBLISH_METHOD_TYPE, GravitonSpitzerPublishMethodClass))

typedef struct _GravitonSpitzerPublishMethod      GravitonSpitzerPublishMethod;
typedef struct _GravitonSpitzerPublishMethodClass GravitonSpitzerPublishMethodClass;
typedef struct _GravitonSpitzerPublishMethodPrivate GravitonSpitzerPublishMethodPrivate;

struct _GravitonSpitzerPublishMethodClass
{
  GravitonServerPublishMethodClass parent_class;
};

struct _GravitonSpitzerPublishMethod
{
  GravitonServerPublishMethod parent;
  GravitonSpitzerPublishMethodPrivate *priv;
};

GType graviton_spitzer_publish_method_get_type (void);

GravitonSpitzerPublishMethod *graviton_spitzer_publish_method_new ();

G_END_DECLS

#endif
