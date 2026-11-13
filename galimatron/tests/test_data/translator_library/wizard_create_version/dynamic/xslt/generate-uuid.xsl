<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:gal="https://github.com/goloveshin/galimatron"
    version="3.0"
>
    <xsl:mode on-no-match="shallow-copy"/>

    <!-- set from gal.cfg for this test -->
    <xsl:param name="new_uuid" as="xs:string"/>

    <!-- a wizard copy marked need_new_uuid="need_gen_uuid" gets $new_uuid -->
    <xsl:template match="tag[dynamic/@need_new_uuid = 'need_gen_uuid']/values/value/string/text()">
        <xsl:value-of select="gal:new-uuid(.)"/>
    </xsl:template>

    <xsl:function name="gal:new-uuid" as="xs:string">
        <xsl:param name="node" as="node()"/>
        <xsl:sequence select="if (empty($node)) then '' else $new_uuid"/>
    </xsl:function>
</xsl:transform>
