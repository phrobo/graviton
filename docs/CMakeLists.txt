find_package(GtkDoc)

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

