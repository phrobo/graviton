#ifndef __GRAVITON_FILE_STREAM_H__
#define __GRAVITON_FILE_STREAM_H__

#include <glib.h>
#include <glib-object.h>

#include "stream.h"

G_BEGIN_DECLS

#define GRAVITON_FILE_STREAM_TYPE            (graviton_file_stream_get_type ())
#define GRAVITON_FILE_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_FILE_STREAM_TYPE, GravitonFileStream))
#define GRAVITON_FILE_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_FILE_STREAM_TYPE, GravitonFileStreamClass))
#define IS_GRAVITON_FILE_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_FILE_STREAM_TYPE))
#define IS_GRAVITON_FILE_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_FILE_STREAM_TYPE))
#define GRAVITON_FILE_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_FILE_STREAM_TYPE, GravitonFileStreamClass))

typedef struct _GravitonFileStream      GravitonFileStream;
typedef struct _GravitonFileStreamClass GravitonFileStreamClass;
typedef struct _GravitonFileStreamPrivate GravitonFileStreamPrivate;

struct _GravitonFileStreamClass
{
  GravitonStreamClass parent_class;
};

struct _GravitonFileStream
{
  GravitonStream parent;
  GravitonFileStreamPrivate *priv;
};

GType graviton_file_stream_get_type (void);

GravitonFileStream *graviton_file_stream_new (GFile *file);

G_END_DECLS

#endif
