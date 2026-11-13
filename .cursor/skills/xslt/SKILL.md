---
name: xslt
description: >-
  Write Galimatron XSLT 3.0 for SaxonC-HE. Say xslt, not stylesheet. Prefer an
  XML template for generated text. Reuse and extend gal-functions.xsl. Use when
  adding or changing a .xsl file, an xsl:function, a match or select, a user
  xslt, edit.xsl, generate-sources.xsl, or a code template.
---

# XSLT

Say xslt. Do not say stylesheet. The processor is SaxonC-HE 13. The xslt is `version="3.0"`.

## Templates

Prefer an XML template when the xslt writes a generated file. It reads like the result. Use `xsl:text` when a template does not fit.

A template is `templates/<name>.xml`. The root is `code:template` in `xmlns:code="galimatron-code-generation"`. Load each file once with `document(concat('templates/', $name, '.xml'))`. `xsl:apply-templates` on that document fills the `code:*` marks. An empty `code:*` prints the tunnel map entry of the same local name. A hyphen in that name is an underscore in the tag: `struct-name` reads `struct_name`.

Match the compiled document with `$primary_xml_root`. Fill the templates in their own mode, `on-no-match="text-only-copy"`. A match on `/` also matches a loaded template, and that match replaces the generated text.

## Namespace

Functions live in `xmlns:gal="https://github.com/goloveshin/galimatron"`. A typed parameter also needs `xmlns:xs`. When the xslt writes XML, set `exclude-result-prefixes="gal xs"` so those prefixes stay off the result elements.

## Functions

Use a function from `galimatron/translator_library/xslt/gal-functions.xsl` before writing a path inline. A new selector becomes an `xsl:function` there, and a row in the table below. The caller includes the file. Do not paste the function into the caller.

```xml
<!-- with an installed Galimatron use <xsl:include href="https://github.com/goloveshin/galimatron/xslt/gal-functions.xsl"/> -->
<xsl:include href="gal-functions.xsl"/>
```

The comment sits above every include of this library. An installed Galimatron loads `share/galimatron/catalog.xml`, and that catalog maps the address to the installed file. The source tree keeps a relative `href`: Saxon resolves it against the cwd, `saxon-processor.working-dir` (`xslt/`). A user xslt keeps `gal-functions.xsl` in that directory. The MQTT and UUID `generate-sources.xsl` include the library by a relative path from their own `xslt/` directory. The seal cases are copied into the build tree, so CMake copies `gal-functions.xsl` beside `edit.xsl` and that include uses `href="gal-functions.xsl"`. A second function with the same name and arity is a compile error.

A function has no context item. Start the path at a parameter. A leading `//` has nothing to start from; use `root($anchor)//` or `$parent//`.

Saxon rejects a function as the whole `match` (`XTSE0340`). Put the call in a predicate: `match="array[gal:array-has-label(., 'role_mark')]"`. A `select` may call the function directly.

## Two documents

A user xslt in production runs on the language XML from `cpp2xml`. A static label there is `tag/label-refs/static/label-ref/@name`. `cpp2xml` writes `label-ref` only under `label-refs/static`. `gal:tags-labeled` and `gal:array-has-label` read that path.

The language document carries `history` beside `language`, under `galimatron`. That history is the state before the language xslt. `gal:history($anchor)` returns it. A `version/change[@kind='add']` is a tag introduced in that version's `@index`. `xml2cpp` reads `language` and leaves `history` unread.

The compiled XML, after `cpp2post_cpp`, puts the label on the value as an attribute (`field="type"`, `enum_field="name"`). `gal:labeled-string` and `gal:tags-with-label` read that attribute. The MQTT and UUID `generate-sources.xsl` run on the compiled XML.

## Names

| Function | Reads |
| --- | --- |
| `gal:tagvalue($version_node, $name)` | first `tag[@name=$name]/values/value/string`; empty when the tag is absent |
| `gal:has-tag($version_node, $name)` | the tag element exists |
| `gal:labeled-string($tag, $label, $part)` | first string of `values/value` whose attribute name is `$label` and whose value is `$part`; empty when that part is absent |
| `gal:tags-labeled($parent, $label)` | descendant `tag`s with `label-refs/static/label-ref/@name = $label` |
| `gal:tags-with-label($version, $label)` | direct child `tag`s that have `values/value` with an attribute named `$label` |
| `gal:versions-with-tag($anchor, $index, $name)` | `aggregate-version-definition` of that index that contains `tag[@name=$name]` |
| `gal:array-has-label($array, $label)` | the nearest `tag` ancestor has that static label |
| `gal:version-definition($node)` | nearest `root-aggregate-version-definition` or `aggregate-version-definition` ancestor |
| `gal:version-index($node)` | `@index` of that version definition |
| `gal:tag-uses-dynamic($tag, $id)` | a `dynamic` attribute on the tag equals `$id`. a wizard copy has that child; the original tag does not |
| `gal:is-open-version($definition)` | the version definition is live, and no later definition of that aggregate exists |
| `gal:next-statement-index($version)` | one past the highest `@index` among the version's children, or 0 |
| `gal:history($anchor)` | the `history` element beside `language` |
| `gal:dictionary-entry($index, $name, $introduced, $text)` | a `tag` labeled `entry`, with those three strings |
| `gal:next-uuid($uuid)` | the same uuid with the last group incremented |
| `gal:ctx($version, $names)` | a map of those names; a hyphen in the name is an underscore in the tag |
| `gal:is-system($version)` | the version has tag `system_message` |
| `gal:is-payload($version)` | the version has tag `payload_message` |
| `gal:is-payload-enum($version)` | a payload that also has tag `enum_name` |
| `gal:structs($root, $index)` | aggregate versions of that index that have tag `struct_name`. the key is `struct-by-index` |
| `gal:structs($root, $index, $kind)` | those structs filtered by `system`, `payload`, or `plain` |
| `gal:payload-items($root, $index)` | payload structs and payload enums of that index |
| `gal:file-uri($path)` | `file:///` with backslashes turned into slashes |
| `gal:directory-uri($path)` | `gal:file-uri` of the path, with a trailing slash |

`@@generated@@relative/path@@` in `xsl:result-document/@href` is a filesystem alias. Leave it.
