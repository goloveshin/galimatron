<xsl:transform
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:map="http://www.w3.org/2005/xpath-functions/map"
    xmlns:gal="https://github.com/goloveshin/galimatron"
    xmlns:code="galimatron-code-generation"
    version="3.0"
>
    <xsl:param name="protocol_name"/>
    <xsl:param name="input_gal_dir" as="xs:string"/>
    <xsl:param name="primary_xml_root" select="/"/>

    <!-- with an installed Galimatron use <xsl:include href="https://github.com/goloveshin/galimatron/xslt/gal-functions.xsl"/> -->
    <xsl:include href="../../../../../../galimatron/translator_library/xslt/gal-functions.xsl"/>

    <!-- text of a template is copied; its code:* marks are filled -->
    <xsl:mode on-no-match="shallow-copy"/>


    <xsl:variable name="skeleton" as="map(xs:string, document-node())" select="
        map:merge(
            for $name in (
                'cpp-file',
                'cpp-registrate',
                'cpp-registrate-struct',
                'cpp-struct',
                'hpp-file',
                'hpp-struct'
            )
            return map { $name : document(concat('templates/', $name, '.xml')) }
        )
    "/>

    <!-- the document node of the compiled gal, not a loaded template -->
    <xsl:template match="$primary_xml_root">
        <xsl:message>Protocol name: <xsl:value-of select="$protocol_name"/></xsl:message>
        <xsl:apply-templates select="//root-aggregate-version-definition[gal:has-tag(., 'published')]"/>
    </xsl:template>

    <!-- one published protocol version. structs are not in namespace system -->
    <xsl:template match="root-aggregate-version-definition">
        <xsl:variable name="index" select="@index"/>
        <xsl:variable name="ctx" as="map(xs:string, xs:string?)" select="map {
            'namespace' : string(gal:tagvalue(., 'version_namespace')),
            'version' : string(gal:tagvalue(., 'version_number'))
        }"/>

        <xsl:result-document href="@@generated-dto@@{$ctx('namespace')}/{$ctx('namespace')}.hpp@@" method="text">
            <xsl:apply-templates select="$skeleton('hpp-file')">
                <xsl:with-param name="index" select="$index" tunnel="yes"/>
                <xsl:with-param name="ctx" select="$ctx" tunnel="yes"/>
            </xsl:apply-templates>
        </xsl:result-document>

        <xsl:result-document href="@@generated-dto@@{$ctx('namespace')}/{$ctx('namespace')}.cpp@@" method="text">
            <xsl:apply-templates select="$skeleton('cpp-file')">
                <xsl:with-param name="index" select="$index" tunnel="yes"/>
                <xsl:with-param name="ctx" select="$ctx" tunnel="yes"/>
            </xsl:apply-templates>
        </xsl:result-document>
    </xsl:template>

    <xsl:template match="code:template">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template name="emit-structs">
        <xsl:param name="skeleton-name" as="xs:string"/>
        <xsl:param name="ctx-names" as="xs:string*"/>
        <xsl:param name="index" tunnel="yes"/>

        <xsl:for-each select="gal:structs($primary_xml_root, $index)">
            <xsl:sort select="string(gal:tagvalue(., 'struct_name'))"/>
            <xsl:apply-templates select="$skeleton($skeleton-name)">
                <xsl:with-param name="ctx" tunnel="yes" select="gal:ctx(., $ctx-names)"/>
            </xsl:apply-templates>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="code:hpp-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="skeleton-name" select="'hpp-struct'"/>
            <xsl:with-param name="ctx-names" select="'struct-name'"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:cpp-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="skeleton-name" select="'cpp-struct'"/>
            <xsl:with-param name="ctx-names" select="('struct-name', 'uuid')"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:cpp-registrate">
        <xsl:apply-templates select="$skeleton('cpp-registrate')"/>
    </xsl:template>

    <xsl:template match="code:cpp-registrate-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="skeleton-name" select="'cpp-registrate-struct'"/>
            <xsl:with-param name="ctx-names" select="('struct-name', 'group-name')"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:*[not(*)]" priority="-1">
        <xsl:param name="ctx" as="map(xs:string, xs:string?)" tunnel="yes" select="map{}"/>
        <xsl:value-of select="$ctx(local-name())"/>
    </xsl:template>

</xsl:transform>
