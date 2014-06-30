set(libgraviton-common_SRCS
  configuration.c
)

set(libgraviton-common_HEADERS
  configuration.h
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(graviton-common SHARED ${libgraviton-common_SRCS})

target_link_libraries(graviton-common
  ${GLIB_LIBRARIES}
  ${UUID_LIBRARIES}
)

install(TARGETS graviton-common DESTINATION lib${LIB_SUFFIX})

install(
  FILES
    ${libgraviton-common_HEADERS}
  DESTINATION
    include/graviton-common-${GRAVITON_VERSION_PLATFORM}/graviton/common/
)

