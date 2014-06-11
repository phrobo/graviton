/*
 * This file is part of Graviton.
 * Copyright (C) 2013-2014 Torrie Fischer <tdfischer@phrobo.net>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "stream.h"

typedef struct _GravitonStreamPrivate GravitonStreamPrivate;

struct _GravitonStreamPrivate
{
  gchar *name;
};

#define GRAVITON_STREAM_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_STREAM_TYPE, GravitonStreamPrivate))

static void graviton_stream_class_init (GravitonStreamClass *klass);
static void graviton_stream_init       (GravitonStream *self);
static void graviton_stream_dispose    (GObject *object);
static void graviton_stream_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonStream, graviton_stream, G_TYPE_OBJECT);

static void
graviton_stream_class_init (GravitonStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonStreamPrivate));

  object_class->dispose = graviton_stream_dispose;
  object_class->finalize = graviton_stream_finalize;
}

static void
graviton_stream_init (GravitonStream *self)
{
  GravitonStreamPrivate *priv;
  priv = self->priv = GRAVITON_STREAM_GET_PRIVATE (self);
  priv->name = NULL;
}

static void
graviton_stream_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_stream_parent_class)->dispose (object);
}

static void
graviton_stream_finalize (GObject *object)
{
  GravitonStream *self = GRAVITON_STREAM (object);
  g_free (self->priv->name);

  G_OBJECT_CLASS (graviton_stream_parent_class)->finalize (object);
}

GravitonStream *
graviton_stream_new (const gchar *name)
{
  //FIXME: This is not actually ever set!
  return g_object_new (GRAVITON_STREAM_TYPE, "name", name, NULL);
}

const gchar *
graviton_stream_get_name (GravitonStream *self)
{
  return self->priv->name;
}

GInputStream *
graviton_stream_open_read (GravitonStream *self, GError **error)
{
  return GRAVITON_STREAM_GET_CLASS(self)->open_read (self, error);
}

GOutputStream *
graviton_stream_open_write (GravitonStream *self, GError **error)
{
  return GRAVITON_STREAM_GET_CLASS(self)->open_write (self, error);
}
