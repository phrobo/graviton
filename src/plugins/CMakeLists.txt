option(USE_MPD "Enable MPD plugin" TRUE)

if (USE_MPD)
  pkg_search_module(MPD libmpdclient)
  pkg_search_module(GIO_UNIX gio-unix-2.0)

  if (${MPD_FOUND})
    include_directories(${MPD_INCLUDE_DIRS} ${GIO_UNIX_INCLUDE_DIRS})
    add_graviton_plugin(mpd mpd-plugin.c)
    target_link_libraries(graviton-plugin-mpd ${MPD_LIBRARIES} ${GIO_UNIX_LIBRARIES})
    install(TARGETS graviton-plugin-mpd DESTINATION ${GRAVITON_PLUGIN_PATH})
  endif()
endif()
