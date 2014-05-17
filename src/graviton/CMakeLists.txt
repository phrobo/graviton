set(libgraviton_SRCS
  cloud.c
  service.c
  root-service.c
  discovery-method.c
  file-stream.c
  introspection-interface.c
  node.c
  service-interface.c
  node-io-stream.c
  node-input-stream.c
  node-stream.c
  server.c
  stream.c
  introspection-service.c
  ${CMAKE_CURRENT_BINARY_DIR}/server-interface.c
)

set(libgraviton_HEADERS
  cloud.h
  service.h
  discovery-method.h
  file-stream.h
  introspection-interface.h
  node.h
  service-interface.h
  node-input-stream.h
  node-io-stream.h
  node-stream.h
  root-service.h
  server.h
  stream.h
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/server-interface.c
    ${CMAKE_CURRENT_BINARY_DIR}/server-interface.h
  COMMAND
    gdbus-codegen 
      --generate-c-code server-interface
      --c-namespace GravitonDBus
      --interface-prefix org.aether.graviton
      ${CMAKE_CURRENT_SOURCE_DIR}/org.aether.graviton.Server.xml
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/org.aether.graviton.Server.xml
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(graviton SHARED ${libgraviton_SRCS})

target_link_libraries(graviton
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
  ${GIO_LIBRARIES}
  ${GIO_UNIX_LIBRARIES}
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
        service.h
        service.h
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
