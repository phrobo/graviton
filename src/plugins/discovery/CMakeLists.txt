option(USE_AVAHI "Enable avahi discovery plugin" TRUE)

if (USE_AVAHI)
  add_graviton_discovery_plugin(NAME graviton-discovery-avahi SOURCES avahi-discovery.c)
  install(TARGETS graviton-discovery-avahi DESTINATION
    ${GRAVITON_DISCOVERY_PLUGIN_PATH})
  target_link_libraries(graviton-discovery-avahi avahi-client avahi-glib)
endif()

add_graviton_discovery_plugin(NAME graviton-discovery-dbus SOURCES dbus-discovery.c)
install(TARGETS graviton-discovery-dbus DESTINATION
  ${GRAVITON_DISCOVERY_PLUGIN_PATH})
