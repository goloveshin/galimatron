<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:gal="https://github.com/goloveshin/galimatron"
    xmlns:local="urn:uuid-protocol:history-page"
    exclude-result-prefixes="gal xs local"
    version="3.0"
>
    <!-- with an installed Galimatron use <xsl:include href="https://github.com/goloveshin/galimatron/xslt/gal-functions.xsl"/> -->
    <xsl:include href="../../../../../../galimatron/translator_library/xslt/gal-functions.xsl"/>

    <!-- the compiled document is the next transform's input -->
    <xsl:mode on-no-match="shallow-copy"/>

    <xsl:variable name="history" select="gal:history(/)"/>

    <!-- listed order: the first name that appears is the green band, the next name the plain band -->
    <xsl:variable name="groups" as="xs:string+" select="('system_message', 'payload_message')"/>

    <!-- service wins, then the first listed marker tag. the root stays ungrouped -->
    <xsl:function name="local:group-of" as="xs:string?">
        <xsl:param name="change" as="element(change)"/>
        <xsl:variable name="definition" select="
            root($history)//*[self::aggregate or self::root-aggregate][@name = $change/@aggregate]
                /*[self::aggregate-version-definitions or self::root-aggregate-version-definitions]
                /*[@index = $change/parent::version/@index]
        "/>
        <xsl:sequence select="
            if ($change/@root or empty($definition)) then ()
            else if (gal:has-tag($definition, 'service')) then 'service'
            else head($groups[gal:has-tag($definition, .)])
        "/>
    </xsl:function>

    <xsl:variable name="first-group" as="xs:string?" select="
        head($groups[. = (for $change in $history/version/change return local:group-of($change))])
    "/>

    <xsl:template match="galimatron">
        <xsl:copy>
            <xsl:apply-templates select="@* | node()"/>
        </xsl:copy>
        <xsl:result-document href="@@history@@history.html@@" method="html" html-version="5" encoding="UTF-8" include-content-type="no">
            <html>
                <head>
                    <meta charset="UTF-8"/>
                    <title>Galimatron history</title>
                    <style>
                        body { font-family: sans-serif; margin: 1.5rem; }
                        input { width: 100%; }
                        .version { display: none; }
                        .version.active { display: block; }
                        h3 { margin: 1rem 0 0.25rem; }
                        .aggregate[data-root] { background: #eef3f7; border-left: 4px solid #246; padding: 0.2rem 0.75rem; }
                        .aggregate[data-root] h3 {
                            margin: -0.2rem -0.75rem 0.45rem;
                            padding: 0.45rem 0.75rem;
                            font-size: 1.35rem;
                            color: #fff;
                            background: #163a66;
                        }
                        li { font-family: ui-monospace, monospace; margin: 0.2rem 0; }
                    </style>
                    <xsl:if test="$first-group">
                        <style>
                            .history-group { margin: 1rem 0; padding: 0.3rem 0.75rem 0.5rem; }
                            .history-group-first { background: #e7f6ea; border-left: 4px solid #2a6; }
                            .history-group-other { background: #f8f1e6; border-left: 4px solid #c70; }
                            .group-name {
                                margin: -0.3rem -0.75rem 0.6rem;
                                padding: 0.45rem 0.75rem;
                                font-size: 1.35rem;
                                color: #fff;
                                background: #3a5166;
                            }
                            .history-group-first .group-name { background: #146c36; }
                            .history-group-other .group-name { background: #a35a00; }
                            .history-group .aggregate h3 {
                                margin: 0.75rem -0.75rem 0.3rem;
                                padding: 0.3rem 0.75rem;
                                font-size: 1.15rem;
                                background: #e4ebf0;
                            }
                            .history-group-first .aggregate h3 { background: #d3ead9; }
                            .history-group-other .aggregate h3 { background: #f3e4cf; }
                            .history-group-service { background: #eee8f7; border-left: 4px solid #63a; }
                            .history-group-service .group-name { background: #5b3f8c; }
                            .history-group-service .aggregate h3 { background: #ddd4ef; }
                        </style>
                    </xsl:if>
                </head>
                <body>
                    <h1>History</h1>
                    <xsl:choose>
                        <xsl:when test="$history/version">
                            <xsl:variable name="indexes" select="$history/version/@index ! xs:integer(.)"/>
                            <input id="slider" type="range" min="{min($indexes)}" max="{max($indexes)}" value="{max($indexes)}"/>
                            <xsl:apply-templates select="$history/version"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <p>no versions</p>
                        </xsl:otherwise>
                    </xsl:choose>
                    <script>
                        const slider = document.getElementById("slider");
                        if (slider) {
                            const sections = document.querySelectorAll(".version");
                            const show = function (value) {
                                for (const section of sections) {
                                    section.classList.toggle("active", section.dataset.index === value);
                                }
                            };
                            slider.addEventListener("input", function () { show(slider.value); });
                            show(slider.value);
                        }
                    </script>
                </body>
            </html>
        </xsl:result-document>
    </xsl:template>

    <xsl:template match="version">
        <section class="version" data-index="{@index}">
            <h2>version <xsl:value-of select="@index"/></h2>
            <xsl:choose>
                <xsl:when test="change">
                    <!-- root, then system_message and payload_message, then service, then aggregates in no group -->
                    <xsl:variable name="ordered" as="element(change)*">
                        <xsl:perform-sort select="change">
                            <xsl:sort select="if (@root) then 0 else if (local:group-of(.) = 'service') then 2 else if (local:group-of(.)) then 1 else 3" data-type="number"/>
                            <xsl:sort select="if (local:group-of(.) = $groups) then index-of($groups, local:group-of(.)) else 0" data-type="number"/>
                        </xsl:perform-sort>
                    </xsl:variable>
                    <xsl:for-each-group select="$ordered" group-adjacent="if (@root) then 'root' else (local:group-of(.), 'other')[1]">
                        <xsl:choose>
                            <xsl:when test="current-grouping-key() = 'service'">
                                <section class="history-group history-group-service">
                                    <h3 class="group-name">service</h3>
                                    <xsl:call-template name="aggregates"/>
                                </section>
                            </xsl:when>
                            <xsl:when test="$groups = current-grouping-key()">
                                <section class="history-group">
                                    <xsl:if test="current-grouping-key() = $first-group">
                                        <xsl:attribute name="class">history-group history-group-first</xsl:attribute>
                                    </xsl:if>
                                    <h3 class="group-name">
                                        <xsl:value-of select="current-grouping-key()"/>
                                    </h3>
                                    <xsl:call-template name="aggregates"/>
                                </section>
                            </xsl:when>
                            <xsl:when test="current-grouping-key() = 'other'">
                                <section class="history-group history-group-other">
                                    <h3 class="group-name">other</h3>
                                    <xsl:call-template name="aggregates"/>
                                </section>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:call-template name="aggregates"/>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:for-each-group>
                </xsl:when>
                <xsl:otherwise>
                    <p>no changes</p>
                </xsl:otherwise>
            </xsl:choose>
        </section>
    </xsl:template>

    <xsl:template name="aggregates">
        <xsl:for-each-group select="current-group()" group-adjacent="@aggregate">
            <section class="aggregate">
                <xsl:if test="local:group-of(current-group()[1])">
                    <xsl:attribute name="data-group" select="local:group-of(current-group()[1])"/>
                </xsl:if>
                <xsl:if test="current-group()[1]/@root">
                    <xsl:attribute name="data-root" select="'true'"/>
                </xsl:if>
                <h3><xsl:value-of select="current-grouping-key()"/></h3>
                <ul>
                    <xsl:apply-templates select="current-group()"/>
                </ul>
            </section>
        </xsl:for-each-group>
    </xsl:template>

    <xsl:template match="change">
        <li>
            <xsl:choose>
                <xsl:when test="@kind = 'create-aggregate'">create aggregate</xsl:when>
                <xsl:when test="@kind = 'delete-version'">delete version</xsl:when>
                <xsl:when test="@kind = 'delete'">
                    <xsl:text>delete </xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:if test="before/string">
                        <xsl:text> = </xsl:text>
                        <xsl:value-of select="before/string" separator=", "/>
                    </xsl:if>
                </xsl:when>
                <xsl:when test="@kind = 'label'">
                    <xsl:text>label </xsl:text>
                    <xsl:value-of select="@name"/>
                </xsl:when>
                <xsl:when test="@kind = 'ordering'">
                    <xsl:text>ordering </xsl:text>
                    <xsl:value-of select="@name"/>
                </xsl:when>
                <xsl:when test="@kind = 'add'">
                    <xsl:text>add </xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:if test="after/string">
                        <xsl:text> = </xsl:text>
                        <xsl:value-of select="after/string" separator=", "/>
                    </xsl:if>
                </xsl:when>
                <xsl:when test="@kind = 'replace'">
                    <xsl:value-of select="@name"/>
                    <xsl:text> </xsl:text>
                    <xsl:value-of select="before/string" separator=", "/>
                    <xsl:text> -&gt; </xsl:text>
                    <xsl:value-of select="after/string" separator=", "/>
                </xsl:when>
            </xsl:choose>
        </li>
    </xsl:template>

</xsl:transform>
