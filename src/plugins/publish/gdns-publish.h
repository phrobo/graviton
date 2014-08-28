#ifndef __GRAVITON_GDNS_PUBLISH_METHOD_H__
#define __GRAVITON_GDNS_PUBLISH_METHOD_H__

#include <glib.h>
#include <glib-object.h>

#include <graviton/server/server-publish-method.h>

G_BEGIN_DECLS

#define GRAVITON_GDNS_PUBLISH_METHOD_TYPE            (graviton_gdns_publish_method_get_type ())
#define GRAVITON_GDNS_PUBLISH_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_GDNS_PUBLISH_METHOD_TYPE, GravitonGdnsPublishMethod))
#define GRAVITON_GDNS_PUBLISH_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_GDNS_PUBLISH_METHOD_TYPE, GravitonGdnsPublishMethodClass))
#define IS_GRAVITON_GDNS_PUBLISH_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_GDNS_PUBLISH_METHOD_TYPE))
#define IS_GRAVITON_GDNS_PUBLISH_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_GDNS_PUBLISH_METHOD_TYPE))
#define GRAVITON_GDNS_PUBLISH_METHOD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_GDNS_PUBLISH_METHOD_TYPE, GravitonGdnsPublishMethodClass))

typedef struct _GravitonGdnsPublishMethod      GravitonGdnsPublishMethod;
typedef struct _GravitonGdnsPublishMethodClass GravitonGdnsPublishMethodClass;
typedef struct _GravitonGdnsPublishMethodPrivate GravitonGdnsPublishMethodPrivate;

struct _GravitonGdnsPublishMethodClass
{
  GravitonServerPublishMethodClass parent_class;
};

struct _GravitonGdnsPublishMethod
{
  GravitonServerPublishMethod parent;
  GravitonGdnsPublishMethodPrivate *priv;
};

GType graviton_gdns_publish_method_get_type (void);

GravitonGdnsPublishMethod *graviton_gdns_publish_method_new ();

G_END_DECLS

#endif
