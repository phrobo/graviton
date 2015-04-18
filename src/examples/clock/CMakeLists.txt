set(graviton-clock-client_SRCS
  client.c
)

add_executable(graviton-clock-client ${graviton-clock-client_SRCS})

target_link_libraries(graviton-clock-client
  graviton-client
)

install(TARGETS graviton-clock-client DESTINATION bin)

set(graviton-clock-server_SRCS
  server.c
)

add_executable(graviton-clock-server ${graviton-clock-server_SRCS})

target_link_libraries(graviton-clock-server
  graviton-server
)

install(TARGETS graviton-clock-server DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/graviton-clock-server.service
  DESTINATION /lib/systemd/system/)
