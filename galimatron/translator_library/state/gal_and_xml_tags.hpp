#pragma once


// testing: yes (unit)
//
namespace galimatron::translator::state::gal_and_xml_tags
{

namespace document
{

static constexpr std::string_view galimatron = "galimatron";
static constexpr std::string_view description = "description";
static constexpr std::string_view language = "language";
static constexpr std::string_view compiled = "compiled";
static constexpr std::string_view history = "history";
static constexpr std::string_view string_delimiters = "string-delimiters";
static constexpr std::string_view string_delimiter = "string-delimiter";

}// namespace document

namespace root_aggregate
{

static constexpr std::string_view root_aggregate = "root-aggregate";
static constexpr std::string_view root_aggregate_version_declarations = "root-aggregate-version-declarations";
static constexpr std::string_view root_aggregate_version_declaration = "root-aggregate-version-declaration";
static constexpr std::string_view root_aggregate_version_definitions = "root-aggregate-version-definitions";
static constexpr std::string_view root_aggregate_version_definition = "root-aggregate-version-definition";

}// namespace root_aggregate

namespace root_aggregate_gal
{

static constexpr std::string_view root_aggregate = "root aggregate";
static constexpr std::string_view versions = "versions";
static constexpr std::string_view version = "version";
static constexpr std::string_view before = "before";

}// namespace root_aggregate_gal

namespace aggregate
{

static constexpr std::string_view aggregate = "aggregate";
static constexpr std::string_view aggregate_version_declarations = "aggregate-version-declarations";
static constexpr std::string_view aggregate_version_declaration = "aggregate-version-declaration";
static constexpr std::string_view aggregate_version_definitions = "aggregate-version-definitions";
static constexpr std::string_view aggregate_version_definition = "aggregate-version-definition";

static constexpr std::string_view orderings = "orderings";
static constexpr std::string_view ordering = "ordering";

}// namespace aggregate

namespace aggregate_gal
{

static constexpr std::string_view aggregate = "aggregate";
static constexpr std::string_view versions = "versions";
static constexpr std::string_view version = "version";
static constexpr std::string_view before = "before";

}// namespace aggregate_gal

namespace tag
{

static constexpr std::string_view tag = "tag";
static constexpr std::string_view label_refs = "label-refs";
static constexpr std::string_view label_ref = "label-ref";
static constexpr std::string_view label_static = "static";
static constexpr std::string_view label_dynamic = "dynamic";
static constexpr std::string_view values = "values";
static constexpr std::string_view value = "value";
static constexpr std::string_view record = "record";
static constexpr std::string_view array = "array";
static constexpr std::string_view key = "key";

}// namespace tag

namespace tag_gal
{

static constexpr std::string_view once = "once";
static constexpr std::string_view local = "local";
static constexpr std::string_view own = "own";

}// namespace tag_gal

namespace label
{

static constexpr std::string_view label = "label";
static constexpr std::string_view reference = "reference";
static constexpr std::string_view agg_is_root = "agg-is-root";
static constexpr std::string_view agg_name = "agg-name";
static constexpr std::string_view agg_index = "agg-index";
static constexpr std::string_view dynamic_highlight = "dynamic";
static constexpr std::string_view static_highlight = "static";
static constexpr std::string_view important = "important";
static constexpr std::string_view values = "values";
static constexpr std::string_view value = "value";
static constexpr std::string_view tag_name_from = "tag-name-from";

}// namespace label

namespace label_gal
{

static constexpr std::string_view label = "label";
static constexpr std::string_view static_highlight = "static";
static constexpr std::string_view tag_name_from = "tag-name-from";

};// namespace label_gal

namespace ordering
{

static constexpr std::string_view ordering = "ordering";
static constexpr std::string_view ordering_parts = "ordering-parts";
static constexpr std::string_view ordering_part = "ordering-part";
static constexpr std::string_view aggregate_ref = "aggregate-ref";
static constexpr std::string_view name = "name";

}// namespace ordering

namespace ordering_gal
{

static constexpr std::string_view ordering = "ordering";

};// namespace ordering_gal

namespace wizard_mark
{

static constexpr std::string_view dynamic = "dynamic";
static constexpr std::string_view wizard = "wizard";
static constexpr std::string_view new_root_aggregate = "new-root-aggregate";
static constexpr std::string_view new_aggregate = "new-aggregate";
static constexpr std::string_view new_version = "new-version";

}// namespace wizard_mark

namespace operation
{

static constexpr std::string_view delete_tag = "delete";

}// namespace operation

namespace operation_gal
{

static constexpr std::string_view delete_tag = "delete";

}// namespace operation_gal

namespace GaL
{

static constexpr std::string_view COL = ":";
static constexpr std::string_view SEMI = ";";
static constexpr std::string_view LC = "{";
static constexpr std::string_view RC = "}";
static constexpr std::string_view LP = "(";
static constexpr std::string_view RP = ")";
static constexpr std::string_view LS = "[";
static constexpr std::string_view RS = "]";
static constexpr std::string_view EQ = "=";
static constexpr std::string_view DOT = ".";
static constexpr std::string_view COMMA = ",";
static constexpr std::string_view COMMA_WITH_SPACE = ", ";
static constexpr std::string_view NEWLINE = "\n";
static constexpr char NEWLINE_CHAR = '\n';
static constexpr std::string_view CR = "\r";
static constexpr std::string_view CRLF = "\r\n";
static constexpr std::string_view SPACE = " ";
static constexpr char SPACE_CHAR = ' ';
static constexpr std::string_view CPP_COMMENT = "//";
static constexpr std::string_view C_COMMENT = "/*";
static constexpr std::string_view CPP_COMMENT_WITH_SPACE = "// ";

}// namespace GaL

}// namespace galimatron::translator::state::gal_and_xml_tags
