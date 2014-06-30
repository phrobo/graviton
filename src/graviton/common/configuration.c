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

#include "configuration.h"
#include <uuid/uuid.h>

const gchar *
graviton_config_get_default_cloud_id ()
{
  static gchar *cloud_id = NULL;
  GKeyFile *config;
  uuid_t uuid;
  const gchar ** config_paths;
  guint num_sys_config_paths;
  const gchar *const *sys_config_paths;
  int i;

  if (cloud_id)
    return cloud_id;

  sys_config_paths = g_get_system_config_dirs ();
  num_sys_config_paths = g_strv_length ((gchar**)sys_config_paths);
  config_paths = g_new0 (const gchar*, num_sys_config_paths+1);
  config_paths[0] = g_get_user_config_dir ();

  for (i = 1; i < num_sys_config_paths; i++) {
    config_paths[i] = sys_config_paths[i];
  }

  config = g_key_file_new ();
  g_key_file_load_from_dirs (config, "graviton.cfg", config_paths, NULL, G_KEY_FILE_KEEP_COMMENTS, NULL);

  cloud_id = g_key_file_get_string (config, "general", "cloud-id", NULL);

  if (cloud_id) {
    if (uuid_parse (cloud_id, uuid) == -1) {
      g_free (cloud_id);
      cloud_id = NULL;
    }
  }

  if (cloud_id == NULL) {
    gchar *user_config;
    gchar *config_data;

    user_config = g_strdup_printf ("%s/%s", g_get_user_config_dir (), "graviton.cfg");
    g_debug ("Writing new cloud id to %s", user_config);
    uuid_generate (uuid);
    cloud_id = g_new0(gchar, 37);
    uuid_unparse_upper (uuid, cloud_id);
    g_key_file_set_string (config, "general", "cloud-id", cloud_id);
    config_data = g_key_file_to_data (config, NULL, NULL);

    g_file_set_contents (user_config, config_data, -1, NULL);

    g_free (config_data);
    g_free (user_config);
  }

  g_free (config_paths);

  return cloud_id;
}
