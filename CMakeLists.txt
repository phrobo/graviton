cmake_minimum_required(VERSION 2.8)
project(graviton)

set(GRAVITON_VERSION_MAJOR 0)
set(GRAVITON_VERSION_MINOR 0)
set(GRAVITON_VERSION_PATCH 1)
set(GRAVITON_VERSION_PLATFORM
  "1.0")
set(GRAVITON_PLUGIN_PATH "${CMAKE_INSTALL_PREFIX}/share/graviton/plugins/")
set(GRAVITON_DISCOVERY_PLUGIN_PATH
  "${CMAKE_INSTALL_PREFIX}/share/graviton/discovery-plugins/")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GravitonInternal)

set(GRAVITON_LIBRARIES "graviton")

if (NOT LIB_SUFFIX)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(LIB_SUFFIX "")
  else()
    set(LIB_SUFFIX "64")
  endif()
endif()

add_subdirectory(src)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/GravitonConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/GravitonConfig.cmake
  @ONLY
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/GravitonConfig.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GravitonInternal.cmake
  DESTINATION
    lib${LIB_SUFFIX}/cmake/Graviton/
)

install(
  FILES
    graviton.service
  DESTINATION
    lib/systemd/user/
)
