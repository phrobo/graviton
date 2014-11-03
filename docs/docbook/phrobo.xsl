<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:l="http://docbook.sourceforge.net/xmlns/l10n/1.0"
                exclude-result-prefixes="l"
                version="1.0">
  <xsl:import href="gtk-doc.xsl"/>
  <xsl:param name="admon.graphics" select="'1'"/>

  <xsl:param name="chunker.output.doctype-public" select="''"/>

  <xsl:template name="body.attributes">
  </xsl:template>

  <xsl:template name="user.preroot">
    <xsl:text disable-output-escaping="yes">&lt;!DOCTYPE html&gt;
</xsl:text>
  </xsl:template>

  <xsl:template name="user.head.content">
    <link rel="stylesheet" href="/css/docs.css" type="text/css"/>
  </xsl:template>

  <xsl:template name="user.header.content">
    <nav class="top-bar" data-topbar="1">
      <ul class="title-area">
        <li class="name">
          <h1><a href="#">Graviton</a></h1>
        </li>
      </ul>
    </nav>
  </xsl:template>

  <xsl:template name="user.footer.content">
    <footer>
      <div class="row">
        <div class="small-9 columns">
          <a href="http://phrobo.net/">Phong Robotics</a> - We make tools for hackers.
          <p>©2014 Phong Robotics Corporation. Released under a CC-BY license.</p>
          <p>Made with <span style="color:#f00">❤</span> in Akron, Ohio</p>
        </div>
      </div>
      <div class="small-3 columns">
        <p><script data-gittip-username="PhongRobotics" src="//gttp.co/v1.js"></script></p>
      </div>
    </footer>
  </xsl:template>

  <xsl:template match="title" mode="book.titlepage.recto.mode">
    <div class="row">
      <div class="large-12 columns">
        <h2><xsl:value-of select="."/></h2>
      </div>
    </div>
  </xsl:template>

  <xsl:template match="title" mode="sect1.titlepage.recto.mode">
    <div class="row">
      <div class="large-12 columns">
        <h2><xsl:value-of select="."/></h2>
      </div>
    </div>
  </xsl:template>

  <xsl:template match="title" mode="chapter.titlepage.recto.mode">
    <div class="row">
      <div class="large-12 columns">
        <h2><xsl:value-of select="."/></h2>
      </div>
    </div>
  </xsl:template>
  
  <xsl:template match="*" mode="chapter.titlepage.recto.mode">
    <div class="row">
      <div class="large-12 columns">
        <h2><xsl:value-of select="."/></h2>
      </div>
    </div>
  </xsl:template>

  <xsl:template match="sect1">
    <div class="row">
      <div class="large-12 columns">
        <p><xsl:value-of select="."/></p>
      </div>
    </div>
  </xsl:template>

  <xsl:template match="title" mode="section.titlepage.recto.mode">
    <div class="row">
      <div class="large-12 columns">
        <p><xsl:apply-templates mode="titlepage.mode"/></p>
      </div>
    </div>
  </xsl:template>

  <xsl:template match="releaseinfo" mode="titlepage.mode">
    <div class="row">
      <div class="large-12 columns">
        <p><xsl:apply-templates mode="titlepage.mode"/></p>
      </div>
    </div>
  </xsl:template>

  <xsl:template name="division.toc" mode="toc">
    <xsl:param name="toc-context" select="."/>
    <xsl:param name="toc.title.p" select="true()"/>
    <div class="row">
      <div class="large-12 columns">

        <xsl:call-template name="make.toc">
          <xsl:with-param name="toc-context" select="$toc-context"/>
          <xsl:with-param name="toc.title.p" select="$toc.title.p"/>
          <xsl:with-param name="nodes" select="part|reference
                                               |preface|chapter|appendix
                                               |article
                                               |topic
                                               |bibliography|glossary|index
                                               |refentry
                                               |bridgehead[$bridgehead.in.toc != 0]"/>

        </xsl:call-template>
      </div>
    </div>
  </xsl:template>

</xsl:stylesheet>
