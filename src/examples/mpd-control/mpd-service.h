#ifndef GRAVITON_MPD_SERVICE_H

#include <glib-object.h>
#include <graviton/server/service.h>
#include <mpd/client.h>

#define GRAVITON_MPD_SERVICE_TYPE     (graviton_mpd_service_get_type ())
#define GRAVITON_MPD_SERVICE(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVITON_MPD_SERVICE_TYPE, GravitonMPDService))
#define GRAVITON_IS_MPD_SERVICE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVITON_MPD_SERVICE_TYPE))
#define GRAVITON_MPD_SERVICE_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVITON_MPD_SERVICE_TYPE, GravitonMPDServiceClass))
#define GRAVITON_IS_MPD_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVITON_MPD_SERVICE_TYPE))
#define GRAVITON_MPD_SERVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVITON_MPD_SERVICE_TYPE, GravitonMPDServiceClass))

#define GRAVITON_MPD_ERROR (graviton_mpd_error_quark ())
#define GRAVITON_MPD_SERVER_ERROR (graviton_mpd_server_error_quark ())
#define GRAVITON_MPD_SYSTEM_ERROR (graviton_mpd_system_error_quark ())

typedef enum {
  GRAVITON_MPD_ERROR_SUCCESS = MPD_ERROR_SUCCESS,
  GRAVITON_MPD_ERROR_OOM = MPD_ERROR_OOM,
  GRAVITON_MPD_ERROR_ARGUMENT = MPD_ERROR_ARGUMENT,
  GRAVITON_MPD_ERROR_STATE = MPD_ERROR_STATE,
  GRAVITON_MPD_ERROR_TIMEOUT = MPD_ERROR_TIMEOUT,
  GRAVITON_MPD_ERROR_SYSTEM = MPD_ERROR_SYSTEM,
  GRAVITON_MPD_ERROR_RESOLVER = MPD_ERROR_RESOLVER,
  GRAVITON_MPD_ERROR_MALFORMED = MPD_ERROR_MALFORMED,
  GRAVITON_MPD_ERROR_CLOSED = MPD_ERROR_CLOSED,
  GRAVITON_MPD_ERROR_SERVER = MPD_ERROR_SERVER
} GravitonMPDError;

typedef struct _GravitonMPDService GravitonMPDService;
typedef struct _GravitonMPDServiceClass GravitonMPDServiceClass;

typedef struct _GravitonMPDServicePrivate GravitonMPDServicePrivate;

struct _GravitonMPDService
{
  GravitonService parent_instance;
  GravitonMPDServicePrivate *priv;
};

struct _GravitonMPDServiceClass
{
  GravitonServiceClass parent_class;
};

GType graviton_mpd_service_get_type ();

GravitonMPDService *graviton_mpd_service_new (const gchar *address, guint port);

#endif // GRAVITON_MPD_SERVICE_H
