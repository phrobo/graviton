set(graviton-server_SRCS
    main.c
    )

find_package(PkgConfig REQUIRED)

pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(SOUP REQUIRED libsoup-2.4)
pkg_search_module(JSON REQUIRED json-glib-1.0)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${GLIB_INCLUDE_DIRS}
  ${SOUP_INCLUDE_DIRS}
  ${JSON_INCLUDE_DIRS}
)

add_executable(graviton-server ${graviton-server_SRCS})

target_link_libraries(graviton-server
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
)
