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


    <!-- a wizard copy of a tag labeled need_new_uuid. the original tag has no dynamic child -->
    <xsl:template match="tag[gal:tag-uses-dynamic(., 'need_gen_uuid')]/values/value/string/text()">
        <xsl:value-of select="gal:next-uuid(.)"/>
    </xsl:template>
</xsl:transform>
