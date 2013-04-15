set(libgraviton-server_SRCS
  server.c
  internal-plugin.c
)

add_library(graviton-server ${libgraviton-server_SRCS})

target_link_libraries(graviton-server
  graviton-plugin
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
)

install(TARGETS graviton-server DESTINATION lib${LIB_SUFFIX})
