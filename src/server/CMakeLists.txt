set(graviton-server_SRCS
    main.c
    )

find_package(PkgConfig REQUIRED)

pkg_search_module(GLIB REQUIRED glib-2.0)

include_directories(${GLIB_INCLUDE_DIRS})

add_executable(graviton-server ${graviton-server_SRCS})

target_link_libraries(graviton-server ${GLIB_LIBRARIES})
