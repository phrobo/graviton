set(libgraviton-plugin_SRCS
  plugin.c
  plugin-manager.c
)
set(graviton-server_SRCS
  main.c
  info-plugin.c
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
add_library(graviton-plugin SHARED ${libgraviton-plugin_SRCS})

target_link_libraries(graviton-plugin
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
)

target_link_libraries(graviton-server
  graviton-plugin
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
)

install(TARGETS graviton-server DESTINATION bin)
