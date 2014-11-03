find_package(GtkDoc)

if (GTKDOC_FOUND)

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/css/)

  file(
    COPY
      ${CMAKE_CURRENT_SOURCE_DIR}/img
      ${CMAKE_CURRENT_SOURCE_DIR}/fonts
    DESTINATION
      ${CMAKE_CURRENT_BINARY_DIR}/html/
  )

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/xml/book/)

  file(
    COPY
      ${CMAKE_CURRENT_SOURCE_DIR}/package.json
      ${CMAKE_CURRENT_SOURCE_DIR}/bower.json
    DESTINATION
      ${CMAKE_CURRENT_BINARY_DIR}
  )

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/bower_components/
    COMMAND
      bower install
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/bower.json
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
  )

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/node_modules/
    COMMAND
      npm install
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/package.json
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
  )

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/html/css/docs.css
    COMMAND
      ${CMAKE_CURRENT_BINARY_DIR}/node_modules/node-sass/bin/node-sass
      --include-path
      ${CMAKE_CURRENT_BINARY_DIR}/bower_components/foundation/scss
      --output-style compressed
      ${CMAKE_CURRENT_SOURCE_DIR}/scss/docs.scss
      ${CMAKE_CURRENT_BINARY_DIR}/html/css/docs.css
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/scss/docs.scss
      ${CMAKE_CURRENT_BINARY_DIR}/bower_components/
      ${CMAKE_CURRENT_BINARY_DIR}/node_modules/
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
      /usr/bin/xsltproc --path /usr/share/gtk-doc/data/ --nonet --xinclude
        --stringparam gtkdoc.bookname Graviton
        --stringparam gtkdoc.version "1.19"
        --stringparam chunk.quietly true
        --stringparam chunker.output.quiet true 
        ${CMAKE_CURRENT_SOURCE_DIR}/docbook/phrobo.xsl
        ${CMAKE_CURRENT_BINARY_DIR}/Graviton-docs.xml
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/Graviton-docs.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/docbook/phrobo.xsl
      ${CMAKE_CURRENT_BINARY_DIR}/xml/book/intro.xml
      ${CMAKE_CURRENT_BINARY_DIR}/xml/book/concepts.xml
      ${CMAKE_CURRENT_BINARY_DIR}/html/css/docs.css
      ${CMAKE_CURRENT_BINARY_DIR}/bower_components/
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/html/
  )

  add_custom_target(
    docs_html
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  )

endif()

