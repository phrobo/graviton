set(libgraviton-plugin_SRCS
  plugin.c
  plugin-manager.c
)

add_library(graviton-plugin SHARED ${libgraviton-plugin_SRCS})

target_link_libraries(graviton-plugin
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
)
