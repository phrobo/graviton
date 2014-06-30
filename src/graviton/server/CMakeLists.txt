set(libgraviton-server_SRCS
  service.c
  root-service.c
  file-stream.c
  server.c
  static-stream.c
  stream.c
  introspection-service.c
  ${CMAKE_CURRENT_BINARY_DIR}/server-interface.c
)

set(libgraviton-server_HEADERS
  service.h
  file-stream.h
  root-service.h
  server.h
  static-stream.h
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

add_library(graviton-server SHARED ${libgraviton-server_SRCS})

target_link_libraries(graviton-server
  graviton-common
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
  ${GIO_LIBRARIES}
  ${GIO_UNIX_LIBRARIES}
  ${AVAHI_GLIB_LIBRARIES}
  ${AVAHI_CLIENT_LIBRARIES}
  ${UUID_LIBRARIES}
)

install(TARGETS graviton-server DESTINATION lib${LIB_SUFFIX})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/graviton-server.pc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/graviton-server-${GRAVITON_VERSION_PLATFORM}.pc
  @ONLY
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/graviton-server-${GRAVITON_VERSION_PLATFORM}.pc
  DESTINATION
    lib${LIB_SUFFIX}/pkgconfig
)

install(
  FILES
    ${libgraviton-server_HEADERS}
  DESTINATION
    include/graviton-server-${GRAVITON_VERSION_PLATFORM}/graviton/server/
)

