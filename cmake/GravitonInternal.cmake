set(GRAVITON_VERSION
  "${GRAVITON_VERSION_MAJOR}.${GRAVITON_VERSION_MINOR}.${GRAVITON_VERSION_PATCH}")

function(add_graviton_discovery_plugin)
  cmake_parse_arguments(_plugin "" "NAME" "SOURCES" ${ARGN})
  add_library(${_plugin_NAME} MODULE ${_plugin_SOURCES})
  string(REPLACE ";" " " _LDFLAGS "${GRAVITON_LDFLAGS};${GRAVITON_LDFLAGS_OTHER}")
  set_target_properties(${_plugin_NAME} PROPERTIES
    OUTPUT_NAME ${_plugin_NAME}
    PREFIX ""
    LINK_FLAGS "${_LDFLAGS}"
  )
  target_link_libraries(${_plugin_NAME} graviton-client)
endfunction(add_graviton_discovery_plugin)

function(add_graviton_publish_plugin)
  cmake_parse_arguments(_plugin "" "NAME" "SOURCES" ${ARGN})
  add_library(${_plugin_NAME} MODULE ${_plugin_SOURCES})
  string(REPLACE ";" " " _LDFLAGS "${GRAVITON_LDFLAGS};${GRAVITON_LDFLAGS_OTHER}")
  set_target_properties(${_plugin_NAME} PROPERTIES
    OUTPUT_NAME ${_plugin_NAME}
    PREFIX ""
    LINK_FLAGS "${_LDFLAGS}"
  )
  target_link_libraries(${_plugin_NAME} graviton-server)
endfunction(add_graviton_publish_plugin)
