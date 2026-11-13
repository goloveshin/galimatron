<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:map="http://www.w3.org/2005/xpath-functions/map"
    xmlns:gal="https://github.com/goloveshin/galimatron"
    exclude-result-prefixes="gal xs map"
    version="3.0"
>

    <!-- compiled aggregate versions, looked up by @index -->
    <xsl:key name="struct-by-index" match="aggregate-version-definition" use="@index"/>
    <!-- first string value of a tag, or empty when the tag is absent -->
    <xsl:function name="gal:tagvalue" as="xs:string?">
        <xsl:param name="version_node" as="node()"/>
        <xsl:param name="name" as="xs:string"/>
        <xsl:sequence select="head($version_node/tag[@name=$name]/values/value/string)/string(.)"/>
    </xsl:function>

    <xsl:function name="gal:has-tag" as="xs:boolean">
        <xsl:param name="version_node" as="node()"/>
        <xsl:param name="name" as="xs:string"/>
        <xsl:sequence select="exists($version_node/tag[@name=$name])"/>
    </xsl:function>

    <!-- first string of the value marked by a static label, or empty when that part is absent -->
    <xsl:function name="gal:labeled-string" as="xs:string?">
        <xsl:param name="tag" as="node()"/>
        <xsl:param name="label" as="xs:string"/>
        <xsl:param name="part" as="xs:string"/>
        <xsl:sequence select="head($tag/values/value[@*[local-name() eq $label and . eq $part]]/string)/string(.)"/>
    </xsl:function>

    <xsl:function name="gal:tags-labeled" as="element(tag)*">
        <xsl:param name="parent" as="node()"/>
        <xsl:param name="label" as="xs:string"/>
        <xsl:sequence select="$parent//tag[label-refs/static/label-ref/@name = $label]"/>
    </xsl:function>

    <!-- direct tags of a version that carry a value marked by this label -->
    <xsl:function name="gal:tags-with-label" as="element(tag)*">
        <xsl:param name="version" as="node()"/>
        <xsl:param name="label" as="xs:string"/>
        <xsl:sequence select="$version/tag[values/value/@*[local-name() eq $label]]"/>
    </xsl:function>

    <!-- aggregate versions of one index that contain a tag of this name -->
    <xsl:function name="gal:versions-with-tag" as="element()*">
        <xsl:param name="anchor" as="node()"/>
        <xsl:param name="index" as="xs:anyAtomicType"/>
        <xsl:param name="name" as="xs:string"/>
        <xsl:sequence select="root($anchor)//aggregate-version-definition[@index=$index][tag[@name=$name]]"/>
    </xsl:function>

    <xsl:function name="gal:array-has-label" as="xs:boolean">
        <xsl:param name="array" as="element(array)"/>
        <xsl:param name="label" as="xs:string"/>
        <xsl:sequence select="$array/ancestor::tag[1]/label-refs/static/label-ref/@name = $label"/>
    </xsl:function>

    <!-- the root or aggregate version definition that contains the node -->
    <xsl:function name="gal:version-definition" as="element()?">
        <xsl:param name="node" as="node()"/>
        <xsl:sequence select="$node/ancestor::*[self::root-aggregate-version-definition or self::aggregate-version-definition][1]"/>
    </xsl:function>

    <!-- index of that version definition -->
    <xsl:function name="gal:version-index" as="xs:string">
        <xsl:param name="node" as="node()"/>
        <xsl:sequence select="string(gal:version-definition($node)/@index)"/>
    </xsl:function>

    <!-- a wizard copy: dynamic child attribute equals $id. the original tag has no dynamic child -->
    <xsl:function name="gal:tag-uses-dynamic" as="xs:boolean">
        <xsl:param name="tag" as="element(tag)"/>
        <xsl:param name="id" as="xs:string"/>
        <xsl:sequence select="$tag/dynamic/@* = $id"/>
    </xsl:function>

    <!-- the version definition is live, and no later definition of that aggregate exists -->
    <xsl:function name="gal:is-open-version" as="xs:boolean">
        <xsl:param name="definition" as="element()"/>
        <xsl:variable name="owner" select="$definition/parent::*/parent::*"/>
        <xsl:variable name="declaration" select="$owner/*[self::aggregate-version-declarations or self::root-aggregate-version-declarations]/*[@index = $definition/@index]"/>
        <xsl:variable name="later" select="$definition/../*[xs:integer(@index) gt xs:integer($definition/@index)]"/>
        <xsl:sequence select="string($declaration/@status) != 'deleted' and empty($later)"/>
    </xsl:function>

    <!-- one past the highest statement index in the version, or 0 when the version is empty -->
    <xsl:function name="gal:next-statement-index" as="xs:integer">
        <xsl:param name="version" as="element()"/>
        <xsl:variable name="used" select="$version/*/@index ! xs:integer(.)"/>
        <xsl:sequence select="if (exists($used)) then max($used) + 1 else 0"/>
    </xsl:function>

    <!-- history beside language, the state before the language xslt -->
    <xsl:function name="gal:history" as="element(history)?">
        <xsl:param name="anchor" as="node()"/>
        <xsl:sequence select="root($anchor)/galimatron/history"/>
    </xsl:function>

    <!-- a tag labeled entry: name, the version that introduced it, and the prose -->
    <xsl:function name="gal:dictionary-entry" as="element(tag)">
        <xsl:param name="index" as="xs:integer"/>
        <xsl:param name="name" as="xs:string"/>
        <xsl:param name="introduced" as="xs:string"/>
        <xsl:param name="text" as="xs:string"/>
        <tag index="{$index}">
            <label-refs>
                <static>
                    <label-ref index="0" name="entry"/>
                </static>
            </label-refs>
            <values>
                <value index="0">
                    <string>
                        <xsl:value-of select="$name"/>
                    </string>
                </value>
                <value index="1">
                    <string>
                        <xsl:value-of select="$introduced"/>
                    </string>
                </value>
                <value index="2">
                    <string>
                        <xsl:value-of select="$text"/>
                    </string>
                </value>
            </values>
        </tag>
    </xsl:function>

    <!-- the first group names the aggregate and stays. the last group is the version counter. -->
    <xsl:function name="gal:next-uuid" as="xs:string">
        <xsl:param name="uuid" as="xs:string"/>
        <xsl:variable name="part" select="tokenize($uuid, '-')"/>
        <xsl:sequence select="string-join((
            $part[position() lt 5],
            format-integer(xs:integer($part[5]) + 1, '000000000000')
        ), '-')"/>
    </xsl:function>

    <!-- one map entry per name. a hyphen in the name is an underscore in the tag -->
    <xsl:function name="gal:ctx" as="map(xs:string, xs:string?)">
        <xsl:param name="version" as="node()"/>
        <xsl:param name="names" as="xs:string*"/>
        <xsl:sequence select="map:merge(
            for $name in $names
            return map { $name : string(gal:tagvalue($version, translate($name, '-', '_'))) }
        )"/>
    </xsl:function>

    <xsl:function name="gal:is-system" as="xs:boolean">
        <xsl:param name="version" as="node()"/>
        <xsl:sequence select="gal:has-tag($version, 'system_message')"/>
    </xsl:function>

    <xsl:function name="gal:is-payload" as="xs:boolean">
        <xsl:param name="version" as="node()"/>
        <xsl:sequence select="gal:has-tag($version, 'payload_message')"/>
    </xsl:function>

    <xsl:function name="gal:is-payload-enum" as="xs:boolean">
        <xsl:param name="version" as="node()"/>
        <xsl:sequence select="gal:is-payload($version) and gal:has-tag($version, 'enum_name')"/>
    </xsl:function>

    <!-- every struct in that version of the document -->
    <xsl:function name="gal:structs" as="element()*">
        <xsl:param name="root" as="node()"/>
        <xsl:param name="index" as="xs:anyAtomicType"/>
        <xsl:sequence select="key('struct-by-index', $index, $root)[gal:has-tag(., 'struct_name')]"/>
    </xsl:function>

    <!-- kind: system, payload, or plain (neither tag) -->
    <xsl:function name="gal:structs" as="element()*">
        <xsl:param name="root" as="node()"/>
        <xsl:param name="index" as="xs:anyAtomicType"/>
        <xsl:param name="kind" as="xs:string"/>
        <xsl:sequence select="gal:structs($root, $index)[
            if ($kind eq 'system') then gal:is-system(.)
            else if ($kind eq 'payload') then gal:is-payload(.)
            else not(gal:is-system(.) or gal:is-payload(.))
        ]"/>
    </xsl:function>

    <!-- structs and enums of one version. an enum has no uuid and is not registered -->
    <xsl:function name="gal:payload-items" as="element()*">
        <xsl:param name="root" as="node()"/>
        <xsl:param name="index" as="xs:anyAtomicType"/>
        <xsl:sequence select="key('struct-by-index', $index, $root)[
            gal:is-payload-enum(.) or (gal:is-payload(.) and gal:has-tag(., 'struct_name'))
        ]"/>
    </xsl:function>

    <xsl:function name="gal:file-uri" as="xs:string">
        <xsl:param name="path" as="xs:string"/>
        <xsl:sequence select="concat('file:///', replace($path, '\\', '/'))"/>
    </xsl:function>

    <xsl:function name="gal:directory-uri" as="xs:string">
        <xsl:param name="path" as="xs:string"/>
        <xsl:variable name="with-slash" select="if (matches($path, '[\\/]$')) then $path else concat($path, '/')"/>
        <xsl:sequence select="gal:file-uri($with-slash)"/>
    </xsl:function>
</xsl:transform>
