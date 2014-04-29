find_package(GtkDoc)

if (GTKDOC_FOUND)
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
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    COMMAND
      ${GTKDOC_MKHTML_EXE}
        Graviton
        ${CMAKE_CURRENT_BINARY_DIR}/Graviton-docs.xml
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-docs.xml
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/html/
  )
  add_custom_target(
    docs_html
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  )

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/)

endif()

