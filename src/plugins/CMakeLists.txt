set(GRAVITON_PLUGIN_PATH "${CMAKE_INSTALL_PREFIX}/share/graviton/plugins/")
macro(add_graviton_plugin _name _source)
  add_library(graviton-plugin-${_name} MODULE ${_source})
  target_link_libraries(graviton-plugin-${_name}
    graviton-plugin
  )
  set_target_properties(graviton-plugin-${_name} PROPERTIES
    OUTPUT_NAME ${_name}
    PREFIX ""
  )
endmacro(add_graviton_plugin)

add_graviton_plugin(info info-plugin.c)
install(TARGETS graviton-plugin-info DESTINATION ${GRAVITON_PLUGIN_PATH})

option(USE_MPD "Enable MPD plugin" TRUE)

if (USE_MPD)
  pkg_search_module(MPD libmpdclient)

  if (${MPD_FOUND})
    include_directories(${MPD_INCLUDE_DIRS})
    add_graviton_plugin(mpd mpd-plugin.c)
    target_link_libraries(graviton-plugin-mpd ${MPD_LIBRARIES})
    install(TARGETS graviton-plugin-mpd DESTINATION ${GRAVITON_PLUGIN_PATH})
  endif()
endif()
