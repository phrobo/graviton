set(graviton-server_SRCS
  main.c
  info-plugin.c
)

add_executable(graviton-server-bin ${graviton-server_SRCS})

target_link_libraries(graviton-server-bin
  graviton-plugin
  graviton-server
)

set_target_properties(graviton-server-bin PROPERTIES OUTPUT_NAME
graviton-server)

install(TARGETS graviton-server-bin DESTINATION bin)
