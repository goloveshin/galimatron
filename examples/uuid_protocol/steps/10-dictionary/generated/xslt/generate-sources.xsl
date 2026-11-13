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
    <xsl:param name="output_gal_dir" as="xs:string"/>
    <xsl:param name="primary_xml_root" select="/"/>

    <!-- with an installed Galimatron use <xsl:include href="https://github.com/goloveshin/galimatron/xslt/gal-functions.xsl"/> -->
    <xsl:include href="../../../../../../galimatron/translator_library/xslt/gal-functions.xsl"/>

    <!-- text of a template is copied; its code:* marks are filled -->
    <xsl:mode on-no-match="shallow-copy"/>


    <xsl:variable name="skeleton" as="map(xs:string, document-node())" select="
        map:merge(
            for $name in (
                'conversion-cpp',
                'conversion-declaration',
                'conversion-definition',
                'conversion-hpp',
                'cpp-enum-field',
                'cpp-file',
                'cpp-payload',
                'cpp-payload-enum',
                'cpp-registrate',
                'cpp-registrate-payload',
                'cpp-registrate-struct',
                'cpp-registrate-struct-in-namespace',
                'cpp-registrate-system',
                'cpp-struct',
                'cpp-system',
                'hpp-enum-field',
                'hpp-file',
                'hpp-payload',
                'hpp-payload-enum',
                'hpp-struct',
                'hpp-system'
            )
            return map { $name : document(concat('templates/', $name, '.xml')) }
        )
    "/>

    <!-- the document node of the compiled gal, not a loaded template -->
    <xsl:template match="$primary_xml_root">
        <xsl:message>Protocol name: <xsl:value-of select="$protocol_name"/></xsl:message>
        <xsl:apply-templates select="//root-aggregate-version-definition[gal:has-tag(., 'published')]"/>
        <xsl:call-template name="emit-conversions"/>
    </xsl:template>

    <!-- one published protocol version. system_message structs go in namespace system, payload_message in namespace payload -->
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

    <xsl:template match="code:hpp-system | code:hpp-payload | code:cpp-system | code:cpp-payload">
        <xsl:apply-templates select="$skeleton(local-name())"/>
    </xsl:template>

    <!-- the using-block is omitted when that kind has no struct -->
    <xsl:template match="code:cpp-registrate-system | code:cpp-registrate-payload">
        <xsl:param name="index" tunnel="yes"/>
        <xsl:variable name="kind" select="if (local-name() eq 'cpp-registrate-system') then 'system' else 'payload'"/>
        <xsl:if test="exists(gal:structs($primary_xml_root, $index, $kind))">
            <xsl:apply-templates select="$skeleton(local-name())"/>
        </xsl:if>
    </xsl:template>

    <xsl:template name="emit-structs">
        <xsl:param name="kind" as="xs:string"/>
        <xsl:param name="skeleton-name" as="xs:string"/>
        <xsl:param name="ctx-names" as="xs:string*"/>
        <xsl:param name="group" as="xs:string?" select="()"/>
        <xsl:param name="index" tunnel="yes"/>

        <xsl:for-each select="gal:structs($primary_xml_root, $index, $kind)">
            <xsl:sort select="string(gal:tagvalue(., 'struct_name'))"/>
            <xsl:variable name="base" select="gal:ctx(., $ctx-names)"/>
            <xsl:apply-templates select="$skeleton($skeleton-name)">
                <xsl:with-param name="ctx" tunnel="yes" select="
                    if (exists($group)) then map:put($base, 'group-name', $group) else $base
                "/>
                <xsl:with-param name="version" tunnel="yes" select="."/>
            </xsl:apply-templates>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="code:hpp-fields">
        <xsl:param name="version" as="node()" tunnel="yes"/>
        <xsl:for-each select="gal:tags-with-label($version, 'field')">
            <xsl:variable name="default" select="gal:labeled-string(., 'field', 'default')"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="gal:labeled-string(., 'field', 'type')"/>
            <xsl:text> </xsl:text>
            <xsl:value-of select="gal:labeled-string(., 'field', 'name')"/>
            <xsl:if test="normalize-space(string($default))">
                <xsl:text> = </xsl:text>
                <xsl:value-of select="$default"/>
            </xsl:if>
            <xsl:text>;&#10;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="code:hpp-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="kind" select="'plain'"/>
            <xsl:with-param name="skeleton-name" select="'hpp-struct'"/>
            <xsl:with-param name="ctx-names" select="'struct-name'"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:hpp-system-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="kind" select="'system'"/>
            <xsl:with-param name="skeleton-name" select="'hpp-struct'"/>
            <xsl:with-param name="ctx-names" select="'struct-name'"/>
        </xsl:call-template>
    </xsl:template>

    <!-- ordering enum_precedes_struct ranks the header. a version without it stays sorted by name -->
    <xsl:template name="emit-payload">
        <xsl:param name="header" as="xs:boolean"/>
        <xsl:param name="index" tunnel="yes"/>

        <xsl:for-each select="gal:payload-items($primary_xml_root, $index)">
            <xsl:sort select="
                if (orderings/ordering[@name = 'enum_precedes_struct'])
                then xs:integer(orderings/ordering[@name = 'enum_precedes_struct']/@index)
                else 1000000
            " data-type="number"/>
            <xsl:sort select="concat(gal:tagvalue(., 'enum_name'), gal:tagvalue(., 'struct_name'))"/>

            <xsl:choose>
                <xsl:when test="gal:is-payload-enum(.)">
                    <xsl:apply-templates select="$skeleton(if ($header) then 'hpp-payload-enum' else 'cpp-payload-enum')">
                        <xsl:with-param name="ctx" tunnel="yes" select="gal:ctx(., 'enum-name')"/>
                        <xsl:with-param name="version" tunnel="yes" select="."/>
                    </xsl:apply-templates>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:apply-templates select="$skeleton(if ($header) then 'hpp-struct' else 'cpp-struct')">
                        <xsl:with-param name="ctx" tunnel="yes" select="gal:ctx(., ('struct-name', 'uuid'))"/>
                        <xsl:with-param name="version" tunnel="yes" select="."/>
                    </xsl:apply-templates>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="code:hpp-payload-structs">
        <xsl:call-template name="emit-payload">
            <xsl:with-param name="header" select="true()"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:cpp-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="kind" select="'plain'"/>
            <xsl:with-param name="skeleton-name" select="'cpp-struct'"/>
            <xsl:with-param name="ctx-names" select="('struct-name', 'uuid')"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:cpp-system-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="kind" select="'system'"/>
            <xsl:with-param name="skeleton-name" select="'cpp-struct'"/>
            <xsl:with-param name="ctx-names" select="('struct-name', 'uuid')"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:cpp-payload-structs">
        <xsl:call-template name="emit-payload">
            <xsl:with-param name="header" select="false()"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:hpp-enum-fields | code:cpp-enum-fields">
        <xsl:param name="version" as="node()" tunnel="yes"/>
        <xsl:variable name="skeleton-name" select="if (local-name() = 'hpp-enum-fields') then 'hpp-enum-field' else 'cpp-enum-field'"/>
        <xsl:for-each select="gal:tags-with-label($version, 'enum_field')">
            <xsl:variable name="value" select="gal:labeled-string(., 'enum_field', 'value')"/>
            <xsl:apply-templates select="$skeleton($skeleton-name)">
                <xsl:with-param name="ctx" tunnel="yes" select="map:merge((
                    map { 'enum-field-name' : string(gal:labeled-string(., 'enum_field', 'name')) },
                    if (exists($value)) then map { 'enum-field-value' : string($value) } else ()
                ))"/>
            </xsl:apply-templates>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="code:enum-field-value">
        <xsl:param name="ctx" as="map(xs:string, xs:string?)" tunnel="yes" select="map{}"/>
        <xsl:variable name="value" select="$ctx('enum-field-value')"/>
        <xsl:if test="exists($value)">
            <xsl:value-of select="concat(' = ', $value)"/>
        </xsl:if>
    </xsl:template>

    <xsl:template match="code:cpp-registrate">
        <xsl:apply-templates select="$skeleton('cpp-registrate')"/>
    </xsl:template>

    <xsl:template match="code:cpp-registrate-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="kind" select="'plain'"/>
            <xsl:with-param name="skeleton-name" select="'cpp-registrate-struct'"/>
            <xsl:with-param name="ctx-names" select="('struct-name', 'group-name')"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:cpp-registrate-system-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="kind" select="'system'"/>
            <xsl:with-param name="skeleton-name" select="'cpp-registrate-struct-in-namespace'"/>
            <xsl:with-param name="ctx-names" select="('struct-name', 'group-name')"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template match="code:cpp-registrate-payload-structs">
        <xsl:call-template name="emit-structs">
            <xsl:with-param name="kind" select="'payload'"/>
            <xsl:with-param name="skeleton-name" select="'cpp-registrate-struct-in-namespace'"/>
            <xsl:with-param name="ctx-names" select="('struct-name', 'uuid')"/>
            <xsl:with-param name="group" select="'no_group'"/>
        </xsl:call-template>
    </xsl:template>

    <!-- a payload struct with no uuid stays a type and is not registered.
         a missing uuid key still emits, so system structs keep their declarations. -->
    <xsl:template match="code:when-uuid">
        <xsl:param name="ctx" as="map(xs:string, xs:string?)" tunnel="yes" select="map{}"/>
        <xsl:variable name="uuid" select="$ctx('uuid')"/>
        <xsl:if test="empty($uuid) or normalize-space(string($uuid))">
            <xsl:apply-templates/>
        </xsl:if>
    </xsl:template>

    <xsl:template match="code:*[not(*)]" priority="-1">
        <xsl:param name="ctx" as="map(xs:string, xs:string?)" tunnel="yes" select="map{}"/>
        <xsl:value-of select="$ctx(local-name())"/>
    </xsl:template>

    <!-- an empty .hpp/.cpp for each older version into the last, in the generated-conversion store.
         incoming conversion-state.xml lists pairs carried from the previous step.
         actual conversion-state.xml lists those pairs and every pair into the last version.
         a later run reads both and does not rewrite a recorded file. -->


    <xsl:template match="code:conversion-declarations | code:conversion-definitions">
        <xsl:param name="messages" as="element(message)*" tunnel="yes"/>
        <xsl:param name="ctx" as="map(xs:string, xs:string?)" tunnel="yes"/>
        <xsl:variable name="skeleton-name" select="if (local-name() = 'conversion-declarations') then 'conversion-declaration' else 'conversion-definition'"/>

        <xsl:for-each select="$messages">
            <xsl:apply-templates select="$skeleton($skeleton-name)">
                <xsl:with-param name="ctx" tunnel="yes" select="map:put($ctx, 'type-name', string(@type))"/>
            </xsl:apply-templates>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="write-empty-conversion">
        <xsl:param name="stem" as="xs:string"/>
        <xsl:variable name="ctx" as="map(xs:string, xs:string?)" select="map {
            'from-namespace' : string(@from),
            'file-stem' : $stem
        }"/>

        <xsl:result-document href="@@generated-conversion@@{$stem}.hpp@@" method="text">
            <xsl:apply-templates select="$skeleton('conversion-hpp')">
                <xsl:with-param name="ctx" tunnel="yes" select="$ctx"/>
                <xsl:with-param name="messages" tunnel="yes" select="message"/>
            </xsl:apply-templates>
        </xsl:result-document>
        <xsl:result-document href="@@generated-conversion@@{$stem}.cpp@@" method="text">
            <xsl:apply-templates select="$skeleton('conversion-cpp')">
                <xsl:with-param name="ctx" tunnel="yes" select="$ctx"/>
                <xsl:with-param name="messages" tunnel="yes" select="message"/>
            </xsl:apply-templates>
        </xsl:result-document>
    </xsl:template>

    <xsl:template name="emit-conversions">
        <xsl:variable name="incoming-uri" select="concat(gal:directory-uri($input_gal_dir), 'conversion-state.xml')"/>
        <xsl:variable name="actual-uri" select="concat(gal:directory-uri($output_gal_dir), 'conversion-state.xml')"/>
        <!-- read both state files. the write uses @@conversion-state@@, a different URI, so Saxon allows both -->
        <xsl:variable name="incoming-state" select="if (doc-available($incoming-uri)) then doc($incoming-uri) else ()"/>
        <xsl:variable name="actual-state" select="if (doc-available($actual-uri)) then doc($actual-uri) else ()"/>

        <!-- the open version holds the dictionary and is not published, so it is not a conversion target -->
        <xsl:variable name="roots" select="$primary_xml_root//root-aggregate-version-definition[gal:has-tag(., 'published')]"/>
        <xsl:variable name="last-index" select="max(for $index in $roots/@index return xs:integer($index))"/>
        <xsl:variable name="last-ns" select="string(gal:tagvalue($roots[xs:integer(@index) = $last-index], 'version_namespace'))"/>

        <xsl:variable name="pairs" as="element(pair)*">
            <xsl:for-each select="$roots[xs:integer(@index) &lt; $last-index]">
                <xsl:sort select="@index" data-type="number"/>
                <xsl:variable name="from-ns" select="string(gal:tagvalue(., 'version_namespace'))"/>
                <pair from="{$from-ns}" to="{$last-ns}">
                    <xsl:attribute name="recorded" select="
                        exists($incoming-state/conversion-state/made[@from = $from-ns and @to = $last-ns])
                        or exists($actual-state/conversion-state/made[@from = $from-ns and @to = $last-ns])
                    "/>
                    <xsl:for-each select="gal:structs($primary_xml_root, @index, 'payload')[normalize-space(gal:tagvalue(., 'uuid'))]">
                        <message type="{concat('payload::', gal:tagvalue(., 'struct_name'))}"/>
                    </xsl:for-each>
                </pair>
            </xsl:for-each>
        </xsl:variable>

        <xsl:for-each select="$pairs">
            <xsl:choose>
                <xsl:when test="@recorded = 'true'">
                    <xsl:message>conversion <xsl:value-of select="@from"/> to <xsl:value-of select="@to"/>: kept</xsl:message>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:message>conversion <xsl:value-of select="@from"/> to <xsl:value-of select="@to"/>: empty template</xsl:message>
                    <xsl:call-template name="write-empty-conversion">
                        <xsl:with-param name="stem" select="concat(@from, '-to-', @to)"/>
                    </xsl:call-template>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>

        <xsl:result-document href="@@conversion-state@@conversion-state.xml@@" method="xml" indent="yes" exclude-result-prefixes="gal xs map code">
            <conversion-state>
                <xsl:for-each select="$incoming-state/conversion-state/made">
                    <xsl:if test="not($pairs[@from = current()/@from and @to = current()/@to])">
                        <made from="{@from}" to="{@to}"/>
                    </xsl:if>
                </xsl:for-each>
                <xsl:for-each select="$pairs">
                    <made from="{@from}" to="{@to}"/>
                </xsl:for-each>
            </conversion-state>
        </xsl:result-document>
    </xsl:template>

</xsl:transform>
