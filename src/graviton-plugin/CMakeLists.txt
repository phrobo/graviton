set(libgraviton-plugin_SRCS
  plugin.c
  plugin-manager.c
  control.c
)

add_library(graviton-plugin SHARED ${libgraviton-plugin_SRCS})

target_link_libraries(graviton-plugin
  ${GLIB_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${JSON_LIBRARIES}
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/graviton-plugin.pc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/graviton-plugin.pc
  @ONLY
)

install(
  TARGETS
    graviton-plugin
  DESTINATION
    lib${LIB_SUFFIX}
)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/graviton-plugin.pc
  DESTINATION
    lib${LIB_SUFFIX}/pkgconfig
)
