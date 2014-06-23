set(graviton-stream-client_SRCS
  main.c
)

add_executable(graviton-stream-client ${graviton-stream-client_SRCS})

target_link_libraries(graviton-stream-client
  graviton-client
)

install(TARGETS graviton-stream-client DESTINATION bin)
