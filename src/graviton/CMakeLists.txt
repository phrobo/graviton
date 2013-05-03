set(libgraviton_SRCS
  control.c
  internal-plugin.c
  plugin.c
  plugin-manager.c
  server.c
)

set(libgraviton_HEADERS
  control.h
  plugin.h
  plugin-manager.h
  server.h
)

add_library(graviton SHARED ${libgraviton_SRCS})

target_link_libraries(graviton
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
  ${AVAHI_GLIB_LIBRARIES}
  ${AVAHI_CLIENT_LIBRARIES}
)

install(TARGETS graviton DESTINATION lib${LIB_SUFFIX})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/graviton.pc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/graviton.pc
  @ONLY
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/graviton.pc
  DESTINATION
    lib${LIB_SUFFIX}/pkgconfig
)

add_custom_command(
  OUTPUT
  ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_MAJOR}.gir
  COMMAND
    g-ir-scanner
      -n Graviton
      --library graviton
      --verbose
      --warn-all
      --pkg=glib-2.0
      -L${CMAKE_CURRENT_BINARY_DIR}
      plugin.c
      plugin.h
      control.h
      control.h
      -I${CMAKE_CURRENT_SOURCE_DIR}/../
      --no-libtool
      --nsversion=${GRAVITON_VERSION_MAJOR}
      --output=${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_MAJOR}.gir
  DEPENDS
    ${libgraviton_SRCS}
    ${libgraviton_HEADERS}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
  OUTPUT
  ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_MAJOR}.vapi
  COMMAND
    vapigen
      --library Graviton
      --pkg=glib-2.0
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_MAJOR}.gir
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_MAJOR}.gir
    graviton
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(
  do_gir
  DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_MAJOR}.gir
)

add_custom_target(
  do_vapi
  ALL
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_MAJOR}.vapi
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_MAJOR}.gir
  DESTINATION
    share/gir-1.0/
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/graviton-${GRAVITON_VERSION_MAJOR}.vapi
  DESTINATION
    share/vala/vapi/
)
