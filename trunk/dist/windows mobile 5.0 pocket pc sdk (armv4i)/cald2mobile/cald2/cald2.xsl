<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/TR/xhtml1/strict"
                xmlns:p="Edi">

  <xsl:output method="html" encoding="UTF-16" />

  <xsl:variable name="root" select="/skshell/@root" />
  <xsl:variable name="isVGA" select="/skshell/@isVGA" />

  <xsl:template match="/">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="skshell">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="content">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="results">
    <xsl:choose>
      <xsl:when test="count(resultItem)=0">
        <p>Not Found</p>
      </xsl:when>
      <xsl:otherwise>
        <span class="listHeader">
          [<xsl:value-of select="@begin" />-<xsl:value-of select="@end" />]/<xsl:value-of select="@total" />
        </span>
        <span class="listBody">
          <table>
            <xsl:for-each select="resultItem[position() mod 2 = 1]">
              <tr>
                <td width="50%" align="left">
                  <xsl:apply-templates select="."/>
                </td>
                <td width="50%" align="left">
                  <xsl:apply-templates select="following-sibling::resultItem[position() = 1]"/>
                </td>
              </tr>
            </xsl:for-each>
          </table>
        </span>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="resultItem">
    <xsl:element name="a">
    	<xsl:attribute name="target">resultItem</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/entry/filesystem.cff!/@</xsl:text>
        <xsl:value-of select="./entryId" />
        <xsl:if test="./contextId">
          <xsl:text>#</xsl:text>
          <xsl:value-of select="./contextId" />
        </xsl:if>
        <xsl:text>']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:attribute name="tabindex">
        <xsl:value-of select="position()" />
      </xsl:attribute>
      <xsl:apply-templates select="./label" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="wordlist">
    <span class="listHeader">
      [<xsl:value-of select="@begin" />-<xsl:value-of select="@end" />]/<xsl:value-of select="@total" />
    </span>
    <span class="listBody">
      <table>
        <xsl:for-each select="wordListItem[position() mod 2 = 1]">
          <tr>
            <td width="50%" align="left">
              <xsl:apply-templates select="."/>
            </td>
            <td width="50%" align="left">
              <xsl:apply-templates select="following-sibling::wordListItem[position() = 1]"/>
            </td>
          </tr>
        </xsl:for-each>
      </table>
    </span>
  </xsl:template>

  <xsl:template match="wordListItem">
    <xsl:element name="a">
      <xsl:attribute name="id">
        <xsl:text>wordlist-</xsl:text>
        <xsl:value-of select="./entryId" />
      </xsl:attribute>
      <xsl:attribute name="name">
        <xsl:text>wordlist-</xsl:text>
        <xsl:value-of select="./entryId" />
      </xsl:attribute>
      <xsl:attribute name="target">wordListItem</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('clickWordList', ["</xsl:text>
        <xsl:value-of select="./label" />
        <xsl:text>"]);return false;</xsl:text>
      </xsl:attribute>
      <xsl:attribute name="tabindex">
        <xsl:value-of select="position()" />
      </xsl:attribute>
      <xsl:apply-templates select="./label"/>
    </xsl:element>
  </xsl:template>

  <xsl:template match="wordListItem/label/*">
    <span class="listItem">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="wordListItem/label/hw">
    <span class="wordlistHW">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:CALD2">
    <!-- <div style="background-color: #FFFFE7;"> -->
    <xsl:apply-templates />
    <!--	</div> -->
  </xsl:template>

  <xsl:template match="p:head">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="head">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:sensehead">
    <span class="sensehead">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:w">
    <span class="w">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:gw">
    <span class="gw">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:phrase">
    <span class="phrase">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:idiom">
    <span class="idiom">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:pv">
    <span class="pv">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:hwsense">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="hwsense">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:pvsense">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="pvsense">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:rosense">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="rosense">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:idsense">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="idsense">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:sense">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="sense">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:ro">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="ro">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:def">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="def">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:examp">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="examp">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:var">
    <span class="var">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:v">
    <xsl:element name="a">
      <xsl:attribute name="name">
        <xsl:value-of select="./@url" />
      </xsl:attribute>
    </xsl:element>
    <span class="v">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="label/p:v">
    <span class="v">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:b">
    <b>
      <xsl:apply-templates />
    </b>
  </xsl:template>

  <xsl:template match="p:i">
    <i>
      <xsl:apply-templates />
    </i>
  </xsl:template>

  <xsl:template match="p:gl">
    <span class="gl">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:pron">
    <span class="pron">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:unote">
    <span class="unote">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:pos">
    <span class="pos">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:type">
    <span class="type">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:gram">
    <span class="gram">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:rel">
    <span class="rel">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:vrel">
    <span class="vrel">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:region">
    <span class="region">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:usage">
    <span class="usage">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:xref">
    <span class="xref">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:xtext">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="p:x">
    <xsl:text> : </xsl:text>
    <xsl:element name="a">
    	<xsl:attribute name="target">px</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/entry/filesystem.cff!/@</xsl:text>
        <xsl:value-of select="./@entry" />
        <xsl:text>']);return false;</xsl:text>
      </xsl:attribute>
      <span class="x">
        <xsl:apply-templates />
      </span>
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:SemCat">
    <!-- don't support SmartThesaurus
  <xsl:element name="a">
    <xsl:attribute name="href">.</xsl:attribute>
    <xsl:attribute name="onclick">
      <xsl:text>sendCommand('SmartThesaurus', ['</xsl:text>
      <xsl:value-of select="./p:rel/@id" />
      <xsl:text>']);return false;</xsl:text>
    </xsl:attribute>
    <img src="{$root}/cald2/gif/ST.png" />
  </xsl:element>
-->
  </xsl:template>

  <xsl:template match="p:ExtraExamples">
    <xsl:element name="a">
    	<xsl:attribute name="target">ExtraExamples</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/extraexample/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./@id" />
        <xsl:text>.xml']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:element name="img">
        <xsl:attribute name="src">
          <xsl:value-of select="$root" />
          <xsl:text>/cald2/png/ExtEx.png</xsl:text>
        </xsl:attribute>
        <xsl:choose>
          <xsl:when test="1 = $isVGA">
            <xsl:attribute name="width">100</xsl:attribute>
            <xsl:attribute name="height">20</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="width">50</xsl:attribute>
            <xsl:attribute name="height">10</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:element>
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:WordBuilding">
    <xsl:element name="a">
    	<xsl:attribute name="target">WordBuilding</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:wbuild/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:element name="img">
        <xsl:attribute name="src">
          <xsl:value-of select="$root" />
          <xsl:text>/cald2/gif/WBuild.png</xsl:text>
        </xsl:attribute>
        <xsl:choose>
          <xsl:when test="1 = $isVGA">
            <xsl:attribute name="width">100</xsl:attribute>
            <xsl:attribute name="height">20</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="width">50</xsl:attribute>
            <xsl:attribute name="height">10</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:element>
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:Collocations">
    <xsl:element name="a">
    	<xsl:attribute name="target">Collocations</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:collpan/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:element name="img">
        <xsl:attribute name="src">
          <xsl:value-of select="$root" />
          <xsl:text>/cald2/gif/Colls.png</xsl:text>
        </xsl:attribute>
        <xsl:choose>
          <xsl:when test="1 = $isVGA">
            <xsl:attribute name="width">100</xsl:attribute>
            <xsl:attribute name="height">20</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="width">50</xsl:attribute>
            <xsl:attribute name="height">10</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:element>      
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:VerbInflections">
    <xsl:element name="a">
    	<xsl:attribute name="target">VerbInflections</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:vinf/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:element name="img">
        <xsl:attribute name="src">
          <xsl:value-of select="$root" />
          <xsl:text>/cald2/gif/VerbEnd.png</xsl:text>
        </xsl:attribute>
        <xsl:choose>
          <xsl:when test="1 = $isVGA">
            <xsl:attribute name="width">100</xsl:attribute>
            <xsl:attribute name="height">20</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="width">50</xsl:attribute>
            <xsl:attribute name="height">10</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:element>
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:VerbInflectionsNew">
    <xsl:element name="a">
    	<xsl:attribute name="target">VerbInflectionsNew</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:vinf/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:element name="img">
        <xsl:attribute name="src">
          <xsl:value-of select="$root" />
          <xsl:text>/cald2/gif/VerbEnd.png</xsl:text>
        </xsl:attribute>
        <xsl:choose>
          <xsl:when test="1 = $isVGA">
            <xsl:attribute name="width">100</xsl:attribute>
            <xsl:attribute name="height">20</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="width">50</xsl:attribute>
            <xsl:attribute name="height">10</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:element>
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:CommonLearnerError">
    <xsl:element name="a">
    	<xsl:attribute name="target">CommonLearnerError</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:clepan/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:element name="img">
        <xsl:attribute name="src">
          <xsl:value-of select="$root" />
          <xsl:text>/cald2/gif/CLE.png</xsl:text>
        </xsl:attribute>
        <xsl:choose>
          <xsl:when test="1 = $isVGA">
            <xsl:attribute name="width">160</xsl:attribute>
            <xsl:attribute name="height">20</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="width">80</xsl:attribute>
            <xsl:attribute name="height">10</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:element>
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:UsageNote">
    <xsl:element name="a">
    	<xsl:attribute name="target">UsageNote</xsl:attribute>
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:upan/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:element name="img">
        <xsl:attribute name="src">
          <xsl:value-of select="$root" />
          <xsl:text>/cald2/gif/UseNote.png</xsl:text>
        </xsl:attribute>
        <xsl:choose>
          <xsl:when test="1 = $isVGA">
            <xsl:attribute name="width">100</xsl:attribute>
            <xsl:attribute name="height">20</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="width">50</xsl:attribute>
            <xsl:attribute name="height">10</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:element>      
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:PictureReferences">
    <!-- don't support PictureReferences  
	<span class="PictureReferences">
		<xsl:apply-templates />
	</span>
-->
  </xsl:template>

  <xsl:template match="p:picref">
    <span class="picref">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:lab">
    <span class="lab">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:infgrp">
    <span class="infgrp">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:inf">
    <span class="inf">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:pron[@class='ukpron']">
    <span class="ukpron">
      <xsl:apply-templates />
    </span>
  </xsl:template>

  <xsl:template match="p:pron[@class='uspron']">
    <span class="uspron">
      <xsl:apply-templates />
    </span>
  </xsl:template>





  <xsl:template match="span[@class='E']">
    <xsl:element name="img">
      <xsl:attribute name="src">
        <xsl:value-of select="$root" />
        <xsl:text>/cald2/png/E.png</xsl:text>
      </xsl:attribute>
      <xsl:attribute name="align">absmiddle</xsl:attribute>
      <xsl:attribute name="style">margin: 3px;</xsl:attribute>
      <xsl:choose>
        <xsl:when test="1 = $isVGA">
          <xsl:attribute name="width">25</xsl:attribute>
          <xsl:attribute name="height">15</xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="width">12</xsl:attribute>
          <xsl:attribute name="height">8</xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:element>
  </xsl:template>

  <xsl:template match="span[@class='I']">
    <xsl:element name="img">
      <xsl:attribute name="src">
        <xsl:value-of select="$root" />
        <xsl:text>/cald2/png/I.png</xsl:text>
      </xsl:attribute>
      <xsl:attribute name="align">absmiddle</xsl:attribute>
      <xsl:attribute name="style">margin: 3px;</xsl:attribute>
      <xsl:choose>
        <xsl:when test="1 = $isVGA">
          <xsl:attribute name="width">25</xsl:attribute>
          <xsl:attribute name="height">15</xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="width">12</xsl:attribute>
          <xsl:attribute name="height">8</xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:element>
  </xsl:template>

  <xsl:template match="span[@class='A']">
    <xsl:element name="img">
      <xsl:attribute name="src">
        <xsl:value-of select="$root" />
        <xsl:text>/cald2/png/A.png</xsl:text>
      </xsl:attribute>
      <xsl:attribute name="align">absmiddle</xsl:attribute>
      <xsl:attribute name="style">margin: 3px;</xsl:attribute>
      <xsl:choose>
        <xsl:when test="1 = $isVGA">
          <xsl:attribute name="width">25</xsl:attribute>
          <xsl:attribute name="height">15</xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="width">12</xsl:attribute>
          <xsl:attribute name="height">8</xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:element>
  </xsl:template>

  <xsl:template match="span[@class='US_pron.bmp']">
    <xsl:element name="img">
      <xsl:attribute name="src">
        <xsl:value-of select="$root" />
        <xsl:text>/cald2/png/us.png</xsl:text>
      </xsl:attribute>
      <xsl:attribute name="align">absmiddle</xsl:attribute>
      <xsl:choose>
        <xsl:when test="1 = $isVGA">
          <xsl:attribute name="width">15</xsl:attribute>
          <xsl:attribute name="height">13</xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="width">8</xsl:attribute>
          <xsl:attribute name="height">7</xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:element>    
  </xsl:template>

  <xsl:template match="span[@class='pronus']">
    <xsl:element name="a">
      <xsl:attribute name="class">pronus</xsl:attribute>
      <xsl:attribute name="href">
        <xsl:text>#</xsl:text>
        <xsl:value-of select="ancestor-or-self::*[@url][1]/@url" />
      </xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('playSound', ['sk://fs/2.0/data/pronus/filesystem.cff!/</xsl:text>
        <xsl:value-of select="@path" />
        <xsl:text>']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:element name="img">
        <xsl:attribute name="src">
          <xsl:value-of select="$root" />
          <xsl:text>/cald2/gif/entry/pronus.gif</xsl:text>
        </xsl:attribute>
        <xsl:choose>
          <xsl:when test="1 = $isVGA">
            <xsl:attribute name="width">42</xsl:attribute>
            <xsl:attribute name="height">17</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="width">21</xsl:attribute>
            <xsl:attribute name="height">8</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:element>
    </xsl:element>
  </xsl:template>

  <xsl:template match="span[@class='pronuk']">
    <xsl:element name="a">
      <xsl:attribute name="class">pronuk</xsl:attribute>
      <xsl:attribute name="href">
        <xsl:text>#</xsl:text>
        <xsl:value-of select="ancestor-or-self::*[@url][1]/@url" />
      </xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('playSound', ['sk://fs/2.0/data/pronuk/filesystem.cff!/</xsl:text>
        <xsl:value-of select="@path" />
        <xsl:text>']);return false;</xsl:text>
      </xsl:attribute>
      <xsl:element name="img">
        <xsl:attribute name="src">
          <xsl:value-of select="$root" />
          <xsl:text>/cald2/gif/entry/pronuk.gif</xsl:text>
        </xsl:attribute>
        <xsl:choose>
          <xsl:when test="1 = $isVGA">
            <xsl:attribute name="width">42</xsl:attribute>
            <xsl:attribute name="height">17</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="width">21</xsl:attribute>
            <xsl:attribute name="height">8</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:element>
    </xsl:element>
  </xsl:template>

  <xsl:template match="span[@class='rec']">
    <!-- don't support rec 
	<img src="{$root}/cald2/gif/entry/micro.gif" align="absmiddle" style="margin: 2px;cursor: pointer;" onclick="alert('rec');" />
-->
  </xsl:template>

  <xsl:template match="span[@class='atlink']">
    <!-- copy it -->
    <span class="atlink">
      <xsl:apply-templates />
      <xsl:text> </xsl:text>
    </span>
  </xsl:template>

  <xsl:template match="span[@class='numsense']">
    <span class="numsense">
      <xsl:apply-templates />
      <xsl:text>. </xsl:text>
    </span>
  </xsl:template>

</xsl:stylesheet>
