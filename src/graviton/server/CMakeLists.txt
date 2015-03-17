set(libgraviton-server_SRCS
  service.c
  root-service.c
  server-publish-method.c
  server.c
  introspection-service.c
  quickserver.c
)

set(stream_SRCS
  file-stream.c
  static-stream.c
  stream.c
)

set(libgraviton-server_HEADERS
  service.h
  root-service.h
  server.h
  server-publish-method.h
  quickserver.h
)

set(stream_HEADERS
  file-stream.h
  static-stream.h
  stream.h
)

if (GRAVITON_ENABLE_STREAMS)
  message(STATUS "Streams API enabled in server.")
  list(libgraviton-server_SRCS ${stream_SRCS})
  list(libgraviton-server_HEADERS ${stream_HEADERS})
endif()

include_directories(
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

