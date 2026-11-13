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

    <xsl:mode on-no-match="shallow-copy"/>

    <!-- the open dictionary version receives one entry per tag name that is not entered yet -->
    <xsl:template match="aggregate[@name = 'dictionary']//aggregate-version-definition[gal:is-open-version(.)]">
        <xsl:copy>
            <xsl:apply-templates select="@* | node()"/>
            <xsl:variable name="history" select="gal:history(.)"/>
            <xsl:variable name="known" as="xs:string*" select="$history/version/change[@kind = 'add'][@aggregate = 'dictionary']/@name ! string(.)"/>
            <xsl:variable name="fresh" as="element(row)*">
                <xsl:for-each-group select="$history/version/change[@kind = 'add'][@name][@aggregate != 'dictionary']" group-by="@name">
                    <xsl:sort select="min(current-group()/parent::version ! xs:integer(@index))" data-type="number"/>
                    <xsl:sort select="current-grouping-key()"/>
                    <xsl:if test="not(current-grouping-key() = $known)">
                        <row name="{current-grouping-key()}" introduced="{min(current-group()/parent::version ! xs:integer(@index))}"/>
                    </xsl:if>
                </xsl:for-each-group>
            </xsl:variable>
            <xsl:variable name="start" select="gal:next-statement-index(.)"/>
            <xsl:for-each select="$fresh">
                <xsl:copy-of select="gal:dictionary-entry($start + position() - 1, string(@name), string(@introduced), '')"/>
            </xsl:for-each>
        </xsl:copy>
    </xsl:template>
</xsl:transform>
