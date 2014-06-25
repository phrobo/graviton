set(graviton-ping-client_SRCS
  client.c
)

add_executable(graviton-ping-client ${graviton-ping-client_SRCS})

target_link_libraries(graviton-ping-client
  graviton-client
)

install(TARGETS graviton-ping-client DESTINATION bin)

set(graviton-ping-server_SRCS
  server.c
)

add_executable(graviton-ping-server ${graviton-ping-server_SRCS})

target_link_libraries(graviton-ping-server
  graviton-server
)

install(TARGETS graviton-ping-server DESTINATION bin)
