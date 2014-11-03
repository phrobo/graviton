pkg_search_module(AVAHI_GLIB REQUIRED avahi-glib)
pkg_search_module(AVAHI_CLIENT REQUIRED avahi-client)
include_directories(
  ${AVAHI_GLIB_INCLUDE_DIRS}
  ${AVAHI_CLIENT_INCLUDE_DIRS}
)
add_graviton_publish_plugin(NAME avahi-publish SOURCES avahi-publish.c)
install(TARGETS avahi-publish DESTINATION
  ${GRAVITON_PUBLISH_PLUGIN_PATH})
target_link_libraries(avahi-publish avahi-client avahi-glib)

add_graviton_publish_plugin(NAME dbus-publish SOURCES dbus-publish.c
  ${CMAKE_CURRENT_BINARY_DIR}/server-interface.c)
install(TARGETS dbus-publish DESTINATION
  ${GRAVITON_PUBLISH_PLUGIN_PATH})

pkg_search_module(GUPNP_IGD REQUIRED gupnp-igd-1.0)
add_graviton_publish_plugin(NAME spitzer-publish SOURCES spitzer-publish.c)
install(TARGETS spitzer-publish DESTINATION
  ${GRAVITON_PUBLISH_PLUGIN_PATH})

target_link_libraries(spitzer-publish
  ${GUPNP_IGD_LIBRARIES})

include_directories(
  ${GUPNP_IGD_INCLUDE_DIRS}
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

