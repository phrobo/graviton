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

#define GRAVITON_STREAM_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_STREAM_TYPE, \
                                GravitonStreamPrivate))

static void graviton_stream_class_init (GravitonStreamClass *klass);
static void graviton_stream_init       (GravitonStream *self);
static void graviton_stream_dispose    (GObject *object);
static void graviton_stream_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonStream, graviton_stream, G_TYPE_OBJECT);

static void
graviton_stream_class_init (GravitonStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = graviton_stream_dispose;
  object_class->finalize = graviton_stream_finalize;
}

static void
graviton_stream_init (GravitonStream *self)
{
}

static void
graviton_stream_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_stream_parent_class)->dispose (object);
}

static void
graviton_stream_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_stream_parent_class)->finalize (object);
}

GravitonStream *
graviton_stream_new ()
{
  return g_object_new (GRAVITON_STREAM_TYPE, NULL);
}

GInputStream *
graviton_stream_open_read (GravitonStream *self, GError **error)
{
  return GRAVITON_STREAM_GET_CLASS(self)->open_read (self, error);
}

GOutputStream *
graviton_stream_open_write (GravitonStream *self, GError **error)
{
  if (GRAVITON_STREAM_GET_CLASS(self)->open_write)
    return GRAVITON_STREAM_GET_CLASS(self)->open_write (self, error);
  g_set_error (error,
               G_IO_ERROR,
               G_IO_ERROR_READ_ONLY,
               "Stream is read only");
  return NULL;
}
