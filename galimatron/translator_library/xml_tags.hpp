#pragma once


// testing: yes (unit)
//
namespace galimatron::translator::xml_tags
{

namespace common
{

static constexpr std::string_view name = "name";
static constexpr std::string_view index = "index";
static constexpr std::string_view owner = "owner";
static constexpr std::string_view status = "status";
static constexpr std::string_view values = "values";
static constexpr std::string_view value = "value";
static constexpr std::string_view string = "string";
static constexpr std::string_view hash = "hash";
static constexpr std::string_view comment = "comment";
static constexpr std::string_view mode = "mode";
// joins user formatting across an xslt; cpp2gal does not print it
static constexpr std::string_view layout_mark = "layout-mark";
// index of the quotes this string was written with
static constexpr std::string_view delimiter = "delimiter";
// a _te string. absent on a language string means the text is not a template.
// the compiled string omits it and stores the rendered text
static constexpr std::string_view template_engine = "template-engine";
static constexpr std::string_view raw = "raw";
static constexpr std::string_view quote = "quote";
static constexpr std::string_view dashes = "dashes";
// the text of a boolean attribute
static constexpr std::string_view bool_true = "true";
static constexpr std::string_view bool_false = "false";

}// namespace common

namespace file
{

static constexpr std::string_view chunk = "chunk";
static constexpr std::string_view location = "location";
static constexpr std::string_view filesystem = "filesystem";
static constexpr std::string_view path = "path";

// filesystem
static constexpr std::string_view root = "root";
static constexpr std::string_view relative = "relative";
static constexpr std::string_view relative_and_filename = "relative-and-filename";
static constexpr std::string_view stem = "stem";
static constexpr std::string_view extension = "extension";

}// namespace file

namespace xml_document
{

static constexpr std::string_view version = "version";
static constexpr std::string_view encoding = "encoding";

}// namespace xml_document

}// namespace galimatron::translator::xml_tags

namespace tag_c = galimatron::translator::xml_tags::common;
