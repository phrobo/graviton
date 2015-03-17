<?xml version="1.0"?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.3//EN"
               "http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd"
[
  <!ENTITY % local.common.attrib "xmlns:xi  CDATA  #FIXED 'http://www.w3.org/2003/XInclude'">
]>
<book id="index">
  <bookinfo>
    <title>Graviton Reference Manual</title>
    <releaseinfo>
    for Graviton @GRAVITON_VERSION_MAJOR@.@GRAVITON_VERSION_MINOR@.@GRAVITON_VERSION_PATCH@
      The latest version of this documentation can be found on-line at
      <ulink role="online-location" url="http://docs.phrobo.net/graviton/index.html">http://docs.phrobo.net/graviton/latest/</ulink>.
    </releaseinfo>
  </bookinfo>

  <chapter>
    <title>The Internet of Things</title>
    <xi:include href="xml/book/intro.xml"/>
    <xi:include href="xml/book/concepts.xml"/>
  </chapter>
  <chapter>
  <title>Graviton API</title>
    <section>
      <title>Server</title>
      @server_DOCS_XML@
    </section>
    <section>
      <title>Client</title>
      @client_DOCS_XML@
    </section>
    <section>
      <title>Introspection</title>
      @introspection_DOCS_XML@
    </section>
    <section>
      <title>Transports</title>
      @transports_DOCS_XML@
    </section>
  </chapter>
  <index id="api-index-full">
    <title>API Index</title>
    <xi:include href="xml/api-index-full.xml"><xi:fallback /></xi:include>
  </index>
  <index id="deprecated-api-index" role="deprecated">
    <title>Index of deprecated API</title>
    <xi:include href="xml/api-index-deprecated.xml"><xi:fallback /></xi:include>
  </index>

  <xi:include href="xml/annotation-glossary.xml"><xi:fallback /></xi:include>
</book>
