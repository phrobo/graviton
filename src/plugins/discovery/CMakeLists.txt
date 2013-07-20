option(USE_AVAHI "Enable avahi discovery plugin" TRUE)

if (USE_AVAHI)
  add_graviton_discovery_plugin(NAME avahi SOURCES avahi-discovery.c)
  install(TARGETS graviton-discovery-plugin-avahi DESTINATION
    ${GRAVITON_DISCOVERY_PLUGIN_PATH})
endif()
