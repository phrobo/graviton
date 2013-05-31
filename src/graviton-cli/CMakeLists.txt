set(graviton-cli_SRCS
  main.c
)

add_executable(graviton-cli ${graviton-cli_SRCS})

target_link_libraries(graviton-cli
  graviton
)

install(TARGETS graviton-cli DESTINATION bin)
