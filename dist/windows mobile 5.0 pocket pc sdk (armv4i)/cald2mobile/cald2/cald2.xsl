<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/TR/xhtml1/strict"
                xmlns:p="Edi">

  <xsl:output method="html" encoding="UTF-16" />

  <xsl:variable name="root" select="/skshell/@root" />

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
        <br/>
        <xsl:apply-templates />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="resultItem">
    <xsl:element name="a">
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
      <xsl:apply-templates select="./label" />
    </xsl:element>
    <br/>
  </xsl:template>

  <xsl:template match="wordlist">
    <span class="listHeader">
      [<xsl:value-of select="@begin" />-<xsl:value-of select="@end" />]/<xsl:value-of select="@total" />
    </span>
    <br/>
    <xsl:apply-templates />
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
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('clickWordList', ["</xsl:text>
        <xsl:value-of select="./label" />
        <xsl:text>"]);return false;</xsl:text>
      </xsl:attribute>
      <xsl:apply-templates select="./label"/>
    </xsl:element>
    <br/>
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
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/extraexample/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./@id" />
        <xsl:text>.xml']);return false;</xsl:text>
      </xsl:attribute>
      <img src="{$root}/cald2/png/ExtEx.png" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:WordBuilding">
    <xsl:element name="a">
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:wbuild/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <img src="{$root}/cald2/gif/WBuild.png" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:Collocations">
    <xsl:element name="a">
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:collpan/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <img src="{$root}/cald2/gif/Colls.png" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:VerbInflections">
    <xsl:element name="a">
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:vinf/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <img src="{$root}/cald2/gif/VerbEnd.png" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:VerbInflectionsNew">
    <xsl:element name="a">
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:vinf/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <img src="{$root}/cald2/gif/VerbEnd.png" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:CommonLearnerError">
    <xsl:element name="a">
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:clepan/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <img src="{$root}/cald2/gif/CLE.png" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="p:UsageNote">
    <xsl:element name="a">
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('setContentTab', ['sk://fs/2.0/data/htmlpanel/filesystem.cff!/</xsl:text>
        <xsl:value-of select="./p:upan/@url" />
        <xsl:text>.HTM']);return false;</xsl:text>
      </xsl:attribute>
      <img src="{$root}/cald2/gif/UseNote.png" />
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
    <img src="{$root}/cald2/png/E.png" align="absmiddle" style="margin: 3px;" />
  </xsl:template>

  <xsl:template match="span[@class='I']">
    <img src="{$root}/cald2/png/I.png" align="absmiddle" style="margin: 3px;" />
  </xsl:template>

  <xsl:template match="span[@class='A']">
    <img src="{$root}/cald2/png/A.png" align="absmiddle" style="margin: 3px;" />
  </xsl:template>

  <xsl:template match="span[@class='US_pron.bmp']">
    <img src="{$root}/cald2/png/us.png" align="absmiddle" />
  </xsl:template>

  <xsl:template match="span[@class='pronus']">
    <xsl:element name="a">
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('playSound', ['sk://fs/2.0/data/pronus/filesystem.cff!/</xsl:text>
        <xsl:value-of select="@path" />
        <xsl:text>']);return false;</xsl:text>
      </xsl:attribute>
      <img src="{$root}/cald2/gif/entry/pronus.gif" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="span[@class='pronuk']">
    <xsl:element name="a">
      <xsl:attribute name="href">.</xsl:attribute>
      <xsl:attribute name="onclick">
        <xsl:text>sendCommand('playSound', ['sk://fs/2.0/data/pronuk/filesystem.cff!/</xsl:text>
        <xsl:value-of select="@path" />
        <xsl:text>']);return false;</xsl:text>
      </xsl:attribute>
      <img src="{$root}/cald2/gif/entry/pronuk.gif" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="span[@class='rec']">
    <!-- don't support rec 
	<img src="{$root}/cald2/gif/entry/micro.gif" align="absmiddle" style="margin: 2px;cursor: pointer;" onclick="alert('rec');" />
-->
  </xsl:template>

  <xsl:template match="span[@class='atlink']">
    <!-- ignore it -->
  </xsl:template>

  <xsl:template match="span[@class='numsense']">
    <span class="numsense">
      <xsl:apply-templates />
      <xsl:text>. </xsl:text>
    </span>
  </xsl:template>


  <!--
<xsl:template match="span[@class='ExtEx']">
	<img src="{$root}/cald2/png/ExtEx.png" align="absmiddle" style="cursor: pointer;" onclick="alert('ExtEx');" />
</xsl:template>

<xsl:template match="span[@class='WBuild']">
	<img src="{$root}/cald2/gif/WBuild.png" align="absmiddle" style="cursor: pointer;" onclick="alert('WBuild');" />
</xsl:template>

<xsl:template match="span[@class='Colls']">
	<img src="{$root}/cald2/gif/Colls.png" align="absmiddle" style="cursor: pointer;" onclick="alert('Colls');" />
</xsl:template>

<xsl:template match="span[@class='VerbEnd']">
	<img src="{$root}/cald2/gif/VerbEnd.png" align="absmiddle" style="cursor: pointer;" onclick="alert('VerbEnd');" />
</xsl:template>

<xsl:template match="span[@class='CLE']">
	<img src="{$root}/cald2/gif/CLE.png" align="absmiddle" style="cursor: pointer;" onclick="alert('CLE');" />
</xsl:template>

<xsl:template match="span[@class='UseNote']">
	<img src="{$root}/cald2/gif/UseNote.png" align="absmiddle" style="cursor: pointer;" onclick="alert('UseNote');" />
</xsl:template>

<xsl:template match="span[@class='ST']">
	<form name='SmartThesaurus' action='' method='post' >
		<input type='image' src="{$root}/cald2/gif/ST.png" />
	</form>
</xsl:template>
-->

</xsl:stylesheet>