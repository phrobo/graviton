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

#include "static-stream.h"

#include <string.h>
#include <gio/gio.h>

typedef struct _GravitonStaticStreamPrivate GravitonStaticStreamPrivate;

struct _GravitonStaticStreamPrivate
{
  gchar *contents;
};

#define GRAVITON_STATIC_STREAM_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVITON_STATIC_STREAM_TYPE, GravitonStaticStreamPrivate))

static void graviton_static_stream_class_init (GravitonStaticStreamClass *klass);
static void graviton_static_stream_init       (GravitonStaticStream *self);
static void graviton_static_stream_dispose    (GObject *object);
static void graviton_static_stream_finalize   (GObject *object);
static void graviton_static_stream_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void graviton_static_stream_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE (GravitonStaticStream, graviton_static_stream, GRAVITON_STREAM_TYPE);

enum {
  PROP_ZERO,
  PROP_CONTENTS,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static GInputStream *
open_read (GravitonStream *stream, GError **error)
{
  GravitonStaticStream *self = GRAVITON_STATIC_STREAM (stream);
  GInputStream *ret = g_memory_input_stream_new_from_data (self->priv->contents, strlen(self->priv->contents), NULL);
  return ret;
}

static void
graviton_static_stream_class_init (GravitonStaticStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GravitonStaticStreamPrivate));

  object_class->dispose = graviton_static_stream_dispose;
  object_class->finalize = graviton_static_stream_finalize;
  object_class->set_property =  graviton_static_stream_set_property;
  object_class->get_property =  graviton_static_stream_get_property;

  obj_properties[PROP_CONTENTS] = 
    g_param_spec_string ("contents",
                         "contents",
                         "The contents, as a string",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT );

  g_object_class_install_properties (object_class,
      N_PROPERTIES,
      obj_properties);

  GravitonStreamClass *stream_class = GRAVITON_STREAM_CLASS (klass);
  stream_class->open_read = open_read;
}

static void
graviton_static_stream_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  GravitonStaticStream *self = GRAVITON_STATIC_STREAM (object);
  switch (property_id) {
    case PROP_CONTENTS:
      if (self->priv->contents)
        g_free (self->priv->contents);
      self->priv->contents = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_static_stream_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  GravitonStaticStream *self = GRAVITON_STATIC_STREAM (object);
  switch (property_id) {
    case PROP_CONTENTS:
      g_value_set_string (value, self->priv->contents);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
graviton_static_stream_init (GravitonStaticStream *self)
{
  GravitonStaticStreamPrivate *priv;
  priv = self->priv = GRAVITON_STATIC_STREAM_GET_PRIVATE (self);
  priv->contents = NULL;
}

static void
graviton_static_stream_dispose (GObject *object)
{
  GravitonStaticStream *self = GRAVITON_STATIC_STREAM (object);
  G_OBJECT_CLASS (graviton_static_stream_parent_class)->dispose (object);
  g_free (self->priv->contents);
}

static void
graviton_static_stream_finalize (GObject *object)
{
  G_OBJECT_CLASS (graviton_static_stream_parent_class)->finalize (object);
}

GravitonStaticStream *
graviton_static_stream_new_contents (const gchar *contents)
{
  return g_object_new (GRAVITON_STATIC_STREAM_TYPE, "contents", contents, NULL);
}

GravitonStaticStream *
graviton_static_stream_new ()
{
  return g_object_new (GRAVITON_STATIC_STREAM_TYPE, NULL);
}
