set(graviton-stream-server_SRCS
  main.c
)

add_executable(graviton-stream-server ${graviton-stream-server_SRCS})

target_link_libraries(graviton-stream-server
  graviton-server
)

install(TARGETS graviton-stream-server DESTINATION bin)
