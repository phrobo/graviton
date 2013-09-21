set(graviton-ping-server_SRCS
  main.c
)

add_executable(graviton-ping-server ${graviton-ping-server_SRCS})

target_link_libraries(graviton-ping-server
  graviton
)

install(TARGETS graviton-ping-server DESTINATION bin)
