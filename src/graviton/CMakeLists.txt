# Streams are really all sorts of broken and buggy right now. Might be revisited
# in the future, but not yet.
option(GRAVITON_ENABLE_STREAMS "Enable streams API (Experimental)" FALSE)


add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
