option(USE_AVAHI "Enable avahi discovery plugin" TRUE)

if (USE_AVAHI)
  pkg_search_module(AVAHI REQUIRED avahi-glib)
  pkg_search_module(AVAHI_CLIENT REQUIRED avahi-client)
  include_directories(${AVAHI_INCLUDE_DIRS})
  link_directories(${AVAHI_LIBRARY_DIRS})
  add_graviton_discovery_plugin(NAME graviton-discovery-avahi SOURCES avahi-discovery.c)
  install(TARGETS graviton-discovery-avahi DESTINATION
    ${GRAVITON_DISCOVERY_PLUGIN_PATH})
  target_link_libraries(graviton-discovery-avahi ${AVAHI_LIBRARIES}
    ${AVAHI_CLIENT_LIBRARIES})
endif()

add_graviton_discovery_plugin(NAME graviton-discovery-dbus SOURCES dbus-discovery.c)
install(TARGETS graviton-discovery-dbus DESTINATION
  ${GRAVITON_DISCOVERY_PLUGIN_PATH})

option(USE_GEEBEE "Enable GeeBee XBee discovery plugin" TRUE)

if (USE_GEEBEE)
  pkg_search_module(GEEBEE REQUIRED geebee-1.0)
  link_directories(${GEEBEE_LIBRARY_DIRS})
  include_directories(${GEEBEE_INCLUDE_DIRS})

  add_graviton_discovery_plugin(NAME graviton-discovery-xbee SOURCES xbee-discovery.c)
  install(TARGETS graviton-discovery-xbee DESTINATION
    ${GRAVITON_DISCOVERY_PLUGIN_PATH})

  target_link_libraries(graviton-discovery-xbee ${GEEBEE_LIBRARIES})
endif()
