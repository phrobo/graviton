set(GRAVITON_VERSION
  "${GRAVITON_VERSION_MAJOR}.${GRAVITON_VERSION_MINOR}.${GRAVITON_VERSION_PATCH}")

macro(add_graviton_plugin _name _source)
  add_library(graviton-plugin-${_name} MODULE ${_source})
  string(REPLACE ";" " " _LDFLAGS "${GRAVITON_LDFLAGS};${GRAVITON_LDFLAGS_OTHER}")
  set_target_properties(graviton-plugin-${_name} PROPERTIES
    OUTPUT_NAME ${_name}
    PREFIX ""
    LINK_FLAGS "${_LDFLAGS}"
  )
endmacro(add_graviton_plugin)
