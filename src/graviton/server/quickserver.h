#ifndef __GRAVITON_QUICKSERVER_H__
#define __GRAVITON_QUICKSERVER_H__

#include <glib.h>
#include <glib-object.h>

#include "server.h"

G_BEGIN_DECLS

#define GRAVITON_QUICKSERVER_TYPE            (graviton_quickserver_get_type ())
#define GRAVITON_QUICKSERVER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_QUICKSERVER_TYPE, GravitonQuickserver))
#define GRAVITON_QUICKSERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_QUICKSERVER_TYPE, GravitonQuickserverClass))
#define IS_GRAVITON_QUICKSERVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_QUICKSERVER_TYPE))
#define IS_GRAVITON_QUICKSERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_QUICKSERVER_TYPE))
#define GRAVITON_QUICKSERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_QUICKSERVER_TYPE, GravitonQuickserverClass))

typedef struct _GravitonQuickserver      GravitonQuickserver;
typedef struct _GravitonQuickserverClass GravitonQuickserverClass;
typedef struct _GravitonQuickserverPrivate GravitonQuickserverPrivate;

struct _GravitonQuickserverClass
{
  GravitonServerClass parent_class;
};

struct _GravitonQuickserver
{
  GravitonServer parent;
  GravitonQuickserverPrivate *priv;
};

GType graviton_quickserver_get_type (void);

GravitonQuickserver *graviton_quickserver_new ();

G_END_DECLS

#endif
