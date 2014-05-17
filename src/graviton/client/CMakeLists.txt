set(libgraviton-client_SRCS
  cloud.c
  discovery-method.c
  introspection-interface.c
  node.c
  service-interface.c
  node-io-stream.c
  node-input-stream.c
  node-stream.c
)

set(libgraviton-client_HEADERS
  cloud.h
  discovery-method.h
  introspection-interface.h
  node.h
  service-interface.h
  node-input-stream.h
  node-io-stream.h
  node-stream.h
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(graviton-client SHARED ${libgraviton-client_SRCS})

target_link_libraries(graviton-client
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
  ${UUID_LIBRARIES}
)

install(TARGETS graviton-client DESTINATION lib${LIB_SUFFIX})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/graviton-client.pc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/graviton-client-${GRAVITON_VERSION_PLATFORM}.pc
  @ONLY
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/graviton-client-${GRAVITON_VERSION_PLATFORM}.pc
  DESTINATION
    lib${LIB_SUFFIX}/pkgconfig
)

install(
  FILES
    ${libgraviton-client_HEADERS}
  DESTINATION
    include/graviton-client-${GRAVITON_VERSION_PLATFORM}/graviton/client/
)
