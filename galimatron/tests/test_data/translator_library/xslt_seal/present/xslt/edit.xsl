<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="3.0"
>
    <xsl:mode on-no-match="shallow-copy"/>
    <xsl:output method="xml" cdata-section-elements="string"/>

    <xsl:template match="root-aggregate-version-definition[@index='1']/tag[@name='roles']/values/value/array">
        <xsl:copy>
            <xsl:apply-templates select="@* | node()"/>
            <value>
                <string>global ws</string>
            </value>
        </xsl:copy>
    </xsl:template>
</xsl:transform>
