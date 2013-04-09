set(graviton-server_SRCS
  main.c
  info-plugin.c
)

add_executable(graviton-server ${graviton-server_SRCS})

target_link_libraries(graviton-server
  graviton-plugin
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
)

install(TARGETS graviton-server DESTINATION bin)
