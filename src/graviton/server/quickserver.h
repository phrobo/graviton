/*
 * This file is part of Graviton.
 * Copyright (C) 2013-2015 Torrie Fischer <tdfischer@phrobo.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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
GravitonService *graviton_quickserver_add_service (GravitonQuickserver *server,
                                                   const gchar *service_name);
void graviton_quickserver_add_method (GravitonQuickserver *server,
                                      const gchar *service_name, 
                                      const gchar *method_name,
                                      GravitonServiceMethod func,
                                      gpointer user_data,
                                      GDestroyNotify destroy_func);

void graviton_quickserver_run (GravitonQuickserver *server);

G_END_DECLS

#endif
