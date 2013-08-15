set(graviton-ping-client_SRCS
  main.c
)

add_executable(graviton-ping-client ${graviton-ping-client_SRCS})

target_link_libraries(graviton-ping-client
  graviton
)

install(TARGETS graviton-ping-client DESTINATION bin)

