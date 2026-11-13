<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:gal="https://github.com/goloveshin/galimatron"
    exclude-result-prefixes="gal xs"
    version="3.0"
>
    <!-- with an installed Galimatron use <xsl:include href="https://github.com/goloveshin/galimatron/xslt/gal-functions.xsl"/> -->
    <xsl:include href="../../../galimatron/translator_library/xslt/gal-functions.xsl"/>

    <xsl:mode on-no-match="shallow-copy"/>

    <!-- every tag string gains the version index: benchmark -> benchmark_1 -->
    <xsl:template match="tag/values/value/string/text()">
        <xsl:value-of select="concat(., '_', gal:version-index(.))"/>
    </xsl:template>
</xsl:transform>
