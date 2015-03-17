find_package(GtkDoc)

macro(make_xml prefix)
  set(_args "${ARGN}")
  foreach(_xml ${_args})
    set(_v "${prefix}_DOCS_XML")
    list(APPEND ${_v} "<xi:include href=\"${_xml}\" />")
  endforeach()
endmacro()

if (GTKDOC_FOUND)

  message(STATUS "gtk-goc found. Documentation will be built.")

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-docs.xml
    COMMAND
      ${GTKDOC_MKDB_EXE}
        --module=Graviton
        --output-format=xml
        --xml-mode
        --source-dir=${CMAKE_SOURCE_DIR}/src/graviton/
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-decl.txt
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
  )

  make_xml(client
    xml/cloud.xml
    xml/discovery-method.xml
    xml/node.xml
    xml/node-transport.xml
    xml/service-interface.xml
  )

  make_xml(server
    xml/node-browser.xml
    xml/root-service.xml
    xml/server.xml
    xml/service.xml
  )

  make_xml(introspection
    xml/introspection-interface.xml
    xml/introspection-service.xml
  )

  make_xml(transports
    xml/jsonrpc-node-transport.xml
  )

  if (GRAVITON_ENABLE_STREAMS)
    make_xml(client_streams
      xml/file-stream.xml
      xml/node-input-stream.xml
      xml/node-io-stream.xml
      xml/node-stream.xml
      xml/stream.xml
    )
  endif()

  foreach(_xml IN LISTS client_DOCS)
    set(client_DOCS_XML "<xi:include href=\"${_xml}\" />")
  endforeach()


  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Graviton-docs.xml.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Graviton-docs.xml
    @ONLY
  )

#FIXME: Make this a macro!
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/xml/book/intro.xml
    COMMAND
      pandoc
        -t docbook
        -o ${CMAKE_CURRENT_BINARY_DIR}/xml/book/intro.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/book/intro.md
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/book/intro.md
  )

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/xml/book/concepts.xml
    COMMAND
      pandoc
        -t docbook
        -o ${CMAKE_CURRENT_BINARY_DIR}/xml/book/concepts.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/book/concepts.md
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/book/concepts.md
  )

  file(
    COPY
      ${CMAKE_CURRENT_SOURCE_DIR}/xml/
    DESTINATION
      ${CMAKE_CURRENT_BINARY_DIR}/
  )

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-decl.txt
    COMMAND
      ${GTKDOC_SCAN_EXE}
        --module=Graviton
        --source-dir=${CMAKE_SOURCE_DIR}/src/graviton/
        --rebuild-types
    DEPENDS
      ${libgraviton_SRCS}
      ${libgraviton_HEADERS}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
  )

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    COMMAND
      ${GTKDOC_MKHTML_EXE}
        Graviton
        ${CMAKE_CURRENT_BINARY_DIR}/Graviton-docs.xml
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-docs.xml
      ${CMAKE_CURRENT_BINARY_DIR}/xml/book/intro.xml
      ${CMAKE_CURRENT_BINARY_DIR}/xml/book/concepts.xml
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/html/
  )

  add_custom_target(
    docs_html
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  )

  add_custom_target(
    docs
    DEPENDS
      docs_html
  )

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/xml/book/)

else()
  message(WARNING "gtk-goc is missing. Documentation will not be built.")
endif()

