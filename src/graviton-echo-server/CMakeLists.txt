set(graviton-echo-server_SRCS
  main.c
)

add_executable(graviton-echo-server ${graviton-echo-server_SRCS})

target_link_libraries(graviton-echo-server
  graviton
)

install(TARGETS graviton-echo-server DESTINATION bin)
