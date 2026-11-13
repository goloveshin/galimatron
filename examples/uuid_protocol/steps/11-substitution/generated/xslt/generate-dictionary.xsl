<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:gal="https://github.com/goloveshin/galimatron"
    exclude-result-prefixes="gal xs"
    version="3.0"
>
    <!-- with an installed Galimatron use <xsl:include href="https://github.com/goloveshin/galimatron/xslt/gal-functions.xsl"/> -->
    <xsl:include href="../../../../../../galimatron/translator_library/xslt/gal-functions.xsl"/>

    <!-- the compiled document is the next transform's input -->
    <xsl:mode on-no-match="shallow-copy"/>

    <xsl:template match="galimatron">
        <xsl:copy>
            <xsl:apply-templates select="@* | node()"/>
        </xsl:copy>
        <xsl:result-document href="@@dictionary@@dictionary.md@@" method="text">
            <xsl:text># Dictionary&#10;&#10;</xsl:text>
            <xsl:for-each-group select="//aggregate[@name = 'dictionary']//tag[values/value/@entry]" group-by="string(gal:labeled-string(., 'entry', 'introduced'))">
                <xsl:sort select="xs:integer(current-grouping-key())" data-type="number"/>
                <xsl:text>## Version </xsl:text>
                <xsl:value-of select="current-grouping-key()"/>
                <xsl:text>&#10;&#10;</xsl:text>
                <xsl:for-each select="current-group()">
                    <xsl:sort select="@name"/>
                    <xsl:text>- `</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>`</xsl:text>
                    <xsl:variable name="text" select="string(gal:labeled-string(., 'entry', 'text'))"/>
                    <xsl:if test="$text != ''">
                        <xsl:text>: </xsl:text>
                        <xsl:value-of select="$text"/>
                    </xsl:if>
                    <xsl:text>&#10;</xsl:text>
                </xsl:for-each>
                <xsl:text>&#10;</xsl:text>
            </xsl:for-each-group>
        </xsl:result-document>
    </xsl:template>
</xsl:transform>
