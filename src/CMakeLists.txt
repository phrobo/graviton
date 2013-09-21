find_package(PkgConfig REQUIRED)

pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(SOUP REQUIRED libsoup-2.4)
pkg_search_module(JSON REQUIRED json-glib-1.0)
pkg_search_module(AVAHI_GLIB REQUIRED avahi-glib)
pkg_search_module(AVAHI_CLIENT REQUIRED avahi-client)
pkg_search_module(UUID REQUIRED uuid)

exec_program(
  pkg-config 
  ARGS
    --variable=vapigen vapigen
  OUTPUT_VARIABLE
    VAPIGEN
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLIB_INCLUDE_DIRS}
  ${SOUP_INCLUDE_DIRS}
  ${JSON_INCLUDE_DIRS}
  ${AVAHI_GLIB_INCLUDE_DIRS}
  ${AVAHI_CLIENT_INCLUDE_DIRS}
  ${UUID_INCLUDE_DIRS}
)

add_definitions (-DHAVE_CONFIG_H)

add_subdirectory(graviton)
add_subdirectory(graviton-ping-server)
add_subdirectory(graviton-ping-client)
#add_subdirectory(plugins)
add_subdirectory(graviton-cli)
