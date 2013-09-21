set(libgraviton_SRCS
  cloud.c
  control.c
  root-control.c
  discovery-method.c
  file-stream.c
  introspection-control.c
  node.c
  service.c
  node-io-stream.c
  node-input-stream.c
  node-stream.c
  server.c
  stream.c
  internal-plugin.c
)

set(libgraviton_HEADERS
  cloud.h
  control.h
  discovery-method.h
  file-stream.h
  introspection-control.h
  node.h
  service.h
  node-input-stream.h
  node-io-stream.h
  node-stream.h
  root-control.h
  server.h
  stream.h
)

add_library(graviton SHARED ${libgraviton_SRCS})

target_link_libraries(graviton
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
  ${AVAHI_GLIB_LIBRARIES}
  ${AVAHI_CLIENT_LIBRARIES}
  ${UUID_LIBRARIES}
)

install(TARGETS graviton DESTINATION lib${LIB_SUFFIX})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/graviton.pc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/graviton-${GRAVITON_VERSION_PLATFORM}.pc
  @ONLY
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/graviton-${GRAVITON_VERSION_PLATFORM}.pc
  DESTINATION
    lib${LIB_SUFFIX}/pkgconfig
)

option(BUILD_G_BINDINGS "Build experimental g-i-r and vala bindings" FALSE)

if (BUILD_G_BINDINGS)
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_PLATFORM}.gir
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
        --nsversion=${GRAVITON_VERSION_PLATFORM}
        --output=${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_PLATFORM}.gir
    DEPENDS
      ${libgraviton_SRCS}
      ${libgraviton_HEADERS}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_PLATFORM}.vapi
    COMMAND
      ${VAPIGEN}
      --library Graviton-${GRAVITON_VERSION_PLATFORM}
        --pkg=glib-2.0
        ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_PLATFORM}.gir
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_PLATFORM}.gir
      graviton
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
  )

  add_custom_target(
    do_gir
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_PLATFORM}.gir
  )

  add_custom_target(
    do_vapi
    ALL
    #DEPENDS
    #    ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_PLATFORM}.vapi
  )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_PLATFORM}.gir
    DESTINATION
      share/gir-1.0/
  )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-${GRAVITON_VERSION_PLATFORM}.vapi
    DESTINATION
      share/vala/vapi/
  )
endif()

install(
  FILES
    ${libgraviton_HEADERS}
  DESTINATION
    include/graviton-${GRAVITON_VERSION_PLATFORM}/graviton/
)
