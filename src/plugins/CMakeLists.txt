macro(add_graviton_plugin _name _source)
  add_library(graviton-plugin-${_name} MODULE ${_source})
  target_link_libraries(graviton-plugin-${_name}
    graviton-plugin
  )
  set_target_properties(graviton-plugin-${_name} PROPERTIES
    OUTPUT_NAME ${_name}
    PREFIX ""
  )
endmacro(add_graviton_plugin)

add_graviton_plugin(info info-plugin.c)
