add_subdirectory(ping)
add_subdirectory(mpd-control)
add_subdirectory(clock)

if (GRAVITON_ENABLE_STREAMS)
  add_subdirectory(stream)
endif()
