set(libgraviton-client_SRCS
  cloud.c
  discovery-method.c
  introspection-interface.c
  node.c
  service-interface.c
  node-transport.c
  jsonrpc-node-transport.c
  node-browser.c
)

set(stream_SRCS
  jsonrpc-io-stream.c
  jsonrpc-input-stream.c
  node-stream.c
)

set(libgraviton-client_HEADERS
  cloud.h
  discovery-method.h
  introspection-interface.h
  node.h
  service-interface.h
  node-transport.h
  jsonrpc-node-transport.h
  node-browser.h
)

set(stream_HEADERS
  jsonrpc-input-stream.h
  jsonrpc-io-stream.h
  node-stream.h
)

if (GRAVITON_ENABLE_STREAMS)
  message(STATUS "Streams API enabled in client.")
  list(APPEND libgraviton-client_SRCS ${stream_SRCS})
  list(APPEND libgraviton-client_HEADERS ${stream_HEADERS})
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(graviton-client SHARED ${libgraviton-client_SRCS})

target_link_libraries(graviton-client
  graviton-common
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
