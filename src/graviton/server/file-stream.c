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

#include "file-stream.h"

typedef struct _GravitonFileStreamPrivate GravitonFileStreamPrivate;

struct _GravitonFileStreamPrivate
{
  GFile *file;
};

#define GRAVITON_FILE_STREAM_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_FILE_STREAM_TYPE, \
                                GravitonFileStreamPrivate))

static void graviton_file_stream_class_init (GravitonFileStreamClass *klass);
static void graviton_file_stream_init       (GravitonFileStream *self);
static void graviton_file_stream_dispose    (GObject *object);
static void graviton_file_stream_finalize   (GObject *object);

G_DEFINE_TYPE (GravitonFileStream, graviton_file_stream, GRAVITON_STREAM_TYPE);

enum {
  PROP_0,
  PROP_FILE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
set_property (GObject *object,
              guint property_id,
              const GValue *value,
              GParamSpec *pspec)
{
  GravitonFileStream *self = GRAVITON_FILE_STREAM (object);
  switch (property_id) {
  case PROP_FILE:
    self->priv->file = g_value_dup_object (value);
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
  GravitonFileStream *self = GRAVITON_FILE_STREAM (object);
  switch (property_id) {
  case PROP_FILE:
    g_value_set_object (value, self->priv->file);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static GInputStream *
open_read (GravitonStream *stream, GError **error)
{
  GravitonFileStream *self = GRAVITON_FILE_STREAM (stream);
  GFileInputStream *ret = g_file_read (self->priv->file, NULL, error);
  return G_INPUT_STREAM (ret);
}

static GOutputStream *
open_write (GravitonStream *stream, GError **error)
{
  GravitonFileStream *self = GRAVITON_FILE_STREAM (stream);
  return G_OUTPUT_STREAM (g_file_append_to (self->priv->file,
                                            G_FILE_CREATE_NONE, NULL, error));
}

static void
graviton_file_stream_class_init (GravitonFileStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonFileStreamPrivate));

  object_class->dispose = graviton_file_stream_dispose;
  object_class->finalize = graviton_file_stream_finalize;
  object_class->set_property = set_property;
  object_class->get_property = get_property;

  obj_properties[PROP_FILE] =
    g_param_spec_object ("file",
                         "GFile",
                         "Underlying GFile",
                         G_TYPE_FILE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY );

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);

  GravitonStreamClass *stream_class = GRAVITON_STREAM_CLASS (klass);

  stream_class->open_read = open_read;
  stream_class->open_write = open_write;
}

static void
graviton_file_stream_init (GravitonFileStream *self)
{
  GravitonFileStreamPrivate *priv;
  priv = self->priv = GRAVITON_FILE_STREAM_GET_PRIVATE (self);
  priv->file = NULL;
}

static void
graviton_file_stream_dispose (GObject *object)
{
  G_OBJECT_CLASS (graviton_file_stream_parent_class)->dispose (object);
}

static void
graviton_file_stream_finalize (GObject *object)
{
  GravitonFileStream *self = GRAVITON_FILE_STREAM (object);
  G_OBJECT_CLASS (graviton_file_stream_parent_class)->finalize (object);
  g_object_unref (self->priv->file);
}

GravitonFileStream *
graviton_file_stream_new (GFile *file)
{
  return g_object_new (GRAVITON_FILE_STREAM_TYPE, "file", file, NULL);
}