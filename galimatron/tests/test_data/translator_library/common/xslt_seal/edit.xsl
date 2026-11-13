<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:gal="https://github.com/goloveshin/galimatron"
    exclude-result-prefixes="gal xs"
    version="3.0"
>
    <!-- with an installed Galimatron use <xsl:include href="https://github.com/goloveshin/galimatron/xslt/gal-functions.xsl"/> -->
    <xsl:include href="gal-functions.xsl"/>

    <xsl:mode on-no-match="shallow-copy"/>
    <xsl:output method="xml" cdata-section-elements="string"/>

    <xsl:template match="array[gal:array-has-label(., 'role_mark')]">
        <xsl:copy>
            <xsl:apply-templates select="@* | node()"/>
            <value>
                <string>global ws</string>
            </value>
        </xsl:copy>
    </xsl:template>
</xsl:transform>
