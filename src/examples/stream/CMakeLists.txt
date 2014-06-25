set(graviton-stream-client_SRCS
  client.c
)

add_executable(graviton-stream-client ${graviton-stream-client_SRCS})

target_link_libraries(graviton-stream-client
  graviton-client
)

install(TARGETS graviton-stream-client DESTINATION bin)

set(graviton-stream-server_SRCS
  server.c
)

add_executable(graviton-stream-server ${graviton-stream-server_SRCS})

target_link_libraries(graviton-stream-server
  graviton-server
)

install(TARGETS graviton-stream-server DESTINATION bin)
